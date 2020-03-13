#ifndef GLOBALFLAGCONSTANTS_H
#define GLOBALFLAGCONSTANTS_H

extern const unsigned int FLAG_COLLECT_FLAGS[36];
extern const unsigned int FATE_SIGNALER_CHOICE[2];

enum class MiscGlobalFlags {
	CarControlsLearned,
	UndoLearned,
	WorldResetLearned,
	JumpLearned,
	SwitchPlayerLearned,
	COUNT,
};

extern const unsigned int MISC_GLOBAL_FLAGS[static_cast<int>(MiscGlobalFlags::COUNT)];

int get_clear_flag_index(char zone);
unsigned int get_clear_flag_code(char zone);
unsigned int get_misc_flag(MiscGlobalFlags flag);

#endif //GLOBALFLAGCONSTANTS_H