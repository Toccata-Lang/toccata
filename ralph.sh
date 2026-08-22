#!/usr/bin/env bash
# ralph.sh — run the pi agent in a Ralph loop using prompt.md
# Each iteration is a fresh, stateless pi session; progress is carried
# by git history and the checklist in docs/hash-map-plan-toccata.md.
cd /home/jim/toccata || exit 1

MAX_ITER=60   # 50 roadmap tasks; headroom for no-progress iterations
for i in $(seq 1 "$MAX_ITER"); do
  echo "=== Ralph iteration $i ==="
  out=$(pi -p @prompt.md 2>&1)
  rc=$?
  echo "$out"
  echo

  if printf '%s\n' "$out" | grep -q "Roadmap complete"; then
    echo "=== DONE at iteration $i ==="
    exit 0
  fi
  if printf '%s\n' "$out" | grep -q "STUCK:"; then
    echo "=== STUCK at iteration $i ==="
    exit 1
  fi
  if [ "$rc" -ne 0 ]; then
    echo "Iteration $i failed (rc=$rc), retrying"
  fi
done
echo "=== Max iterations reached ==="
exit 2
