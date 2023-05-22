#pragma once

class VertexBuffer
{
private:
	unsigned int m_RendererID;
	unsigned int m_Size, m_Count;
public:
	VertexBuffer(const void* data, unsigned int size, unsigned int count, unsigned int usage);
	~VertexBuffer();

	void Bind() const;
	void Unbind() const;

	inline unsigned int GetSize() const { return m_Size; }
	inline unsigned int GetCount() const { return m_Count; }
};
