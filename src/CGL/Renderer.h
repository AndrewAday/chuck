#pragma once

#include <glad/glad.h>
#include "VertexArray.h"
#include "Shader.h"

class Renderer
{
public:
	void Clear(bool color = true, bool depth = true);
	void Draw(VertexArray& va, Shader& shader); // TODO: refactor to use materials
};
