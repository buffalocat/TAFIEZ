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
    Signaler(const std::string& label, int count, int threshold);
    virtual ~Signaler();

    void serialize(MapFileO& file);

    void push_switchable(Switchable*);
    void push_switch(Switch*);
    void push_switchable_mutual(Switchable*);
    void push_switch_mutual(Switch*);
    void receive_signal(bool signal);
    void toggle();
    void check_send_signal(RoomMap*, DeltaFrame*, MoveProcessor*);

    void remove_object(ObjectModifier*);

private:
    std::vector<Switch*> switches_;
    std::vector<Switchable*> switchables_;
    std::string label_;
    int count_;
    int threshold_;
	bool active_;

    friend class SwitchTab;
};

#endif // SIGNALER_H
