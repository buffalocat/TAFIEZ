#include "stdafx.h"
#include "soundmanager.h"

#include <stdlib.h>
#include <AL/alut.h>

#include <Windows.h>


QueuedSound::QueuedSound(const char* file_name, ALCdevice* device) : power{ 0 } {
	std::string full_file = "resources/sounds/";
	full_file = full_file + file_name;
	buffer = alutCreateBufferFromFile("switch_on.wav");
	auto buffer1 = alutCreateBufferHelloWorld();
	auto buffer2 = alutCreateBufferWaveform(ALUT_WAVEFORM_SINE, 440, 0, 1);
	auto buffer0 = alutCreateBufferFromFile("switch_on.wav");
	auto error = alcGetError(device);
	std::cout << (_alutSanityCheck() == AL_TRUE) << std::endl;
	if (error != AL_NO_ERROR) {
		std::cout << error << std::endl;
	}
	buffer = buffer2;
}


SoundManager::SoundManager() {
	device_ = alcOpenDevice(nullptr);
	context_ = alcCreateContext(device_, nullptr);
	alcMakeContextCurrent(context_);
	alutInitWithoutContext(nullptr, nullptr);
	queued_sounds_.push_back({ "switch_on.wav", device_ });
	queued_sounds_.push_back({ "switch_off.wav", device_ });
	auto buffer_0 = alutCreateBufferHelloWorld();
	auto buffer = alutCreateBufferFromFile("resources/sounds/switch_on.wav");
	auto error = alGetError();
	if (error != AL_NO_ERROR) {
		std::cout << error << std::endl;
	}
	ALuint source;
	alGenSources(1, &source);
	alSourcei(source, AL_BUFFER, 1);
	alSourcePlay(source);
}

SoundManager::~SoundManager() {
	//TODO: del sources
	for (auto& q : queued_sounds_) {
		alDeleteBuffers(1, &q.buffer);
	}
	alutExit();
}

void SoundManager::queue_sound(SoundName name) {
	++queued_sounds_[static_cast<int>(name)].power;
}

void SoundManager::flush_sounds() {
	active_sources_.erase(remove_if(active_sources_.begin(), active_sources_.end(),
		[](ALuint source) {
		ALint status;
		alGetSourcei(source, AL_SOURCE_STATE, &status);
		if (status == AL_STOPPED) {
			alDeleteSources(1, &source);
			return true;
		} else {
			return false;
		}
	}), active_sources_.end());

	for (auto& q : queued_sounds_) {
		if (q.power) {
			ALuint source;
			alGenSources(1, &source);
			alSourcei(source, AL_BUFFER, q.buffer);
			q.power = 0;
			active_sources_.push_back(source);
			alSourcePlay(source);
		}
	}
}
