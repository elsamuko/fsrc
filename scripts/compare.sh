#!/usr/bin/env bash
# compare fsrc with ripgrep

case $(uname) in
    Linux)
        SEARCH_DIR="/usr/include/"
        ;;
    Darwin)
        SEARCH_DIR="/usr/include/"
        alias grep=ggrep
        ;;
    CYGWIN*)
        SEARCH_DIR='C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC'
        ;;
    *)
        echo "Unknown OS"
        ;;
esac

TERM="test"
echo "Searching in $SEARCH_DIR for $TERM"

echo
echo "===FSRC==="
echo "./fsrc -t \"$TERM\" -d \"$SEARCH_DIR\""
./fsrc -t "$TERM" -d "$SEARCH_DIR" | tail -n 1
echo

echo "====RG===="
echo "rg --stats -u -e \"$TERM\" \"$SEARCH_DIR\""
rg --stats -u -e "$TERM" "$SEARCH_DIR" | tail -n 8
echo

# echo "===GREP==="
# echo "grep -qnr \"$TERM\" \"$SEARCH_DIR\""
# time grep -qnr "$TERM" "$SEARCH_DIR" 2>&1 | grep real
# echo

echo "===FIND==="
echo "find \"$SEARCH_DIR\" -type f | wc -l"
find "$SEARCH_DIR" -type f | wc -l
echo
