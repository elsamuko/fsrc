
# set debug/release info
CONFIG(debug, debug|release) {
    COMPILE_FLAG=d
    COMPILE_MODE=debug
}

CONFIG(release, debug|release) {
    COMPILE_MODE=release
}

CONFIG -= qt
CONFIG += c++1z
CONFIG += console
CONFIG -= app_bundle
CONFIG += static

macx:       PLATFORM=mac
win32:      PLATFORM=win
unix:!macx: PLATFORM=linux
unix:!macx: CONFIG += linux

OBJECTS_DIR = $${MAIN_DIR}/tmp/$${TARGET}/$${COMPILE_MODE}/objects
MOC_DIR = $${MAIN_DIR}/tmp/$${TARGET}/$${COMPILE_MODE}/moc
UI_DIR = $${MAIN_DIR}/tmp/$${TARGET}/$${COMPILE_MODE}/uic
RCC_DIR = $${MAIN_DIR}/tmp/$${TARGET}/$${COMPILE_MODE}/rcc

INCLUDEPATH += $${SRC_DIR}

DEFINES += __PRETTY_FUNCTION__=__FUNCTION__

message($${TARGET} $${PLATFORM} $${COMPILE_MODE})
