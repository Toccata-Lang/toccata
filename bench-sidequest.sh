#!/bin/bash
# Run sidequest 50 times, starting at 1, incrementing by 3
# Output: CSV suitable for Google Sheets

echo "count,itrs"

for i in $(seq 0 200); do
    param=$((1 + i * 100))
    output=$(./sidequest "$param" 2>&1)
    itrs=$(echo "$output" | grep -oP 'ITRS: \K[0-9]+')
    echo "${param},${itrs}"
done
