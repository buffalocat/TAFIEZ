#include "stdafx.h"
#include "audiomanager.h"

#include <OpenAL/al.h>

AudioManager::AudioManager() : sounds_{}, device_ {}, context_{} {
	device_ = alcOpenDevice(NULL);
	if (!device_) {
		return;
	}
	context_ = alcCreateContext(device_, NULL);
	alcMakeContextCurrent(context_);
}

AudioManager::~AudioManager() {
	alcMakeContextCurrent(NULL);
	if (context_) {
		alcDestroyContext(context_);
	}
	if (device_) {
		alcCloseDevice(device_);
	}
}

void AudioManager::load_wav(std::string file_name) {
	
}
