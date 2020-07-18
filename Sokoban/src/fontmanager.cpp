#include "stdafx.h"
#include "fontmanager.h"
#include "common_constants.h"
#include "color_constants.h"

FontManager::FontManager(Shader* text_shader) :
	text_shader_{ text_shader } {
	if (FT_Init_FreeType(&ft_)) {
		std::cout << "Failed to initialize FreeType" << std::endl;
	}
}

FontManager::~FontManager() {}

Font* FontManager::get_font(std::string path, unsigned int size) {
	auto key = std::make_pair(path, size);
	auto& font = fonts_[key];
	if (!font) {
		text_shader_->use();
		font = make_font(path, size);
	}
	return font.get();
}

std::unique_ptr<Font> FontManager::make_font(std::string path, unsigned int font_size) {
	return std::make_unique<Font>(ft_, text_shader_, path, font_size);
}

Font::Font(FT_Library ft, Shader* text_shader, std::string path, unsigned int font_size) :
	shader_{ text_shader },
	tex_width_{ 1 << 9 }, tex_height_{ 1 << 9 }, font_size_{ font_size } {
	FT_Face face;
	if (FT_New_Face(ft, path.c_str(), 0, &face)) {
		std::cout << "Failed to load the font" << std::endl;
	}
	FT_Set_Pixel_Sizes(face, 0, font_size);
	init_glyphs(font_size, face);
	FT_Done_Face(face);
}

Font::~Font() {
	glDeleteTextures(1, &tex_);
}

void Font::init_glyphs(int font_size, FT_Face face) {
	// Figure out the size the texture has to be
	auto* g = face->glyph;

	unsigned int left = 0;
	unsigned int top = 0;
	unsigned int max_height = 0;
	// Not every character is even in the font!
	const char* good_characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.:,;'\"(!?)+-*/= ";
	for (const char* c = good_characters; *c; ++c) {
		if (FT_Load_Char(face, *c, FT_LOAD_RENDER)) {
			continue;
		}
		if (left + g->bitmap.width >= tex_width_) {
			top += max_height + 1;
			left = 0;
			max_height = 0;
		}
		glyphs_[*c] = {
			left,
			top,
			g->bitmap_left,
			g->bitmap_top,
			g->bitmap.width,
			g->bitmap.rows,
			g->advance.x >> 6,
			g->advance.y >> 6,
		};
		left += g->bitmap.width + 1;
		max_height = std::max(max_height, g->bitmap.rows);
	}
	while (tex_height_ <= top + max_height) {
		tex_height_ <<= 1;
	}

	glGenTextures(1, &tex_);
	glBindTexture(GL_TEXTURE_2D, tex_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, tex_width_, tex_height_, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Copy the glyphs into the texture
	for (const char* c = good_characters; *c; ++c) {
		if (FT_Load_Char(face, *c, FT_LOAD_RENDER)) {
			continue;
		}
		GlyphPos cur = glyphs_[*c];

		glTexSubImage2D(GL_TEXTURE_2D, 0, cur.left, cur.top, cur.width, cur.height, GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);
	}
}

void Font::generate_string_verts(const char* text, float x, float y, float sx, float sy,
	std::vector<TextVertex>& text_verts, float* width, float* height) {
	sx *= 2.0f / SCREEN_WIDTH;
	sy *= 2.0f / SCREEN_HEIGHT;
	float su = 1.0f / tex_width_;
	float sv = 1.0f / tex_height_;


	text_verts.reserve(6 * strlen(text));
	const char* line_start = text;
	const char* line_end = text;
	const char* p;

	int lines = 0;
	// Stop when we've reached a null character
	while (*line_end) {
		float cur_width = 0;
		y -= font_size_ * sy;

		// Stop this group at a null character *or* newline
		for (p = line_start; *p && (*p != '\n'); p++) {
			cur_width += glyphs_[*p].advance_x;
		}
		line_end = p;

		cur_width *= sx;
		float line_x = x - cur_width / 2;
		*width = std::max(*width, cur_width);

		// Every character takes 6 vertices
		for (const char* p = line_start; p != line_end; p++) {
			GlyphPos glyph = glyphs_[*p];

			float x2 = line_x + glyph.left_bear * sx;
			float y2 = y + glyph.top_bear * sy;
			float w = glyph.width * sx;
			float h = glyph.height * sy;

			float u = glyph.left * su;
			float v = glyph.top * sv;
			float du = glyph.width * su;
			float dv = glyph.height * sv;

			TextVertex box[4] = {
				TextVertex{{x2, y2}, {u, v}},
				TextVertex{{x2 + w, y2}, {u + du, v}},
				TextVertex{{x2, y2 - h}, {u, v + dv}},
				TextVertex{{x2 + w, y2 - h}, {u + du, v + dv}},
			};
			for (int i : {0, 1, 2, 2, 1, 3}) {
				text_verts.push_back(box[i]);
			}
			line_x += glyph.advance_x * sx;
			y += glyph.advance_y * sy;
		}
		line_start = line_end + 1;
		++lines;
	}
	*height = (float)font_size_ * sy * lines;
}


void Font::generate_spacial_char_verts(char c, glm::vec3 center, glm::vec3 nx, glm::vec3 ny, float scale,
	std::vector<TextVertex3>& text_verts) {
	GlyphPos glyph = glyphs_[c];
	glm::vec3 vx = (scale * glyph.width) * nx;
	glm::vec3 vy = (scale * glyph.height) * ny;
	float su = 1.0f / tex_width_;
	float sv = 1.0f / tex_height_;
	float u = glyph.left * su;
	float v = glyph.top * sv;
	float du = glyph.width * su;
	float dv = glyph.height * sv;
	TextVertex3 box[4] = {
		TextVertex3{center - vx + vy, {u, v}},
		TextVertex3{center + vx + vy, {u + du, v}},
		TextVertex3{center - vx - vy, {u, v + dv}},
		TextVertex3{center + vx - vy, {u + du, v + dv}},
	};
	for (int i : {0, 1, 2, 2, 1, 3}) {
		text_verts.push_back(box[i]);
	}
	for (int i : {0, 2, 1, 1, 2, 3}) {
		text_verts.push_back(box[i]);
	}
}

