#include <iostream>
#include <fstream>
#include <string>

#include <GLAD\glad.h>
#include <GLFW\glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "model.h"

#define STB_IMAGE_IMPLEMENTATION
#include "STB\stb_image.h"

#include "utils.h"

// Forward Declarations
void renderTerrain(glm::mat4 view, glm::mat4 projection);
void renderSkybox(glm::mat4 view, glm::mat4 projection);
void setupResources();
void renderModel(Model* model, unsigned int shader, glm::vec3 position, glm::vec3 rotation, float scale, glm::mat4 view, glm::mat4 projection);

glm::vec3 cameraPosition(100, 100, 100), cameraForward(0, 0, 1), cameraUp(0, 1, 0);

unsigned int plane, planeSize, VAO, cubeSize;
unsigned int quadVAO, quadVBO;
unsigned int myProgram, skyProgram, modelProgram, screenProgram;

// Textures
unsigned int heightmapID, heightmapNormalID, rockID, snowID, grassID;

Model* grass;

class Grass {
public:
    Model* grass = nullptr;
    glm::vec3 position = glm::vec3(0, 0, 0);
    glm::vec3 rotation = glm::vec3(0, 0, 0);
    float scale = 1;

    Grass() {

    }

    Grass(Model* grass, glm::vec3 position, glm::vec3 rotation, float scale) {
        this->grass = grass;
        this->position = position;
        this->rotation = rotation;
        this->scale = scale;
    }
};

const int grassCount = 2048;
Grass patches[grassCount];

const int screenWidth = 1280;
const int screenHeight = 720;

unsigned int fbo;
unsigned int textureColorbuffer;


void distributeGrass();
void renderGrass(unsigned int shader, Grass patches[], glm::mat4 view, glm::mat4 projection);

void handleInput(GLFWwindow* window, float deltaTime)
{
    static int w, s, a, d;
    static int space, ctrl;
    static double cursorX = -1, cursorY = -1, lastCursorX, lastCursorY;
    static float pitch, yaw;
    static float speed = 100.0f;

    float sensitivity = 20.0f * deltaTime;
    float step = speed * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)				w =  1;
    else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE)		w =  0;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)				s = -1;
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE)		s =  0;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)				a =  1;
    else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE)		a =  0;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)				d = -1;
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE)		d =  0;

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)				space = 1;
    else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)		space = 0;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)		ctrl  =-1;
    else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE)	ctrl  = 0;

    if (cursorX == -1) {
        glfwGetCursorPos(window, &cursorX, &cursorY);
    }

    lastCursorX = cursorX;
    lastCursorY = cursorY;
    glfwGetCursorPos(window, &cursorX, &cursorY);

    glm::vec2 mouseDelta(cursorX - lastCursorX, cursorY - lastCursorY);

    // TODO: calculate rotation & movement
    yaw += -mouseDelta.x * sensitivity;
    pitch += mouseDelta.y * sensitivity;

    pitch = glm::max(glm::min(pitch, 90.0f), -90.0f);

    if (yaw < -180.0f) yaw += 360;
    else if (yaw > 180) yaw -= 360;

    glm::vec3 euler(glm::radians(pitch), glm::radians(yaw), 0);
    glm::quat q(euler);
    
    // update camera position / forward & up
    glm::vec3 translation(a + d, 0, w + s);
    cameraPosition += q * translation * step;

    cameraPosition += glm::vec3(0, space + ctrl, 0);

    cameraUp = q * glm::vec3(0, 1, 0);
    cameraForward = q * glm::vec3(0, 0, 1);
}

