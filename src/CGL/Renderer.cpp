#include "Renderer.h"
#include "Util.h"

void Renderer::Clear(bool color, bool depth)
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	unsigned int clearBitfield = 0;
	if (color)
		clearBitfield |= GL_COLOR_BUFFER_BIT;
	if (depth)
		clearBitfield |= GL_DEPTH_BUFFER_BIT;
	GLCall(glClear(clearBitfield));
}

void Renderer::Draw(VertexArray& va, Shader& shader)
{
	shader.Bind();
	va.Bind();
	if (va.GetIndexBuffer() == nullptr) {
		GLCall(glDrawArrays(
			GL_TRIANGLES,
			0,  // starting index
			va.GetVertexBuffer()->GetCount()
		));
	}
	else
	{
		GLCall(glDrawElements(
			GL_TRIANGLES,
			va.GetIndexBufferCount(),
			GL_UNSIGNED_INT,   // type of index in EBO
			0  // offset
		));
	}
}
