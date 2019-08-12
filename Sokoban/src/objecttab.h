#ifndef OBJECTTAB_H
#define OBJECTTAB_H

#include "editortab.h"

enum class ObjCode;
class GameObject;
class RoomMap;

class ObjectTab: public EditorTab {
public:
    ObjectTab(EditorState*, GraphicsManager*);
    ~ObjectTab();

    void init();
    void main_loop(EditorRoom*);
    void handle_left_click(EditorRoom*, Point3);
    void handle_right_click(EditorRoom*, Point3);

private:
	void object_type_choice(ObjCode*);
	void object_tab_options();

	std::unique_ptr<GameObject> create_from_model(ObjCode, GameObject* prev);
	RoomMap* map_;
};

#endif // OBJECTTAB_H
