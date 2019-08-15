#pragma once

struct TextVertex;
class Font;

class StringDrawer {
public:
	StringDrawer(Font* font, glm::vec4 color,
		std::string label, float x, float y, float sx, float sy);
	virtual ~StringDrawer();

	void set_color(int color);
	void set_color(glm::vec4 color);
	void render();

	virtual void update();
	void set_alive_ptr(bool*);
	void kill_instance();

	bool active_ = true;

protected:
	std::vector<TextVertex> vertices_{};
	glm::vec4 color_;
	Shader shader_;
	GLuint VAO_, VBO_, tex_;
	float width_;
	bool* alive_ptr_{};
};

class RoomLabelDrawer : public StringDrawer {
public:
	RoomLabelDrawer(Font* font, glm::vec4 color, std::string label, float y);
	~RoomLabelDrawer();

	void init();
	void update();

private:
	unsigned int cooldown_;
};