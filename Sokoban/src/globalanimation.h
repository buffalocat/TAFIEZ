#ifndef GLOBALANIMATION_H
#define GLOBALANIMATION_H

class GlobalAnimation {
public:
	GlobalAnimation();
	virtual ~GlobalAnimation();

	virtual bool update() = 0;
};

class PlayingGlobalData;

class FlagGetAnimation : public GlobalAnimation {
	FlagGetAnimation(PlayingGlobalData* global, char zone);
	~FlagGetAnimation();

	bool update();

};

#endif //GLOBALANIMATION_H