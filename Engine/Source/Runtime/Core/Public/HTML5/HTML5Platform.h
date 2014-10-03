// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

/*================================================================================
	HTML5Platform.h: Setup for the HTML5 platform
==================================================================================*/

#pragma once




/**
 * HTML5 specific types
 **/
struct FHTML5Types : public FGenericPlatformTypes
{
	typedef unsigned long SIZE_T;
#if !PLATFORM_HTML5_WIN32
	// Emscripten uses the musl libc implementation, where NULL is defined as '0L', which is of type long.
	typedef long					TYPE_OF_NULL;
#endif
};
typedef FHTML5Types FPlatformTypes;

// Platform defines, overrides from the default (except first 3)
#define PLATFORM_DESKTOP							0
#define PLATFORM_64BITS								0
#define PLATFORM_LITTLE_ENDIAN						1
#define PLATFORM_SUPPORTS_PRAGMA_PACK				1
#define PLATFORM_USE_LS_SPEC_FOR_WIDECHAR			1
#if PLATFORM_HTML5_WIN32
#define PLATFORM_HAS_BSD_TIME						0
#define PLATFORM_COMPILER_HAS_DEFAULTED_FUNCTIONS	0
#define PLATFORM_COMPILER_HAS_VARIADIC_TEMPLATES	0
#define PLATFORM_USES_MICROSOFT_LIBC_FUNCTIONS		1
#define PLATFORM_MAX_FILEPATH_LENGTH				MAX_PATH
#define PLATFORM_HAS_BSD_SOCKETS					0
#else
#define PLATFORM_HAS_BSD_TIME						1
#define PLATFORM_MAX_FILEPATH_LENGTH				PATH_MAX
#define PLATFORM_TCHAR_IS_4_BYTES					1
#define PLATFORM_HAS_BSD_SOCKETS					1
#define PLATFORM_HAS_NO_EPROCLIM					1
#endif
#define PLATFORM_USE_PTHREADS						0
#define PLATFORM_SUPPORTS_TEXTURE_STREAMING			1
#define PLATFORM_USES_DYNAMIC_RHI					1
#define PLATFORM_REQUIRES_FILESERVER				1
#define PLATFORM_SUPPORTS_TBB						0
#define PLATFORM_ENABLE_VECTORINTRINSICS			0
#define PLATFORM_USES_ES2							1

// Function type macros.
#if PLATFORM_HTML5_WIN32
#define FORCEINLINE _forceinline
#define FORCENOINLINE __declspec(noinline)	/* Force code to NOT be inline */
#define PLATFORM_COMPILER_HAS_EXPLICIT_OPERATORS 0
#define CONSTEXPR    
#else
#define FORCEINLINE		inline __attribute__((__always_inline__))		/* Force code to be inline */
#define FORCENOINLINE	__attribute__((noinline))			/* Force code to NOT be inline */
#endif

// Optimization macros
// @todo can we disable optimizations? do we need to?
#define PRAGMA_DISABLE_OPTIMIZATION_ACTUAL
#define PRAGMA_ENABLE_OPTIMIZATION_ACTUAL

#if PLATFORM_HTML5_WIN32
#pragma warning(disable : 4481) // nonstandard extension used: override specifier 'override'
// disable this now as it is annoying for generic platfrom implementations
#pragma warning(disable : 4100) // unreferenced formal parameter
#pragma warning(disable : 4996) // 'function' was was declared deprecated  (needed for the secure string functions)
#pragma warning( disable : 4530 ) // warning C4530: C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc. for HTML5Win32 code, which uses stl. 
#endif
#if PLATFORM_HTML5_WIN32
// Backwater of the spec. All compilers support this except microsoft, and they will soon
#define TYPENAME_OUTSIDE_TEMPLATE
#define EXCEPTION_EXECUTE_HANDLER       1
#endif
#define ABSTRACT			abstract

// Alignment.
#if PLATFORM_HTML5_WIN32
#define MS_ALIGN(n) __declspec(align(n))
#else
#define GCC_PACK(n)			__attribute__((packed,aligned(n)))
#define GCC_ALIGN(n)		__attribute__((aligned(n)))
#define REQUIRES_ALIGNED_ACCESS 1 
#endif

// Operator new/delete handling.
#if !PLATFORM_HTML5_WIN32
#define OPERATOR_NEW_THROW_SPEC throw (std::bad_alloc)
#define OPERATOR_DELETE_THROW_SPEC throw()
#endif


#define MAXUINT8    ((uint8)~((uint8)0))
#define MAX_PATH 1024
