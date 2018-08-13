#!/usr/bin/env bash
# compare fsrc with ripgrep

case $(uname) in
    Linux)
        BOOST_DIR="/usr/include/boost/"
        ;;
    Darwin)
        BOOST_DIR="/usr/include/boost/"
        ;;
    CYGWIN*)
        BOOST_DIR="C:\\ProgramData\\boost"
        ;;
    *)
        echo "Unknown OS"
        ;;
esac


FSRC="$PWD/fsrc"

echo
echo "fsrc"
(cd "$BOOST_DIR" && ($FSRC filesystem | tail -n 1))
echo
echo "rg"
(cd "$BOOST_DIR" && (rg --stats filesystem | tail -n 3 ))
echo

