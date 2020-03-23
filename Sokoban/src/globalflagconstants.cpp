#include "stdafx.h"
#include "globalflagconstants.h"

const unsigned int FLAG_COLLECT_FLAGS[NUM_ZONES] = {
	3471853932,
	2112462674,
	928351178,
	316878516,
	4139118543,
	1308885309,
	784932951,
	1547327771,
	3156813654,
	4223394085,
	1466062630,
	827815434,
	1795136990,
	1017736850,
	3885623896,
	1458859156,
	1730882767,
	2289975154,
	2114549908,
	1144501547,
	953129282,
	4235308581,
	2902806527,
	3786094745,
	3592504473,
	1617436400,
	2699793442,
	3018731737,
	2831894647,
	2081718090,
	592011528,
	924002580,
	1974937881,
	4022503305,
	206542323,
	1367301943,
	2721011355,
};

const unsigned int ZONE_ACCESSED_GLOBAL_FLAGS[NUM_ZONES] = {
	4026383313,
	2666641764,
	2565848306,
	1729351408,
	680022602,
	1921815784,
	1751684134,
	1109576799,
	343429394,
	1245270588,
	56671806,
	1669789591,
	4009435570,
	1025182569,
	2484447942,
	444673442,
	585395297,
	1740848846,
	1091019098,
	1929196515,
	1932675714,
	2153943421,
	630202837,
	1289730605,
	2759785338,
	4042211390,
	3027588208,
	4035088536,
	4027133132,
	3570602799,
	697508646,
	1270888974,
	6982996,
	476783426,
	3566209726,
	1490322265,
	2615727639,
};

const unsigned int HUB_ACCESSED_GLOBAL_FLAGS[5] = {
	383775262,
	3975789010,
	3433508970,
	2300076748,
	3620035186,
};

const unsigned int HUB_ALT_ACCESSED_GLOBAL_FLAGS[5] = {
	3971864520,
	2641351301,
	1187047280,
	3599405035,
	3075449329,
};

const unsigned int X_ALT_ACCESSED_GLOBAL_FLAGS[4] = {
	1677068454,
	4173185786,
	2256081595,
	683528293,
};

const unsigned int FATE_SIGNALER_CHOICE[2] = {
	2641701143,
	2202838590,
};

const unsigned int MISC_GLOBAL_FLAGS[static_cast<int>(MiscGlobalFlags::COUNT)] = {
	841293067,
	3565400062,
	2679708926,
	3263730501,
	706432533,
};


int get_clear_flag_index(char zone) {
	if ('A' <= zone && zone <= 'Z') {
		return zone - 'A';
	} else if ('0' <= zone && zone <= '9') {
		return (zone - '0') + 26;
	} else { // Zone ?
		return 36;
	}
}


unsigned int get_clear_flag_code(char zone) {
	return FLAG_COLLECT_FLAGS[get_clear_flag_index(zone)];
}


unsigned int get_zone_access_code(char zone) {
	return ZONE_ACCESSED_GLOBAL_FLAGS[get_clear_flag_index(zone)];
}


unsigned int get_misc_flag(MiscGlobalFlags flag) {
	return MISC_GLOBAL_FLAGS[static_cast<int>(flag)];
}



const unsigned int UNUSED_FLAGS[200] = {
	4223427354,
	2803065334,
	3779384409,
	1820508520,
	250185505,
	2581486075,
	2421360155,
	3041601978,
	883224518,
	2954665274,
	1995777758,
	2216583444,
	750113062,
	4189740853,
	3072501499,
	1318176511,
	1036087534,
	3337401368,
	581635234,
	3535291228,
	2832916134,
	1477806471,
	3841071354,
	389352163,
	78705892,
	204616401,
	2051041489,
	1261875265,
	2216179811,
	256538355,
	3000626525,
	3377028055,
	92626414,
	3847133239,
	1825772700,
	1853714670,
	4067583291,
	1894757742,
	4252155025,
	3502230466,
	2134229822,
	352433634,
	2601769995,
	635455287,
	1264415602,
	1271730061,
	2905788098,
	1458693894,
	1790884994,
	3025538220,
	2412843537,
	1736951203,
	1353900173,
	2532996264,
	3194324749,
	2563540298,
	3171583503,
	3400580674,
	1533785558,
	2318096548,
	2124830589,
	1267924870,
	1387135735,
	2806634795,
	3677675252,
	1270381750,
	3007097071,
	3158232801,
	4023828930,
	2796678483,
	2365341242,
	586578140,
	1004129701,
	2892766734,
	1999324947,
	1163075488,
	2845048164,
	110514219,
	1336930773,
	3575430982,
	2177135879,
	326665896,
	2402746331,
	2613859849,
	2380594567,
	1016115104,
	4157998591,
	1049231090,
	3945572550,
	2614599788,
	2736783166,
	2493088318,
	2426182960,
	4286813126,
	2959965595,
	107125447,
	2144114899,
	3541818308,
	1336526853,
	2861647527,
	146078016,
	743220397,
	1970023241,
	683264180,
	1605574229,
	2765743787,
	4093903830,
	1144566547,
	1020311092,
	3802688514,
	1059551525,
	1235912029,
	1987552232,
	3797418917,
	1568662326,
	1819705334,
	1383950640,
	3976659722,
	4083691008,
	2072355669,
	3552192469,
	3924787014,
	2693552261,
	714735387,
	1580869404,
	127127210,
	1748439801,
	1746838140,
	1195877548,
	2753200209,
	3622407705,
	2487989430,
	1178019996,
	3938882961,
	3289621489,
	3487687112,
	527796684,
	865127029,
	1968305069,
	1599547709,
	649822114,
	3766977458,
	3102866480,
	139119894,
	1425331055,
	4115243602,
	3935922081,
	1218984866,
	2064350860,
	4257475915,
	3708213175,
	2479104360,
	2926556681,
	3296732495,
	3894018414,
	1927283622,
	3274958978,
	1060627274,
	1066456776,
	3535352403,
	2090659069,
	3848792494,
	1025554630,
	611674714,
	2188306647,
	1708847532,
	1787954560,
	904251999,
	2262205627,
	3114871595,
	755098865,
	2975139128,
	4007918859,
	2714364490,
	417482611,
	3323121042,
	4080693760,
	2556127689,
	890564427,
	830657928,
	1422081053,
	1816365535,
	233989872,
	1145182192,
	1254214502,
	3959573475,
	2271173890,
	1156274123,
	3284784970,
	1009189897,
	698479602,
	2195103968,
	2036387747,
	1957095391,
	518940840,
	1725913273,
	4067679005,
	3447292363,
	2046603588,
	1591147930,
};