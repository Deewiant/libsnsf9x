/* Identifiers that can be used with the SetExtendParamVoid function. */
#define XSFDRIVER_EXTENDPARAM_INTERPOLATION    1
#define XSFDRIVER_EXTENDPARAM_RESAMPLER        2
#define XSFDRIVER_EXTENDPARAM_SAMPLE_RATE      3
#define XSFDRIVER_EXTENDPARAM_DISABLE_SURROUND 4
#define XSFDRIVER_EXTENDPARAM_REVERSE_STEREO   5

#define XSFDRIVER_INTERPOLATION_NAME { "Gaussian", "None", "Linear", "Cubic", "Sinc" }
#define XSFDRIVER_INTERPOLATION_VALUE { 0, -2, -1, 1, 2 }
#define XSFDRIVER_RESAMPLER_NAME { "Hermite", "Linear", "Bspline", "Osculating", "Sinc" }
#define XSFDRIVER_RESAMPLER_VALUE {1, -1, 2, 3, 4}

#define SNES9X_NAME "Snes9x"
#define SNES9X_VERSIONS "v1.53"
#define SNES9X_COPYRIGHT \
"  (c) Copyright 1996 - 2002  Gary Henderson (gary.henderson@ntlworld.com),\n" \
"                             Jerremy Koot (jkoot@snes9x.com)\n" \
"\n" \
"  (c) Copyright 2002 - 2004  Matthew Kendora\n" \
"\n" \
"  (c) Copyright 2002 - 2005  Peter Bortas (peter@bortas.org)\n" \
"\n" \
"  (c) Copyright 2004 - 2005  Joel Yliluoma (http://iki.fi/bisqwit/)\n" \
"\n" \
"  (c) Copyright 2001 - 2006  John Weidman (jweidman@slip.net)\n" \
"\n" \
"  (c) Copyright 2002 - 2006  funkyass (funkyass@spam.shaw.ca),\n" \
"                             Kris Bleakley (codeviolation@hotmail.com)\n" \
"\n" \
"  (c) Copyright 2002 - 2010  Brad Jorsch (anomie@users.sourceforge.net),\n" \
"                             Nach (n-a-c-h@users.sourceforge.net),\n" \
"                             zones (kasumitokoduck@yahoo.com)\n" \
"\n" \
"  (c) Copyright 2006 - 2007  nitsuja\n" \
"\n" \
"  (c) Copyright 2009 - 2010  BearOso,\n" \
"                             OV2\n" \
"\n" \
"\n" \
"  BS-X C emulator code\n" \
"  (c) Copyright 2005 - 2006  Dreamer Nom,\n" \
"                             zones\n" \
"\n" \
"  C4 x86 assembler and some C emulation code\n" \
"  (c) Copyright 2000 - 2003  _Demo_ (_demo_@zsnes.com),\n" \
"                             Nach,\n" \
"                             zsKnight (zsknight@zsnes.com)\n" \
"\n" \
"  C4 C++ code\n" \
"  (c) Copyright 2003 - 2006  Brad Jorsch,\n" \
"                             Nach\n" \
"\n" \
"  DSP-1 emulator code\n" \
"  (c) Copyright 1998 - 2006  _Demo_,\n" \
"                             Andreas Naive (andreasnaive@gmail.com),\n" \
"                             Gary Henderson,\n" \
"                             Ivar (ivar@snes9x.com),\n" \
"                             John Weidman,\n" \
"                             Kris Bleakley,\n" \
"                             Matthew Kendora,\n" \
"                             Nach,\n" \
"                             neviksti (neviksti@hotmail.com)\n" \
"\n" \
"  DSP-2 emulator code\n" \
"  (c) Copyright 2003         John Weidman,\n" \
"                             Kris Bleakley,\n" \
"                             Lord Nightmare (lord_nightmare@users.sourceforge.net),\n" \
"                             Matthew Kendora,\n" \
"                             neviksti\n" \
"\n" \
"  DSP-3 emulator code\n" \
"  (c) Copyright 2003 - 2006  John Weidman,\n" \
"                             Kris Bleakley,\n" \
"                             Lancer,\n" \
"                             z80 gaiden\n" \
"\n" \
"  DSP-4 emulator code\n" \
"  (c) Copyright 2004 - 2006  Dreamer Nom,\n" \
"                             John Weidman,\n" \
"                             Kris Bleakley,\n" \
"                             Nach,\n" \
"                             z80 gaiden\n" \
"\n" \
"  OBC1 emulator code\n" \
"  (c) Copyright 2001 - 2004  zsKnight,\n" \
"                             pagefault (pagefault@zsnes.com),\n" \
"                             Kris Bleakley\n" \
"                             Ported from x86 assembler to C by sanmaiwashi\n" \
"\n" \
"  SPC7110 and RTC C++ emulator code used in 1.39-1.51\n" \
"  (c) Copyright 2002         Matthew Kendora with research by\n" \
"                             zsKnight,\n" \
"                             John Weidman,\n" \
"                             Dark Force\n" \
"\n" \
"  SPC7110 and RTC C++ emulator code used in 1.52+\n" \
"  (c) Copyright 2009         byuu,\n" \
"                             neviksti\n" \
"\n" \
"  S-DD1 C emulator code\n" \
"  (c) Copyright 2003         Brad Jorsch with research by\n" \
"                             Andreas Naive,\n" \
"                             John Weidman\n" \
"\n" \
"  S-RTC C emulator code\n" \
"  (c) Copyright 2001 - 2006  byuu,\n" \
"                             John Weidman\n" \
"\n" \
"  ST010 C++ emulator code\n" \
"  (c) Copyright 2003         Feather,\n" \
"                             John Weidman,\n" \
"                             Kris Bleakley,\n" \
"                             Matthew Kendora\n" \
"\n" \
"  Super FX x86 assembler emulator code\n" \
"  (c) Copyright 1998 - 2003  _Demo_,\n" \
"                             pagefault,\n" \
"                             zsKnight\n" \
"\n" \
"  Super FX C emulator code\n" \
"  (c) Copyright 1997 - 1999  Ivar,\n" \
"                             Gary Henderson,\n" \
"                             John Weidman\n" \
"\n" \
"  Sound emulator code used in 1.5-1.51\n" \
"  (c) Copyright 1998 - 2003  Brad Martin\n" \
"  (c) Copyright 1998 - 2006  Charles Bilyue'\n" \
"\n" \
"  Sound emulator code used in 1.52+\n" \
"  (c) Copyright 2004 - 2007  Shay Green (gblargg@gmail.com)\n" \
"\n" \
"  SH assembler code partly based on x86 assembler code\n" \
"  (c) Copyright 2002 - 2004  Marcus Comstedt (marcus@mc.pp.se)\n" \
"\n" \
"  2xSaI filter\n" \
"  (c) Copyright 1999 - 2001  Derek Liauw Kie Fa\n" \
"\n" \
"  HQ2x, HQ3x, HQ4x filters\n" \
"  (c) Copyright 2003         Maxim Stepin (maxim@hiend3d.com)\n" \
"\n" \
"  NTSC filter\n" \
"  (c) Copyright 2006 - 2007  Shay Green\n" \
"\n" \
"  GTK+ GUI code\n" \
"  (c) Copyright 2004 - 2010  BearOso\n" \
"\n" \
"  Win32 GUI code\n" \
"  (c) Copyright 2003 - 2006  blip,\n" \
"                             funkyass,\n" \
"                             Matthew Kendora,\n" \
"                             Nach,\n" \
"                             nitsuja\n" \
"  (c) Copyright 2009 - 2010  OV2\n" \
"\n" \
"  Mac OS GUI code\n" \
"  (c) Copyright 1998 - 2001  John Stiles\n" \
"  (c) Copyright 2001 - 2010  zones\n" \
"\n" \
"\n" \
"  Specific ports contains the works of other authors. See headers in\n" \
"  individual files.\n"
