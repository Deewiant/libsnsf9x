// snes_spc 0.9.0. http://www.slack.net/~ant/

#include <algorithm>
#include <cstring>
#include "SPC_DSP.h"

#include "blargg_endian.h"

/* Copyright (C) 2007 Shay Green. This module is free software; you
can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. This
module is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
details. You should have received a copy of the GNU Lesser General Public
License along with this module; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA */

#if INT_MAX < 0x7FFFFFFF
# error "Requires that int type have at least 32 bits"
#endif

static const uint8_t initial_regs[] =
{
	0x45, 0x8B, 0x5A, 0x9A, 0xE4, 0x82, 0x1B, 0x78, 0x00, 0x00, 0xAA, 0x96, 0x89, 0x0E, 0xE0, 0x80,
	0x2A, 0x49, 0x3D, 0xBA, 0x14, 0xA0, 0xAC, 0xC5, 0x00, 0x00, 0x51, 0xBB, 0x9C, 0x4E, 0x7B, 0xFF,
	0xF4, 0xFD, 0x57, 0x32, 0x37, 0xD9, 0x42, 0x22, 0x00, 0x00, 0x5B, 0x3C, 0x9F, 0x1B, 0x87, 0x9A,
	0x6F, 0x27, 0xAF, 0x7B, 0xE5, 0x68, 0x0A, 0xD9, 0x00, 0x00, 0x9A, 0xC5, 0x9C, 0x4E, 0x7B, 0xFF,
	0xEA, 0x21, 0x78, 0x4F, 0xDD, 0xED, 0x24, 0x14, 0x00, 0x00, 0x77, 0xB1, 0xD1, 0x36, 0xC1, 0x67,
	0x52, 0x57, 0x46, 0x3D, 0x59, 0xF4, 0x87, 0xA4, 0x00, 0x00, 0x7E, 0x44, 0x00, 0x4E, 0x7B, 0xFF,
	0x75, 0xF5, 0x06, 0x97, 0x10, 0xC3, 0x24, 0xBB, 0x00, 0x00, 0x7B, 0x7A, 0xE0, 0x60, 0x12, 0x0F,
	0xF7, 0x74, 0x1C, 0xE5, 0x39, 0x3D, 0x73, 0xC1, 0x00, 0x00, 0x7A, 0xB3, 0xFF, 0x4E, 0x7B, 0xFF
};

// if ( io < -32768 ) io = -32768;
// if ( io >  32767 ) io =  32767;
template<typename T> static inline void CLAMP16(T &io)
{
	if (static_cast<int16_t>(io) != io)
		io = (io >> 31) ^ 0x7FFF;
}

void SPC_DSP::set_output(sample_t *out, int size)
{
	assert(!(size & 1)); // must be even
	if (!out)
	{
		out = this->m.extra;
		size = extra_size;
	}
	this->m.out_begin = this->m.out = out;
	this->m.out_end = out + size;
}

// Volume registers and efb are signed! Easy to forget int8_t cast.
// Prefixes are to avoid accidental use of locals with same names.

// Gaussian interpolation

static const short gauss[] =
{
	   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   2,   2,   2,   2,   2,
	   2,   2,   3,   3,   3,   3,   3,   4,   4,   4,   4,   4,   5,   5,   5,   5,
	   6,   6,   6,   6,   7,   7,   7,   8,   8,   8,   9,   9,   9,  10,  10,  10,
	  11,  11,  11,  12,  12,  13,  13,  14,  14,  15,  15,  15,  16,  16,  17,  17,
	  18,  19,  19,  20,  20,  21,  21,  22,  23,  23,  24,  24,  25,  26,  27,  27,
	  28,  29,  29,  30,  31,  32,  32,  33,  34,  35,  36,  36,  37,  38,  39,  40,
	  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,
	  58,  59,  60,  61,  62,  64,  65,  66,  67,  69,  70,  71,  73,  74,  76,  77,
	  78,  80,  81,  83,  84,  86,  87,  89,  90,  92,  94,  95,  97,  99, 100, 102,
	 104, 106, 107, 109, 111, 113, 115, 117, 118, 120, 122, 124, 126, 128, 130, 132,
	 134, 137, 139, 141, 143, 145, 147, 150, 152, 154, 156, 159, 161, 163, 166, 168,
	 171, 173, 175, 178, 180, 183, 186, 188, 191, 193, 196, 199, 201, 204, 207, 210,
	 212, 215, 218, 221, 224, 227, 230, 233, 236, 239, 242, 245, 248, 251, 254, 257,
	 260, 263, 267, 270, 273, 276, 280, 283, 286, 290, 293, 297, 300, 304, 307, 311,
	 314, 318, 321, 325, 328, 332, 336, 339, 343, 347, 351, 354, 358, 362, 366, 370,
	 374, 378, 381, 385, 389, 393, 397, 401, 405, 410, 414, 418, 422, 426, 430, 434,
	 439, 443, 447, 451, 456, 460, 464, 469, 473, 477, 482, 486, 491, 495, 499, 504,
	 508, 513, 517, 522, 527, 531, 536, 540, 545, 550, 554, 559, 563, 568, 573, 577,
	 582, 587, 592, 596, 601, 606, 611, 615, 620, 625, 630, 635, 640, 644, 649, 654,
	 659, 664, 669, 674, 678, 683, 688, 693, 698, 703, 708, 713, 718, 723, 728, 732,
	 737, 742, 747, 752, 757, 762, 767, 772, 777, 782, 787, 792, 797, 802, 806, 811,
	 816, 821, 826, 831, 836, 841, 846, 851, 855, 860, 865, 870, 875, 880, 884, 889,
	 894, 899, 904, 908, 913, 918, 923, 927, 932, 937, 941, 946, 951, 955, 960, 965,
	 969, 974, 978, 983, 988, 992, 997,1001,1005,1010,1014,1019,1023,1027,1032,1036,
	1040,1045,1049,1053,1057,1061,1066,1070,1074,1078,1082,1086,1090,1094,1098,1102,
	1106,1109,1113,1117,1121,1125,1128,1132,1136,1139,1143,1146,1150,1153,1157,1160,
	1164,1167,1170,1174,1177,1180,1183,1186,1190,1193,1196,1199,1202,1205,1207,1210,
	1213,1216,1219,1221,1224,1227,1229,1232,1234,1237,1239,1241,1244,1246,1248,1251,
	1253,1255,1257,1259,1261,1263,1265,1267,1269,1270,1272,1274,1275,1277,1279,1280,
	1282,1283,1284,1286,1287,1288,1290,1291,1292,1293,1294,1295,1296,1297,1297,1298,
	1299,1300,1300,1301,1302,1302,1303,1303,1303,1304,1304,1304,1304,1304,1305,1305,
};

