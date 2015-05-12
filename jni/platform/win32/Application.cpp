#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "platform/CwqGL.h"
#include "base/LogHelper.h"
#include "engine/CwqEngine.h"

static const int WIDTH = 768;
static const int HEIGHT = 1024;

static void error_callback(int error, const char* description)
{
    LOGE("error code: %d, description: %s", error, description);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if(button == GLFW_MOUSE_BUTTON_LEFT) {
        //left mouse button
        double xpos, ypos;
        switch (action)
        {
        case GLFW_PRESS:
            glfwGetCursorPos(window, &xpos, &ypos);
            LOGD("press: (%f,%f)", xpos, ypos);
            break;;
        case GLFW_RELEASE:
            glfwGetCursorPos(window, &xpos, &ypos);
            LOGD("release: (%f,%f)", xpos, ypos);
            break;
        default:
            break;
        }
    }
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    LOGD("move: (%f,%f)", xpos, ypos);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    LOGD("key: %d, action: %d", key, action);
}

int main()
{
    GLFWwindow* window;

    glfwSetErrorCallback(error_callback);

    /* Initialize the library */
    if(!glfwInit())
    {
        LOGE("glfwInit error");
        return -1;
    }

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(WIDTH, HEIGHT, "CwqEngine", NULL, NULL);
    if(!window)
    {
        glfwTerminate();
        LOGE("glfwCreateWindow error");
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetKeyCallback(window, key_callback);

    /* in win32 to use opengl hight level must call glewInit */
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        LOGE("GLEW Error: %s", glewGetErrorString(err));
        return -1;
    }

    CwqEngine engine;
    engine.onSurfaceChanged(WIDTH, HEIGHT);
    engine.onSurfaceCreated();
    engine.onResume();

    /* Loop until the user closes the window */
    while(!glfwWindowShouldClose(window))
    {
        /* Render here */
        engine.onDrawFrame();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    engine.onExit();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