int main()
{
    static double previousT = 0;

    //std::cout << "Wayo.\n";

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow * window = glfwCreateWindow(screenWidth, screenHeight, "Cyberpunk 2077", nullptr, nullptr);

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, screenWidth, screenHeight);

    setupResources();
    distributeGrass();

    // OPENGL SETTINGS //
    glEnable(GL_CULL_FACE);


    while (!glfwWindowShouldClose(window))
    {
        float t = (float)glfwGetTime();

        handleInput(window, t - (float)previousT);
        previousT = t;

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glEnable(GL_DEPTH_TEST);

        glm::mat4 view = glm::lookAt(cameraPosition, cameraPosition + cameraForward, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(60.0f), screenWidth / (float)screenHeight, 0.1f, 1000.0f);

        // iets tekenen
        glClearColor(0.2, 0.2, 0.2, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render Scene
        renderSkybox(view, projection);
        renderTerrain(view, projection);

        //for (int i = 0; i < grassCount; i++) {
        //    Grass myGrass = patches[i];
        //    renderModel(myGrass.grass, modelProgram, myGrass.position, myGrass.rotation, myGrass.scale, view, projection);
        //}

        renderGrass(modelProgram, patches, view, projection);

        //glEnable(GL_BLEND);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        //glDisable(GL_BLEND);


        glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
        glDisable(GL_DEPTH_TEST);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(screenProgram);
        glBindVertexArray(quadVAO);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void renderTerrain(glm::mat4 view, glm::mat4 projection) {
    glUseProgram(myProgram);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);

    glm::mat4 world = glm::mat4(1.f);
    world = glm::translate(world, glm::vec3(0, 0, 0));

    glUniformMatrix4fv(glGetUniformLocation(myProgram, "world"), 1, GL_FALSE, glm::value_ptr(world));
    glUniformMatrix4fv(glGetUniformLocation(myProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(myProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(glGetUniformLocation(myProgram, "cameraPosition"), 1, glm::value_ptr(cameraPosition));

    float t = glfwGetTime();
    t = t * 0.1f;
    glUniform3f(glGetUniformLocation(myProgram, "lightDirection"), glm::cos(t), -0.5f, glm::sin(t));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, heightmapID);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, heightmapNormalID);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, rockID);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, snowID);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, grassID);

    glBindVertexArray(plane);
    glDrawElements(GL_TRIANGLES, planeSize, GL_UNSIGNED_INT, 0);
}

void distributeGrass() {
    int width, height, comps;
    unsigned char* data = stbi_load("heightmap.png", &width, &height, &comps, 4);

    for (int i = 0; i < grassCount; i++) {
        int x, z;
        float y;
        x = rand() % width;
        z = rand() % height;
        y = data[(width * z + x) * 4] / 255.0f;
        y *= 100.f;

        glm::vec3 position = glm::vec3(x, y, z);
        patches[i] = Grass(grass, position, glm::vec3(0, 0, 0), 1);
    }
}