static short const cubic[514] =
{
	0, -4, -8, -12, -16, -20, -23, -27, -30, -34, -37, -41, -44, -47, -50, -53,
	-56, -59, -62, -65, -68, -71, -73, -76, -78, -81, -84, -87, -89, -91, -93, -95,
	-98, -100, -102, -104, -106, -109, -110, -112, -113, -116, -117, -119, -121, -122, -123, -125,
	-126, -128, -129, -131, -132, -134, -134, -136, -136, -138, -138, -140, -141, -141, -142, -143,
	-144, -144, -145, -146, -147, -148, -147, -148, -148, -149, -149, -150, -150, -150, -150, -151,
	-151, -151, -151, -151, -152, -152, -151, -152, -151, -152, -151, -151, -151, -151, -150, -150,
	-150, -149, -149, -149, -149, -148, -147, -147, -146, -146, -145, -145, -144, -144, -143, -142,
	-141, -141, -140, -139, -139, -138, -137, -136, -135, -135, -133, -133, -132, -131, -130, -129,
	-128, -127, -126, -125, -124, -123, -121, -121, -119, -118, -117, -116, -115, -114, -112, -111,
	-110, -109, -107, -106, -105, -104, -102, -102, -100, -99, -97, -97, -95, -94, -92, -91,
	-90, -88, -87, -86, -85, -84, -82, -81, -79, -78, -76, -76, -74, -73, -71, -70,
	-68, -67, -66, -65, -63, -62, -60, -60, -58, -57, -55, -55, -53, -52, -50, -49,
	-48, -46, -45, -44, -43, -42, -40, -39, -38, -37, -36, -35, -34, -32, -31, -30,
	-29, -28, -27, -26, -25, -24, -23, -22, -21, -20, -19, -19, -17, -16, -15, -14,
	-14, -13, -12, -11, -11, -10, -9, -9, -8, -8, -7, -7, -6, -5, -4, -4,
	-3, -3, -3, -2, -2, -2, -1, -1, 0, -1, 0, -1, 0, 0, 0, 0,
	0,
	2048, 2048, 2048, 2048, 2047, 2047, 2046, 2045, 2043, 2042, 2041, 2039, 2037, 2035, 2033, 2031,
	2028, 2026, 2024, 2021, 2018, 2015, 2012, 2009, 2005, 2002, 1999, 1995, 1991, 1987, 1982, 1978,
	1974, 1969, 1965, 1960, 1955, 1951, 1946, 1940, 1934, 1929, 1924, 1918, 1912, 1906, 1900, 1895,
	1888, 1882, 1875, 1869, 1862, 1856, 1849, 1842, 1835, 1828, 1821, 1814, 1806, 1799, 1791, 1783,
	1776, 1768, 1760, 1753, 1744, 1737, 1728, 1720, 1711, 1703, 1695, 1686, 1677, 1668, 1659, 1651,
	1641, 1633, 1623, 1614, 1605, 1596, 1587, 1577, 1567, 1559, 1549, 1539, 1529, 1520, 1510, 1499,
	1490, 1480, 1470, 1460, 1450, 1440, 1430, 1420, 1408, 1398, 1389, 1378, 1367, 1357, 1346, 1336,
	1325, 1315, 1304, 1293, 1282, 1272, 1261, 1250, 1239, 1229, 1218, 1207, 1196, 1185, 1174, 1163,
	1152, 1141, 1130, 1119, 1108, 1097, 1086, 1075, 1063, 1052, 1042, 1030, 1019, 1008, 997, 986,
	974, 964, 952, 941, 930, 919, 908, 897, 886, 875, 864, 853, 842, 831, 820, 809,
	798, 787, 776, 765, 754, 744, 733, 722, 711, 700, 690, 679, 668, 658, 647, 637,
	626, 616, 605, 595, 584, 574, 564, 554, 543, 534, 524, 514, 503, 494, 483, 473,
	464, 454, 444, 435, 425, 416, 407, 397, 387, 378, 370, 360, 351, 342, 333, 325,
	315, 307, 298, 290, 281, 273, 265, 256, 248, 241, 233, 225, 216, 209, 201, 193,
	186, 178, 171, 164, 157, 150, 143, 137, 129, 123, 117, 110, 103, 97, 91, 85,
	79, 74, 68, 62, 56, 51, 46, 41, 35, 31, 27, 22, 17, 13, 8, 4,
	0
};

