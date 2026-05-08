#!/usr/bin/env bash
# Configure passwordless sudo for bringing up the CAN interface used by ranger_base.
# Installs a drop-in file under /etc/sudoers.d/ via visudo -c for safe syntax check.

set -euo pipefail

SUDOERS_FILE="/etc/sudoers.d/ranger-can"
TARGET_USER="${SUDO_USER:-${USER}}"

if [[ "${EUID}" -ne 0 ]]; then
  echo "This script must be run as root. Re-running with sudo..." >&2
  exec sudo -E "$0" "$@"
fi

if ! command -v visudo >/dev/null 2>&1; then
  echo "Error: visudo not found. Please install the 'sudo' package." >&2
  exit 1
fi

IP_BIN="$(command -v ip || echo /usr/sbin/ip)"
if [[ ! -x "${IP_BIN}" ]]; then
  echo "Error: 'ip' binary not found at ${IP_BIN}." >&2
  exit 1
fi

TMP_FILE="$(mktemp)"
trap 'rm -f "${TMP_FILE}"' EXIT

cat >"${TMP_FILE}" <<EOF
# Allow ${TARGET_USER} to bring CAN interfaces up/down without a password.
# Managed by scripts/setup_can_sudoers.sh
${TARGET_USER} ALL=(ALL) NOPASSWD: ${IP_BIN} link set can[0-9]* up type can bitrate *
${TARGET_USER} ALL=(ALL) NOPASSWD: ${IP_BIN} link set can[0-9]* down
EOF

if ! visudo -cf "${TMP_FILE}" >/dev/null; then
  echo "Error: generated sudoers file failed visudo syntax check." >&2
  exit 1
fi

install -m 0440 -o root -g root "${TMP_FILE}" "${SUDOERS_FILE}"

echo "Installed sudoers rule at ${SUDOERS_FILE} for user '${TARGET_USER}':"
cat "${SUDOERS_FILE}"
echo
echo "You can now run, without a password prompt, e.g.:"
echo "  sudo ip link set can0 up type can bitrate 500000"
