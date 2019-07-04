#!/usr/bin/env bash
# compare fsrc with ripgrep

SEARCH_DIR="libs/boost/include"
TERM="filesystem"
TIMEFORMAT=%R

echo "Searching in $SEARCH_DIR for $TERM"

echo
echo -e "\033[1;32m===FSRC===\033[0m"
echo "./fsrc -t \"$TERM\" --no-colors -d \"$SEARCH_DIR\""
time ./fsrc -t "$TERM" --no-colors -d "$SEARCH_DIR" | tail -n 2 | grep .
echo

echo -e "\033[1;32m====RG====\033[0m"
echo "rg --stats -u -e \"$TERM\" \"$SEARCH_DIR\""
time rg --stats -u -e "$TERM" "$SEARCH_DIR" | tail -n 8
echo

echo -e "\033[1;32m===FIND===\033[0m"
echo "find \"$SEARCH_DIR\" -type f | wc -l"
time find "$SEARCH_DIR" -type f | wc -l
echo
