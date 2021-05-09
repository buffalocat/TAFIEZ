#ifndef SWITCH_H
#define SWITCH_H

#include "objectmodifier.h"
#include "delta.h"

class GameObject;
class RoomMap;
class DeltaFrame;
class Signaler;

class Switch: public ObjectModifier {
public:
    Switch(GameObject* parent, bool persistent, bool active);
    virtual ~Switch();

	void make_str(std::string&);

    void push_signaler(Signaler*);
	void remove_signaler(Signaler*);
    void connect_to_signalers();
	void remove_from_signalers();
    virtual bool check_send_signal(RoomMap*, DeltaFrame*) = 0;
    virtual bool should_toggle(RoomMap*) = 0;
    void toggle();
	void send_signal(bool signal);

	virtual void setup_on_put(RoomMap*, DeltaFrame*, bool real);
	virtual void cleanup_on_take(RoomMap*, DeltaFrame*, bool real);

protected:
    bool persistent_;
    bool active_;
    std::vector<Signaler*> signalers_;

    friend class ModifierTab;
	friend class FateSignaler;
};


class SwitchToggleDelta : public Delta {
public:
	SwitchToggleDelta(Switch* obj);
	SwitchToggleDelta(FrozenObject obj);
	~SwitchToggleDelta();
	void serialize(MapFileO&);
	void revert(RoomMap*);
	DeltaCode code();
	static std::unique_ptr<Delta> deserialize(MapFileIwithObjs& file);

private:
	FrozenObject obj_;
};

#endif // SWITCH_H
