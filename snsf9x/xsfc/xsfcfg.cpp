#define WIN32_LEAN_AND_MEAN
#include "leakchk.h"

#include <windows.h>
#include <windowsx.h>

#include "xsfcfg.h"
#include "tagget.h"
#include "xsfui.rh"
#include "../pversion.h"

unsigned long dwChannelMute[4] = { 0, 0, 0, 0 };
#ifdef XSFDRIVER_SAMPLERATE_COUNT
#ifndef XSFDRIVER_SAMPLERATE
unsigned long dwSampleRate = 44100;
#else
unsigned long dwSampleRate = XSFDRIVER_SAMPLERATE;
#endif
const char* const	SampleRateName[] = XSFDRIVER_SAMPLERATE_NAME;
const unsigned long SampleRateValue[] = XSFDRIVER_SAMPLERATE_VALUE;
#endif


long loInterpolationLevel = 0;
long loResampler = 1;
unsigned long dwDisableSurround = 0;
unsigned long dwReverseStereo = 0;

//combo box
const char* const	InterpolationName[]		= XSFDRIVER_INTERPOLATION_NAME;
const long			InterpolationValue[]	= XSFDRIVER_INTERPOLATION_VALUE;
const char* const	ResamplerName[]			= XSFDRIVER_RESAMPLER_NAME;
const long			ResamplerValue[]		= XSFDRIVER_RESAMPLER_VALUE;



namespace
{

const int VolumeBase = 16;

unsigned long dwPlayInfinitely;
unsigned long dwSkipSilenceOnStartSec;
unsigned long dwDetectSilenceSec;
unsigned long dwDefaultLength;
unsigned long dwDefaultFade;
double dVolume;




xsfc::TString sDefaultLengthC(L"1:55");
xsfc::TString sDefaultFadeC(L"5");
xsfc::TString sDefaultLength;
xsfc::TString sDefaultFade;

typedef struct
{
	int devch;
	const char *devname;
} CHANNELMAP;

const CHANNELMAP chmap[] = XSFDRIVER_CHANNELMAP;

#ifdef XSFDRIVER_EXTENDPARAM1NAME
xsfc::TString sExtendParam1C(XSFDRIVER_EXTENDPARAM1DEFAULT);
xsfc::TString sExtendParam1;
#endif

#ifdef XSFDRIVER_EXTENDPARAM2NAME
xsfc::TString sExtendParam2C(XSFDRIVER_EXTENDPARAM2DEFAULT);
xsfc::TString sExtendParam2;
#endif

}

unsigned long CFGGetChannelMute(int page)
{
	return (page < 4) ? dwChannelMute[page] : ~unsigned long(0);
}

void CFGSetChannelMute(int ch, bool mute)
{
	int page = ch >> 5;
	if (page < 4)
	{
		if (mute)
			dwChannelMute[page] |= ((unsigned long(1)) << (ch & 0x1f));
		else
			dwChannelMute[page] &= ~((unsigned long(1)) << (ch & 0x1f));
	}
}


unsigned CFGGetPlayInfinitely(void)
{
	return dwPlayInfinitely;
}
unsigned CFGGetSkipSilenceOnStartSec(void)
{
	return dwSkipSilenceOnStartSec;
}
unsigned CFGGetDetectSilenceSec(void)
{
	return dwDetectSilenceSec;
}
unsigned CFGGetDefaultLength(void)
{
	return dwDefaultLength;
}
unsigned CFGGetDefaultFade(void)
{
	return dwDefaultFade;
}
bool CFGGetVolume(double &vol)
{
	int iVolume = int(dVolume);
	if (iVolume == 0 || iVolume == VolumeBase)
	{
		vol = 1;
		return false;
	}
	vol = dVolume / VolumeBase;
	return true;
}

