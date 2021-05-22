#pragma once

struct TextVertex;
class Font;
class Shader;

extern const float ZONE_STRING_HEIGHT;
extern const float LEVEL_STRING_HEIGHT;
extern const float SIGN_STRING_HEIGHT;
extern const float DEATH_STRING_HEIGHT;
extern const float DEATH_SUBSTRING_HEIGHT;

extern const float ZONE_STRING_BG_OPACITY;
extern const float LEVEL_STRING_BG_OPACITY;
extern const float SIGN_STRING_BG_OPACITY;

extern const int FLOOR_SIGN_FADE_FRAMES;
extern const int DEATH_STRING_FADE_FRAMES;

class StringDrawer {
public:
	StringDrawer(Font* font, glm::vec4 color,
		std::string label, float x, float y, float sx, float sy, float bg);
	virtual ~StringDrawer();

	void generate_bg_verts(float x, float y, float font_height);
	void bind_bg_vbo();
	void set_color(int color);
	void set_color(glm::vec4 color);
	void render_bg(Shader* shader);
	void render();

	virtual void update();
	virtual void kill_instance();
	virtual void cleanup();
	void refresh_buffers();

	void set_alive_ptr(bool*);

	bool active_ = true;

protected:
	Font* font_;
	std::string label_;
	std::vector<TextVertex> vertices_{};
	std::vector<TextVertex> bg_vertices_{};
	glm::vec4 color_;
	glm::vec4 bg_color_;
	Shader* shader_;
	GLuint VAO_, VBO_, tex_;
	GLuint bg_VAO_, bg_VBO_;
	float x_, y_, sx_, sy_;
	float width_, height_;
	float opacity_, bg_;
	bool* alive_ptr_{};
};

class IndependentStringDrawer : public StringDrawer {
public:
	IndependentStringDrawer(Font* font, glm::vec4 color, std::string label, float y, int fade_frames, float bg);
	~IndependentStringDrawer();

	void own_self(std::unique_ptr<StringDrawer> self);

	void update();
	void kill_instance();
	void cleanup();

private:
	std::unique_ptr<StringDrawer> self_{};
	int fade_counter_ = 0;
	int fade_frames_;
	bool prepare_to_kill_ = false;
};

class RoomLabelDrawer : public StringDrawer {
public:
	RoomLabelDrawer(Font* font, glm::vec4 color, std::string label, float y, float bg, bool cam_icon);
	~RoomLabelDrawer();

	void init();
	void force_fade();
	void update();
	void generate_cam_icon_verts();

private:
	unsigned int lifetime_;
};