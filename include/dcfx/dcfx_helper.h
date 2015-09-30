#pragma once

// Generic helper definitions for shared library support
#if defined _WIN32 || defined __CYGWIN__
#define DCFX_HELPER_DLL_IMPORT __declspec(dllimport)
#define DCFX_HELPER_DLL_EXPORT __declspec(dllexport)
#define DCFX_HELPER_DLL_LOCAL
#else
#if __GNUC__ >= 4
#define DCFX_HELPER_DLL_IMPORT __attribute__ ((visibility ("default")))
#define DCFX_HELPER_DLL_EXPORT __attribute__ ((visibility ("default")))
#define DCFX_HELPER_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
#else	
#define DCFX_HELPER_DLL_IMPORT
#define DCFX_HELPER_DLL_EXPORT
#define DCFX_HELPER_DLL_LOCAL
#endif
#endif

#if defined(_DLL)
#define DCFX_API DCFX_HELPER_DLL_EXPORT
#else
#define DCFX_API DCFX_HELPER_DLL_IMPORT
#endif
#define DCFX_LOCAL DCFX_HELPER_DLL_LOCAL
