#ifndef EDITORTAB_H
#define EDITORTAB_H

#include <dear/imgui.h>

struct EditorRoom;
class EditorState;
class GraphicsManager;

struct Point3;
struct Color4;

class EditorTab {
public:
    EditorTab(EditorState*, GraphicsManager*);
    virtual ~EditorTab();
    virtual void init();
    virtual void main_loop(EditorRoom*) = 0;
    virtual void handle_left_click(EditorRoom*, Point3);
    virtual void handle_right_click(EditorRoom*, Point3);

protected:
    EditorState* editor_;
    GraphicsManager* gfx_;
};

void clamp(int* n, int a, int b);
ImVec4 unpack_color(Color4 v);

#endif // EDITORTAB_H
