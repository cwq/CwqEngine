#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "platform/CwqGL.h"
#include "base/LogHelper.h"
#include "engine/CwqEngine.h"

//Window size must be smaller screen resolution
//if not glfwGetCursorPos will get wrong position
static const int WIDTH = 600;
static const int HEIGHT = 600;

static void error_callback(int error, const char* description)
{
    LOGE("error code: %d, description: %s", error, description);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    CwqEngine* engine = (CwqEngine*)glfwGetWindowUserPointer(window);
    if(button == GLFW_MOUSE_BUTTON_LEFT) {
        //left mouse button
        double xpos, ypos;
        switch (action)
        {
        case GLFW_PRESS:
            glfwGetCursorPos(window, &xpos, &ypos);
            if (engine)
            {
                engine->onTouchesBegin(button, xpos, ypos);
            }
            break;;
        case GLFW_RELEASE:
            glfwGetCursorPos(window, &xpos, &ypos);
            if (engine)
            {
                engine->onTouchesEnd(button, xpos, ypos);
            }
            break;
        default:
            break;
        }
    }
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    int num = 1;
    int pIDs[1] = {0};
    float pXs[1] = {xpos};
    float pYs[1] = {ypos};
    CwqEngine* engine = (CwqEngine*)glfwGetWindowUserPointer(window);
    if (engine)
    {
        engine->onTouchesMove(pIDs, pXs, pYs, num);
    }
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    LOGD("key: %d, action: %d", key, action);
    CwqEngine* engine = (CwqEngine*)glfwGetWindowUserPointer(window);
    if (engine && action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_P)
        {
            engine->onPause();
        }
        if (key == GLFW_KEY_R)
        {
            engine->onResume();
        }
        if (key == GLFW_KEY_A)
        {
            engine->postEventToEngine(true, 100, 0, 0, NULL);
        }
    }

}

static void close_callback(GLFWwindow* window)
{
    CwqEngine* engine = (CwqEngine*)glfwGetWindowUserPointer(window);
    if (engine)
    {
        engine->onExit();
    }
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
    glfwSetWindowCloseCallback(window, close_callback);

    /* in win32 to use opengl hight level must call glewInit */
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        LOGE("GLEW Error: %s", glewGetErrorString(err));
        return -1;
    }

    CwqEngine engine;

    glfwSetWindowUserPointer(window, &engine);

    engine.onSurfaceCreated(WIDTH, HEIGHT);
    engine.onSurfaceChanged(WIDTH, HEIGHT);
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

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
