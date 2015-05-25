#include "leakchk.h"

#include "../SDK/foobar2000.h"
#include "../helpers//helpers.h"
#include "pversion.h"
#include "xsfui.rh"
#include "tagget.h"


static const GUID guid_xsfcfg = XSFDRIVER_GUID1;
// {5026C16F-1654-4a06-A059-119E71A9DB0A}
static const GUID guid_cfg_play_infinitely =
{ 0x5026c16f, 0x1654, 0x4a06, { 0xa0, 0x59, 0x11, 0x9e, 0x71, 0xa9, 0xdb, 0xa } };
// {9ABCA67A-6FE0-47eb-AB75-2EDA242AC526}
static const GUID guid_cfg_default_len =
{ 0x9abca67a, 0x6fe0, 0x47eb, { 0xab, 0x75, 0x2e, 0xda, 0x24, 0x2a, 0xc5, 0x26 } };
// {BBCBB9AD-5D75-46dd-9375-299097401977}
static const GUID guid_cfg_default_fade_len =
{ 0xbbcbb9ad, 0x5d75, 0x46dd, { 0x93, 0x75, 0x29, 0x90, 0x97, 0x40, 0x19, 0x77 } };
// {ED76051E-4F11-42ab-899F-64732DFC05FD}
static const GUID guid_cfg_skip_silence =
{ 0xed76051e, 0x4f11, 0x42ab, { 0x89, 0x9f, 0x64, 0x73, 0x2d, 0xfc, 0x5, 0xfd } };
// {0CE085B9-C6ED-4c8e-ABA6-A16D8E07DFF1}
static const GUID guid_cfg_detect_silence =
{ 0xce085b9, 0xc6ed, 0x4c8e, { 0xab, 0xa6, 0xa1, 0x6d, 0x8e, 0x7, 0xdf, 0xf1 } };
// {8385C090-6D5E-456d-BFCB-1ECD3980F590}
static const GUID guid_cfg_volume =
{ 0x8385c090, 0x6d5e, 0x456d, { 0xbf, 0xcb, 0x1e, 0xcd, 0x39, 0x80, 0xf5, 0x90 } };
// {9F038B7B-1C3F-4d3e-AF2D-33F327C129F0}
static const GUID guid_cfg_mute =
{ 0x9f038b7b, 0x1c3f, 0x4d3e, { 0xaf, 0x2d, 0x33, 0xf3, 0x27, 0xc1, 0x29, 0xf0 } };
// {FA1B8746-A76C-4369-B53F-91B7DC678BB1}
static const GUID guid_cfg_interpolation =
{ 0xfa1b8746, 0xa76c, 0x4369, { 0xb5, 0x3f, 0x91, 0xb7, 0xdc, 0x67, 0x8b, 0xb1 } };
// {41230FCF-8CBB-48de-A20E-9426176C753B}
static const GUID guid_cfg_resampler =
{ 0x41230fcf, 0x8cbb, 0x48de, { 0xa2, 0xe, 0x94, 0x26, 0x17, 0x6c, 0x75, 0x3b } };
// {1D4E2F8E-9726-44c4-BA88-C7A45E7DF814}
static const GUID guid_cfg_sample_rate =
{ 0x1d4e2f8e, 0x9726, 0x44c4, { 0xba, 0x88, 0xc7, 0xa4, 0x5e, 0x7d, 0xf8, 0x14 } };
// {80A219BD-F7AC-48b3-99CB-26E32628834D}
static const GUID guid_cfg_disable_surround =
{ 0x80a219bd, 0xf7ac, 0x48b3, { 0x99, 0xcb, 0x26, 0xe3, 0x26, 0x28, 0x83, 0x4d } };
// {5BD6F9B9-2543-445b-AC87-2084176291F7}
static const GUID guid_cfg_reverse_stereo =
{ 0x5bd6f9b9, 0x2543, 0x445b, { 0xac, 0x87, 0x20, 0x84, 0x17, 0x62, 0x91, 0xf7 } };



cfg_int cfg_play_infinitely(guid_cfg_play_infinitely, 0);
cfg_string cfg_default_len(guid_cfg_default_len, "1:55");
cfg_string cfg_default_fade_len(guid_cfg_default_fade_len, "5");
cfg_int cfg_skip_silence(guid_cfg_skip_silence, 5);
cfg_int cfg_detect_silence(guid_cfg_detect_silence, 5);
cfg_string cfg_volume(guid_cfg_volume, "1.0");
cfg_uint cfg_mute(guid_cfg_mute, 0);
cfg_int cfg_interpolation(guid_cfg_interpolation, 0);
cfg_int cfg_resampler(guid_cfg_resampler, 0);
cfg_uint cfg_sample_rate(guid_cfg_sample_rate, 48000);
cfg_int cfg_disable_surround(guid_cfg_disable_surround, 0);
cfg_int cfg_reverse_stereo(guid_cfg_reverse_stereo, 0);


