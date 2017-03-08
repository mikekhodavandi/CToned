#define PLUG_MFR "FYP2"
#define PLUG_NAME "CToned"

#define PLUG_CLASS_NAME CToned

#define BUNDLE_MFR "FYP2"
#define BUNDLE_NAME "CToned"

#define PLUG_ENTRY CToned_Entry
#define PLUG_VIEW_ENTRY CToned_ViewEntry

#define PLUG_ENTRY_STR "CToned_Entry"
#define PLUG_VIEW_ENTRY_STR "CToned_ViewEntry"

#define VIEW_CLASS CToned_View
#define VIEW_CLASS_STR "CToned_View"

//
#define PLUG_VER 0x00010000
#define VST3_VER_STR "1.0.0"

#define PLUG_UNIQUE_ID 'Ipef'
// not the same as BUNDLE_MFR
#define PLUG_MFR_ID 'Acme'

// ProTools stuff

#if (defined(AAX_API) || defined(RTAS_API)) && !defined(_PIDS_)
  #define _PIDS_
  const int PLUG_TYPE_IDS[2] = {'EFN1', 'EFN2'};
  const int PLUG_TYPE_IDS_AS[2] = {'EFA1', 'EFA2'}; // AudioSuite
#endif

#define PLUG_MFR_PT "FYP2\nFYP2\nAcme"
#define PLUG_NAME_PT "CToned\nIPEF"
#define PLUG_TYPE_PT "Effect"
#define PLUG_DOES_AUDIOSUITE 1


#if (defined(AAX_API) || defined(RTAS_API))
#define PLUG_CHANNEL_IO "1-1 2-2"
#else

// no audio input. mono or stereo output

#define PLUG_CHANNEL_IO "0-1 0-2"
#endif

#define PLUG_LATENCY 0
#define PLUG_IS_INST 1

#define PLUG_DOES_MIDI 1

#define PLUG_DOES_STATE_CHUNKS 0

//image resource ID .

#define BG_ID         101
#define WHITE_KEY_ID  102
#define BLACK_KEY_ID  103
#define WAVEFORM_ID   104
#define KNOB_ID       105

// Image resource locations for this plug.

#define BG_FN         "resources/img/bg.png"
#define WHITE_KEY_FN  "resources/img/whitekey.png"
#define BLACK_KEY_FN  "resources/img/blackkey.png"
#define WAVEFORM_FN   "resources/img/waveform.png"
#define KNOB_FN       "resources/img/knob.png"

// GUI default dimensions

#define GUI_WIDTH 634
#define GUI_HEIGHT 896

#if defined(SA_API) && !defined(OS_IOS)
#include "app_wrapper/app_resource.h"
#endif


