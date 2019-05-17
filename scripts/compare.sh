#!/usr/bin/env bash
# compare fsrc with ripgrep

case $(uname) in
    Linux)
        SEARCH_DIR="/usr/include/"
        ;;
    Darwin)
        SEARCH_DIR="/usr/include/"
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
echo "fsrc"
./fsrc -t "$TERM" -d "$SEARCH_DIR" | tail -n 1
echo
echo "rg"
rg --stats -u -e "$TERM" "$SEARCH_DIR" | tail -n 5
echo

echo "find"
find "$SEARCH_DIR" -type f | wc -l
echo