static short const sinc[2048] =
{
	39, -315, 666, 15642, 666, -315, 39, -38,
	38, -302, 613, 15642, 718, -328, 41, -38,
	36, -288, 561, 15641, 772, -342, 42, -38,
	35, -275, 510, 15639, 826, -355, 44, -38,
	33, -263, 459, 15636, 880, -369, 46, -38,
	32, -250, 408, 15632, 935, -383, 47, -38,
	31, -237, 358, 15628, 990, -396, 49, -38,
	29, -224, 309, 15622, 1046, -410, 51, -38,
	28, -212, 259, 15616, 1103, -425, 53, -38,
	27, -200, 211, 15609, 1159, -439, 54, -38,
	25, -188, 163, 15601, 1216, -453, 56, -38,
	24, -175, 115, 15593, 1274, -467, 58, -38,
	23, -164, 68, 15583, 1332, -482, 60, -38,
	22, -152, 22, 15573, 1391, -496, 62, -37,
	21, -140, -24, 15562, 1450, -511, 64, -37,
	19, -128, -70, 15550, 1509, -526, 66, -37,
	18, -117, -115, 15538, 1569, -540, 68, -37,
	17, -106, -159, 15524, 1629, -555, 70, -37,
	16, -94, -203, 15510, 1690, -570, 72, -36,
	15, -83, -247, 15495, 1751, -585, 74, -36,
	14, -72, -289, 15479, 1813, -600, 76, -36,
	13, -62, -332, 15462, 1875, -616, 79, -36,
	12, -51, -374, 15445, 1937, -631, 81, -35,
	11, -40, -415, 15426, 2000, -646, 83, -35,
	11, -30, -456, 15407, 2063, -662, 85, -35,
	10, -20, -496, 15387, 2127, -677, 88, -34,
	9, -9, -536, 15366, 2191, -693, 90, -34,
	8, 1, -576, 15345, 2256, -708, 92, -34,
	7, 10, -614, 15323, 2321, -724, 95, -33,
	7, 20, -653, 15300, 2386, -740, 97, -33,
	6, 30, -690, 15276, 2451, -755, 99, -33,
	5, 39, -728, 15251, 2517, -771, 102, -32,
	5, 49, -764, 15226, 2584, -787, 104, -32,
	4, 58, -801, 15200, 2651, -803, 107, -32,
	3, 67, -836, 15173, 2718, -819, 109, -31,
	3, 76, -871, 15145, 2785, -835, 112, -31,
	2, 85, -906, 15117, 2853, -851, 115, -30,
	2, 93, -940, 15087, 2921, -867, 117, -30,
	1, 102, -974, 15057, 2990, -883, 120, -29,
	1, 110, -1007, 15027, 3059, -899, 122, -29,
	0, 118, -1039, 14995, 3128, -915, 125, -29,
	0, 127, -1071, 14963, 3198, -931, 128, -28,
	-1, 135, -1103, 14930, 3268, -948, 131, -28,
	-1, 142, -1134, 14896, 3338, -964, 133, -27,
	-1, 150, -1164, 14862, 3409, -980, 136, -27,
	-2, 158, -1194, 14827, 3480, -996, 139, -26,
	-2, 165, -1224, 14791, 3551, -1013, 142, -26,
	-3, 172, -1253, 14754, 3622, -1029, 144, -25,
	-3, 179, -1281, 14717, 3694, -1045, 147, -25,
	-3, 187, -1309, 14679, 3766, -1062, 150, -24,
	-3, 193, -1337, 14640, 3839, -1078, 153, -24,
	-4, 200, -1363, 14601, 3912, -1094, 156, -23,
	-4, 207, -1390, 14561, 3985, -1110, 159, -23,
	-4, 213, -1416, 14520, 4058, -1127, 162, -22,
	-4, 220, -1441, 14479, 4131, -1143, 165, -22,
	-4, 226, -1466, 14437, 4205, -1159, 168, -22,
	-5, 232, -1490, 14394, 4279, -1175, 171, -21,
	-5, 238, -1514, 14350, 4354, -1192, 174, -21,
	-5, 244, -1537, 14306, 4428, -1208, 177, -20,
	-5, 249, -1560, 14261, 4503, -1224, 180, -20,
	-5, 255, -1583, 14216, 4578, -1240, 183, -19,
	-5, 260, -1604, 14169, 4653, -1256, 186, -19,
	-5, 265, -1626, 14123, 4729, -1272, 189, -18,
	-5, 271, -1647, 14075, 4805, -1288, 192, -18,
	-5, 276, -1667, 14027, 4881, -1304, 195, -17,
	-6, 280, -1687, 13978, 4957, -1320, 198, -17,
	-6, 285, -1706, 13929, 5033, -1336, 201, -16,
	-6, 290, -1725, 13879, 5110, -1352, 204, -16,
	-6, 294, -1744, 13829, 5186, -1368, 207, -15,
	-6, 299, -1762, 13777, 5263, -1383, 210, -15,
	-6, 303, -1779, 13726, 5340, -1399, 213, -14,
	-6, 307, -1796, 13673, 5418, -1414, 216, -14,
	-6, 311, -1813, 13620, 5495, -1430, 219, -13,
	-5, 315, -1829, 13567, 5573, -1445, 222, -13,
	-5, 319, -1844, 13512, 5651, -1461, 225, -13,
	-5, 322, -1859, 13458, 5728, -1476, 229, -12,
	-5, 326, -1874, 13402, 5806, -1491, 232, -12,
	-5, 329, -1888, 13347, 5885, -1506, 235, -11,
	-5, 332, -1902, 13290, 5963, -1521, 238, -11,
	-5, 335, -1915, 13233, 6041, -1536, 241, -10,
	-5, 338, -1928, 13176, 6120, -1551, 244, -10,
	-5, 341, -1940, 13118, 6199, -1566, 247, -10,
	-5, 344, -1952, 13059, 6277, -1580, 250, -9,
	-5, 347, -1964, 13000, 6356, -1595, 253, -9,
	-5, 349, -1975, 12940, 6435, -1609, 256, -8,
	-4, 352, -1986, 12880, 6514, -1623, 259, -8,
	-4, 354, -1996, 12819, 6594, -1637, 262, -8,
	-4, 356, -2005, 12758, 6673, -1651, 265, -7,
	-4, 358, -2015, 12696, 6752, -1665, 268, -7,
	-4, 360, -2024, 12634, 6831, -1679, 271, -7,
	-4, 362, -2032, 12572, 6911, -1693, 274, -6,
	-4, 364, -2040, 12509, 6990, -1706, 277, -6,
	-4, 366, -2048, 12445, 7070, -1719, 280, -6,
	-3, 367, -2055, 12381, 7149, -1732, 283, -5,
	-3, 369, -2062, 12316, 7229, -1745, 286, -5,
	-3, 370, -2068, 12251, 7308, -1758, 289, -5,
	-3, 371, -2074, 12186, 7388, -1771, 291, -4,
	-3, 372, -2079, 12120, 7467, -1784, 294, -4,
	-3, 373, -2084, 12054, 7547, -1796, 297, -4,
	-3, 374, -2089, 11987, 7626, -1808, 300, -4,
	-2, 375, -2094, 11920, 7706, -1820, 303, -3,
	-2, 376, -2098, 11852, 7785, -1832, 305, -3,
	-2, 376, -2101, 11785, 7865, -1844, 308, -3,
	-2, 377, -2104, 11716, 7944, -1855, 311, -3,
	-2, 377, -2107, 11647, 8024, -1866, 313, -2,
	-2, 378, -2110, 11578, 8103, -1877, 316, -2,
	-2, 378, -2112, 11509, 8182, -1888, 318, -2,
	-1, 378, -2113, 11439, 8262, -1899, 321, -2,
	-1, 378, -2115, 11369, 8341, -1909, 323, -2,
	-1, 378, -2116, 11298, 8420, -1920, 326, -2,
	-1, 378, -2116, 11227, 8499, -1930, 328, -1,
	-1, 378, -2116, 11156, 8578, -1940, 331, -1,
	-1, 378, -2116, 11084, 8656, -1949, 333, -1,
	-1, 377, -2116, 11012, 8735, -1959, 335, -1,
	-1, 377, -2115, 10940, 8814, -1968, 337, -1,
	-1, 377, -2114, 10867, 8892, -1977, 340, -1,
	-1, 376, -2112, 10795, 8971, -1985, 342, -1,
	0, 375, -2111, 10721, 9049, -1994, 344, -1,
	0, 375, -2108, 10648, 9127, -2002, 346, 0,
	0, 374, -2106, 10574, 9205, -2010, 348, 0,
	0, 373, -2103, 10500, 9283, -2018, 350, 0,
	0, 372, -2100, 10426, 9360, -2025, 352, 0,
	0, 371, -2097, 10351, 9438, -2032, 354, 0,
	0, 370, -2093, 10276, 9515, -2039, 355, 0,
	0, 369, -2089, 10201, 9592, -2046, 357, 0,
	0, 367, -2084, 10126, 9669, -2052, 359, 0,
	0, 366, -2080, 10050, 9745, -2058, 360, 0,
	0, 365, -2075, 9974, 9822, -2064, 362, 0,
	0, 363, -2070, 9898, 9898, -2070, 363, 0,
	0, 362, -2064, 9822, 9974, -2075, 365, 0,
	0, 360, -2058, 9745, 10050, -2080, 366, 0,
	0, 359, -2052, 9669, 10126, -2084, 367, 0,
	0, 357, -2046, 9592, 10201, -2089, 369, 0,
	0, 355, -2039, 9515, 10276, -2093, 370, 0,
	0, 354, -2032, 9438, 10351, -2097, 371, 0,
	0, 352, -2025, 9360, 10426, -2100, 372, 0,
	0, 350, -2018, 9283, 10500, -2103, 373, 0,
	0, 348, -2010, 9205, 10574, -2106, 374, 0,
	0, 346, -2002, 9127, 10648, -2108, 375, 0,
	-1, 344, -1994, 9049, 10721, -2111, 375, 0,
	-1, 342, -1985, 8971, 10795, -2112, 376, -1,
	-1, 340, -1977, 8892, 10867, -2114, 377, -1,
	-1, 337, -1968, 8814, 10940, -2115, 377, -1,
	-1, 335, -1959, 8735, 11012, -2116, 377, -1,
	-1, 333, -1949, 8656, 11084, -2116, 378, -1,
	-1, 331, -1940, 8578, 11156, -2116, 378, -1,
	-1, 328, -1930, 8499, 11227, -2116, 378, -1,
	-2, 326, -1920, 8420, 11298, -2116, 378, -1,
	-2, 323, -1909, 8341, 11369, -2115, 378, -1,
	-2, 321, -1899, 8262, 11439, -2113, 378, -1,
	-2, 318, -1888, 8182, 11509, -2112, 378, -2,
	-2, 316, -1877, 8103, 11578, -2110, 378, -2,
	-2, 313, -1866, 8024, 11647, -2107, 377, -2,
	-3, 311, -1855, 7944, 11716, -2104, 377, -2,
	-3, 308, -1844, 7865, 11785, -2101, 376, -2,
	-3, 305, -1832, 7785, 11852, -2098, 376, -2,
	-3, 303, -1820, 7706, 11920, -2094, 375, -2,
	-4, 300, -1808, 7626, 11987, -2089, 374, -3,
	-4, 297, -1796, 7547, 12054, -2084, 373, -3,
	-4, 294, -1784, 7467, 12120, -2079, 372, -3,
	-4, 291, -1771, 7388, 12186, -2074, 371, -3,
	-5, 289, -1758, 7308, 12251, -2068, 370, -3,
	-5, 286, -1745, 7229, 12316, -2062, 369, -3,
	-5, 283, -1732, 7149, 12381, -2055, 367, -3,
	-6, 280, -1719, 7070, 12445, -2048, 366, -4,
	-6, 277, -1706, 6990, 12509, -2040, 364, -4,
	-6, 274, -1693, 6911, 12572, -2032, 362, -4,
	-7, 271, -1679, 6831, 12634, -2024, 360, -4,
	-7, 268, -1665, 6752, 12696, -2015, 358, -4,
	-7, 265, -1651, 6673, 12758, -2005, 356, -4,
	-8, 262, -1637, 6594, 12819, -1996, 354, -4,
	-8, 259, -1623, 6514, 12880, -1986, 352, -4,
	-8, 256, -1609, 6435, 12940, -1975, 349, -5,
	-9, 253, -1595, 6356, 13000, -1964, 347, -5,
	-9, 250, -1580, 6277, 13059, -1952, 344, -5,
	-10, 247, -1566, 6199, 13118, -1940, 341, -5,
	-10, 244, -1551, 6120, 13176, -1928, 338, -5,
	-10, 241, -1536, 6041, 13233, -1915, 335, -5,
	-11, 238, -1521, 5963, 13290, -1902, 332, -5,
	-11, 235, -1506, 5885, 13347, -1888, 329, -5,
	-12, 232, -1491, 5806, 13402, -1874, 326, -5,
	-12, 229, -1476, 5728, 13458, -1859, 322, -5,
	-13, 225, -1461, 5651, 13512, -1844, 319, -5,
	-13, 222, -1445, 5573, 13567, -1829, 315, -5,
	-13, 219, -1430, 5495, 13620, -1813, 311, -6,
	-14, 216, -1414, 5418, 13673, -1796, 307, -6,
	-14, 213, -1399, 5340, 13726, -1779, 303, -6,
	-15, 210, -1383, 5263, 13777, -1762, 299, -6,
	-15, 207, -1368, 5186, 13829, -1744, 294, -6,
	-16, 204, -1352, 5110, 13879, -1725, 290, -6,
	-16, 201, -1336, 5033, 13929, -1706, 285, -6,
	-17, 198, -1320, 4957, 13978, -1687, 280, -6,
	-17, 195, -1304, 4881, 14027, -1667, 276, -5,
	-18, 192, -1288, 4805, 14075, -1647, 271, -5,
	-18, 189, -1272, 4729, 14123, -1626, 265, -5,
	-19, 186, -1256, 4653, 14169, -1604, 260, -5,
	-19, 183, -1240, 4578, 14216, -1583, 255, -5,
	-20, 180, -1224, 4503, 14261, -1560, 249, -5,
	-20, 177, -1208, 4428, 14306, -1537, 244, -5,
	-21, 174, -1192, 4354, 14350, -1514, 238, -5,
	-21, 171, -1175, 4279, 14394, -1490, 232, -5,
	-22, 168, -1159, 4205, 14437, -1466, 226, -4,
	-22, 165, -1143, 4131, 14479, -1441, 220, -4,
	-22, 162, -1127, 4058, 14520, -1416, 213, -4,
	-23, 159, -1110, 3985, 14561, -1390, 207, -4,
	-23, 156, -1094, 3912, 14601, -1363, 200, -4,
	-24, 153, -1078, 3839, 14640, -1337, 193, -3,
	-24, 150, -1062, 3766, 14679, -1309, 187, -3,
	-25, 147, -1045, 3694, 14717, -1281, 179, -3,
	-25, 144, -1029, 3622, 14754, -1253, 172, -3,
	-26, 142, -1013, 3551, 14791, -1224, 165, -2,
	-26, 139, -996, 3480, 14827, -1194, 158, -2,
	-27, 136, -980, 3409, 14862, -1164, 150, -1,
	-27, 133, -964, 3338, 14896, -1134, 142, -1,
	-28, 131, -948, 3268, 14930, -1103, 135, -1,
	-28, 128, -931, 3198, 14963, -1071, 127, 0,
	-29, 125, -915, 3128, 14995, -1039, 118, 0,
	-29, 122, -899, 3059, 15027, -1007, 110, 1,
	-29, 120, -883, 2990, 15057, -974, 102, 1,
	-30, 117, -867, 2921, 15087, -940, 93, 2,
	-30, 115, -851, 2853, 15117, -906, 85, 2,
	-31, 112, -835, 2785, 15145, -871, 76, 3,
	-31, 109, -819, 2718, 15173, -836, 67, 3,
	-32, 107, -803, 2651, 15200, -801, 58, 4,
	-32, 104, -787, 2584, 15226, -764, 49, 5,
	-32, 102, -771, 2517, 15251, -728, 39, 5,
	-33, 99, -755, 2451, 15276, -690, 30, 6,
	-33, 97, -740, 2386, 15300, -653, 20, 7,
	-33, 95, -724, 2321, 15323, -614, 10, 7,
	-34, 92, -708, 2256, 15345, -576, 1, 8,
	-34, 90, -693, 2191, 15366, -536, -9, 9,
	-34, 88, -677, 2127, 15387, -496, -20, 10,
	-35, 85, -662, 2063, 15407, -456, -30, 11,
	-35, 83, -646, 2000, 15426, -415, -40, 11,
	-35, 81, -631, 1937, 15445, -374, -51, 12,
	-36, 79, -616, 1875, 15462, -332, -62, 13,
	-36, 76, -600, 1813, 15479, -289, -72, 14,
	-36, 74, -585, 1751, 15495, -247, -83, 15,
	-36, 72, -570, 1690, 15510, -203, -94, 16,
	-37, 70, -555, 1629, 15524, -159, -106, 17,
	-37, 68, -540, 1569, 15538, -115, -117, 18,
	-37, 66, -526, 1509, 15550, -70, -128, 19,
	-37, 64, -511, 1450, 15562, -24, -140, 21,
	-37, 62, -496, 1391, 15573, 22, -152, 22,
	-38, 60, -482, 1332, 15583, 68, -164, 23,
	-38, 58, -467, 1274, 15593, 115, -175, 24,
	-38, 56, -453, 1216, 15601, 163, -188, 25,
	-38, 54, -439, 1159, 15609, 211, -200, 27,
	-38, 53, -425, 1103, 15616, 259, -212, 28,
	-38, 51, -410, 1046, 15622, 309, -224, 29,
	-38, 49, -396, 990, 15628, 358, -237, 31,
	-38, 47, -383, 935, 15632, 408, -250, 32,
	-38, 46, -369, 880, 15636, 459, -263, 33,
	-38, 44, -355, 826, 15639, 510, -275, 35,
	-38, 42, -342, 772, 15641, 561, -288, 36,
	-38, 41, -328, 718, 15642, 613, -302, 38,
};

