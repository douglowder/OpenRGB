#!/bin/bash
#
# Manage the openrgbd LaunchAgent on macOS.
#
#   openrgbd-launchd.sh install [--binary PATH] [--profile NAME] [-- ARGS...]
#   openrgbd-launchd.sh stop
#   openrgbd-launchd.sh restart
#   openrgbd-launchd.sh uninstall
#   openrgbd-launchd.sh status
#
# See Documentation/Daemon.md.

set -euo pipefail

LABEL="org.openrgb.openrgbd"
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TEMPLATE="$REPO_ROOT/mac/$LABEL.plist.in"
PLIST="$HOME/Library/LaunchAgents/$LABEL.plist"
# launchd's stdio, kept out of OpenRGB's own log directory so the two do not
# interleave in a directory listing.
LOG_DIR="$HOME/Library/Logs/OpenRGB"
DOMAIN="gui/$(id -u)"

die() {
  echo "openrgbd-launchd: $*" >&2
  exit 1
}

usage() {
  # The header comment block, so adding a command does not shift a line range.
  sed -n '3,/^$/p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'
  exit "${1:-1}"
}

# Locate the daemon: an explicit path, then the build tree, then PATH.
find_binary() {
  if [[ -n "${1:-}" ]]; then
    [[ -x "$1" ]] || die "not an executable: $1"
    cd "$(dirname "$1")" && echo "$PWD/$(basename "$1")"
    return
  fi

  if [[ -x "$REPO_ROOT/openrgbd" ]]; then
    echo "$REPO_ROOT/openrgbd"
    return
  fi

  command -v openrgbd || die "no openrgbd found; build it or pass --binary PATH"
}

# XML-escape a value destined for a <string> element.
xml_escape() {
  local value="$1"
  value="${value//&/&amp;}"
  value="${value//</&lt;}"
  value="${value//>/&gt;}"
  printf '%s' "$value"
}

# Fill in the template.  Substitution is literal: a path is not a regular
# expression, and one containing & or | would corrupt a sed replacement.
render() {
  local binary="$1"
  shift

  local line arg
  while IFS= read -r line; do
    if [[ "$line" == *@EXTRA_ARGUMENTS@* ]]; then
      for arg in "$@"; do
        printf '            <string>%s</string>\n' "$(xml_escape "$arg")"
      done
      continue
    fi

    line="${line//@OPENRGBD@/$(xml_escape "$binary")}"
    line="${line//@HOME@/$(xml_escape "$HOME")}"
    line="${line//@LOG_DIR@/$(xml_escape "$LOG_DIR")}"
    printf '%s\n' "$line"
  done <"$TEMPLATE"
}

# Is the service loaded, and does it have a process?
service_loaded() {
  launchctl print "$DOMAIN/$LABEL" >/dev/null 2>&1
}

service_running() {
  launchctl print "$DOMAIN/$LABEL" 2>/dev/null | grep -qE '^\tpid = '
}

# bootout returns before the service is really gone, and a bootstrap that races
# the teardown fails with "Input/output error".  Wait for the label to go away.
bootout_and_wait() {
  launchctl bootout "$DOMAIN/$LABEL" 2>/dev/null || true

  for _ in $(seq 1 60); do
    service_loaded || return 0
    sleep 0.5
  done

  die "$LABEL is still loaded after 30s; check: launchctl print $DOMAIN/$LABEL"
}

# Wait for the service to have no process.  launchctl print lists a pid only
# while one is running.
wait_until_stopped() {
  for _ in $(seq 1 60); do
    service_running || return 0
    sleep 0.5
  done

  die "$LABEL is still running after 30s; check: launchctl print $DOMAIN/$LABEL"
}

cmd_install() {
  local binary=""
  local -a extra_args=()

  while [[ $# -gt 0 ]]; do
    case "$1" in
      --binary)  binary="${2:?--binary needs a path}"; shift 2 ;;
      --profile) extra_args+=(--profile "${2:?--profile needs a name}"); shift 2 ;;
      --)        shift; extra_args+=("$@"); break ;;
      -h|--help) usage 0 ;;
      *)         die "unknown option: $1" ;;
    esac
  done

  [[ -f "$TEMPLATE" ]] || die "missing template: $TEMPLATE"
  binary="$(find_binary "$binary")"

  # launchd writes the log files but will not create the directory for them.
  mkdir -p "$LOG_DIR" "$(dirname "$PLIST")"

  render "$binary" ${extra_args+"${extra_args[@]}"} >"$PLIST.new"
  plutil -lint "$PLIST.new" >/dev/null || die "generated plist is malformed: $PLIST.new"
  mv "$PLIST.new" "$PLIST"

  bootout_and_wait
  launchctl bootstrap "$DOMAIN" "$PLIST"

  echo "installed $PLIST"
  echo "  binary  $binary"
  echo "  args    --server ${extra_args+${extra_args[*]}}"
  echo "  log     $LOG_DIR/openrgbd.launchd.log"
}

cmd_uninstall() {
  bootout_and_wait
  rm -f "$PLIST"
  echo "removed $PLIST"
}

cmd_stop() {
  [[ -f "$PLIST" ]] || die "not installed; run: $0 install"
  service_loaded || die "$LABEL is not loaded; run: $0 install"

  if ! service_running; then
    echo "$LABEL is already stopped"
    return 0
  fi

  #-----------------------------------------------------------------------------
  # SIGTERM, which openrgbd handles: it closes the listening sockets, releases
  # the devices and exits zero.  KeepAlive is SuccessfulExit=false, so a clean
  # exit stays stopped rather than being restarted.
  #
  # The agent stays loaded and enabled, so "restart" brings it back, as does the
  # next login.  Use "uninstall" to stop it permanently.
  #-----------------------------------------------------------------------------
  launchctl kill TERM "$DOMAIN/$LABEL"
  wait_until_stopped

  echo "stopped $LABEL"
}

cmd_restart() {
  [[ -f "$PLIST" ]] || die "not installed; run: $0 install"
  launchctl kickstart -k "$DOMAIN/$LABEL"
  echo "restarted $LABEL"
}

cmd_status() {
  local detail

  [[ -f "$PLIST" ]] || die "not installed; run: $0 install"

  if ! detail="$(launchctl print "$DOMAIN/$LABEL" 2>/dev/null)"; then
    echo "$PLIST exists but $LABEL is not loaded; run: $0 install"
    return 1
  fi

  # Only the service's own keys: the nested endpoint dictionaries repeat "state".
  printf '%s\n' "$detail" | awk '/^\t(state|pid|last exit (code|status)) = /'
}

case "${1:-}" in
  install)   shift; cmd_install "$@" ;;
  stop)      shift; cmd_stop ;;
  restart)   shift; cmd_restart ;;
  uninstall) shift; cmd_uninstall ;;
  status)    shift; cmd_status ;;
  -h|--help) usage 0 ;;
  *)         usage ;;
esac
