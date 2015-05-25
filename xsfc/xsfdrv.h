#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef int BOOL;
typedef uint32_t DWORD;
typedef const wchar_t* LPCWSTR;
typedef char* LPSTR;
typedef void* LPVOID;

typedef int (* LPFNGETLIB_XSFDRV)(void *lpWork, LPSTR lpszFilename, void **ppBuffer, DWORD *pdwSize);
typedef struct
{
	/* V1 */
	void * (* LibAlloc)(DWORD dwSize);
	void (* LibFree)(void *lpPtr);
	int (* Start)(void *lpPtr, DWORD dwSize);
	void (* Gen)(void *lpPtr, DWORD dwSamples);
	void (* Term)(void);

	/* V2 */
	DWORD dwInterfaceVersion;
	void (* SetChannelMute)(DWORD dwPage, DWORD dwMute);

	/* V3 */
	void (* SetExtendParam)(DWORD dwId, LPCWSTR lpPtr);
	void (* SetExtendParamVoid)(DWORD dwId, const LPVOID lpPtr);
} IXSFDRV;

typedef IXSFDRV * (* LPFNXSFDRVSETUP)(LPFNGETLIB_XSFDRV lpfn, void *lpWork);

#ifdef __cplusplus
}
#endif