int SPC_DSP::interpolate(const voice_t *v)
{
	// Make pointers into gaussian based on fractional position between samples
	int offset = (v->interp_pos >> 4) & 0xFF;
	auto fwd = gauss + 255 - offset;
	auto rev = gauss + offset; // mirror left half of gaussian

	auto in = &v->buf[(v->interp_pos >> 12) + v->buf_pos];
	int out = (fwd[0] * in[0]) >> 11;
	out += (fwd[256] * in[1]) >> 11;
	out += (rev[256] * in[2]) >> 11;
	out = static_cast<int16_t>(out);
	out += (rev[0] * in[3]) >> 11;

	CLAMP16(out);
	out &= ~1;
	return out;
}

inline int SPC_DSP::interpolate_cubic(voice_t const* v)
{
	// Make pointers into cubic based on fractional position between samples
	int offset = v->interp_pos >> 4 & 0xFF;
	short const* fwd = cubic + offset;
	short const* rev = cubic + 256 - offset; // mirror left half of cubic

	int const* in = &v->buf[(v->interp_pos >> 12) + v->buf_pos];
	int out;
	out = fwd[0] * in[0];
	out += fwd[257] * in[1];
	out += rev[257] * in[2];
	out += rev[0] * in[3];
	out >>= 11;

	CLAMP16(out);
	out &= ~1;
	return out;
}

