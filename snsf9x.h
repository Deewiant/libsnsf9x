#ifndef SNSF9X_H
#define SNSF9X_H

#ifdef __cplusplus
extern "C" {
#endif

#include "pversion.h"
#include "xsfc/tagget.h"
#include "xsfc/xsfdrv.h"

IXSFDRV * XSFSetup(LPFNGETLIB_XSFDRV lpfn, void *lpWork);

#ifdef __cplusplus
}
#endif

#endif
