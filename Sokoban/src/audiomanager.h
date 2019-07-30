#pragma once

#include <OpenAL/alc.h>

class AudioManager {
public:
	AudioManager();
	~AudioManager();

	void load_wav(std::string);

private:
	std::vector<std::unique_ptr<Sound>> sounds_;
	ALCdevice* device_;
	ALCcontext* context_;
};

class Sound {
public:
	Sound();
	~Sound();

private:
	ALuint buffer_;
};