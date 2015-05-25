#define STRICT
#define WIN32_LEAN_AND_MEAN
#include "leakchk.h"

#include <stdio.h>
#include <stdlib.h>

#include "../pversion.h"

#include "drvimpl.h"
#include "xsfdrv.h"

#if _MSC_VER >= 1200
#pragma comment(linker, "/EXPORT:XSFSetup=_XSFSetup@8")
#endif

#if defined(_MSC_VER) && !defined(_DEBUG)
#pragma comment(linker,"/MERGE:.rdata=.text")
#endif

typedef int BOOL;
typedef uint32_t DWORD;
typedef const wchar_t* LPCWSTR;
typedef void* LPVOID;

static void * XSFLibAlloc(DWORD dwSize)
{
	return malloc(dwSize);
}
static void XSFLibFree(void *lpPtr)
{
	free(lpPtr);
}
static int XSFStart(void *lpPtr, DWORD dwSize)
{
	return !xsf_start(lpPtr, dwSize);
}
static void XSFGen(void *lpPtr, DWORD dwSamples)
{
	xsf_gen(lpPtr, dwSamples);
}
static void XSFTerm(void)
{
	xsf_term();
}

unsigned long dwChannelMute = 0;

static void XSFSetChannelMute(DWORD dwPage, DWORD dwMute)
{
	if (dwPage == 0)
		dwChannelMute = dwMute;
}

#ifdef XSFDRIVER_EXTENDPARAM1NAME
static void XSFSetExtendParam(DWORD dwId, LPCWSTR lpPtr)
{
	xsf_set_extend_param(dwId, lpPtr);
}
static void XSFSetExtendParamVoid(DWORD dwId, const LPVOID lpPtr)
{
	xsf_set_extend_param_void(dwId, lpPtr);
}
#endif

static void *lpUserWrok = 0;
static LPFNGETLIB_XSFDRV lpfnGetLib = 0;
static IXSFDRV ifaossf =
{
	XSFLibAlloc,
	XSFLibFree,
	XSFStart,
	XSFGen,
	XSFTerm,
#ifdef XSFDRIVER_EXTENDPARAM1NAME
	5,
	XSFSetChannelMute,
	XSFSetExtendParam, 
	XSFSetExtendParamVoid
#else
	2,
	XSFSetChannelMute
#endif
};

IXSFDRV * XSFSetup(LPFNGETLIB_XSFDRV lpfn, void *lpWork)
{
	lpfnGetLib = lpfn;
	lpUserWrok = lpWork;
	return &ifaossf;
}

int xsf_get_lib(char *pfilename, void **ppbuffer, unsigned *plength)
{
	DWORD length32;
	if (!lpfnGetLib || lpfnGetLib(lpUserWrok, pfilename, ppbuffer, &length32)) return 0;
	if (plength) *plength = length32;
	return 1;
}

#ifdef _WIN32
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
#if defined(_MSC_VER) && defined(_DEBUG)
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
		DisableThreadLibraryCalls(hinstDLL);
	}
	return TRUE;
}
#endif
