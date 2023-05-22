#pragma once
#include <vector>
#include "glm/glm.hpp"

class TransformationStack
{
public:
	TransformationStack() { m_Stack.push_back(glm::mat4(1.0f)); };
	void Push(const glm::mat4& transform) {
		m_Stack.push_back(m_Stack.back() * transform);
	};
	inline void Pop() { m_Stack.pop_back(); };
	inline const glm::mat4& GetTop() const { return m_Stack.back(); }
private:
	std::vector<glm::mat4> m_Stack;
};