inline int SPC_DSP::interpolate_sinc(voice_t const* v)
{
	// Make pointers into cubic based on fractional position between samples
	int offset = v->interp_pos & 0xFF0;
	short const* filt = (short const*)(((char const*)sinc) + offset);

	int const* in = &v->buf[(v->interp_pos >> 12) + v->buf_pos];
	int out;
	out = filt[0] * in[0];
	out += filt[1] * in[1];
	out += filt[2] * in[2];
	out += filt[3] * in[3];
	out += filt[4] * in[4];
	out += filt[5] * in[5];
	out += filt[6] * in[6];
	out += filt[7] * in[7];
	out >>= 14;

	CLAMP16(out);
	out &= ~1;
	return out;
}

inline int SPC_DSP::interpolate_linear(voice_t const* v)
{
	int fract = v->interp_pos & 0xFFF;

	int const* in = &v->buf[(v->interp_pos >> 12) + v->buf_pos];
	int out;
	out = (0x1000 - fract) * in[0];
	out += fract  * in[1];
	out >>= 12;

	// no need to clamp
	out &= ~1;
	return out;
}

inline int SPC_DSP::interpolate_nearest(voice_t const* v)
{
	return v->buf[(v->interp_pos >> 12) + v->buf_pos] & ~1;
}


//// Counters

static const int simple_counter_range = 2048 * 5 * 3; // 30720

static const unsigned counter_rates[] =
{
	simple_counter_range + 1, // never fires
	2048, 1536,
	1280, 1024, 768,
	640, 512, 384,
	320, 256, 192,
	160, 128, 96,
	80, 64, 48,
	40, 32, 24,
	20, 16, 12,
	10, 8, 6,
	5, 4, 3,
	2,
	1
};

static const unsigned counter_offsets[] =
{
	1, 0, 1040,
	536, 0, 1040,
	536, 0, 1040,
	536, 0, 1040,
	536, 0, 1040,
	536, 0, 1040,
	536, 0, 1040,
	536, 0, 1040,
	536, 0, 1040,
	536, 0, 1040,
	0,
	0
};

void SPC_DSP::init_counter()
{
	this->m.counter = 0;
}

void SPC_DSP::run_counters()
{
	if (--this->m.counter < 0)
		this->m.counter = simple_counter_range - 1;
}

unsigned SPC_DSP::read_counter(int rate)
{
	return (static_cast<unsigned>(this->m.counter) + counter_offsets[rate]) % counter_rates[rate];
}

//// Envelope

void SPC_DSP::run_envelope(voice_t *const v)
{
	int env = v->env;
	if (v->env_mode == env_release) // 60%
	{
		if ((env -= 0x8) < 0)
			env = 0;
		v->env = env;
	}
	else
	{
		int rate;
		int env_data = v->regs[v_adsr1];
		if (this->m.t_adsr0 & 0x80) // 99% ADSR
		{
			if (v->env_mode >= env_decay) // 99%
			{
				--env;
				env -= env >> 8;
				rate = env_data & 0x1F;
				if (v->env_mode == env_decay) // 1%
					rate = ((this->m.t_adsr0 >> 3) & 0x0E) + 0x10;
			}
			else // env_attack
			{
				rate = (this->m.t_adsr0 & 0x0F) * 2 + 1;
				env += rate < 31 ? 0x20 : 0x400;
			}
		}
		else // GAIN
		{
			env_data = v->regs[v_gain];
			int mode = env_data >> 5;
			if (mode < 4) // direct
			{
				env = env_data * 0x10;
				rate = 31;
			}
			else
			{
				rate = env_data & 0x1F;
				if (mode == 4) // 4: linear decrease
					env -= 0x20;
				else if (mode < 6) // 5: exponential decrease
				{
					--env;
					env -= env >> 8;
				}
				else // 6,7: linear increase
				{
					env += 0x20;
					if (mode > 6 && static_cast<unsigned>(v->hidden_env) >= 0x600)
						env += 0x8 - 0x20; // 7: two-slope linear increase
				}
			}
		}

		// Sustain level
		if ((env >> 8) == (env_data >> 5) && v->env_mode == env_decay)
			v->env_mode = env_sustain;

		v->hidden_env = env;

		// unsigned cast because linear decrease going negative also triggers this
		if (static_cast<unsigned>(env) > 0x7FF)
		{
			env = env < 0 ? 0 : 0x7FF;
			if (v->env_mode == env_attack)
				v->env_mode = env_decay;
		}

		if (!this->read_counter(rate))
			v->env = env; // nothing else is controlled by the counter
	}
}

//// BRR Decoding

