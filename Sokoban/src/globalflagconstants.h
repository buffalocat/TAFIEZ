#ifndef GLOBALFLAGCONSTANTS_H
#define GLOBALFLAGCONSTANTS_H

const int NUM_ZONES = 37;

extern const unsigned int FLAG_COLLECT_FLAGS[NUM_ZONES]; // Do we have the flag?
extern const unsigned int ZONE_ACCESSED_GLOBAL_FLAGS[NUM_ZONES]; // Do we show the zone on the map?
extern const unsigned int HUB_ACCESSED_GLOBAL_FLAGS[5]; // Do we show the hub on the map?
extern const unsigned int HUB_ALT_ACCESSED_GLOBAL_FLAGS[5]; // Do we show the H-alt exit on the map?
extern const unsigned int X_ALT_ACCESSED_GLOBAL_FLAGS[4]; // Do we show the X-alt exit on the map?
extern const unsigned int FATE_SIGNALER_CHOICE[2]; // Which way did the Fate Signaler go?
extern const unsigned int UNUSED_FLAGS[200]; // Some flags to pick from later

enum class HubCode {
	Alpha,
	Beta,
	Gamma,
	Delta,
	Omega,
};

enum class MiscGlobalFlag {
	CarControlsLearned,
	UndoLearned,
	WorldResetLearned,
	JumpLearned,
	SwitchPlayerLearned,
	COUNT,
};

extern const unsigned int MISC_GLOBAL_FLAGS[static_cast<int>(MiscGlobalFlag::COUNT)];

int get_clear_flag_index(char zone);
unsigned int get_clear_flag_code(char zone);
unsigned int get_zone_access_code(char zone);
unsigned int get_misc_flag(MiscGlobalFlag flag);

#endif //GLOBALFLAGCONSTANTS_H