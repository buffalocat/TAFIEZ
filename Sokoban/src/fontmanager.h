#pragma once

class OpenGLWindow;
class StringDrawer;
class FontManager;

struct TextVertex {
	glm::vec2 Position;
	glm::vec2 TexCoords;
};

struct TextVertex3 {
	glm::vec3 Position;
	glm::vec2 TexCoords;
};

struct GlyphPos {
	unsigned int left, top;
	int left_bear, top_bear;
	unsigned int width, height;
	int advance_x, advance_y;
};

class Font {
public:
	Font(FontManager* fm, FT_Library ft, OpenGLWindow* window,
		Shader* text_shader, std::string path, unsigned int font_size);
	~Font();

	void init_glyphs(int font_size, FT_Face face);
	void generate_string_verts(const char* text, float x, float y, float sx, float sy,
		std::vector<TextVertex>& text_verts, float* width, float* height);
	void generate_spacial_char_verts(char c, glm::vec3 center, glm::vec3 vx, glm::vec3 vy, float scale,
		std::vector<TextVertex3>& text_verts);

	FontManager* fm_;
	GLuint tex_;
	OpenGLWindow* window_;
	Shader* shader_;
	unsigned int font_size_;

private:
	GlyphPos glyphs_[128];
	unsigned int tex_width_, tex_height_;
};

class FontManager {
public:
	FontManager(OpenGLWindow* window, Shader* text_shader);
	~FontManager();

	Font* get_font(std::string path, unsigned int size);

	OpenGLWindow* window_;
	Shader* text_shader_;

	std::set<StringDrawer*> string_drawers_{};

private:
	std::unique_ptr<Font> make_font(std::string path, unsigned int font_size);
	std::map<std::pair<std::string, unsigned int>, std::unique_ptr<Font>> fonts_{};
	FT_Library ft_;
};