void SPC_DSP::decode_brr(voice_t *v)
{
	// Arrange the four input nybbles in 0xABCD order for easy decoding
	int nybbles = this->m.t_brr_byte * 0x100 + this->m.ram[(v->brr_addr + v->brr_offset + 1) & 0xFFFF];

	int header = this->m.t_brr_header;

	// Write to next four samples in circular buffer
	int *pos = &v->buf[v->buf_pos];
	int *end;
	if ((v->buf_pos += 4) >= brr_buf_size)
		v->buf_pos = 0;

	// Decode four samples
	for (end = pos + 4; pos < end; ++pos, nybbles <<= 4)
	{
		// Extract nybble and sign-extend
		int s = static_cast<int16_t>(nybbles) >> 12;

		// Shift sample based on header
		int shift = header >> 4;
		s = (s << shift) >> 1;
		if (shift >= 0xD) // handle invalid range
			s = (s >> 25) << 11; // same as: s = (s < 0 ? -0x800 : 0)

		// Apply IIR filter (8 is the most commonly used)
		int filter = header & 0x0C;
		int p1 = pos[brr_buf_size - 1];
		int p2 = pos[brr_buf_size - 2] >> 1;
		if (filter >= 8)
		{
			s += p1;
			s -= p2;
			if (filter == 8) // s += p1 * 0.953125 - p2 * 0.46875
			{
				s += p2 >> 4;
				s += (p1 * -3) >> 6;
			}
			else // s += p1 * 0.8984375 - p2 * 0.40625
			{
				s += (p1 * -13) >> 7;
				s += (p2 * 3) >> 4;
			}
		}
		else if (filter) // s += p1 * 0.46875
		{
			s += p1 >> 1;
			s += -p1 >> 5;
		}

		// Adjust and write sample
		CLAMP16(s);
		s = static_cast<int16_t>(s * 2);
		pos[brr_buf_size] = pos[0] = s; // second copy simplifies wrap-around
	}
}

//// Misc

void SPC_DSP::misc_27()
{
	this->m.t_pmon = this->m.regs[r_pmon] & 0xFE; // voice 0 doesn't support PMON
}
void SPC_DSP::misc_28()
{
	this->m.t_non = this->m.regs[r_non];
	this->m.t_eon = this->m.regs[r_eon];
	this->m.t_dir = this->m.regs[r_dir];
}
void SPC_DSP::misc_29()
{
	this->m.every_other_sample = !this->m.every_other_sample;
	if (this->m.every_other_sample)
		this->m.new_kon &= ~this->m.kon; // clears KON 63 clocks after it was last read
}
void SPC_DSP::misc_30()
{
	if (this->m.every_other_sample)
	{
		this->m.kon = this->m.new_kon;
		this->m.t_koff = this->m.regs[r_koff] | this->m.mute_mask;
	}

	this->run_counters();

	// Noise
	if (!this->read_counter(this->m.regs[r_flg] & 0x1F))
	{
		int feedback = (this->m.noise << 13) ^ (this->m.noise << 14);
		this->m.noise = (feedback & 0x4000) ^ (this->m.noise >> 1);
	}
}

//// Voices

void SPC_DSP::voice_V1(voice_t *const v)
{
	this->m.t_dir_addr = this->m.t_dir * 0x100 + this->m.t_srcn * 4;
	this->m.t_srcn = v->regs[v_srcn];
}
void SPC_DSP::voice_V2(voice_t *const v)
{
	// Read sample pointer (ignored if not needed)
	auto entry = &this->m.ram[m.t_dir_addr];
	if (!v->kon_delay)
		entry += 2;
	this->m.t_brr_next_addr = get_le16(entry);

	this->m.t_adsr0 = v->regs[v_adsr0];

	// Read pitch, spread over two clocks
	this->m.t_pitch = v->regs[v_pitchl];
}
void SPC_DSP::voice_V3a(voice_t *const v)
{
	this->m.t_pitch += (v->regs[v_pitchh] & 0x3F) << 8;
}
void SPC_DSP::voice_V3b(voice_t *const v)
{
	// Read BRR header and byte
	this->m.t_brr_byte = this->m.ram[(v->brr_addr + v->brr_offset) & 0xFFFF];
	this->m.t_brr_header = this->m.ram[v->brr_addr]; // brr_addr doesn't need masking
}
void SPC_DSP::voice_V3c(voice_t *const v)
{
	// Pitch modulation using previous voice's output
	if (this->m.t_pmon & v->vbit)
		this->m.t_pitch += ((this->m.t_output >> 5) * this->m.t_pitch) >> 10;

	if (v->kon_delay)
	{
		// Get ready to start BRR decoding on next sample
		if (v->kon_delay == 5)
		{
			v->brr_addr = this->m.t_brr_next_addr;
			v->brr_offset = 1;
			v->buf_pos = 0;
			this->m.t_brr_header = 0; // header is ignored on this sample
		}

		// Envelope is never run during KON
		v->env = 0;
		v->hidden_env = 0;

		// Disable BRR decoding until last three samples
		v->interp_pos = 0;
		if (--v->kon_delay & 3)
			v->interp_pos = 0x4000;

		// Pitch is never added during KON
		this->m.t_pitch = 0;
	}

	// Gaussian interpolation
	{
		int output;
		switch (m.interpolation_level)
		{
		case 0:
		default:
			output = this->interpolate(v);
			break;

		case 1:
			output = this->interpolate_cubic(v);
			break;

		case 2:
			output = this->interpolate_sinc(v);
			break;

		case -1:
			output = this->interpolate_linear(v);
			break;

		case -2:
			output = this->interpolate_nearest(v);
			break;
		}

		// Noise
		if (this->m.t_non & v->vbit)
			output = static_cast<int16_t>(this->m.noise * 2);

		// Apply envelope
		this->m.t_output = (output * v->env) >> 11 & ~1;
		v->t_envx_out = static_cast<uint8_t>(v->env >> 4);
	}

	// Immediate silence due to end of sample or soft reset
	if (this->m.regs[r_flg] & 0x80 || (this->m.t_brr_header & 3) == 1)
	{
		v->env_mode = env_release;
		v->env = 0;
	}

	if (this->m.every_other_sample)
	{
		// KOFF
		if (this->m.t_koff & v->vbit)
			v->env_mode = env_release;

		// KON
		if (this->m.kon & v->vbit)
		{
			v->kon_delay = 5;
			v->env_mode  = env_attack;
		}
	}

	// Run envelope for next sample
	if (!v->kon_delay)
		this->run_envelope(v);
}