class foo_input_xsfcfg_instance : public preferences_page_instance
{
protected:
	HWND m_parent;
	HWND m_hwnd;
	const preferences_page_callback::ptr m_callback;
	bool m_initialized;



static const char* const	combo_interpolation_name	[XSFDRIVER_INTERPOLATION_COUNT];
static const int			combo_interpolation_value	[XSFDRIVER_INTERPOLATION_COUNT];
static const char* const	combo_resampler_name		[XSFDRIVER_RESAMPLER_COUNT];
static const int			combo_resampler_value		[XSFDRIVER_RESAMPLER_COUNT];
static const char* const	combo_sample_rate_name		[XSFDRIVER_SAMPLERATE_COUNT];
static const t_uint32		combo_sample_rate_value		[XSFDRIVER_SAMPLERATE_COUNT];

#define snsf_channel_count 8
static const char* const	snsf_channel_name			[snsf_channel_count];

static BOOL CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);


public:
	t_uint32 get_state()
	{
		int cur_interpolation = uSendDlgItemMessage(m_hwnd, IDC_INTERPOLATION, CB_GETCURSEL, 0, 0);
		int cur_resampler = uSendDlgItemMessage(m_hwnd, IDC_RESAMPLER, CB_GETCURSEL, 0, 0);
		int cur_sample_rate = uSendDlgItemMessage(m_hwnd, IDC_SAMPLE_RATE, CB_GETCURSEL, 0, 0);

		if (cfg_play_infinitely != IsDlgButtonChecked(m_hwnd, IDC_PLAY_INFINITELY) ||
			strcmp(cfg_default_len, string_utf8_from_window(m_hwnd, IDC_DEFAULT_LEN)) ||
			strcmp(cfg_default_fade_len, string_utf8_from_window(m_hwnd, IDC_DEFAULT_FADE_LEN)) ||
			cfg_skip_silence != uGetDlgItemInt(m_hwnd, IDC_SKIP_SILENCE, NULL, FALSE) ||
			cfg_detect_silence != uGetDlgItemInt(m_hwnd, IDC_DETECT_SILENCE, NULL, FALSE) ||
			strcmp(cfg_volume, string_utf8_from_window(m_hwnd, IDC_VOLUME)) ||		
			cur_interpolation != CB_ERR && cfg_interpolation != combo_interpolation_value[cur_interpolation] ||
			cur_resampler != CB_ERR && cfg_resampler != combo_resampler_value[cur_resampler] ||
			cur_sample_rate != CB_ERR && cfg_sample_rate != combo_sample_rate_value[cur_sample_rate] ||
			cfg_disable_surround != IsDlgButtonChecked(m_hwnd, IDC_DISABLE_SURROUND) ||
			cfg_reverse_stereo != IsDlgButtonChecked(m_hwnd, IDC_REVERSE_STEREO)
			)
			return preferences_state::changed | preferences_state::resettable;
		return preferences_state::resettable;
	}
	HWND get_wnd()
	{
		return m_hwnd;
	}

	HWND create(HWND p_parent)
	{
		return uCreateDialog(IDD_DIALOG1, p_parent, DialogProc, reinterpret_cast<LPARAM>(this));
	}

	void reset()
	{
		CheckDlgButton(m_hwnd, IDC_PLAY_INFINITELY, BST_UNCHECKED);
		uSetDlgItemText(m_hwnd, IDC_DEFAULT_LEN, "1:55");
		uSetDlgItemText(m_hwnd, IDC_DEFAULT_FADE_LEN, "5");
		uSetDlgItemInt(m_hwnd, IDC_SKIP_SILENCE, 5, FALSE);
		uSetDlgItemInt(m_hwnd, IDC_DETECT_SILENCE, 5, FALSE);
		uSetDlgItemText(m_hwnd, IDC_VOLUME, "1.0");
		CheckDlgButton(m_hwnd, IDC_DISABLE_SURROUND, BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_REVERSE_STEREO, BST_UNCHECKED);

		//combo box
		uSendDlgItemMessage(m_hwnd, IDC_INTERPOLATION, CB_SETCURSEL, 0, 0);
		uSendDlgItemMessage(m_hwnd, IDC_RESAMPLER, CB_SETCURSEL, 0, 0);
		uSendDlgItemMessage(m_hwnd, IDC_SAMPLE_RATE, CB_SETCURSEL, 0, 0);

		//mute
		HWND h = uGetDlgItem(m_hwnd, IDC_MUTE);
		for (unsigned i = 0; i < snsf_channel_count; i++)
		{
			uSendMessage(h, LB_SETSEL, 0, i);
			cfg_mute = 0;
		}
	}
	void apply()
	{
		cfg_play_infinitely = IsDlgButtonChecked(m_hwnd, IDC_PLAY_INFINITELY);
		uGetDlgItemText(m_hwnd, IDC_DEFAULT_LEN, cfg_default_len);
		uGetDlgItemText(m_hwnd, IDC_DEFAULT_FADE_LEN, cfg_default_fade_len);
		cfg_skip_silence = uGetDlgItemInt(m_hwnd, IDC_SKIP_SILENCE, NULL, FALSE);
		cfg_detect_silence = uGetDlgItemInt(m_hwnd, IDC_DETECT_SILENCE, NULL, FALSE);	
		uGetDlgItemText(m_hwnd, IDC_VOLUME, cfg_volume);
		
		int cur_sel = SendDlgItemMessage(m_hwnd, IDC_INTERPOLATION, CB_GETCURSEL, 0, 0);
		if (cur_sel != CB_ERR) cfg_interpolation = combo_interpolation_value[cur_sel];
		cur_sel = SendDlgItemMessage(m_hwnd, IDC_RESAMPLER, CB_GETCURSEL, 0, 0);
		if (cur_sel != CB_ERR) cfg_resampler = combo_resampler_value[cur_sel];
		cur_sel = SendDlgItemMessage(m_hwnd, IDC_SAMPLE_RATE, CB_GETCURSEL, 0, 0);
		if (cur_sel != CB_ERR) cfg_sample_rate = combo_sample_rate_value[cur_sel];

		cfg_disable_surround = IsDlgButtonChecked(m_hwnd, IDC_DISABLE_SURROUND);
		cfg_reverse_stereo = IsDlgButtonChecked(m_hwnd, IDC_REVERSE_STEREO);


		m_callback->on_state_changed();
	}


	foo_input_xsfcfg_instance(HWND parent, preferences_page_callback::ptr callback) : 
		m_initialized(false), 
		m_parent(parent), 
	    m_callback(callback)
	{
		m_hwnd = create(m_parent);
	}
	~foo_input_xsfcfg_instance() {}
};

