#ifndef SIGNALER_H
#define SIGNALER_H

class Switchable;
class Switch;
class RoomMap;
class DeltaFrame;
class MoveProcessor;
class MapFileO;
class ObjectModifier;

#include "delta.h"

class Signaler {
public:
    Signaler(const std::string& label, int count);
    virtual ~Signaler();

    virtual void serialize(MapFileO& file) = 0;

    void push_switch(Switch*, bool mutual);
	void remove_switch(Switch*);
	virtual void push_switchable(Switchable*, bool mutual, int index) = 0;
	virtual void remove_switchable(Switchable*, int index) = 0;

    void receive_signal(bool signal);
    virtual void check_send_signal(RoomMap*, DeltaFrame*, MoveProcessor*) = 0;
	virtual void check_send_initial(RoomMap*, DeltaFrame*, MoveProcessor*) = 0;
	void update_count(DeltaFrame*);
	void reset_prev_count(int count);

	std::string label_;

protected:
	std::vector<Switch*> switches_{};
	int prev_count_;
    int count_;

    friend class SwitchTab;
};

class ThresholdSignaler : public Signaler {
public:
	ThresholdSignaler(std::string label, int count, int threshold);
	virtual ~ThresholdSignaler();

	virtual void serialize(MapFileO& file);

	void push_switchable(Switchable*, bool mutual, int index);
	void remove_switchable(Switchable*, int index);

	virtual void check_send_signal(RoomMap*, DeltaFrame*, MoveProcessor*);
	void check_send_initial(RoomMap*, DeltaFrame*, MoveProcessor*);

protected:
	std::vector<Switchable*> switchables_{};
	int threshold_;

	friend class SwitchTab;
};

class ParitySignaler : public Signaler {
public:
	ParitySignaler(std::string label, int count, int parity_level, bool initialized);
	~ParitySignaler();

	void serialize(MapFileO& file);

	void push_switchable(Switchable*, bool mutual, int index);
	void remove_switchable(Switchable*, int index);

	void check_send_signal(RoomMap*, DeltaFrame*, MoveProcessor*);
	void check_send_initial(RoomMap*, DeltaFrame*, MoveProcessor*);

private:
	std::vector<std::vector<Switchable*>> switchables_{};
	int parity_level_;
	bool initialized_;

	friend class SwitchTab;
};

class FateSignaler : public ThresholdSignaler {
public:
	FateSignaler(std::string label, int count, int threshold);
	~FateSignaler();

	void serialize(MapFileO& file);
	void check_send_signal(RoomMap*, DeltaFrame*, MoveProcessor*);
};

class SignalerCountDelta : public Delta {
public:
	SignalerCountDelta(Signaler*, int count);
	~SignalerCountDelta();
	void revert();

private:
	Signaler* sig_;
	int count_;
};

#endif // SIGNALER_H
