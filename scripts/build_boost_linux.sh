#!/usr/bin/env bash

OS=linux
PROJECT=boost
VERSION="1.68.0"
VERSION_DL="${VERSION//./_}"
DL_URL="https://downloads.sourceforge.net/project/boost/boost/${VERSION}/boost_${VERSION_DL}.tar.gz"

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
MAIN_DIR="$SCRIPT_DIR/.."
TARGET_DIR="$MAIN_DIR/libs/$PROJECT"
PROJECT_DIR="$MAIN_DIR/tmp/$PROJECT"
DOWNLOAD="$PROJECT_DIR/$PROJECT-$VERSION.tar.gz"
SRC_DIR="$PROJECT_DIR/src"
BUILD_DIR="$SRC_DIR/${PROJECT}_${VERSION_DL}"

function indent {
    sed  's/^/     /'
}

function doPrepare {
    if [ -d "$SRC_DIR" ]; then
        rm -rf "$SRC_DIR"
    fi
    if [ -d "$TARGET_DIR" ]; then
        rm -rf "$TARGET_DIR"
    fi
    mkdir -p "$PROJECT_DIR"
    mkdir -p "$TARGET_DIR/include/boost"
    mkdir -p "$SRC_DIR"
}

function doDownload {
    if [ ! -f "$DOWNLOAD" ]; then
        curl -s -L "$DL_URL" -o "$DOWNLOAD"
    fi
}

function doUnzip {
    tar xzf "$DOWNLOAD" -C "$SRC_DIR"
}

function doBuild {
    cd "$BUILD_DIR"
    
    ./bootstrap.sh

    # debug
    ./b2 -j 8 --stagedir=stage_debug toolset=gcc variant=debug link=static threading=multi address-model=64 cxxflags="-std=c++17"

    # release
    ./b2 -j 8 --stagedir=stage_release toolset=gcc variant=release link=static threading=multi address-model=64 cxxflags="-std=c++17 -msse2 -oFast"  linkflags="-flto"
}

function doCopy {
    mkdir -p "$TARGET_DIR/lib/$OS/debug"
    mkdir -p "$TARGET_DIR/lib/$OS/release"
    mkdir -p "$TARGET_DIR/include"
    cp -r "$BUILD_DIR/stage_debug/lib/"*.a "$TARGET_DIR/lib/$OS/debug/"
    cp -r "$BUILD_DIR/stage_release/lib/"*.a "$TARGET_DIR/lib/$OS/release/"
    cp -r "$BUILD_DIR/boost/"* "$TARGET_DIR/include/boost"
}


echo "Prepare"
doPrepare | indent

echo "Download"
doDownload | indent

echo "Unzip"
doUnzip | indent

echo "Build"
doBuild 2>&1 | indent

echo "Copy"
doCopy | indent