void renderSkybox(glm::mat4 view, glm::mat4 projection) {
    // SKYBOX
    glUseProgram(skyProgram);
    glCullFace(GL_FRONT);
    glDisable(GL_DEPTH_TEST);

    glm::mat4 world = glm::mat4(1.f);
    world = glm::translate(world, cameraPosition);

    glUniformMatrix4fv(glGetUniformLocation(skyProgram, "world"), 1, GL_FALSE, glm::value_ptr(world));
    glUniformMatrix4fv(glGetUniformLocation(skyProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(skyProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(glGetUniformLocation(skyProgram, "cameraPosition"), 1, glm::value_ptr(cameraPosition));

    float t = glfwGetTime();
    t = t * 0.1f;
    glUniform3f(glGetUniformLocation(skyProgram, "lightDirection"), glm::cos(t), -0.5f, glm::sin(t));

    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, diffuseTexID);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, cubeSize, GL_UNSIGNED_INT, 0);
}

void setupResources() {
    /// SETUP OBJECT ///
    stbi_set_flip_vertically_on_load(true);

    grass = new Model("grass/GrassPatch.obj");

// need 24 vertices for normal/uv-mapped Cube

    float vertices[] = {
        // positions            //colors            // tex coords   // normals
        0.5f, -0.5f, -0.5f,     1.0f, 1.0f, 1.0f,   1.f, 0.f,       0.f, -1.f, 0.f,
        0.5f, -0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,       0.f, -1.f, 0.f,
        -0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 1.0f,   0.f, 1.f,       0.f, -1.f, 0.f,
        -0.5f, -0.5f, -.5f,     1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, -1.f, 0.f,

        0.5f, 0.5f, -0.5f,      1.0f, 1.0f, 1.0f,   2.f, 0.f,       1.f, 0.f, 0.f,
        0.5f, 0.5f, 0.5f,       1.0f, 1.0f, 1.0f,   2.f, 1.f,       1.f, 0.f, 0.f,

        0.5f, 0.5f, 0.5f,       1.0f, 1.0f, 1.0f,   1.f, 2.f,       0.f, 0.f, 1.f,
        -0.5f, 0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   0.f, 2.f,       0.f, 0.f, 1.f,

        -0.5f, 0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   -1.f, 1.f,      -1.f, 0.f, 0.f,
        -0.5f, 0.5f, -.5f,      1.0f, 1.0f, 1.0f,   -1.f, 0.f,      -1.f, 0.f, 0.f,

        -0.5f, 0.5f, -.5f,      1.0f, 1.0f, 1.0f,   0.f, -1.f,      0.f, 0.f, -1.f,
        0.5f, 0.5f, -0.5f,      1.0f, 1.0f, 1.0f,   1.f, -1.f,      0.f, 0.f, -1.f,

        -0.5f, 0.5f, -.5f,      1.0f, 1.0f, 1.0f,   3.f, 0.f,       0.f, 1.f, 0.f,
        -0.5f, 0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   3.f, 1.f,       0.f, 1.f, 0.f,

        0.5f, -0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,       0.f, 0.f, 1.f,
        -0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 1.0f,   0.f, 1.f,       0.f, 0.f, 1.f,

        -0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 1.0f,   0.f, 1.f,       -1.f, 0.f, 0.f,
        -0.5f, -0.5f, -.5f,     1.0f, 1.0f, 1.0f,   0.f, 0.f,       -1.f, 0.f, 0.f,

        -0.5f, -0.5f, -.5f,     1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, 0.f, -1.f,
        0.5f, -0.5f, -0.5f,     1.0f, 1.0f, 1.0f,   1.f, 0.f,       0.f, 0.f, -1.f,

        0.5f, -0.5f, -0.5f,     1.0f, 1.0f, 1.0f,   1.f, 0.f,       1.f, 0.f, 0.f,
        0.5f, -0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,       1.f, 0.f, 0.f,

        0.5f, 0.5f, -0.5f,      1.0f, 1.0f, 1.0f,   2.f, 0.f,       0.f, 1.f, 0.f,
        0.5f, 0.5f, 0.5f,       1.0f, 1.0f, 1.0f,   2.f, 1.f,       0.f, 1.f, 0.f
    };

    unsigned int indices[] = {  // note that we start from 0!
        // DOWN
        0, 1, 2,   // first triangle
        0, 2, 3,    // second triangle
        // BACK
        14, 6, 7,   // first triangle
        14, 7, 15,    // second triangle
        // RIGHT
        20, 4, 5,   // first triangle
        20, 5, 21,    // second triangle
        // LEFT
        16, 8, 9,   // first triangle
        16, 9, 17,    // second triangle
        // FRONT
        18, 10, 11,   // first triangle
        18, 11, 19,    // second triangle
        // UP
        22, 12, 13,   // first triangle
        22, 13, 23,    // second triangle
    };

    cubeSize = sizeof(indices);

    glGenVertexArrays(1, &VAO);

    unsigned int VBO;
    glGenBuffers(1, &VBO);

    unsigned int EBO;
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    /// END GENERATE FRAME BUFFER

    int stride = sizeof(float) * 11;

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    // Color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);

    // UV
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 6));
    glEnableVertexAttribArray(2);

    // Normal
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 8));
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    /// END SETUP OBJECT ///

    plane = GeneratePlane("heightmap.png", GL_RGBA, 4, 1.0f, 1.0f, planeSize, heightmapID);

    // terrainTextures
    heightmapNormalID = loadTexture("heightmapNormal.png", GL_RGBA, 4);
    snowID = loadTexture("snow.jpg", GL_RGB, 3);
    grassID = loadTexture("grass.png", GL_RGBA, 4);
    rockID = loadTexture("rock.jpg", GL_RGB, 3);

    /// SETUP SHADER PROGRAM ///

    unsigned int vertexSource, fragmentSource;
    CreateShader("vertexShader.shader", GL_VERTEX_SHADER, vertexSource);
    CreateShader("fragmentShader.shader", GL_FRAGMENT_SHADER, fragmentSource);

    unsigned int vertSky, fragSky;
    CreateShader("vertexShaderSky.shader", GL_VERTEX_SHADER, vertSky);
    CreateShader("fragmentShaderSky.shader", GL_FRAGMENT_SHADER, fragSky);

    unsigned int vertModel, fragModel;
    CreateShader("vertModel.shader", GL_VERTEX_SHADER, vertModel);
    CreateShader("fragModel.shader", GL_FRAGMENT_SHADER, fragModel);

    unsigned int vertScreen, fragScreen;
    CreateShader("vertScreen.shader", GL_VERTEX_SHADER, vertScreen);
    CreateShader("fragScreen.shader", GL_FRAGMENT_SHADER, fragScreen);

    // LOAD & CREATE TEXTURES

    unsigned int diffuseTexID = loadTexture("gaming.png", GL_RGBA, 4);

    // END

    myProgram = glCreateProgram();
    glAttachShader(myProgram, vertexSource);
    glAttachShader(myProgram, fragmentSource);
    glLinkProgram(myProgram);

    glDeleteShader(vertexSource);
    glDeleteShader(fragmentSource);

    skyProgram = glCreateProgram();
    glAttachShader(skyProgram, vertSky);
    glAttachShader(skyProgram, fragSky);
    glLinkProgram(skyProgram);

    glDeleteShader(vertSky);
    glDeleteShader(fragSky);

    modelProgram = glCreateProgram();
    glAttachShader(modelProgram, vertModel);
    glAttachShader(modelProgram, fragModel);
    glLinkProgram(modelProgram);

    glDeleteShader(vertModel);
    glDeleteShader(fragModel);

    screenProgram = glCreateProgram();
    glAttachShader(screenProgram, vertScreen);
    glAttachShader(screenProgram, fragScreen);
    glLinkProgram(screenProgram);

    /// SCREEN QUAD

    float quadVertices[] = {
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
    };

    ///  GENERATE FRAMEBUFFER
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;

    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glUseProgram(screenProgram);

    glDeleteShader(vertScreen);
    glDeleteShader(fragScreen);


    /// END SETUP SHADER PROGRAM ///

    glUseProgram(myProgram);
    glUniform1i(glGetUniformLocation(myProgram, "heightmap"), 0);
    glUniform1i(glGetUniformLocation(myProgram, "normalmap"), 1);
    glUniform1i(glGetUniformLocation(myProgram, "rock"), 2);
    glUniform1i(glGetUniformLocation(myProgram, "snow"), 3);
    glUniform1i(glGetUniformLocation(myProgram, "grass"), 4);
}

