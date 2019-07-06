#!/usr/bin/env bash

case $(uname) in
    Linux)
        OS=linux
        ;;
    Darwin)
        OS=mac
        ;;
    CYGWIN*|MINGW*|MSYS*)
        OS=win
        EXT=".exe"
        ;;
    *)
        echo "Error: Unknown OS."
        exit 1
        ;;
esac

MAIN_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
TMP_DIR="$MAIN_DIR/tmp/deploy"
QMAKE="$MAIN_DIR/utils/qmake/bin/qmake_$OS"
GIT_TAG=$(git describe --abbrev=0)
ZIP_FILE="fsrc-$OS-$GIT_TAG.zip"
FSRC_BIN="$MAIN_DIR/fsrc$EXT"

function prepare {
    if [ -f "$ZIP_FILE" ]; then rm "$ZIP_FILE"; fi
    mkdir -p "$TMP_DIR"
}

function buildBoost {
    if [ ! -d "$MAIN_DIR/libs/boost" ]; then
        echo "Building boost"
        ./scripts/build_boost.sh
    fi
}

function buildFsrc {
    ( 
        cd qmake || exit 1
        "$QMAKE" fsrc.pro 2> "$TMP_DIR/qmake.log"
        make clean &> "$TMP_DIR/make.log"
        make &> "$TMP_DIR/make.log"
    )
}

function check {
    if [ ! -f "$FSRC_BIN" ]; then
        echo "Failed to build fsrc"
        exit 1
    fi
}

function package {
    zip -j "$ZIP_FILE" "$FSRC_BIN" > "$TMP_DIR/zip.log"
}

function please {
    echo "$1"
    "$1"
}

echo "Deploying fsrc $GIT_TAG to $ZIP_FILE"
please prepare
please buildBoost
please buildFsrc
please check
please package
