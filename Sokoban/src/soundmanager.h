#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include <al.h>
#include <alc.h>

enum class SoundName {
	UndoClick,
	DoorEnter,
	SwitchOn,
	SwitchOff,
	GateChange,
	CarRide,
	SnakeSplit,
	FlagGet,
	BurnBlock,
	ColorChange,
	MoveHorizontal,
	FallThud,
	Fall,
	Jump,
};

struct QueuedSound {
	int power;
	ALuint buffer;

	QueuedSound(const char* file_name);
};

class SoundManager {
public:
	SoundManager();
	~SoundManager();

	void queue_sound(SoundName name);
	void clean_sources();
	void play_sounds();
	void flush_sounds();

private:
	std::vector<QueuedSound> queued_sounds_{};
	std::vector<ALuint> active_sources_{};
	ALCdevice* device_;
	ALCcontext* context_;
};

#endif SOUNDMANAGER_H