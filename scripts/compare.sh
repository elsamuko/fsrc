#!/usr/bin/env bash
# compare fsrc with ripgrep

function red  {
    echo -e "\033[1;31m$@\033[0m"
}

function green  {
    echo -e "\033[1;32m$@\033[0m"
}

function cyan  {
    echo -e "\033[1;34m$@\033[0m"
}

SEARCH_DIR="libs/boost/include"
TERM="filesystem"
TIMEFORMAT="%R"

cyan "Searching in $SEARCH_DIR for $TERM"

echo
green "===FSRC==="
echo "./fsrc -t \"$TERM\" --no-colors -d \"$SEARCH_DIR\""
{ time ./fsrc -t "$TERM" --no-colors -d "$SEARCH_DIR" | tail -n 2 | grep . ;} 2> tmp/time
red $(cat tmp/time)
echo

green "====RG===="
echo "rg --stats -u -e \"$TERM\" \"$SEARCH_DIR\""
{ time rg --stats -u -e "$TERM" "$SEARCH_DIR" | tail -n 8 ;} 2> tmp/time
red $(cat tmp/time)
echo

green "===FIND==="
echo "find \"$SEARCH_DIR\" -type f | wc -l"
{ time find "$SEARCH_DIR" -type f | wc -l ;} 2> tmp/time
red $(cat tmp/time)
echo
