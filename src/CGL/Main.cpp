
#include <iostream>

#include "Includes.h"
#include "Shader.h"
#include "Util.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Window.h"
#include "Renderer.h"
#include "Texture.h"
#include "scenegraph/SceneGraphObject.h"

// glm includes
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// globals, should move into some Manager class
static int viewWidth{ 800 }, viewHeight{ 600 };
static SceneGraphObject camera;
static float deltaTime = 0.0f;

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    viewWidth = width;
    viewHeight = height;
    glViewport(
        0, 0,  // index of lower left corner of viewport, in pixels
        width, height
    );
}

static void processKeyboadInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    const float cameraSpeed = 2.5f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.Translate(cameraSpeed * camera.GetForward());
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.Translate(-cameraSpeed * camera.GetForward());
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.Translate(-cameraSpeed * camera.GetRight());
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.Translate(cameraSpeed * camera.GetRight());

}

bool drawFill = true;
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


int main(void)
{

    // Create window object ==================================
    // Note: you MUST initialize this window context before any openGL classes so that the Window destructor is called LAST
    // this is so that glfwTerminate() is called AFTER all openGL functions
    // glfwTerminate() destroys the openGL context, and calling an openGL 
    // function with no context will raise glGetError(), which causes an infinite loop...
    Window windowObj;
    GLFWwindow* window = windowObj.GetWindow();
    
    // Initialize GLAD =======================================
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // OpenGL Viewport and Callbacks =========================
    glViewport(0, 0, viewWidth, viewHeight);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, keyCallback);

    // VSYNC settings ========================================
    //glfwSwapInterval(0);  // disable
    glfwSwapInterval(1);  // enable (Default) glfwSwapBuffers will wait for N screen updates before swapping buffers and returning
    
    // mouse settings =====================================
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);



    // OpenGL Metadata =======================================
    int nrAttributes;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
    std::cout << "Maximum number of 4-component vertex attributes: " << nrAttributes << std::endl;

    // Mesh Data ==================
    /*
    float vertices[] = {
     // positions          // colors            // texture UVs
     0.5f,  0.5f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 1.0f,      // top right
     0.5f, -0.5f, 0.0f,    1.0f, 0.0f, 0.0f,    1.0f, 0.0f,      // bottom right
    -0.5f, -0.5f, 0.0f,    0.0f, 0.0f, 0.0f,    0.0f, 0.0f,      // bottom left
    -0.5f,  0.5f, 0.0f,    0.0f, 1.0f, 0.0f,    0.0f, 1.0f      // top left 
    };
    */
    
    float vertices[] = {  // for cube
    // position           // texture UV
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };

    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };

    // setting up model matrix
    //glm::quat q = glm::angleAxis(
    //    glm::radians(30.0f),
    //    glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f))
    //);
    //glm::mat4 model = glm::mat4(1.0f) * glm::mat4_cast(q);
    glm::mat4 model = glm::mat4(1.0f);

    // setting up view matrix
    // (not openGL is right-handed system, +z axis is coming OUT of screen
    // note: in NDC space OpenGL actually uses a left-handed system. sigh...
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

    
    // projection matrix (move into Camera) =====================================
    // https://webglfundamentals.org/webgl/lessons/webgl-visualizing-the-camera.html
    glm::ortho(
        0.0f, 800.0f,  // left/right of view frustrum
        0.0f, 600.0f,  // bottom/top
        0.1f, 100.0f   // near and far plane
    );
    
    // TODO: eventually want to update camera aspect in window resize callback
    glm::mat4 projection = glm::perspective(  // gen perspective proj matrix, transforms from view space --> clip space
        glm::radians(45.0f),                    // fov (45 mimics human eye with average size display sitting at avg distance away)
        (float)viewWidth / (float)viewHeight,   // aspect ratio
        0.1f,                                   // near plane
        100.0f                                  // far plane
    );

    // Camera
    camera.SetPosition(0.0, 0.0, 3.0f);
    view = camera.GetInvModelMatrix();



    // Vertex Array Obj setup ===========
    VertexArray va;

    VertexBuffer vb(
        (void *)vertices, 
        sizeof(vertices),  // size in bytes
        sizeof(vertices) / sizeof(vertices[0]),  // num elements
        GL_STATIC_DRAW
    );

    VertexBufferLayout layout;
    layout.Push(GL_FLOAT, 3, false);  // position data
    // layout.Push(GL_FLOAT, 0, true);  // color data
    layout.Push(GL_FLOAT, 2, true);  // uv data

    va.AddBufferAndLayout(vb, layout);  // add vertex attrib pointers to VAO state

    // Element Array Buffer setup
    // IndexBuffer ib(indices, 6, GL_STATIC_DRAW);
    // va.AddIndexBuffer(ib);  // add index buffer to VAO state
    
    // Texture ===========================================
    Texture texture0("./res/textures/wall.jpg");
    Texture texture1("./res/textures/container.jpg");
    Texture texture2("./res/textures/awesomeface.png");
    texture0.Bind(0);
    texture1.Bind(1);
    texture2.Bind(2);

    // Shader ============================================
    Shader basicShader("./res/shaders/BasicVert.glsl", "./res/shaders/BasicFrag.glsl");
    basicShader.Bind();

    // Shader Uniforms ================================
    basicShader.setTextureUnits(3);

    basicShader.setMat4f("u_Model", model);
    basicShader.setMat4f("u_View", camera.GetInvModelMatrix());
    basicShader.setMat4f("u_Projection", projection);


    



    // Renderer ====================================
    Renderer renderer;
    

    // Blending (TODO) should this be part of renderer? =========
    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    // depth testing
    GLCall(glEnable(GL_DEPTH_TEST));

    // Culling (TODO) should be part of renderer too
    //GLCall(glEnable(GL_CULL_FACE));
    //GLCall(glCullFace(GL_BACK));


    // Render Loop ===========================================
    float previousFPSTime = (float) glfwGetTime();
    float prevTickTime = previousFPSTime;
    int frameCount = 0;
    while (!glfwWindowShouldClose(window))
    {
        // input
        processKeyboadInput(window);
        
        // FPS counter
        float currentTime = (float)glfwGetTime();
        // deltaTime
        deltaTime = currentTime - prevTickTime;
        prevTickTime = currentTime;
        ++frameCount;
        if (currentTime - previousFPSTime >= 1.0f) {
            Util::println(std::to_string(frameCount));
            frameCount = 0;
            previousFPSTime = currentTime;
        }

        // OpenGL Rendering Commands =========================

        // clear screen
        renderer.Clear();

        // shader hot reloading (not working :(
         //if (glfwGetKey(window, GLFW_KEY_R)) {
         //   basicShader.Reload();
         //}

        // setup shader pipeline
        float greenValue = (sin(currentTime* 2.0f) / 2.0f) + 0.5f;
        // Note: updating a uniform does require you to first use the program(by calling glUseProgram), 
        // because it sets the uniform on the currently active shader program.

        // update()
        basicShader.setFloat4("u_Color", 0.0f, greenValue, 0.0f, 1.0f);
        
        glm::mat4 trans = glm::mat4(1.0f);  // initialize identity matrix
		model = glm::rotate(model, glm::radians(20*deltaTime), glm::vec3(0.0, 0.0, 1.0));  // 90 deg rotation matrix
		model = glm::rotate(model, glm::radians(21*deltaTime), glm::vec3(1.0, 0.0, 0.0));  // 90 deg rotation matrix
		basicShader.setMat4f("u_Model", model);

        // camera.LookAt(glm::vec3(0.0f, 0.0f, 0.0f));
		basicShader.setMat4f("u_View", camera.GetInvModelMatrix());


        // draw call
        renderer.Draw(va, basicShader);
        

        // Handle Events, Draw framebuffer
        glfwPollEvents();
        //glfwWaitEvents();  // blocking version of PollEvents. How does blocking work? "puts the calling thread to sleep until at least one event is available in the event queue" https://www.glfw.org/docs/latest/group__window.html#ga15a5a1ee5b3c2ca6b15ca209a12efd14
        glfwSwapBuffers(window);  // blocks until glfwSwapInterval screen updates have occured
    }

    return 0;
}