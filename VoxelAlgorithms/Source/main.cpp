#include "mesh.h"
#include "timer.h"
#include "input.h"
#include "orbit-camera.h"

#include "voxel-chunk.h"
#include "voxel-algorithm/raymarcher.h"
#include "voxel-algorithm/marchingcube.h"
#include "voxel-algorithm/marching-cube-cpu.h"

#include <iostream>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

uint32_t gWidth = 1360;
uint32_t gHeight = 769;

void GLAPIENTRY
MessageCallback(GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam)
{
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
        (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
        type, severity, message);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Input* input = Input::getInstance();
    if (action == GLFW_RELEASE)
        input->setKeyState(key, false);
    else 
        input->setKeyState(key, true);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    Input* input = Input::getInstance();
    if (action == GLFW_RELEASE)
        input->setKeyState(button, false);
    else
        input->setKeyState(button, true);
}

static void mouse_pos_callback(GLFWwindow* window, double x, double y) {
    Input* input = Input::getInstance();
    input->setMousePosition((float)x, (float)y, gWidth, gHeight);
}

static void scroll_callback(GLFWwindow* window, double xOffset, double yOffset) {
    Input* input = Input::getInstance();
    input->setScrollOffset((float)xOffset, (float)yOffset);
}

static void window_resize_callback(GLFWwindow* window, int width, int height) {
  gWidth = width;
  gHeight = height;
}

struct GlobalUniforms {
    glm::mat4 P;
    glm::mat4 V;
    glm::mat4 InvVP;
    glm::vec4 cameraPosition;
} gGlobalUniforms;

int main()
{
    if (!glfwInit())
        glfwInit();

    glfwWindowHint(GLFW_SAMPLES, 8);

    GLFWwindow* window = glfwCreateWindow(gWidth, gHeight, "Hello World", 0, 0);
    if (!window)
    {
        std::cerr << "Failed to create GLFW Window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwSwapInterval(1);
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);
    glfwSetWindowSizeCallback(window, window_resize_callback);
    glfwSetCursorPosCallback(window, mouse_pos_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

	glEnable(GL_CULL_FACE);
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_TEXTURE_3D);
    glEnable(GL_DEPTH_TEST);
    glDebugMessageCallback(MessageCallback, 0);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Initialize singleton
    Timer* timer = Timer::getInstance();
    timer->initialize();

    Input* input = Input::getInstance();
    input->initialize();

    FullScreenQuad* fullScreenQuad = FullScreenQuad::getInstance();
    fullScreenQuad->initialize();

    // Initialize shaders
    GLuint densityCS = gl::createShader("Assets/SPIRV/build-density.comp.spv");
    GLuint densityShader = gl::createProgram(&densityCS, 1);
    gl::destroyShader(densityCS);

    DensityParams params = { 1.0f, 1.0f, 0.5f, 0.5f, 5 };
    DensityBuilder densityBuilder(params, densityShader);

    const int CHUNK_SIZE = 256;
    VoxelChunk voxelChunk{ CHUNK_SIZE, 1 };
    voxelChunk.generateDensities(&densityBuilder);

	uint32_t vs = gl::createShader("Assets/SPIRV/main.vert.spv");
	uint32_t fs = gl::createShader("Assets/SPIRV/main.frag.spv");
	uint32_t shaders[] = { vs, fs };
	unsigned int drawShader = gl::createProgram(shaders, 2);
	gl::destroyShader(vs);
	gl::destroyShader(fs);

    VoxelRenderer* renderer = new MarchingCube(&voxelChunk, drawShader);
    renderer->initialize();

    OrbitCamera* camera = new OrbitCamera();
    camera->setTarget(glm::vec3(CHUNK_SIZE * 0.5f));
    camera->setZoom(CHUNK_SIZE * 0.5f);
    camera->initialize();

    float dt = (float)timer->getDeltaSeconds();

    bool enableWireframe = false;
    unsigned int globalUBO = gl::createBuffer(nullptr, sizeof(GlobalUniforms), GL_DYNAMIC_STORAGE_BIT);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glViewport(0, 0, gWidth, gHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (input->wasKeyPressed(GLFW_KEY_W))
            enableWireframe = !enableWireframe;

        if (enableWireframe)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else 
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


        if (input->isKeyDown(GLFW_MOUSE_BUTTON_LEFT)) {
            glm::vec2 delta = input->getMouseDeltaNormalized();
            camera->changeRotation(delta.y, delta.x);
        }

        if (input->isScrolled()) {
            glm::vec2 scrollOffset = input->getScrollOffset();
            camera->changeZoom(-scrollOffset.y);
        }

        camera->update(dt);
        camera->setAspect(float(gWidth) / float(gHeight));

        gGlobalUniforms.P = camera->getProjectionMatrix();
        gGlobalUniforms.V = camera->getViewMatrix();
        gGlobalUniforms.InvVP = glm::inverse(gGlobalUniforms.P * gGlobalUniforms.V);
        gGlobalUniforms.cameraPosition = glm::vec4(camera->getPosition(), 1.0f);
        glNamedBufferSubData(globalUBO, 0, sizeof(GlobalUniforms),  &gGlobalUniforms);

        renderer->setSize(gWidth, gHeight);
        renderer->render(camera, globalUBO);
      
        glfwSwapBuffers(window);
        dt = (float)timer->tick();

        if (input->isKeyDown(GLFW_KEY_ESCAPE))
            glfwSetWindowShouldClose(window, true);

        input->update();
    }


    renderer->destroy();
    fullScreenQuad->destroy();
    gl::destroyProgram(densityShader);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
