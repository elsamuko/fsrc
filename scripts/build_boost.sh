#!/usr/bin/env bash

case $(uname) in
    Linux)
        OS=linux
        ;;
    Darwin)
        OS=mac
        ;;
    CYGWIN*)
        OS=win
        ;;
    *)
        echo "Unknown OS" && exit 1
        ;;
esac

PROJECT=boost
VERSION="1.70.0"
VERSION_DL="${VERSION//./_}"
DL_URL="https://dl.bintray.com/boostorg/release/${VERSION}/source/boost_${VERSION_DL}.tar.gz" 

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
    mkdir -p "$SRC_DIR"
}

function doDownload {
    if [ ! -f "$DOWNLOAD" ]; then
        curl -s -L "$DL_URL" -o "$DOWNLOAD" 2>&1
    fi
}

function doUnzip {
    tar xzf "$DOWNLOAD" -C "$SRC_DIR"
}

function macBuild {
    cd "$BUILD_DIR"

    ./bootstrap.sh

    # debug
    ./b2 -j 8 --stagedir=stage_debug toolset=clang variant=debug \
        link=static threading=multi address-model=64 \
        cxxflags="-std=c++1z -stdlib=libc++ -mmacosx-version-min=10.10" linkflags="-lc++"

    # release
    ./b2 -j 8 --stagedir=stage_release toolset=clang variant=release \
        link=static threading=multi address-model=64 \
        cxxflags="-std=c++1z -msse2 -oFast -stdlib=libc++ -mmacosx-version-min=10.10" linkflags="-lc++ -flto"
}

function winBuild {
    cd "$BUILD_DIR"

    cmd /c bootstrap.bat

    # debug
    ./b2.exe -j 8 --stagedir=stage_debug   toolset=msvc-14.2 variant=debug   \
        link=static runtime-link=static threading=multi address-model=64

    # release
    ./b2.exe -j 8 --stagedir=stage_release toolset=msvc-14.2 variant=release \
        link=static runtime-link=static threading=multi address-model=64 \
        cxxflags="/Qpar /Ox /Ob2 /Oi /Ot /Oy /GT /GL" linkflags="/LTCG /OPT:REF /OPT:ICF"
}

function linuxBuild {
    cd "$BUILD_DIR"
    
    ./bootstrap.sh --without-icu

    # debug
    ./b2 -j 8 --disable-icu --stagedir=stage_debug toolset=gcc variant=debug \
        link=static threading=multi address-model=64 \
        cxxflags="-std=c++17"

    # release
    ./b2 -j 8 --disable-icu --stagedir=stage_release toolset=gcc variant=release \
        link=static threading=multi address-model=64 \
        cxxflags="-std=c++17 -msse2 -oFast" linkflags="-flto"
}

function doCopy {
    mkdir -p "$TARGET_DIR/lib/$OS/debug"
    mkdir -p "$TARGET_DIR/lib/$OS/release"
    mkdir -p "$TARGET_DIR/include/boost"
    cp -r "$BUILD_DIR/stage_debug/lib/"*.* "$TARGET_DIR/lib/$OS/debug/"
    cp -r "$BUILD_DIR/stage_release/lib/"*.* "$TARGET_DIR/lib/$OS/release/"
    cp -r "$BUILD_DIR/boost/"* "$TARGET_DIR/include/boost"
}


echo "Prepare"
doPrepare | indent

echo "Download"
doDownload | indent

echo "Unzip"
doUnzip | indent

echo "Build"
"${OS}Build" 2>&1 | indent

echo "Copy"
doCopy | indent
