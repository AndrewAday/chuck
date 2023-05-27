#pragma once
#include "CGL/Includes.h"

// naive singleton setting up window state (wraps what was Main.cpp in the standalone openGL engine)
//https://stackoverflow.com/questions/1008019/c-singleton-design-pattern
class Window
{
public:

	static Window& GetInstance() {
		static Window instance;
		return instance;
	}
	
	void DisplayLoop();
	
	// set and get
	inline GLFWwindow* GetWindow() { return m_Window; }
	inline float GetDeltaTime() const { return m_DeltaTime; }
	void SetViewSize(int width, int height);
	void Terminate() { glfwTerminate(); }
private:
	Window();  // private constructor for singleton pattern
	
	// member vars
	GLFWwindow* m_Window;
	int m_ViewWidth, m_ViewHeight;
	float m_DeltaTime = 0.0f;
public:
	Window(const Window& obj) = delete;  // delete copy constructor
	void operator=(Window const&) = delete;
	~Window();

};
