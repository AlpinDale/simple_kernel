#!/usr/bin/env bash
set -euo pipefail

image=""
script=""
expect_exit=""
timeout_s=10
declare -a expect_markers=()

while [[ $# -gt 0 ]]; do
  case "$1" in
    --image)
      image="$2"
      shift 2
      ;;
    --script)
      script="$2"
      shift 2
      ;;
    --expect)
      expect_markers+=("$2")
      shift 2
      ;;
    --expect-exit)
      expect_exit="$2"
      shift 2
      ;;
    --timeout)
      timeout_s="$2"
      shift 2
      ;;
    *)
      echo "unknown argument: $1" >&2
      exit 1
      ;;
  esac
done

if [[ -z "$image" || -z "$expect_exit" ]]; then
  echo "missing required arguments" >&2
  exit 1
fi

serial_log="build/qemu-serial.log"
monitor_log="build/qemu-monitor.log"
monitor_fifo="build/qemu-monitor.fifo"
rm -f "$serial_log" "$monitor_log" "$monitor_fifo"
mkfifo "$monitor_fifo"

map_key() {
  case "$1" in
    $'\n') printf '%s\n' "ret" ;;
    ' ') printf '%s\n' "spc" ;;
    '-') printf '%s\n' "minus" ;;
    '_') printf '%s\n' "shift-minus" ;;
    '.') printf '%s\n' "dot" ;;
    '/') printf '%s\n' "slash" ;;
    *) printf '%s\n' "$1" ;;
  esac
}

send_script() {
  local text="$1"
  local i key char
  for ((i = 0; i < ${#text};)); do
    case "${text:i}" in
      "<UP>"*)
        printf 'sendkey up\n' >&3
        i=$((i + 4))
        sleep 0.03
        continue
        ;;
      "<DOWN>"*)
        printf 'sendkey down\n' >&3
        i=$((i + 6))
        sleep 0.03
        continue
        ;;
      "<LEFT>"*)
        printf 'sendkey left\n' >&3
        i=$((i + 6))
        sleep 0.03
        continue
        ;;
      "<RIGHT>"*)
        printf 'sendkey right\n' >&3
        i=$((i + 7))
        sleep 0.03
        continue
        ;;
    esac

    char="${text:i:1}"
    key="$(map_key "$char")"
    printf 'sendkey %s\n' "$key" >&3
    i=$((i + 1))
    sleep 0.03
  done
}

exec 3<>"$monitor_fifo"

qemu-system-x86_64 \
  -drive "format=raw,file=$image,if=ide,index=0" \
  -display none \
  -serial "file:$serial_log" \
  -monitor stdio \
  -no-reboot \
  -device isa-debug-exit,iobase=0xF4,iosize=0x1 \
  <"$monitor_fifo" >"$monitor_log" 2>&1 &

qemu_pid=$!
start_time=$SECONDS
sent=0

while (( SECONDS - start_time < timeout_s )); do
  serial_output=""
  if [[ -f "$serial_log" ]]; then
    serial_output="$(cat "$serial_log")"
  fi

  if [[ $sent -eq 0 && "$serial_output" == *"kernel> "* ]]; then
    send_script "$script"
    sent=1
  fi

  if ! kill -0 "$qemu_pid" 2>/dev/null; then
    break
  fi

  sleep 0.05
done

if kill -0 "$qemu_pid" 2>/dev/null; then
  kill "$qemu_pid" 2>/dev/null || true
  wait "$qemu_pid" || true
  exec 3>&-
  rm -f "$monitor_fifo"
  echo "QEMU timed out" >&2
  [[ -f "$serial_log" ]] && cat "$serial_log" >&2
  [[ -f "$monitor_log" ]] && cat "$monitor_log" >&2
  exit 1
fi

set +e
wait "$qemu_pid"
rc=$?
set -e
exec 3>&-
rm -f "$monitor_fifo"

serial_output=""
if [[ -f "$serial_log" ]]; then
  serial_output="$(cat "$serial_log")"
fi

for marker in "${expect_markers[@]}"; do
  if [[ "$serial_output" != *"$marker"* ]]; then
    echo "missing output marker: $marker" >&2
    printf '%s' "$serial_output" >&2
    [[ -f "$monitor_log" ]] && cat "$monitor_log" >&2
    exit 1
  fi
done

if [[ "$rc" -ne "$expect_exit" ]]; then
  echo "unexpected exit code: $rc != $expect_exit" >&2
  printf '%s' "$serial_output" >&2
  [[ -f "$monitor_log" ]] && cat "$monitor_log" >&2
  exit 1
fi