void renderGrass(unsigned int shader, Grass patches[], glm::mat4 view, glm::mat4 projection) {
    // Use Shader
    glUseProgram(shader);

    glEnable(GL_DEPTH);
    glDisable(GL_CULL_FACE);


    for (int i = 0; i < grassCount; i++) {
        // Build World Matrix
        glm::mat4 world = glm::mat4(1);
        // Position
        world = glm::translate(world, patches[i].position);

        // Scale
        world = glm::scale(world, glm::vec3(patches[i].scale));

        // Rotation
        glm::quat q(patches[i].rotation);
        world *= glm::toMat4(q);

        // Set Shader Settings
        glUniformMatrix4fv(glGetUniformLocation(shader, "world"), 1, GL_FALSE, glm::value_ptr(world));
        glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3fv(glGetUniformLocation(shader, "cameraPosition"), 1, glm::value_ptr(cameraPosition));

        float t = glfwGetTime();
        t = t * 0.1f;
        glUniform3f(glGetUniformLocation(shader, "lightDirection"), glm::cos(t), -0.5f, glm::sin(t));

        patches[i].grass->Draw(shader);
    }
}

void renderModel(Model* model, unsigned int shader, glm::vec3 position, glm::vec3 rotation, float scale, glm::mat4 view, glm::mat4 projection) {
    // Use Shader
    glUseProgram(shader);

    glEnable(GL_DEPTH);
    glDisable(GL_CULL_FACE);

    // Build World Matrix
    glm::mat4 world = glm::mat4(1);
    // Position
    world = glm::translate(world, position);

    // Scale
    world = glm::scale(world, glm::vec3(scale));

    // Rotation
    glm::quat q(rotation);
    world *= glm::toMat4(q);


    // Set Shader Settings
    glUniformMatrix4fv(glGetUniformLocation(shader, "world"), 1, GL_FALSE, glm::value_ptr(world));
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(glGetUniformLocation(shader, "cameraPosition"), 1, glm::value_ptr(cameraPosition));

    float t = glfwGetTime();
    t = t * 0.1f;
    glUniform3f(glGetUniformLocation(shader, "lightDirection"), glm::cos(t), -0.5f, glm::sin(t));

    model->Draw(shader);
}