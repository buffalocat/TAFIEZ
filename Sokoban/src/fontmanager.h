#pragma once

struct TextVertex {
	glm::vec2 Position;
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
	Font(FT_Library ft, Shader* text_shader, std::string path, unsigned int font_size);
	~Font();

	void init_glyphs(int font_size);
	void render_glyphs();
	void generate_string_verts(const char* text, float x, float y, float sx, float sy,
		std::vector<TextVertex>& text_verts, float* width, float* height);

	GLuint tex_;
	Shader* shader_;
	unsigned int font_size_;

private:
	GlyphPos glyphs_[128];
	FT_Face face_;
	unsigned int tex_width_, tex_height_;
};

class FontManager {
public:
	FontManager(Shader* text_shader);
	~FontManager();

	Font* get_font(std::string path, unsigned int size);

	Shader* text_shader_;

private:
	std::unique_ptr<Font> make_font(std::string path, unsigned int font_size);

	std::map<std::pair<std::string, unsigned int>, std::unique_ptr<Font>> fonts_{};

	FT_Library ft_;
};