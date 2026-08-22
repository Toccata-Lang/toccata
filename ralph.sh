#!/usr/bin/env bash
# ralph.sh — run the pi agent in a Ralph loop using prompt.md
# Each iteration is a fresh, stateless pi session; progress is carried
# by git history and the checklist in docs/hash-map-plan-toccata.md.
#
# pi runs in JSON event-stream mode. Process info (turns, tool calls,
# tool results, assistant text — never thinking) is streamed live to
# stderr; only the final answer is captured on stdout for the greps below.
cd /home/jim/toccata || exit 1
set -o pipefail

MAX_ITER=60   # 50 roadmap tasks; headroom for no-progress iterations

# jq filter: one line per process event; thinking blocks are never emitted
PROCESS='
  if .type == "turn_start" then
    "─── turn ───"
  elif .type == "tool_execution_start" then
    "  [call] " + .toolName + " " + ((.args | tostring) | .[0:160])
  elif .type == "tool_execution_end" then
    "  [done] " + .toolName + " " + (if .isError then "ERROR " + ((.result | tostring) | .[0:200]) else "ok" end)
  elif .type == "message_end" and .message.role == "assistant" then
    (.message.content[]? | select(.type == "text") | "  " + .text)
  else
    empty
  end
'

for i in $(seq 1 "$MAX_ITER"); do
  echo "=== Ralph iteration $i ==="
  out=$(
    pi --mode json -p @prompt.md \
      | tee >(jq -r "$PROCESS" >&2) \
      | jq -s -r '
          [ .[] | select(.type == "message_end" and .message.role == "assistant") ]
          | if length == 0 then empty
            else last | .message.content[]? | select(.type == "text") | .text
            end
        '
  )
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
