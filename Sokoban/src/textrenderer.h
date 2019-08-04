#pragma once


class TextRenderer {
public:
	TextRenderer();
	~TextRenderer();
	
	void init();
	void render(const char* text, float x, float y, float sx, float sy, float opacity);

private:
	FT_Library ft;
	FT_Face face;
	Shader shader_;
	unsigned int tex, VAO, VBO, EBO;
};