const wchar_t *CFGGetExtendParam1(void)
{
#ifdef XSFDRIVER_EXTENDPARAM1NAME
	return sExtendParam1;
#else
	return 0;
#endif
}
const wchar_t *CFGGetExtendParam2(void)
{
#ifdef XSFDRIVER_EXTENDPARAM2NAME
	return sExtendParam2;
#else
	return 0;
#endif
}

void CFGDefault(void)
{
	dwPlayInfinitely = 0;
	dwSkipSilenceOnStartSec = 5;
	dwDetectSilenceSec = 5;
	dwDefaultLength = (1 * 60 + 55) * 1000;
	dwDefaultFade = 5 * 1000;
	dVolume = VolumeBase;

	sDefaultLength = sDefaultLengthC;
	sDefaultFade = sDefaultFadeC;

#ifdef XSFDRIVER_EXTENDPARAM1NAME
	sExtendParam1 = sExtendParam1C;
#endif
#ifdef XSFDRIVER_EXTENDPARAM2NAME
	sExtendParam2 = sExtendParam2C;
#endif
#ifdef XSFDRIVER_SAMPLERATE_COUNT
#ifndef XSFDRIVER_SAMPLERATE
	unsigned long dwSampleRate = 44100;
#else
	unsigned long dwSampleRate = XSFDRIVER_SAMPLERATE;
#endif
#endif

	loInterpolationLevel = 0;
	loResampler = 1;
	dwDisableSurround = 0;
	dwReverseStereo = 0;

	int tch = 0;
	for (const CHANNELMAP *pchmap = &chmap[0]; pchmap->devch; pchmap++)
	{
		for (int ch = 1; ch <= pchmap->devch; ch++)
		{
			CFGSetChannelMute(tch++, false);
		}
	}
	while (tch < 32 * 4)
		CFGSetChannelMute(tch++, true);
}

void CFGLoad(LPIConfigIO pcfg)
{
	try
	{

		CFGDefault();

		dwPlayInfinitely = pcfg->GetULong(L"PlayInfinitely", dwPlayInfinitely);
		dwSkipSilenceOnStartSec = pcfg->GetULong(L"SkipSilenceOnStartSec", dwSkipSilenceOnStartSec);
		dwDetectSilenceSec = pcfg->GetULong(L"DetectSilenceSec", dwDetectSilenceSec);
		dVolume = pcfg->GetFloat(L"Volume", dVolume / dVolume) * VolumeBase;
		sDefaultLength = pcfg->GetString(L"DefaultLength", sDefaultLength);
		sDefaultFade = pcfg->GetString(L"DefaultFade", sDefaultFade);
#ifdef XSFDRIVER_EXTENDPARAM1NAME
		sExtendParam1 = pcfg->GetString(XSFDRIVER_EXTENDPARAM1NAME, sExtendParam1);
#endif
#ifdef XSFDRIVER_EXTENDPARAM2NAME
		sExtendParam2 = pcfg->GetString(XSFDRIVER_EXTENDPARAM2NAME, sExtendParam2);
#endif
#ifdef XSFDRIVER_SAMPLERATE_COUNT
		dwSampleRate = pcfg->GetULong(L"SampleRate", dwSampleRate);
#endif

		dwDefaultLength = XSFTag::ToMS(sDefaultLength);
		dwDefaultFade = XSFTag::ToMS(sDefaultFade);

		loInterpolationLevel = pcfg->GetLong(L"Interpolation", loInterpolationLevel);
		loResampler = pcfg->GetLong(L"Resampler", loResampler);

		dwDisableSurround = pcfg->GetULong(L"DisableSurround", dwDisableSurround);
		dwReverseStereo = pcfg->GetULong(L"ReverseStereo", dwReverseStereo);
	}
	catch (xsfc::EShortOfMemory e)
	{
	}
}

