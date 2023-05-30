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
#include "CGL/scenegraph/Camera.h"

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

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // TODO: figure out how to get access to active window object
    // Window::GetInstance().SetViewSize(width, height);
    glViewport(
        0, 0,  // index of lower left corner of viewport, in pixels
        width, height
    );
}

static void processKeyboadInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
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

Window::Window(int viewWidth, int viewHeight) : m_ViewWidth(2400), m_ViewHeight(1800), m_DeltaTime(0.0f)
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
    glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // glfwSetCursorPosCallback(window, mouse_callback);


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
    Renderer renderer;

    // Scene setup ==========================================
	PerspectiveCamera camera(float(m_ViewWidth) / (float)m_ViewHeight); // the camera copy
    camera.SetPosition(0.0f, 0.0f, 3.0f);

    Scene scene;

    BoxGeometry boxGeo;
    // SphereGeometry sphereGeo;

    NormalMaterial normMat;
    normMat.SetWireFrame(true);

    Group sunSystem, earthSystem, moonSystem;

    earthSystem.SetPosition(glm::vec3(2.2f, 0.0f, 0.0f));
    moonSystem.SetPosition(.55f, 0.0f, 0.0f);

    Mesh sun(&boxGeo, nullptr);
    sun.SetScale(glm::vec3(2.0f));

    Mesh earth(&boxGeo, &normMat);
    earth.SetScale(glm::vec3(0.4f));

    Mesh moon(&boxGeo, &normMat);
    moon.SetScale(glm::vec3(0.12f));

    // create graph
    scene.AddChild(&sunSystem);

    sunSystem.AddChild(&sun);
    sunSystem.AddChild(&earthSystem);

    earthSystem.AddChild(&earth);
    earthSystem.AddChild(&moonSystem);

    moonSystem.AddChild(&moon);

    // Copy from CGL scenegraph ====================================    
    scene.SetID(CGL::mainScene.GetID());  // copy scene ID
    scene.RegisterNode(&scene);  // register itself
    camera.SetID(CGL::mainCamera.GetID());  // copy maincam ID
    scene.RegisterNode(&camera);  // register camera



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

        // wait to render until instructed by chuck
        // TODO: will probably have to re-enable this

        // chuck done with writes, renderer is good to read from the scene graph!

        {  // rotate the universe
            float radius = 2.0f;
            float posX = radius * glm::sin(currentTime);
            float posY = radius * glm::cos(currentTime);

            sunSystem.SetRotation(glm::vec3(0.0f, .5f * currentTime, 0.0f));
            earthSystem.SetRotation(glm::vec3(0.0f, .7f * currentTime, 0.0f));

            sun.SetRotation(glm::vec3(0.0f, .1f * currentTime, 0.0f));
            earth.SetRotation(glm::vec3(0.0f, .4f * currentTime, 0.0f));
            moon.SetRotation(glm::vec3(0.0f, .9f * currentTime, 0.0f));
        }

        // camera.SetPosition(CGL::mainCamera.GetPosition());
        // camera.SetRotation(CGL::mainCamera.GetRotation());
        
        CGL::WaitOnUpdateDone();
        /*
        Note: this sync mechanism also gets rid of the problem where chuck runs away
        e.g. if the time it takes the renderer flush the queue is greater than
        the time it takes chuck to write, ie write rate > flush rate,
        each command queue will get longer and longer, continually worsening performance
        shouldn't happen because chuck runs in vm and the flushing happens natively but 
        you never know
        */
        /*
        it is redundant to have two locks here:
        1 for writing/swapping the command queues
        1 for the condition_var used to synchronize
        technically if we use the condition var the command queue lock 
        is unnecessary because the queue will only be swapped when chuck is blocked
        on receiving an UpdateEvent from the renderer
        but having the command queue lock protects against the case where 
        people write CGL code outside of the sync context 
        anyways the overhead is so low (~100ns) its not important

        deadlock shouldn't happen because both locks are never held at the same time
        */
        { // critical section: swap command queus
            CGL::SwapCommandQueues();
        }

        // done deepcopying, let chuck know it's good to continue pushing commands
        CglEvent::Broadcast(CglEventType::CGL_UPDATE);

        // now apply changes from the command queue chuck is NO Longer writing to 
        CGL::FlushCommandQueue(scene, false);

        // now renderer can work on drawing the copied scenegraph ===

        // OpenGL Rendering Commands =========================
        // clear screen
        renderer.Clear();

        // flush command queue!
        // renderer.FlushCommandQueue(scene);

        // draw call
        renderer.RenderScene(&scene, &camera);


        // Handle Events, Draw framebuffer
        glfwPollEvents();
        //glfwWaitEvents();  // blocking version of PollEvents. How does blocking work? "puts the calling thread to sleep until at least one event is available in the event queue" https://www.glfw.org/docs/latest/group__window.html#ga15a5a1ee5b3c2ca6b15ca209a12efd14
        glfwSwapBuffers(m_Window);  // blocks until glfwSwapInterval screen updates have occured
    }
}

