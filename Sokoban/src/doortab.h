#ifndef DOORTAB_H
#define DOORTAB_H




#include "editortab.h"
#include "point.h"

class Door;

class DoorTab: public EditorTab {
public:
    DoorTab(EditorState*);
    ~DoorTab();

    void init();
    void main_loop(EditorRoom*);
    void handle_left_click(EditorRoom*, Point3);
	void handle_right_click(EditorRoom*, Point3);

private:
	Door* ent_door_{};
	EditorRoom* exit_room_{};
    unsigned int exit_door_id_ = 0;
};

#endif // DOORTAB_H