void CFGSave(LPIConfigIO pcfg)
{
	try
	{
		pcfg->SetULong(L"PlayInfinitely", dwPlayInfinitely);
		pcfg->SetULong(L"SkipSilenceOnStartSec", dwSkipSilenceOnStartSec);
		pcfg->SetULong(L"DetectSilenceSec", dwDetectSilenceSec);
		pcfg->SetFloat(L"Volume", dVolume / VolumeBase);
		pcfg->SetString(L"DefaultLength", sDefaultLength);
		pcfg->SetString(L"DefaultFade", sDefaultFade);
#ifdef XSFDRIVER_EXTENDPARAM1NAME
		pcfg->SetString(XSFDRIVER_EXTENDPARAM1NAME, sExtendParam1);
#endif
#ifdef XSFDRIVER_EXTENDPARAM2NAME
		pcfg->SetString(XSFDRIVER_EXTENDPARAM2NAME, sExtendParam2);
#endif
#ifdef XSFDRIVER_SAMPLERATE_COUNT	
		pcfg->SetULong(L"SampleRate", dwSampleRate);
#endif
		pcfg->SetLong(L"Interpolation", loInterpolationLevel);
		pcfg->SetLong(L"Resampler", loResampler);
		pcfg->SetULong(L"DisableSurround", dwDisableSurround);
		pcfg->SetULong(L"ReverseStereo", dwReverseStereo);
	}
	catch (xsfc::EShortOfMemory e)
	{
	}
}

void CFGReset(LPIConfigIO pcfg, void *hwndDlg)
{
	(void)pcfg;
	try
	{
		char buf[128];
		xsfc::TWin32::DlgSetCheck(hwndDlg, 0x200, dwPlayInfinitely == 1);
		xsfc::TWin32::DlgSetText(hwndDlg, 0x201, sDefaultLength);
		xsfc::TWin32::DlgSetText(hwndDlg, 0x202, sDefaultFade);
		xsfc::TWin32::DlgSetText(hwndDlg, 0x203, xsfc::TString(dwSkipSilenceOnStartSec));
		xsfc::TWin32::DlgSetText(hwndDlg, 0x204, xsfc::TString(dwDetectSilenceSec));
		xsfc::TWin32::DlgSetText(hwndDlg, 0x205, xsfc::TString(dVolume / VolumeBase));
#ifdef XSFDRIVER_EXTENDPARAM1NAME
		xsfc::TWin32::DlgSetText(hwndDlg, 0x207, xsfc::TString(XSFDRIVER_EXTENDPARAM1LABEL));
		xsfc::TWin32::DlgSetText(hwndDlg, 0x208, sExtendParam1);
		xsfc::TWin32::DlgSetEnabled(hwndDlg, 0x208, true);
		::ShowWindow(GetDlgItem((HWND)hwndDlg, 0x208), SW_SHOWNA);
#endif
#ifdef XSFDRIVER_EXTENDPARAM2NAME
		xsfc::TWin32::DlgSetText(hwndDlg, 0x209, xsfc::TString(XSFDRIVER_EXTENDPARAM2LABEL));
		xsfc::TWin32::DlgSetText(hwndDlg, 0x20a, sExtendParam2);
		xsfc::TWin32::DlgSetEnabled(hwndDlg, 0x20a, true);
		::ShowWindow(GetDlgItem((HWND)hwndDlg, 0x20a), SW_SHOWNA);
#endif

		for (const CHANNELMAP *pchmap = &chmap[0]; pchmap->devch; pchmap++)
		{
			for (int ch = 1; ch <= pchmap->devch; ch++)
			{
#if defined(HAVE_SPRINTF_S)
				sprintf_s(buf, sizeof(buf), pchmap->devname, ch);
#elif defined(HAVE_SNPRINTF)
				snprintf(buf, sizeof(buf), pchmap->devname, ch);
				buf[sizeof(buf) - 1] = 0;
#elif defined(HAVE_SNPRINTF_)
				_snprintf(buf, sizeof(buf), pchmap->devname, ch);
#else
				sprintf(buf, pchmap->devname, ch);
#endif
				xsfc::TWin32::DlgAddList(hwndDlg, IDC_MUTE, buf);
			}
		}

		//combo box
		HWND h = 0;
		unsigned CurSel = 0;
#ifdef XSFDRIVER_SAMPLERATE_COUNT
		h = GetDlgItem((HWND)hwndDlg, IDC_SAMPLE_RATE);
		CurSel = 0;
		for (unsigned i = 0; i < XSFDRIVER_SAMPLERATE_COUNT; i++)
		{
			xsfc::TWin32::DlgAddCombo(hwndDlg, IDC_SAMPLE_RATE, SampleRateName[i]);
			if (dwSampleRate == SampleRateValue[i])
				CurSel = i;
		}
		xsfc::TWin32::WndMsgSend(h, CB_SETCURSEL, CurSel, 0);
#endif

		h = GetDlgItem((HWND)hwndDlg, IDC_INTERPOLATION);	
		for (unsigned i = 0; i < XSFDRIVER_INTERPOLATION_COUNT; i++)
		{
			xsfc::TWin32::DlgAddCombo(hwndDlg, IDC_INTERPOLATION, InterpolationName[i]);
			if (loInterpolationLevel == InterpolationValue[i])
				CurSel = i;
		}
		xsfc::TWin32::WndMsgSend(h, CB_SETCURSEL, CurSel, 0);
		
		 h = GetDlgItem((HWND)hwndDlg, IDC_RESAMPLER);
		CurSel = 0;
		for (unsigned i = 0; i < XSFDRIVER_RESAMPLER_COUNT; i++)
		{
			xsfc::TWin32::DlgAddCombo(hwndDlg, IDC_RESAMPLER, ResamplerName[i]);
			if (loResampler == ResamplerValue[i])
				CurSel = i;
		}
		xsfc::TWin32::WndMsgSend(h, CB_SETCURSEL, CurSel, 0);

		xsfc::TWin32::DlgSetCheck(hwndDlg, IDC_DISABLE_SURROUND, dwDisableSurround == 1);
		xsfc::TWin32::DlgSetCheck(hwndDlg, IDC_REVERSE_STEREO, dwReverseStereo == 1);

	}
	catch (xsfc::EShortOfMemory e)
	{
	}
}

