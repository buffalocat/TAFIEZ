#include "stdafx.h"
#include "globalflagconstants.h"

const unsigned int FLAG_COLLECT_FLAGS[36] = {
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
	if ('0' <= zone && zone <= '9') {
		return (zone - '0') + 26;
	} else {
		return zone - 'A';
	}
}


unsigned int get_clear_flag_code(char zone) {
	return FLAG_COLLECT_FLAGS[get_clear_flag_index(zone)];
}

unsigned int get_misc_flag(MiscGlobalFlags flag) {
	return MISC_GLOBAL_FLAGS[static_cast<int>(flag)];
}
