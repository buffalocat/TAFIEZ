#pragma once

class Font;
class StringDrawer;
class RoomLabelDrawer;

struct TextVertex {
	glm::vec2 Position;
	glm::vec2 TexCoords;
};

class TextRenderer {
public:
	TextRenderer();
	~TextRenderer();

	void init();
	void render_text();

	void make_room_label(std::string label);

	// Loaded Fonts
	std::unique_ptr<Font> kalam_;

private:
	std::unique_ptr<Font> make_font(std::string path, unsigned int font_size);

	std::vector<std::unique_ptr<StringDrawer>> drawers_{};
	std::unique_ptr<RoomLabelDrawer> room_label_{};

	FT_Library ft_;
	Shader text_shader_;
};

class StringDrawer {
public:
	StringDrawer(Font* font, glm::vec4 color,
		std::string label, float x, float y, float sx, float sy);
	virtual ~StringDrawer();

	void set_color(int color);
	void set_color(glm::vec4 color);
	void render();

protected:
	std::vector<TextVertex> vertices_{};
	glm::vec4 color_;
	Shader shader_;
	GLuint VAO_, VBO_, tex_;
	float width_;
};

class RoomLabelDrawer : public StringDrawer {
public:
	RoomLabelDrawer(Font* font, glm::vec4 color, std::string label);
	~RoomLabelDrawer();

	void update();

private:
	unsigned int cooldown_;
};

struct GlyphPos {
	unsigned int left, top;
	int left_bear, top_bear;
	unsigned int width, height;
	int advance_x, advance_y;
};

class Font {
public:
	Font(FT_Library ft, Shader text_shader, std::string path, unsigned int font_size);
	~Font();

	void init_glyphs(int font_size);
	void render_glyphs();
	void generate_string_verts(const char* text, float x, float y, float sx, float sy, std::vector<TextVertex>& text_verts, float* width);

	GLuint tex_;
	Shader shader_;

private:
	GlyphPos glyphs_[128];
	FT_Face face_;
	unsigned int tex_width_, tex_height_;
};