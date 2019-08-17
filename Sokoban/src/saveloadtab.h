#ifndef SAVELOADTAB_H
#define SAVELOADTAB_H


#include "editortab.h"

class SaveLoadTab: public EditorTab {
public:
    SaveLoadTab(EditorState*);
    ~SaveLoadTab();

    void main_loop(EditorRoom*);

	void map_load_and_create();
	void loaded_rooms_listbox();
};

#endif // SAVELOADTAB_H
