#pragma once
#include "CGL/Includes.h"

// naive singleton setting up window state (wraps what was Main.cpp in the standalone openGL engine)
//https://stackoverflow.com/questions/1008019/c-singleton-design-pattern
class Window
{
public:
	// Window(int viewWidth = 2400, int viewHeight = 1800);
	Window(int viewWidth = 3840, int viewHeight = 2160);
	~Window();

	void DisplayLoop();
	
	void Terminate() { glfwTerminate(); }

	// set and get
	inline GLFWwindow* GetWindow() { return m_Window; }
	inline float GetDeltaTime() const { return m_DeltaTime; }
	inline int GetViewWidth() { return m_ViewWidth; }
	inline int GetViewHeight() { return m_ViewHeight; }
	void SetViewSize(int width, int height);
private:
	// member vars
	GLFWwindow* m_Window;
	int m_ViewWidth, m_ViewHeight;
	float m_DeltaTime = 0.0f;

};
