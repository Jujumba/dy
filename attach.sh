#!/usr/bin/env bash
{# attach-dy.sh
# Finds a running "./dy" process (assumes only one) and attaches gdb running "layout src".
# - Uses ps+grep -F to avoid regex surprises
# - If multiple matches found, lists them and picks the first (warns)
# - Checks for permission to attach

set -euo pipefail

# Find PIDs of processes whose command line contains the literal "./dy"
mapfile -t PIDS < <(ps -eo pid=,args= | grep -F "./dy" | awk '{print $1}')

if [ ${#PIDS[@]} -eq 0 ]; then
  echo "No process with './dy' found in the command line." >&2
  exit 1
fi

if [ ${#PIDS[@]} -gt 1 ]; then
  echo "Warning: multiple processes matched './dy'. Using the first one." >&2
  echo "Matched PIDs: ${PIDS[*]}" >&2
fi

PID=${PIDS[0]}

# Check permissions: you must be same user or root to ptrace/attach
MYUID=$(id -u)
PROCUID=$(awk -v pid="$PID" 'NR==1{print $1}' <(ps -o uid= -p "$PID"))

if [ -z "$PROCUID" ]; then
  echo "Failed to determine UID of PID $PID." >&2
  exit 1
fi

if [ "$MYUID" -ne 0 ] && [ "$MYUID" -ne "$PROCUID" ]; then
  echo "You are UID $MYUID but process $PID is owned by UID $PROCUID." >&2
  echo "You need to run this script as the same user or as root to attach." >&2
  exit 1
fi

echo "Attaching gdb to PID $PID ..."
# Use gdb; -ex runs commands before allowing interactive session.
# We run "layout src" so the TUI source layout opens after attach.
# `--pid` is supported by modern gdb as `-p`, both are fine.
#  -ex "b TerminalReRenderLinesBelowCursor" -ex "b TerminalInsertCharAtCursor"
exec gdb -p "$PID" -ex "layout src" 
