#include "chuck_window.h"

// CGL includes
#include "CGL/Includes.h"
#include "CGL/Shader.h"
#include "CGL/Util.h"
#include "CGL/VertexArray.h"
#include "CGL/VertexBuffer.h"
#include "CGL/IndexBuffer.h"
#include "CGL/Renderer.h"
#include "CGL/Texture.h"
#include "CGL/scenegraph/SceneGraphObject.h"
#include "CGL/Util.h"
#include "CGL/res/geometry/CubeVertices.h"  // TODO clean this up eventually

// chuck CORE includes
#include "core/ulib_cgl.h" // TODO: need to expose graphics entry point in chuck.h

// system includes
#include <iostream>
#include <stdexcept>
#include <condition_variable>

// glm includes
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

SceneGraphObject camera(CGL::mainCamera); // the camera copy
static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    Window::GetInstance().SetViewSize(width, height);
    glViewport(
        0, 0,  // index of lower left corner of viewport, in pixels
        width, height
    );
}

static void processKeyboadInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    
    //const float cameraSpeed = 2.5f * Window::GetInstance().GetDeltaTime();
    //if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    //    camera.Translate(cameraSpeed * camera.GetForward());
    //if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    //    camera.Translate(-cameraSpeed * camera.GetForward());
    //if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    //    camera.Translate(-cameraSpeed * camera.GetRight());
    //if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    //    camera.Translate(cameraSpeed * camera.GetRight());

}

static bool drawFill = true;
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        if (drawFill) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            drawFill = false;
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            drawFill = true;
        }
    }
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        // TODO: reload currently active shader
    }
}


void Window::SetViewSize(int width, int height)
{
    m_ViewWidth = width;
    m_ViewHeight = height;
}

Window::Window() : m_ViewWidth(2400), m_ViewHeight(1800), m_DeltaTime(0.0f)
{
    // init and select openGL version ==========================
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    // create window ============================================
    m_Window = glfwCreateWindow(m_ViewWidth, m_ViewHeight, "CGL", NULL, NULL);
    ASSERT(m_Window);
    if (m_Window == NULL)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }
    glfwMakeContextCurrent(m_Window);

    // VSYNC =================================================
    glfwSwapInterval(1);  

    // Initialize GLAD =======================================
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        throw std::runtime_error("Failed to initialize GLAD");
    }

    // OpenGL Viewport and Callbacks =========================
    glViewport(0, 0, m_ViewWidth, m_ViewHeight);
    glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);
    glfwSetKeyCallback(m_Window, keyCallback);

    // mouse settings =====================================
    // glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


    // OpenGL Metadata =======================================
    int nrAttributes;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
    std::cout << "Maximum number of 4-component vertex attributes: " << nrAttributes << std::endl;
    
    // GLEnables ===============================================
    // Blending (TODO) should this be part of renderer? 
    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    // depth testing
    GLCall(glEnable(GL_DEPTH_TEST));

    

}

Window::~Window()
{   
    std::cout << "calling window destructor, terminating glfw window" << std::endl;
	glfwTerminate();
}


