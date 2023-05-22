#pragma once
#include <glad/glad.h>
#include <cassert>
#include <iostream>
#include <string>
#include <glm/vec3.hpp>


#ifdef _DEBUG
	#ifdef _MSC_VER
	#define ASSERT(x) if (!(x)) __debugbreak();
	#else
	#define ASSERT(x) assert(x);
	#endif
#define GLCall(x) GLClearError(); x; ASSERT(GLLogErrors(#x, __FILE__, __LINE__));
#else
#define ASSERT(x) x
#define GLCall(x) x
#endif


static void GLClearError()
{
	while (glGetError() != GL_NO_ERROR);
}

static bool GLLogErrors(const char* function, const char* file, int line)
{
	while (GLenum error = glGetError())
	{
		std::cout << "[OpenGL Error] (" << error << ")" << 
			function << " " << file << ":" << line <<
			std::endl;
		return false;
	}
	return true;
}
namespace Util
{
	inline void println(const std::string& str) {
		std::cout << str << std::endl;
	}

	inline void printErr(const std::string& str) {
		std::cerr << str << std::endl;
	}

	inline void printVec3(const glm::vec3& vec) {
		std::cout << 
			std::to_string(vec.x) + " " + std::to_string(vec.y) + " " + std::to_string(vec.z) 
			<< std::endl;

	}

}



