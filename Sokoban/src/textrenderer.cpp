#include "stdafx.h"
#include "textrenderer.h"


struct TextVertex {
	glm::vec2 Position;
	glm::vec2 TexCoords;
};


TextRenderer::TextRenderer() :
	shader_{ Shader("shaders/text_shader.vs", "shaders/text_shader.fs") } {}

TextRenderer::~TextRenderer() {}

void TextRenderer::init() {
	shader_.use();

	// Load the font
	if (FT_Init_FreeType(&ft)) {
		std::cout << "Failed to initialize FreeType" << std::endl;
	}
	if (FT_New_Face(ft, "resources/kalam/Kalam-Bold.ttf", 0, &face)) {
		std::cout << "Failed to load the font" << std::endl;
	}
	FT_Set_Pixel_Sizes(face, 0, 368);

	// Initialize the texture
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Initialize the vertex array
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(TextVertex), (void*)offsetof(TextVertex, Position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(TextVertex), (void*)offsetof(TextVertex, TexCoords));

	glBindVertexArray(0);
}

void TextRenderer::render(const char* text, float x, float y, float sx, float sy, float opacity) {
	shader_.use();
	shader_.setVec4("color", glm::vec4(0.2, 1, 0.8, opacity));

	sx *= 0.25f;
	sy *= 0.25f;

	glDisable(GL_DEPTH_TEST);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindTexture(GL_TEXTURE_2D, tex);

	const char *p;

	FT_GlyphSlot g = face->glyph;

	float text_width = 0;

	for (p = text; *p; p++) {
		if (FT_Load_Char(face, *p, FT_LOAD_RENDER)) {
			continue;
		}
		text_width += g->advance.x >> 6;
	}

	x -= text_width * sx / 2;

	for (p = text; *p; p++) {
		if (FT_Load_Char(face, *p, FT_LOAD_RENDER)) {
			continue;
		}

		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			g->bitmap.width,
			g->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			g->bitmap.buffer
		);

		float x2 = x + g->bitmap_left * sx;
		float y2 = -y - g->bitmap_top * sy;
		float w = g->bitmap.width * sx;
		float h = g->bitmap.rows * sy;

		TextVertex box[4] = {
					TextVertex{{x2, -y2}, {0, 0}},
					TextVertex{{x2 + w, -y2}, {1, 0}},
					TextVertex{{x2, -y2 - h}, {0, 1}},
					TextVertex{{x2 + w, -y2 - h}, {1, 1}},
		};
		glBufferData(GL_ARRAY_BUFFER, sizeof box, box, GL_DYNAMIC_DRAW);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		x += (g->advance.x >> 6) * sx;
		y += (g->advance.y >> 6) * sy;
	}
	glBindVertexArray(0);
	glEnable(GL_DEPTH_TEST);
}