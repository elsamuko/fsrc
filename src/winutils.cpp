#include <Windows.h>

#include "utils.hpp"

bool utils::openFile( const sys_string& filename ) {
    HINSTANCE rv = ::ShellExecuteW( nullptr, // HWND   hwnd
                                    L"open", // LPCWSTR lpOperation,
                                    filename.c_str(),
                                    nullptr, // LPCWSTR lpParameters,
                                    nullptr, // LPCWSTR lpDirectory,
                                    SW_SHOW ); // INT    nShowCmd

    return ( int )rv > 32;
}