const char* const	foo_input_xsfcfg_instance::combo_interpolation_name[] = XSFDRIVER_INTERPOLATION_NAME;
const int			foo_input_xsfcfg_instance::combo_interpolation_value[] = XSFDRIVER_INTERPOLATION_VALUE;
const char* const	foo_input_xsfcfg_instance::combo_resampler_name[] = XSFDRIVER_RESAMPLER_NAME;
const int			foo_input_xsfcfg_instance::combo_resampler_value[] = XSFDRIVER_RESAMPLER_VALUE;
const char* const   foo_input_xsfcfg_instance::combo_sample_rate_name[] = XSFDRIVER_SAMPLERATE_NAME;
const t_uint32		foo_input_xsfcfg_instance::combo_sample_rate_value[] = XSFDRIVER_SAMPLERATE_VALUE;

const char* const	foo_input_xsfcfg_instance::snsf_channel_name[] = { "BRRPCM 1", "BRRPCM 2", "BRRPCM 3", "BRRPCM 4", "BRRPCM 5", "BRRPCM 6", "BRRPCM 7", "BRRPCM 8" };


BOOL CALLBACK foo_input_xsfcfg_instance::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
		switch (uMsg)
		{
		case WM_COMMAND:
			{
				foo_input_xsfcfg_instance *pthis = static_cast<foo_input_xsfcfg_instance *>(xsfc::TWin32::WndGetLongPtr(hwndDlg, DWLP_USER));
				switch ( (LOWORD(wParam)))
				{

				case IDC_PLAY_INFINITELY:case IDC_DEFAULT_LEN: case IDC_DEFAULT_FADE_LEN: case IDC_SKIP_SILENCE:
				case IDC_DETECT_SILENCE: case IDC_VOLUME: case IDC_INTERPOLATION: case IDC_RESAMPLER: 
				case IDC_SAMPLE_RATE:case IDC_DISABLE_SURROUND: case IDC_REVERSE_STEREO:
					if (pthis) pthis->m_callback->on_state_changed();
					break;
				case IDC_MUTE:
				{
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						HWND h = uGetDlgItem(hwndDlg, IDC_MUTE);
						unsigned cur_sel = uSendMessage(h, LB_GETCURSEL, 0, 0);
						unsigned mute = uSendMessage(h, LB_GETSEL, cur_sel, 0) ? 1 : 0;
						cfg_mute = cfg_mute & (~(1 << cur_sel));
						cfg_mute = cfg_mute | (mute << cur_sel);
					}
				}
				default:
					break;
				}
				break;
			}

		case WM_INITDIALOG:
			{
				uSetWindowLong(hwndDlg, DWL_USER, lParam);

				CheckDlgButton(hwndDlg, IDC_PLAY_INFINITELY, (UINT)cfg_play_infinitely);
				uSetDlgItemText(hwndDlg, IDC_DEFAULT_LEN, cfg_default_len);
				uSetDlgItemText(hwndDlg, IDC_DEFAULT_FADE_LEN, cfg_default_fade_len);
				uSetDlgItemInt(hwndDlg, IDC_SKIP_SILENCE, cfg_skip_silence, FALSE);
				uSetDlgItemInt(hwndDlg, IDC_DETECT_SILENCE, cfg_detect_silence, FALSE);
				uSetDlgItemText(hwndDlg, IDC_VOLUME, cfg_volume);
				CheckDlgButton(hwndDlg, IDC_DISABLE_SURROUND, (UINT)cfg_disable_surround);
				CheckDlgButton(hwndDlg, IDC_REVERSE_STEREO, (UINT)cfg_reverse_stereo);
				

				//combo box
				HWND h = uGetDlgItem(hwndDlg, IDC_INTERPOLATION);
				unsigned cur_sel = 0;
				for (unsigned i = 0; i < XSFDRIVER_INTERPOLATION_COUNT; i++)
				{
					uSendMessageText(h, CB_ADDSTRING, 0, combo_interpolation_name[i]);
					if (cfg_interpolation == combo_interpolation_value[i])
						cur_sel = i;
				}
				uSendMessage(h, CB_SETCURSEL, cur_sel, 0);

				h = uGetDlgItem(hwndDlg, IDC_RESAMPLER);
				cur_sel = 0;
				for (unsigned i = 0; i < XSFDRIVER_RESAMPLER_COUNT; i++)
				{
					uSendMessageText(h, CB_ADDSTRING, 0, combo_resampler_name[i]);
					if (cfg_resampler == combo_resampler_value[i])
						cur_sel = i;
				}
				uSendMessage(h, CB_SETCURSEL, cur_sel, 0);
				
				h = uGetDlgItem(hwndDlg, IDC_SAMPLE_RATE);
				cur_sel = 0;
				for (unsigned i = 0; i < XSFDRIVER_SAMPLERATE_COUNT; i++)
				{
					uSendMessageText(h, CB_ADDSTRING, 0, combo_sample_rate_name[i]);
					if (cfg_sample_rate == combo_sample_rate_value[i])
						cur_sel = i;
				}
				uSendMessage(h, CB_SETCURSEL, cur_sel, 0);

				//mute
				h = uGetDlgItem(hwndDlg, IDC_MUTE);
				for (unsigned i = 0; i < snsf_channel_count; i++)
				{
					uSendMessageText(h, LB_ADDSTRING, 0, snsf_channel_name[i]);
					uSendMessage(h, LB_SETSEL, (cfg_mute & (1<<i)), i);
				}
				break;
			}
		case WM_DESTROY:
		{
			uSetWindowLong(hwndDlg, DWL_USER, 0);
			break;
		}
		default:
			return FALSE;
		}
	return TRUE;
}


class foo_input_xsfcfg : public preferences_page_v3
{
public:
	const char * get_name() { return XSFDRIVER_SIMPLENAME; }
	GUID get_guid() { return guid_xsfcfg; }
	GUID get_parent_guid() { return guid_input; }
	preferences_page_instance::ptr instantiate(HWND parent, preferences_page_callback::ptr callback)
	{
		return new service_impl_t<foo_input_xsfcfg_instance >(parent, callback);
	}
};

static preferences_page_factory_t<foo_input_xsfcfg> g_input_xsfcfg_factory;





HMODULE hDLL;
extern "C" void fb2k_config_init(HINSTANCE hinstDLL)
{
	hDLL = (HMODULE)hinstDLL;
}
