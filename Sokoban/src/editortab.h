#ifndef EDITORTAB_H
#define EDITORTAB_H

struct EditorRoom;
class EditorState;
class GraphicsManager;

struct Point3;

class EditorTab {
public:
    EditorTab(EditorState*, GraphicsManager*);
    virtual ~EditorTab();
    virtual void init();
    virtual void main_loop(EditorRoom*) = 0;
	virtual bool handle_keyboard_input();
    virtual void handle_left_click(EditorRoom*, Point3);
    virtual void handle_right_click(EditorRoom*, Point3);

protected:
    EditorState* editor_;
    GraphicsManager* gfx_;

	bool inspect_mode_;
};

void color_button(int color_id);


#endif // EDITORTAB_H
