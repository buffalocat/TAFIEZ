#ifndef GAMESTATE_H
#define GAMESTATE_H

class OpenGLWindow;
class GraphicsManager;
class SoundManager;
class TextRenderer;

class GameState {
public:
    GameState();
    virtual ~GameState();
    void create_child(std::unique_ptr<GameState> child);
    void set_csp(std::unique_ptr<GameState>*);
    virtual void main_loop() = 0;
    void check_for_escape();
	virtual void handle_escape();
    bool attempt_queued_quit();
	void queue_quit();
	virtual bool can_quit(bool confirm);
	void defer_to_sibling(std::unique_ptr<GameState> sibling);
	bool key_pressed(int key);
	void toggle_fullscreen();
	void handle_fullscreen_toggle();

	GraphicsManager* gfx_{};
	SoundManager* sound_{};
	std::unique_ptr<TextRenderer> text_{};
	OpenGLWindow* window_{};
	std::unique_ptr<GameState> parent_{};

	GameState(GameState* parent);
	std::unique_ptr<GameState>* current_state_ptr_{};
	bool can_escape_quit_ = true;
	bool can_toggle_fullscreen_ = true;

private:
	void defer_to_parent();
	bool queued_quit_ = false;
};

#endif // GAMESTATE_H
