#include "stdafx.h"
#include "texture_constants.h"

BlockTexture operator |(BlockTexture a, BlockTexture b) {
	return static_cast<BlockTexture>(static_cast<unsigned char>(a) |
		static_cast<unsigned char>(b));
}

glm::vec2 tex_to_vec(BlockTexture tex_id) {
	return { (float)((int)tex_id % TEXTURE_ATLAS_SIZE), (float)((int)tex_id / TEXTURE_ATLAS_SIZE) };
}