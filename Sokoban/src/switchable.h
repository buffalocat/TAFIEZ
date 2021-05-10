#ifndef SWITCHABLE_H
#define SWITCHABLE_H

#include "objectmodifier.h"
#include "delta.h"

class RoomMap;
class DeltaFrame;
class GameObject;
class MoveProcessor;
class Signaler;

class Switchable: public ObjectModifier {
public:
	Switchable(GameObject* parent, int count, bool persistent, bool default_state, bool active, bool waiting);
	virtual ~Switchable();

	void make_str(std::string&);

	void push_signaler(Signaler*, int index);
	void remove_signaler(Signaler*);
	void connect_to_signalers();
	void remove_from_signalers();

	bool state();
	virtual bool can_set_state(bool state, RoomMap*) = 0;
	void receive_signal(bool signal, RoomMap*, DeltaFrame*, MoveProcessor*);
	virtual void apply_state_change(RoomMap*, DeltaFrame*, MoveProcessor*);
	void check_waiting(RoomMap*, DeltaFrame*, MoveProcessor*);
	void check_active_change(RoomMap*, DeltaFrame*, MoveProcessor*);

	void cleanup_on_take(RoomMap*, DeltaFrame*, bool real);
	void setup_on_put(RoomMap*, DeltaFrame*, bool real);

	bool persistent_;

protected:
	int count_;
	int prev_count_;
	bool default_;
	bool active_; // Opposite of default behavior
	bool waiting_; // Toggle active as soon as possible

	std::vector<std::pair<Signaler*, int>> signalers_;

	friend class ModifierTab;
	friend class SwitchableDelta;
	friend struct SwitchableDeltaGuard;
};


class SwitchableDelta : public Delta {
public:
	SwitchableDelta(Switchable* obj, int count, bool active, bool waiting);
	SwitchableDelta(FrozenObject obj, int count, bool active, bool waiting);
	~SwitchableDelta();
	void serialize(MapFileO&);
	void revert(RoomMap*);
	DeltaCode code();
	static std::unique_ptr<Delta> deserialize(MapFileI& file);

private:
	FrozenObject obj_;
	int count_;
	bool active_;
	bool waiting_;
};


#endif // SWITCHABLE_H