void CFGUpdate(LPIConfigIO pcfg, void *hwndDlg)
{
	(void)pcfg;
	try
	{
		dwPlayInfinitely = xsfc::TWin32::DlgGetCheck(hwndDlg, 0x200) ? 1 : 0;
		sDefaultLength = xsfc::TWin32::DlgGetText(hwndDlg, 0x201);
		sDefaultFade = xsfc::TWin32::DlgGetText(hwndDlg, 0x202);
		dwSkipSilenceOnStartSec = xsfc::TWin32::DlgGetText(hwndDlg, 0x203).GetULong();
		dwDetectSilenceSec = xsfc::TWin32::DlgGetText(hwndDlg, 0x204).GetULong();
		dVolume = xsfc::TWin32::DlgGetText(hwndDlg, 0x205).GetFloat() * VolumeBase;
#ifdef XSFDRIVER_EXTENDPARAM1NAME
		sExtendParam1 = xsfc::TWin32::DlgGetText(hwndDlg, 0x208);
#endif
#ifdef XSFDRIVER_EXTENDPARAM2NAME
		sExtendParam2 = xsfc::TWin32::DlgGetText(hwndDlg, 0x20a);
#endif

		dwDefaultLength = XSFTag::ToMS(sDefaultLength);
		dwDefaultFade = XSFTag::ToMS(sDefaultFade);

		int CurSel = 0;
#ifdef XSFDRIVER_SAMPLERATE_COUNT
		CurSel = xsfc::TWin32::DlgCurCombo(hwndDlg, IDC_SAMPLE_RATE);
		if (CurSel != CB_ERR) dwSampleRate = SampleRateValue[CurSel];
#endif
		CurSel						= xsfc::TWin32::DlgCurCombo(hwndDlg, IDC_INTERPOLATION);
		if (CurSel != CB_ERR) loInterpolationLevel = InterpolationValue[CurSel];
		CurSel						= xsfc::TWin32::DlgCurCombo(hwndDlg, IDC_RESAMPLER);
		if (CurSel != CB_ERR) loResampler = ResamplerValue[CurSel];
		dwDisableSurround			= xsfc::TWin32::DlgGetCheck(hwndDlg, IDC_DISABLE_SURROUND) ? 1 : 0;
		dwReverseStereo				= xsfc::TWin32::DlgGetCheck(hwndDlg, IDC_REVERSE_STEREO) ? 1 : 0;
	}
	catch (xsfc::EShortOfMemory e)
	{
	}
}

