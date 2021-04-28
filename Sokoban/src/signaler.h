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
    Signaler(const std::string& label, unsigned int sig_index, int count);
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

	std::vector<Switch*> switches_{};
	unsigned int sig_index_;
	int prev_count_;
    int count_;
};

class ThresholdSignaler : public Signaler {
public:
	ThresholdSignaler(std::string label, unsigned int sig_index, int count, int threshold);
	virtual ~ThresholdSignaler();

	virtual void serialize(MapFileO& file);

	void push_switchable(Switchable*, bool mutual, int index);
	void remove_switchable(Switchable*, int index);

	virtual void check_send_signal(RoomMap*, DeltaFrame*, MoveProcessor*);
	void check_send_initial(RoomMap*, DeltaFrame*, MoveProcessor*);

	std::vector<Switchable*> switchables_{};
	int threshold_;

};

class ParitySignaler : public Signaler {
public:
	ParitySignaler(std::string label, unsigned int sig_index, int count, int parity_level, bool initialized);
	~ParitySignaler();

	void serialize(MapFileO& file);

	void push_switchable(Switchable*, bool mutual, int index);
	void remove_switchable(Switchable*, int index);

	void check_send_signal(RoomMap*, DeltaFrame*, MoveProcessor*);
	void check_send_initial(RoomMap*, DeltaFrame*, MoveProcessor*);

	std::vector<std::vector<Switchable*>> switchables_{};
	int parity_level_;
	bool initialized_;
};

class FateSignaler : public ThresholdSignaler {
public:
	FateSignaler(std::string label, unsigned int sig_index, int count, int threshold);
	~FateSignaler();

	void serialize(MapFileO& file);
	void check_send_signal(RoomMap*, DeltaFrame*, MoveProcessor*);
};

class SignalerCountDelta : public Delta {
public:
	SignalerCountDelta(Signaler* sig, int count);
	SignalerCountDelta(unsigned int sig_index, int count);
	~SignalerCountDelta();
	void serialize(MapFileO&, GameObjectArray*);
	void revert(RoomMap*);
	DeltaCode code();
	static std::unique_ptr<Delta> deserialize(MapFileIwithObjs& file);

private:
	unsigned int index_;
	int count_;
};

#endif // SIGNALER_H
