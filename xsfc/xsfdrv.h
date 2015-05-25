#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define PASCAL
typedef int BOOL;
typedef uint32_t DWORD;
typedef const wchar_t* LPCWSTR;
typedef char* LPSTR;
typedef void* LPVOID;

typedef int (PASCAL * LPFNGETLIB_XSFDRV)(void *lpWork, LPSTR lpszFilename, void **ppBuffer, DWORD *pdwSize);
typedef struct
{
	/* V1 */
	void * (PASCAL * LibAlloc)(DWORD dwSize);
	void (PASCAL * LibFree)(void *lpPtr);
	int (PASCAL * Start)(void *lpPtr, DWORD dwSize);
	void (PASCAL * Gen)(void *lpPtr, DWORD dwSamples);
	void (PASCAL * Term)(void);

	/* V2 */
	DWORD dwInterfaceVersion;
	void (PASCAL * SetChannelMute)(DWORD dwPage, DWORD dwMute);

	/* V3 */
	void (PASCAL * SetExtendParam)(DWORD dwId, LPCWSTR lpPtr);
	void (PASCAL * SetExtendParamVoid)(DWORD dwId, const LPVOID lpPtr);
} IXSFDRV;

typedef IXSFDRV * (PASCAL * LPFNXSFDRVSETUP)(LPFNGETLIB_XSFDRV lpfn, void *lpWork);
/* IXSFDRV * PASCAL XSFDRVSetup(LPFNGETLIB_XSFDRV lpfn, void *lpWork); */

#ifdef __cplusplus
}
#endif

