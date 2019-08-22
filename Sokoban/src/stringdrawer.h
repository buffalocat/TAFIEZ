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
	virtual void kill_instance();
	virtual void cleanup();

	void set_alive_ptr(bool*);

	bool active_ = true;

protected:
	std::vector<TextVertex> vertices_{};
	glm::vec4 color_;
	Shader* shader_;
	GLuint VAO_, VBO_, tex_;
	float width_;
	bool* alive_ptr_{};
};

class IndependentStringDrawer : public StringDrawer {
public:
	IndependentStringDrawer(Font* font, glm::vec4 color, std::string label, float y, int fade_frames);
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
	RoomLabelDrawer(Font* font, glm::vec4 color, std::string label, float y);
	~RoomLabelDrawer();

	void init();
	void update();

private:
	unsigned int lifetime_;
};