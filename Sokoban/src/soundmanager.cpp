#include "stdafx.h"
#include "soundmanager.h"

#include <stdlib.h>
#include <AL/alut.h>

#include <Windows.h>


QueuedSound::QueuedSound(const char* file_name) : power{ 0 } {
	std::string full_file = "resources/sounds/";
	full_file = full_file + file_name;
	buffer = alutCreateBufferFromFile(full_file.c_str());
}


SoundManager::SoundManager() {
	LOG("Trying to start ALUT...");
	if (!alutInit(nullptr, nullptr)) {
		LOG("ALUT error: " << alutGetErrorString(alutGetError()));
	} else {
		LOG("ALUT init'd normally.");
	}
	queued_sounds_.push_back({ "undo_click.wav" });
	queued_sounds_.push_back({ "door_enter.wav" });
	queued_sounds_.push_back({ "switch_on.wav" });
	queued_sounds_.push_back({ "switch_off.wav" });
	queued_sounds_.push_back({ "gate_down.wav" });
	queued_sounds_.push_back({ "car_ride.wav" });
	queued_sounds_.push_back({ "snip1.wav" });
	queued_sounds_.push_back({ "flag_get.wav" });
	queued_sounds_.push_back({ "sizzle.wav" });
	queued_sounds_.push_back({ "color_change.wav" });
	queued_sounds_.push_back({ "slide.wav" });
	queued_sounds_.push_back({ "fall_thud.wav" });
	queued_sounds_.push_back({ "fall.wav" });
	queued_sounds_.push_back({ "jump.wav" });
	LOG("Trying to load sounds...");
	LOG("ALUT error? " << alutGetErrorString (alutGetError ()));
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

void SoundManager::clean_sources() {
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
}

void SoundManager::play_sounds() {
	for (auto& q : queued_sounds_) {
		if (q.power) {
			ALuint source;
			alGenSources(1, &source);
			if (!has_played_sound_) {
				LOG("Trying to generate a source...");
				LOG("ALUT error? " << alutGetErrorString (alutGetError ()));
			}
			alSourcei(source, AL_BUFFER, q.buffer);
			if (!has_played_sound_) {
				LOG("Trying to put sound in the buffer...");
				LOG("ALUT error? " << alutGetErrorString (alutGetError ()));
			}
			q.power = 0;
			active_sources_.push_back(source);
			alSourcePlay(source);
			if (!has_played_sound_) {
				LOG("Trying to play the sound...");
				LOG("ALUT error? " << alutGetErrorString (alutGetError ()));
				has_played_sound_ = true;
			}
		}
	}
}

void SoundManager::flush_sounds() {
	for (auto& q : queued_sounds_) {
		q.power = 0;
	}
}