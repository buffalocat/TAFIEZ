#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include <al.h>
#include <alc.h>

enum class SoundName {
	SwitchOn = 0,
	SwitchOff = 1,
};

struct QueuedSound {
	int power;
	ALuint buffer;

	QueuedSound(const char* file_name, ALCdevice* device_);
};

class SoundManager {
public:
	SoundManager();
	~SoundManager();

	void queue_sound(SoundName name);
	void flush_sounds();

private:
	std::vector<QueuedSound> queued_sounds_{};
	std::vector<ALuint> active_sources_{};
	ALCdevice* device_;
	ALCcontext* context_;
};

#endif SOUNDMANAGER_H