void SPC_DSP::voice_output(const voice_t *v, int ch)
{
	// Check surround removal
	int vol = (int8_t)v->regs[v_voll + ch];
	int voln = (int8_t)v->regs[v_voll + ch ^ 1];
	if (vol * voln < m.surround_threshold)
		vol ^= vol >> 7;

	// Apply left/right volume
	int amp = (this->m.t_output * vol) >> 7;
	amp *= (this->stereo_switch & (1 << (v->voice_number + ch * voice_count))) ? 1 : 0;

	// Add to output total
	this->m.t_main_out[ch] += amp;
	CLAMP16(this->m.t_main_out[ch]);

	// Optionally add to echo total
	if (this->m.t_eon & v->vbit)
	{
		this->m.t_echo_out[ch] += amp;
		CLAMP16(this->m.t_echo_out[ch]);
	}
}
void SPC_DSP::voice_V4(voice_t *const v)
{
	// Decode BRR
	this->m.t_looped = 0;
	if (v->interp_pos >= 0x4000)
	{
		this->decode_brr(v);

		if ((v->brr_offset += 2) >= brr_block_size)
		{
			// Start decoding next BRR block
			assert(v->brr_offset == brr_block_size);
			v->brr_addr = (v->brr_addr + brr_block_size) & 0xFFFF;
			if (this->m.t_brr_header & 1)
			{
				v->brr_addr = this->m.t_brr_next_addr;
				this->m.t_looped = v->vbit;
			}
			v->brr_offset = 1;
		}
	}

	// Apply pitch
	v->interp_pos = (v->interp_pos & 0x3FFF) + m.t_pitch;

	// Keep from getting too far ahead (when using pitch modulation)
	if (v->interp_pos > 0x7FFF)
		v->interp_pos = 0x7FFF;

	// Output left
	this->voice_output(v, 0);
}
void SPC_DSP::voice_V5(voice_t *const v)
{
	// Output right
	this->voice_output(v, 1);

	// ENDX, OUTX, and ENVX won't update if you wrote to them 1-2 clocks earlier
	int endx_buf = this->m.regs[r_endx] | this->m.t_looped;

	// Clear bit in ENDX if KON just began
	if (v->kon_delay == 5)
		endx_buf &= ~v->vbit;
	this->m.endx_buf = static_cast<uint8_t>(endx_buf);
}
void SPC_DSP::voice_V6(voice_t *const)
{
	this->m.outx_buf = static_cast<uint8_t>(this->m.t_output >> 8);
}
void SPC_DSP::voice_V7(voice_t *const v)
{
	// Update ENDX
	this->m.regs[r_endx] = this->m.endx_buf;

	this->m.envx_buf = v->t_envx_out;
}
void SPC_DSP::voice_V8(voice_t *const v)
{
	// Update OUTX
	v->regs[v_outx] = this->m.outx_buf;
}
void SPC_DSP::voice_V9(voice_t *const v)
{
	// Update ENVX
	v->regs[v_envx] = this->m.envx_buf;
}

// Most voices do all these in one clock, so make a handy composite
void SPC_DSP::voice_V3(voice_t *const v)
{
	this->voice_V3a(v);
	this->voice_V3b(v);
	this->voice_V3c(v);
}

// Common combinations of voice steps on different voices. This greatly reduces
// code size and allows everything to be inlined in these functions.
void SPC_DSP::voice_V7_V4_V1(voice_t *const v) { this->voice_V7(v); this->voice_V1(v + 3); this->voice_V4(v + 1); }
void SPC_DSP::voice_V8_V5_V2(voice_t *const v) { this->voice_V8(v); this->voice_V5(v + 1); this->voice_V2(v + 2); }
void SPC_DSP::voice_V9_V6_V3(voice_t *const v) { this->voice_V9(v); this->voice_V6(v + 1); this->voice_V3(v + 2); }

//// Echo

void SPC_DSP::echo_read(int ch)
{
	int s = static_cast<int16_t>(get_le16(this->ECHO_PTR(ch)));
	// second copy simplifies wrap-around handling
	this->ECHO_FIR(0)[ch] = this->ECHO_FIR(8)[ch] = s >> 1;
}

void SPC_DSP::echo_22()
{
	// History
	if (++this->m.echo_hist_pos >= &this->m.echo_hist[echo_hist_size])
		this->m.echo_hist_pos = this->m.echo_hist;

	this->m.t_echo_ptr = (this->m.t_esa * 0x100 + this->m.echo_offset) & 0xFFFF;
	this->echo_read(0);

	// FIR (using l and r temporaries below helps compiler optimize)
	int l = this->CALC_FIR(0, 0);
	int r = this->CALC_FIR(0, 1);

	this->m.t_echo_in[0] = l;
	this->m.t_echo_in[1] = r;
}
void SPC_DSP::echo_23()
{
	int l = this->CALC_FIR(1, 0) + this->CALC_FIR(2, 0);
	int r = this->CALC_FIR(1, 1) + this->CALC_FIR(2, 1);

	this->m.t_echo_in[0] += l;
	this->m.t_echo_in[1] += r;

	echo_read(1);
}
void SPC_DSP::echo_24()
{
	int l = this->CALC_FIR(3, 0) + this->CALC_FIR(4, 0) + this->CALC_FIR(5, 0);
	int r = this->CALC_FIR(3, 1) + this->CALC_FIR(4, 1) + this->CALC_FIR(5, 1);

	this->m.t_echo_in[0] += l;
	this->m.t_echo_in[1] += r;
}
void SPC_DSP::echo_25()
{
	int l = this->m.t_echo_in[0] + this->CALC_FIR(6, 0);
	int r = this->m.t_echo_in[1] + this->CALC_FIR(6, 1);

	l = static_cast<int16_t>(l);
	r = static_cast<int16_t>(r);

	l += static_cast<int16_t>(this->CALC_FIR(7, 0));
	r += static_cast<int16_t>(this->CALC_FIR(7, 1));

	CLAMP16(l);
	CLAMP16(r);

	this->m.t_echo_in[0] = l & ~1;
	this->m.t_echo_in[1] = r & ~1;
}
int SPC_DSP::echo_output(int ch)
{
	// Check surround removal
	int vol = (int8_t)m.regs[r_mvoll + ch * 0x10];
	int voln = (int8_t)m.regs[r_mvoll + ch * 0x10 ^ 0x10];
	if (vol * voln < m.surround_threshold)
		vol ^= vol >> 7;

	int out = static_cast<int16_t>((this->m.t_main_out [ch] * vol) >> 7) +
		static_cast<int16_t>((this->m.t_echo_in [ch] * static_cast<int8_t>(this->m.regs[r_evoll + ch * 0x10])) >> 7);
	CLAMP16(out);
	return out;
}
void SPC_DSP::echo_26()
{
	// Left output volumes
	// (save sample for next clock so we can output both together)
	this->m.t_main_out[0] = echo_output(0);

	// Echo feedback
	int l = this->m.t_echo_out[0] + static_cast<int16_t>((this->m.t_echo_in[0] * static_cast<int8_t>(this->m.regs[r_efb])) >> 7);
	int r = this->m.t_echo_out[1] + static_cast<int16_t>((this->m.t_echo_in[1] * static_cast<int8_t>(this->m.regs[r_efb])) >> 7);

	CLAMP16(l);
	CLAMP16(r);

	this->m.t_echo_out[0] = l & ~1;
	this->m.t_echo_out[1] = r & ~1;
}
void SPC_DSP::echo_27()
{
	// Output
	int l = this->m.t_main_out[0];
	int r = echo_output(1);
	this->m.t_main_out[0] = this->m.t_main_out[1] = 0;

	// TODO: global muting isn't this simple (turns DAC on and off
	// or something, causing small ~37-sample pulse when first muted)
	if (this->m.regs[r_flg] & 0x40)
		l = r = 0;

	// Output sample to DAC
#ifdef SPC_DSP_OUT_HOOK
	SPC_DSP_OUT_HOOK(l, r);
#else
	sample_t *out = this->m.out;
	out[0] = l;
	out[1] = r;
	out += 2;
	if (out >= m.out_end)
	{
		out = this->m.extra;
		this->m.out_end = &this->m.extra[extra_size];
	}
	this->m.out = out;
#endif
}
void SPC_DSP::echo_28()
{
	this->m.t_echo_enabled = this->m.regs[r_flg];
}
void SPC_DSP::echo_write(int ch)
{
	if (!(this->m.t_echo_enabled & 0x20))
	{
		if (this->m.t_echo_ptr >= 0xffc0 && this->rom_enabled)
			set_le16(&this->hi_ram[this->m.t_echo_ptr + ch * 2 - 0xffc0], this->m.t_echo_out [ch]);
		else
			set_le16(this->ECHO_PTR(ch), this->m.t_echo_out[ch]);
	}

	this->m.t_echo_out[ch] = 0;
}
void SPC_DSP::echo_29()
{
	this->m.t_esa = this->m.regs[r_esa];

	if (!this->m.echo_offset)
		this->m.echo_length = (this->m.regs[r_edl] & 0x0F) * 0x800;

	this->m.echo_offset += 4;
	if (this->m.echo_offset >= this->m.echo_length)
		this->m.echo_offset = 0;

	// Write left echo
	this->echo_write(0);

	this->m.t_echo_enabled = this->m.regs[r_flg];
}
void SPC_DSP::echo_30()
{
	// Write right echo
	this->echo_write(1);
}

