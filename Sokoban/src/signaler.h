#ifndef SIGNALER_H
#define SIGNALER_H

class Switchable;
class Switch;
class RoomMap;
class DeltaFrame;
class MoveProcessor;
class MapFileO;
class ObjectModifier;

class Signaler {
public:
    Signaler(const std::string& label, int count);
    virtual ~Signaler();

    virtual void serialize(MapFileO& file) = 0;

    void push_switch(Switch*, bool mutual);
	void remove_switch(Switch*);
	virtual void push_switchable(Switchable*, bool mutual, int index) = 0;
	virtual void remove_switchable(Switchable*, int index) = 0;
	//void remove_object(ObjectModifier*);

    void receive_signal(bool signal);
    virtual void check_send_signal(RoomMap*, DeltaFrame*, MoveProcessor*) = 0;
	void update_count(DeltaFrame*);
	void reset_count(int count);

protected:
	std::vector<Switch*> switches_;
    std::string label_;
	int prev_count_;
    int count_;

    friend class SwitchTab;
};

class ThresholdSignaler : public Signaler {
public:
	ThresholdSignaler(std::string label, int count, int threshold);
	~ThresholdSignaler();

	void serialize(MapFileO& file);

	void push_switchable(Switchable*, bool mutual, int index);
	void remove_switchable(Switchable*, int index);

	void check_send_signal(RoomMap*, DeltaFrame*, MoveProcessor*);

private:
	std::vector<Switchable*> switchables_;
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
	std::vector<std::vector<Switchable*>> switchables_;
	int parity_level_;
	bool initialized_;

	friend class SwitchTab;
};

#endif // SIGNALER_H
