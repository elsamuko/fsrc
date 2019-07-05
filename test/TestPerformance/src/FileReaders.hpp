#pragma once

#include "utils.hpp"

#if !BOOST_OS_WINDOWS
//! memory mapped API with thread local storage
BOOST_FORCEINLINE utils::FileView fromFileMmap( const sys_string& filename ) {
    utils::FileView view;
    int file = open( filename.c_str(), O_RDONLY | O_BINARY );
    IF_RET( file == -1 );
    utils::ScopeGuard onExit( [file] { close( file ); } );

    view.size = utils::fileSize( file );
    IF_RET( !view.size );

    char* map = ( char* )mmap( 0, view.size, PROT_READ, MAP_PRIVATE, file, 0 );
    IF_RET( map == MAP_FAILED );

    // check first 100 bytes for binary
    IF_RET( !utils::isTextFile( std::string_view( map, std::min<size_t>( view.size, 100ul ) ) ) );

    view.content = std::string_view( map, view.size );
    munmap( map, view.size ); // if used, call munmap after parsing
    return view;
}
#else
BOOST_FORCEINLINE utils::FileView fromFileMmap( const sys_string& filename ) {
    utils::FileView view;
    boost::iostreams::mapped_file_source file( std::string( filename.cbegin(), filename.cend() ) );
    IF_RET( !file );

    view.size = file.size();
    view.content = std::string_view( file.data(), view.size );
    // if used, view needs a mapped_file member
    return view;
}
#endif

//! C API with thread local storage
BOOST_FORCEINLINE utils::FileView fromFileLocal( const sys_string& filename ) {
    utils::FileView view;
    FILE* file = fopen( filename.c_str(), O_RB );
    IF_RET( file == NULL );
    const utils::ScopeGuard onExit( [file] { fclose( file ); } );

    view.size = utils::fileSize( fileno( file ) );
    IF_RET( !view.size );

    // growing buffer for each thread
    static thread_local utils::Buffer buffer;
    char* ptr = buffer.grow( view.size );

    // read content
    IF_RET( view.size != fread( ptr, 1, view.size, file ) );

    // check first 100 bytes for binary
    IF_RET( !utils::isTextFile( std::string_view( ptr, std::min<size_t>( view.size, 100ul ) ) ) );

    view.content = std::string_view( ptr, view.size );
    return view;
}

//! C API with string storage
BOOST_FORCEINLINE utils::FileView fromFileString( const sys_string& filename ) {
    utils::FileView view;
    FILE* file = fopen( filename.c_str(), O_RB );
    IF_RET( file == NULL );
    const utils::ScopeGuard onExit( [file] { fclose( file ); } );

    view.size = utils::fileSize( fileno( file ) );
    IF_RET( !view.size );

    std::string buffer;
    buffer.resize( view.size );
    const char* ptr = buffer.data();

    // read content
    IF_RET( view.size != fread( ( void* )ptr, 1, view.size, file ) );

    // check first 100 bytes for binary
    IF_RET( !utils::isTextFile( std::string_view( ptr, std::min<size_t>( view.size, 100ul ) ) ) );

    view.content = std::string_view( ptr, view.size );
    // if used, add buffer to FileView
    return view;
}

//! CPP API with thread local storage
BOOST_FORCEINLINE utils::FileView fromFileCPP( const sys_string& filename ) {
    utils::FileView view;
    std::ifstream file( filename.c_str(), std::ios::binary | std::ios::in );

    IF_RET( !file );

    file.seekg( 0, std::ios::end );
    view.size = ( size_t ) file.tellg();
    file.seekg( 0, std::ios::beg );

    IF_RET( !view.size );

    // growing buffer for each thread
    static thread_local utils::Buffer buffer;
    char* ptr = buffer.grow( view.size );

    file.read( ptr, view.size );

    // check first 100 bytes for binary
    IF_RET( !utils::isTextFile( std::string_view( ptr, std::min<size_t>( view.size, 100ul ) ) ) );

    view.content = std::string_view( ptr, view.size );
    return view;
}

// C API with lseek instead fstat
BOOST_FORCEINLINE utils::FileView fromFileLSeek( const sys_string& filename ) {
    utils::FileView view;
    FILE* file = fopen( filename.c_str(), O_RB );
    IF_RET( file == NULL );
    const utils::ScopeGuard onExit( [file] { fclose( file ); } );

    fseek( file, 0, SEEK_END );
    view.size = ftell( file );
    fseek( file, 0, SEEK_SET );

    IF_RET( !view.size );

    // growing buffer for each thread
    static thread_local utils::Buffer buffer;
    char* ptr = buffer.grow( view.size );

    // read first 4 kB
    size_t offset = std::min<size_t>( view.size, 4_kB );
    size_t bytes = fread( ptr, 1, offset, file );
    IF_RET( offset != bytes );

    // check first 100 bytes for binary
    IF_RET( !utils::isTextFile( std::string_view( ptr, std::min<size_t>( offset, 100ul ) ) ) );

    // read rest
    if( view.size > offset ) {
        size_t newSize = view.size - offset;
        size_t bytes2 = fread( ptr + offset, 1, newSize, file );
        IF_RET( newSize != bytes2 );
    }

    view.content = std::string_view( ptr, view.size );
    return view;
}

BOOST_FORCEINLINE utils::FileView fromFileTwoFread( const sys_string& filename ) {
    utils::FileView view;
    FILE* file = fopen( filename.c_str(), O_RB );
    IF_RET( file == NULL );
    const utils::ScopeGuard onExit( [file] { fclose( file ); } );

    view.size = utils::fileSize( fileno( file ) );
    IF_RET( !view.size );

    // growing buffer for each thread
    static thread_local utils::Buffer buffer;
    char* ptr = buffer.grow( view.size );

    // check first 100 bytes for binary
    if( view.size > 100 ) {
        IF_RET( 100 != fread( ptr, 1, 100, file ) );
        IF_RET( !utils::isTextFile( std::string_view( ptr, 100 ) ) );
        IF_RET( view.size - 100 != fread( ptr + 100, 1, view.size - 100, file ) );
    } else {
        IF_RET( view.size != fread( ptr, 1, view.size, file ) );
        IF_RET( !utils::isTextFile( std::string_view( ptr, view.size ) ) );
    }

    view.content = std::string_view( ptr, view.size );
    return view;
}
