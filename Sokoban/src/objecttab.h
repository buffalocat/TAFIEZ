#ifndef OBJECTTAB_H
#define OBJECTTAB_H

#include "editortab.h"

enum class ObjCode;
class GameObject;
class RoomMap;

class ObjectTab: public EditorTab {
public:
    ObjectTab(EditorState*);
    ~ObjectTab();

    void init();
    void main_loop(EditorRoom*);
	bool handle_keyboard_input();
    void handle_left_click(EditorRoom*, Point3);
    void handle_right_click(EditorRoom*, Point3);

private:
	void object_type_choice(ObjCode*);
	void object_tab_options(RoomMap*);

	std::unique_ptr<GameObject> create_from_model(ObjCode, GameObject* prev);
};

#endif // OBJECTTAB_H
