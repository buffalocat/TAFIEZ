#ifndef SAVELOADTAB_H
#define SAVELOADTAB_H


#include "editortab.h"

class SaveLoadTab: public EditorTab {
public:
    SaveLoadTab(EditorState*, GraphicsManager*);
    ~SaveLoadTab();
    void main_loop(EditorRoom*);
    void handle_left_click(EditorRoom*, Point3);

	void map_load_and_create();
	void loaded_rooms_listbox();
};

#endif // SAVELOADTAB_H
