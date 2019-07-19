#include "pipes.hpp"

#include "boost/predef.h"

#include <vector>
#include <cstdio>
#include <string>
#include <mutex>

#if BOOST_OS_WINDOWS
#include <Windows.h>
#include <winternl.h>
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif

#if BOOST_OS_WINDOWS

// via https://github.com/git/git/blob/master/compat/winansi.c#L558
typedef struct _OBJECT_NAME_INFORMATION {
    UNICODE_STRING Name;
    WCHAR NameBuffer[0];
} OBJECT_NAME_INFORMATION, *POBJECT_NAME_INFORMATION;

#define ObjectNameInformation ( OBJECT_INFORMATION_CLASS )1

bool pipes::stdoutIsMsysPty() {
    static std::once_flag once;
    static bool isMsysPty = false;

    std::call_once( once, [] {
        HANDLE hOut = ( HANDLE ) _get_osfhandle( _fileno( stdout ) );

        using TypeNtQueryObject = decltype( &::NtQueryObject );
        HMODULE ntdll = ::GetModuleHandleW( L"ntdll.dll" );
        TypeNtQueryObject NtQueryObject_ = ( TypeNtQueryObject )GetProcAddress( ntdll, "NtQueryObject" );

        ULONG length = 0;
        std::vector<uint8_t> buffer( 1024 );
        POBJECT_NAME_INFORMATION ni = ( POBJECT_NAME_INFORMATION )buffer.data();
        NTSTATUS ok = NtQueryObject_( hOut, ObjectNameInformation, ni, buffer.size(), &length );

        // unpiped cygwin: \Device\ConDrv
        //   piped cygwin: \Device\NamedPipe\cygwin-e022582115c10879-2492-pipe-0x5
        //   unpiped msys: \Device\NamedPipe\msys-dd50a72ab4668b33-pty0-to-master
        //     piped msys: \Device\NamedPipe\msys-dd50a72ab4668b33-9588-pipe-0xF
        if( NT_SUCCESS( ok ) ) {
            std::wstring_view name( ni->Name.Buffer, ni->Name.Length );
            bool isMsys = name.find( L"msys" ) != std::wstring::npos;
            bool isPty = name.find( L"pty" ) != std::wstring::npos;
            isMsysPty = isMsys && isPty;
        }
    } );

    return isMsysPty;
}

#endif

bool pipes::stdoutIsPipe() {
    static std::once_flag once;
    static bool pipe = false;

    std::call_once( once, [] {

#if BOOST_OS_WINDOWS
        HANDLE hOut = ( HANDLE ) _get_osfhandle( _fileno( stdout ) );
        DWORD type = GetFileType( hOut );

        switch( type ) {
            case FILE_TYPE_CHAR:
                pipe = isatty( fileno( stdout ) ) == 0;
                break;

            case FILE_TYPE_PIPE:
                pipe = !stdoutIsMsysPty();
                break;
        }

#else
        pipe = isatty( fileno( stdout ) ) == 0;
#endif

    } );

    return pipe;
}
