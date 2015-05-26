#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef int (* LPFNGETLIB_XSFDRV)(void *lpWork, char *lpszFilename, void **ppBuffer, uint32_t *pdwSize);
typedef struct
{
	/* V1 */
	void * (* LibAlloc)(uint32_t dwSize);
	void (* LibFree)(void *lpPtr);
	int (* Start)(void *lpPtr, uint32_t dwSize);
	void (* Gen)(void *lpPtr, uint32_t dwSamples);
	void (* Term)(void);

	/* V2 */
	uint32_t dwInterfaceVersion;
	void (* SetChannelMute)(uint32_t dwPage, uint32_t dwMute);

	/* V3 */
	void (* SetExtendParam)(uint32_t dwId, const wchar_t* lpPtr);
	void (* SetExtendParamVoid)(uint32_t dwId, const void* lpPtr);
} IXSFDRV;

#ifdef __cplusplus
}
#endif