//// Timing

// Execute clock for a particular voice
#define V(clock, voice) voice_##clock(&this->m.voices[voice]);

/* The most common sequence of clocks uses composite operations
for efficiency. For example, the following are equivalent to the
individual steps on the right:

V(V7_V4_V1,2) -> V(V7,2) V(V4,3) V(V1,5)
V(V8_V5_V2,2) -> V(V8,2) V(V5,3) V(V2,4)
V(V9_V6_V3,2) -> V(V9,2) V(V6,3) V(V3,4) */

// Voice      0      1      2      3      4      5      6      7
#define GEN_DSP_TIMING \
PHASE(0) V(V5, 0) V(V2, 1) \
PHASE(1) V(V6, 0) V(V3, 1) \
PHASE(2) V(V7_V4_V1, 0) \
PHASE(3) V(V8_V5_V2, 0) \
PHASE(4) V(V9_V6_V3, 0) \
PHASE(5) V(V7_V4_V1, 1) \
PHASE(6) V(V8_V5_V2, 1) \
PHASE(7) V(V9_V6_V3, 1) \
PHASE(8) V(V7_V4_V1, 2) \
PHASE(9) V(V8_V5_V2, 2) \
PHASE(10) V(V9_V6_V3, 2) \
PHASE(11) V(V7_V4_V1, 3) \
PHASE(12) V(V8_V5_V2, 3) \
PHASE(13) V(V9_V6_V3, 3) \
PHASE(14) V(V7_V4_V1, 4) \
PHASE(15) V(V8_V5_V2, 4) \
PHASE(16) V(V9_V6_V3, 4) \
PHASE(17) V(V1, 0) V(V7, 5) V(V4, 6) \
PHASE(18) V(V8_V5_V2, 5) \
PHASE(19) V(V9_V6_V3, 5) \
PHASE(20) V(V1, 1) V(V7, 6) V(V4, 7) \
PHASE(21) V(V8, 6) V(V5, 7) V(V2, 0) /* t_brr_next_addr order dependency */ \
PHASE(22) V(V3a, 0) V(V9, 6) V(V6, 7) echo_22(); \
PHASE(23) V(V7, 7) echo_23(); \
PHASE(24) V(V8, 7) echo_24(); \
PHASE(25) V(V3b, 0) V(V9, 7) echo_25(); \
PHASE(26) echo_26(); \
PHASE(27) misc_27(); echo_27(); \
PHASE(28) misc_28(); echo_28(); \
PHASE(29) misc_29(); echo_29(); \
PHASE(30) misc_30(); V(V3c, 0) echo_30(); \
PHASE(31) V(V4, 0) V(V1, 2)

#ifndef SPC_DSP_CUSTOM_RUN
void SPC_DSP::run(int clocks_remain)
{
	assert(clocks_remain > 0);

	int phase = this->m.phase;
	this->m.phase = (phase + clocks_remain) & 31;
	switch (phase)
	{
		loop:
#define PHASE(n) if (n && !--clocks_remain) break; case n:
		GEN_DSP_TIMING
#undef PHASE

		if (--clocks_remain)
			goto loop;
	}
}
#endif

//// Setup

void SPC_DSP::init(uint8_t *ram_64k)
{
	this->m.ram = ram_64k;
	this->mute_voices(0);
	this->disable_surround(false);
	this->interpolation_level(0);
	this->set_output(nullptr, 0);
	this->reset();

	this->stereo_switch = 0xffff;

#ifndef NDEBUG
	// be sure this sign-extends
	assert(static_cast<int16_t>(0x8000) == -0x8000);

	// be sure right shift preserves sign
	assert((-1 >> 1) == -1);

	// check clamp macro
	int i = 0x8000;
	CLAMP16(i);
	assert(i == 0x7FFF);

	i = -0x8001;
	CLAMP16(i);
	assert(i == -0x8000);

	blargg_verify_byte_order();
#endif
}

void SPC_DSP::soft_reset_common()
{
	assert(this->m.ram); // init() must have been called already

	this->m.noise = 0x4000;
	this->m.echo_hist_pos = this->m.echo_hist;
	this->m.every_other_sample = true;
	this->m.echo_offset = 0;
	this->m.phase = 0;

	this->init_counter();

	for (int i = 0; i < voice_count; ++i)
		this->m.voices[i].voice_number = i;
}

void SPC_DSP::soft_reset()
{
	this->m.regs[r_flg] = 0xE0;
	this->soft_reset_common();
}

void SPC_DSP::load(const uint8_t regs[register_count])
{
	std::copy_n(&regs[0], static_cast<int>(register_count), &this->m.regs[0]);
	memset(&this->m.regs[register_count], 0, offsetof(state_t, ram) - register_count);

	// Internal state
	for (int i = voice_count; --i >= 0;)
	{
		auto &v = this->m.voices[i];
		v.brr_offset = 1;
		v.vbit = 1 << i;
		v.regs = &this->m.regs[i * 0x10];
	}
	this->m.new_kon = this->m.regs[r_kon];
	this->m.t_dir = this->m.regs[r_dir];
	this->m.t_esa = this->m.regs[r_esa];

	this->soft_reset_common();
}

void SPC_DSP::reset()
{
	this->load(initial_regs);
}

//// Snes9x Accessor

void SPC_DSP::set_stereo_switch(int value)
{
	this->stereo_switch = value;
}

uint8_t SPC_DSP::reg_value(int ch, int addr)
{
	return this->m.voices[ch].regs[addr];
}

int SPC_DSP::envx_value(int ch)
{
	return this->m.voices[ch].env;
}
