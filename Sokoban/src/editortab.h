#ifndef EDITORTAB_H
#define EDITORTAB_H

struct EditorRoom;
class EditorState;
class GraphicsManager;
class RoomMap;

struct Point3;

class EditorTab {
public:
    EditorTab(EditorState*);
    virtual ~EditorTab();
    virtual void init();
    virtual void main_loop(EditorRoom*) = 0;
	virtual bool handle_keyboard_input();
    virtual void handle_left_click(EditorRoom*, Point3);
    virtual void handle_right_click(EditorRoom*, Point3);
	bool kill_object(Point3 pos, RoomMap* map);

protected:
    EditorState* editor_;
    GraphicsManager* gfx_;
	
	bool inspect_mode_ = false;
};

void color_button(int color_id);
void file_choice(std::filesystem::path path, char* output, int max_size);

#endif // EDITORTAB_H