// TODO: actually want to move this into chuck_main function, out of CGL window class
// so that chuck_main and my standalone rendering engine can have separate gameloop logic 
// chuck_main version needs to synchronize, standalone rendering engine doesn't
void Window::DisplayLoop()
{
    // Mesh Data for testing ================================
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    // TODO: eventually want to update camera aspect in window resize callback
    glm::mat4 projection = glm::perspective(  // gen perspective proj matrix, transforms from view space --> clip space
        glm::radians(45.0f),                    // fov (45 mimics human eye with average size display sitting at avg distance away)
        (float)m_ViewWidth / (float)m_ViewHeight,   // aspect ratio
        0.1f,                                   // near plane
        100.0f                                  // far plane
    );

    // Camera
    camera.SetPosition(0.0, 0.0, 3.0f);
    view = camera.GetInvModelMatrix();

    // Vertex Array Obj setup ===========
    VertexArray va;

    VertexBuffer vb(
        (void*)vertices,
        sizeof(vertices),  // size in bytes
        sizeof(vertices) / sizeof(vertices[0]),  // num elements
        GL_STATIC_DRAW
    );

    VertexBufferLayout layout;
    layout.Push(GL_FLOAT, 3, false);  // position data
    layout.Push(GL_FLOAT, 2, true);  // uv data

    va.AddBufferAndLayout(vb, layout);  // add vertex attrib pointers to VAO state

    // Texture ===========================================
    Texture texture0("../CGL/res/textures/wall.jpg");
    Texture texture1("../CGL/res/textures/container.jpg");
    Texture texture2("../CGL/res/textures/awesomeface.png");
    Texture texture3("../CGL/res/textures/chuck-logo.png");
    texture0.Bind(0);
    texture1.Bind(1);
    texture2.Bind(2);
    texture3.Bind(3);

    // Shader ============================================
    Shader basicShader("../CGL/res/shaders/BasicVert.glsl", "../CGL/res/shaders/BasicFrag.glsl");
    basicShader.Bind();

    // Shader Uniforms ================================
    basicShader.setTextureUnits(4);

    basicShader.setMat4f("u_Model", model);
    basicShader.setMat4f("u_View", camera.GetInvModelMatrix());
    basicShader.setMat4f("u_Projection", projection);

    // Renderer ====================================
    Renderer renderer;

    // Render Loop ===========================================
    float previousFPSTime = (float)glfwGetTime();
    float prevTickTime = previousFPSTime;
    int frameCount = 0;
    while (!glfwWindowShouldClose(m_Window))
    {
        // input
        processKeyboadInput(m_Window);

        // FPS counter
        float currentTime = (float)glfwGetTime();
        // deltaTime
        m_DeltaTime = currentTime - prevTickTime;
        prevTickTime = currentTime;
        ++frameCount;
        if (currentTime - previousFPSTime >= 1.0f) {
            Util::println(std::to_string(frameCount));
            frameCount = 0;
            previousFPSTime = currentTime;
        }

        // trigger Frame event in Chuck
        CglEvent::Broadcast(CglEventType::CGL_FRAME);

        // broadcast frame event!

        // OpenGL Rendering Commands =========================

        // wait to render until instructed by chuck
        CGL::WaitOnUpdateDone();

        //Util::println("Renderer starting deepcopy");
        // chuck done with writes, renderer is good to read from the scene graph!
        // TODO: deepcopy here
        //Util::println("Renderer deepcopy finished");
        
        { // DEEPCOPY logic
            camera = CGL::mainCamera;  // copy values via assignment operator
            /*
            auto& mainCamPos = CGL::mainCamera.GetPosition();
            auto& copyCamPos = camera.GetPosition();

            Util::println("mainCamera pos");
            Util::printVec3(mainCamPos);
			Util::println("mainCam address");
			 std::cout << &CGL::mainCamera << std::endl;

            Util::println("copyCam pos");
            Util::printVec3(copyCamPos);
			Util::println("copyCam address");
			std::cout << &camera << std::endl;
            */
            
        }

        // done deepcopying, let chuck know it's good to work on the next cycle of updates
        CglEvent::Broadcast(CglEventType::CGL_UPDATE);

        // now renderer can work on drawing the copied scenegraph ===

        // clear screen
        renderer.Clear();

        // shader hot reloading (not working :(
         //if (glfwGetKey(window, GLFW_KEY_R)) {
         //   basicShader.Reload();
         //}

        // setup shader pipeline
        float greenValue = (sin(currentTime * 2.0f) / 2.0f) + 0.5f;
        // Note: updating a uniform does require you to first use the program(by calling glUseProgram), 
        // because it sets the uniform on the currently active shader program.

        // update()
        basicShader.setFloat4("u_Color", 0.0f, greenValue, 0.0f, 1.0f);


        // camera.LookAt(glm::vec3(0.0f, 0.0f, 0.0f));
        basicShader.setMat4f("u_View", camera.GetInvModelMatrix());

        //model = glm::rotate(model, glm::radians(20 * m_DeltaTime), glm::vec3(0.0, 0.0, 1.0));  // 90 deg rotation matrix
        //model = glm::rotate(model, glm::radians(21 * m_DeltaTime), glm::vec3(1.0, 0.0, 0.0));  // 90 deg rotation matrix
        size_t dim = 50;
        for (size_t i = 0; i < dim; i++)
        {
            for (size_t j = 0; j < dim; j++)
            {
                auto tmp_model = glm::translate(model, glm::vec3(i*1.8, j*1.8, 0));
				tmp_model = glm::rotate(tmp_model, glm::radians(20 * currentTime), glm::vec3(0.0, 0.0, 1.0));  // 90 deg rotation matrix
				tmp_model = glm::rotate(tmp_model, glm::radians(21 * currentTime), glm::vec3(1.0, 0.0, 0.0));  // 90 deg rotation matrix
				basicShader.setMat4f("u_Model", tmp_model);

				renderer.Draw(va, basicShader);
            }

        }
        // model matrix
        //glm::mat4 trans = glm::mat4(1.0f);  // initialize identity matrix
        //model = glm::rotate(model, glm::radians(20 * m_DeltaTime), glm::vec3(0.0, 0.0, 1.0));  // 90 deg rotation matrix
        //model = glm::rotate(model, glm::radians(21 * m_DeltaTime), glm::vec3(1.0, 0.0, 0.0));  // 90 deg rotation matrix
        //basicShader.setMat4f("u_Model", model);

        // draw call
        //renderer.Draw(va, basicShader);


        // Handle Events, Draw framebuffer
        glfwPollEvents();
        //glfwWaitEvents();  // blocking version of PollEvents. How does blocking work? "puts the calling thread to sleep until at least one event is available in the event queue" https://www.glfw.org/docs/latest/group__window.html#ga15a5a1ee5b3c2ca6b15ca209a12efd14
        glfwSwapBuffers(m_Window);  // blocks until glfwSwapInterval screen updates have occured
    }
}