int CFGIsChanged(LPIConfigIO pcfg, void *hwndDlg)
{
	(void)pcfg;
	try
	{
		if (dwPlayInfinitely != (xsfc::TWin32::DlgGetCheck(hwndDlg, 0x200) ? 1U : 0U)) return 1;
		xsfc::TString xsDefaultLength = xsfc::TWin32::DlgGetText(hwndDlg, 0x201);
		xsfc::TString xsDefaultFade = xsfc::TWin32::DlgGetText(hwndDlg, 0x202);
		if (dwSkipSilenceOnStartSec != xsfc::TWin32::DlgGetText(hwndDlg, 0x203).GetULong()) return 2;
		if (dwDetectSilenceSec != xsfc::TWin32::DlgGetText(hwndDlg, 0x204).GetULong()) return 2;
		if (dVolume != xsfc::TWin32::DlgGetText(hwndDlg, 0x205).GetFloat() * VolumeBase) return 1;
#ifdef XSFDRIVER_EXTENDPARAM1NAME
		xsfc::TString xsExtendParam1 = xsfc::TWin32::DlgGetText(hwndDlg, 0x208);
		if (lstrcmpW(sExtendParam1, xsExtendParam1)) return 1;
#endif
#ifdef XSFDRIVER_EXTENDPARAM2NAME
		xsfc::TString xsExtendParam2 = xsfc::TWin32::DlgGetText(hwndDlg, 0x20a);
		if (lstrcmpW(sExtendParam2, xsExtendParam2)) return 1;
#endif

		if (dwDefaultLength != XSFTag::ToMS(xsDefaultLength)) return 2;
		if (dwDefaultFade != XSFTag::ToMS(xsDefaultFade)) return 2;

		int CurSel = 0;
#ifdef XSFDRIVER_SAMPLERATE_COUNT
		CurSel = xsfc::TWin32::DlgCurCombo(hwndDlg, IDC_SAMPLE_RATE);
		if (CurSel != CB_ERR && dwSampleRate != SampleRateValue[CurSel]) return 1;
#endif
		CurSel = xsfc::TWin32::DlgCurCombo(hwndDlg, IDC_INTERPOLATION);
		if (CurSel != CB_ERR && loInterpolationLevel != InterpolationValue[CurSel]) return 1;
		CurSel = xsfc::TWin32::DlgCurCombo(hwndDlg, IDC_RESAMPLER);
		if (CurSel != CB_ERR && loResampler != ResamplerValue[CurSel]) return 1;
		if (dwDisableSurround != (xsfc::TWin32::DlgGetCheck(hwndDlg, IDC_DISABLE_SURROUND) ? 1U : 0U)) return 1;
		if (dwReverseStereo != (xsfc::TWin32::DlgGetCheck(hwndDlg, IDC_REVERSE_STEREO) ? 1U : 0U)) return 1;
	}
	catch (xsfc::EShortOfMemory e)
	{
	}
	return 0;
}

void CFGMuteChange(void *hwndDlg, int itm)
{
	int cnt = xsfc::TWin32::DlgCntList(hwndDlg, itm);
	for (int ch = 0; ch < cnt; ch++)
	{
		bool mute = xsfc::TWin32::DlgGetList(hwndDlg, itm, ch);
		CFGSetChannelMute(ch, mute);
	}
}

