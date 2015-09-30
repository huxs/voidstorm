#pragma once

// Generic helper definitions for shared library support
#if defined _WIN32 || defined __CYGWIN__
#define DCUTIL_HELPER_DLL_IMPORT __declspec(dllimport)
#define DCUTIL_HELPER_DLL_EXPORT __declspec(dllexport)
#define DCUTIL_HELPER_DLL_LOCAL
#else
#if __GNUC__ >= 4
#define DCUTIL_HELPER_DLL_IMPORT __attribute__ ((visibility ("default")))
#define DCUTIL_HELPER_DLL_EXPORT __attribute__ ((visibility ("default")))
#define DCUTIL_HELPER_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define DCUTIL_HELPER_DLL_IMPORT
#define DCUTIL_HELPER_DLL_EXPORT
#define DCUTIL_HELPER_DLL_LOCAL
#endif
#endif

#if defined(_DLL)
#define DCUTIL_API DCUTIL_HELPER_DLL_EXPORT
#else
#define DCUTIL_API DCUTIL_HELPER_DLL_IMPORT
#endif
#define DCUTIL_LOCAL DCUTIL_HELPER_DLL_LOCAL
