#include "mesh.h"
#include "timer.h"
#include "input.h"

#include "camera.h"
#include "orbit-camera.h"
#include "first-person-camera.h"
#include "terrain/terrain.h"
#include "terrain/terrain-physics.h"

#include <iostream>
#include <GLFW/glfw3.h>

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
    /*
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
        (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
        type, severity, message);
        */
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
  gWidth = std::max(width, 1);
  gHeight = std::max(height, 1);
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

    std::cout << glGetString(GL_RENDERER) << std::endl;
    std::cout << glGetString(GL_VENDOR) << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    glClearColor(0.5f, 0.6f, 0.7f, 1.0f);

    // Initialize singleton
    Timer* timer = Timer::getInstance();
    timer->initialize();

    Input* input = Input::getInstance();
    input->initialize();

    FullScreenQuad* fullScreenQuad = FullScreenQuad::getInstance();
    fullScreenQuad->initialize();

    FirstPersonCamera* camera = new FirstPersonCamera();
    camera->initialize();
    camera->setPosition(glm::vec3(128.0f, 200.0f, 128.0f));

    Terrain* terrain = Terrain::getInstance();
    terrain->initialize(camera);

    TerrainPhysics* physicsManager = terrain->getPhysicsManager();

    // Instanced Shader
    uint32_t shaders[2] = {};
    shaders[0] = gl::createShader("Assets/SPIRV/instanced-mesh.vert.spv");
    shaders[1] = gl::createShader("Assets/SPIRV/instanced-mesh.frag.spv");
    uint32_t instancedShader = gl::createProgram(shaders, 2);
    gl::destroyShader(shaders[0]);
    gl::destroyShader(shaders[1]);

    shaders[0] = gl::createShader("Assets/SPIRV/main.vert.spv");
    shaders[1] = gl::createShader("Assets/SPIRV/main.frag.spv");
    unsigned int drawShader = gl::createProgram(shaders, 2);
    gl::destroyShader(shaders[0]);
    gl::destroyShader(shaders[1]);

    unsigned int globalUBO = gl::createBuffer(nullptr, sizeof(GlobalUniforms), GL_DYNAMIC_STORAGE_BIT);

    bool enableWireframe = false;
    float dt = (float)timer->getDeltaSeconds();
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glViewport(0, 0, gWidth, gHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (input->wasKeyPressed(GLFW_KEY_T))
            enableWireframe = !enableWireframe;

        if (enableWireframe)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else 
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


        if (input->isKeyDown(GLFW_MOUSE_BUTTON_LEFT)) {
            glm::vec2 delta = input->getMouseDelta();
            camera->rotate(delta.y, -delta.x, dt);
        }

        float walkSpeed = 10.0f;
        if (input->isKeyDown(GLFW_KEY_LEFT_SHIFT))
            walkSpeed *= 5.0f;

        if (input->isKeyDown(GLFW_KEY_W))
            camera->walk(-dt * walkSpeed);
        else if (input->isKeyDown(GLFW_KEY_S))
            camera->walk(dt * walkSpeed);
        if (input->isKeyDown(GLFW_KEY_A))
            camera->strafe(-dt * walkSpeed);
        else if (input->isKeyDown(GLFW_KEY_D))
            camera->strafe(dt * walkSpeed);
        if (input->isKeyDown(GLFW_KEY_1))
            camera->lift(dt * walkSpeed);
        else if (input->isKeyDown(GLFW_KEY_2))
            camera->lift(-dt * walkSpeed);

        if (input->wasKeyPressed(GLFW_KEY_F)) {
            Rigidbody rb = {};
            rb.position = camera->getPosition();
            rb.prevPosition = rb.position;
            rb.velocity = -camera->getForward() * 30.0f;
            physicsManager->addRigidbody(rb);
        }

        camera->setAspect(float(gWidth) / float(gHeight));
        camera->update(dt);

        terrain->update(dt);


        gGlobalUniforms.P = camera->getProjectionMatrix();
        gGlobalUniforms.V = camera->getViewMatrix();
        gGlobalUniforms.InvVP = glm::inverse(gGlobalUniforms.P * gGlobalUniforms.V);
        gGlobalUniforms.cameraPosition = glm::vec4(camera->getPosition(), 1.0f);
        glNamedBufferSubData(globalUBO, 0, sizeof(GlobalUniforms),  &gGlobalUniforms);

        // Draw Terrain
        glUseProgram(drawShader);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, globalUBO);
        terrain->render();

        // Draw Spheres
        glUseProgram(instancedShader);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, globalUBO);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, physicsManager->mRBUniformBuffer);
        DefaultMesh::getInstance()->getSphere()->drawInstanced(physicsManager->mRBCount);
        glUseProgram(0);
      
        glfwSwapBuffers(window);
        dt = (float)timer->tick();

        if (input->isKeyDown(GLFW_KEY_ESCAPE))
            glfwSetWindowShouldClose(window, true);

        input->update();
    }

    terrain->destroy();

    fullScreenQuad->destroy();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
