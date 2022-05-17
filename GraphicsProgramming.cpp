#include <iostream>
#include <fstream>

#include <GLAD\glad.h>
#include <GLFW\glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "STB\stb_image.h"

void loadFromFile(const char* url, char** target)
{
    std::ifstream stream(url, std::ios::binary);

    stream.seekg(0, stream.end);
    int total = stream.tellg();
    *target = new char[total + 1];

    stream.seekg(0, stream.beg);
    stream.read(*target, total);

    (*target)[total] = '\0';

    stream.close();
}

unsigned int loadTexture(std::string url, GLenum format) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, channels;
    unsigned char* data = stbi_load(url.c_str(), &width, &height, &channels, 0);
    if (data != nullptr) {
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture: " << url << std::endl;
    }

    stbi_image_free(data);

    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Texture {
    unsigned int id;
    std::string type;
};

class Mesh {
    public:
        // mesh data
        vector<Vertex>       vertices;
        vector<unsigned int> indices;
        vector<Texture>      textures;

        // Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures);
        Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures)
        {
            this->vertices = vertices;
            this->indices = indices;
            this->textures = textures;

            setupMesh();
        }
        void Draw(Shader &shader);
    private:
        //  render data
        unsigned int VAO, VBO, EBO;

        void setupMesh();
}; 

int main()
{
    //std::cout << "Wayo.\n";

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    int width = 800;
    int height = 600;

    GLFWwindow * window = glfwCreateWindow(width, height, "Cyberpunk 2077", nullptr, nullptr);

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, width, height);

    /// SETUP OBJECT ///

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

    unsigned int VAO;
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

    /// SETUP SHADER PROGRAM ///

    char* vertexSource;
    loadFromFile("vertexShader.shader", &vertexSource);

    char* fragmentSource;
    loadFromFile("fragmentShader.shader", &fragmentSource);

    // LOAD & CREATE TEXTURES

    unsigned int diffuseTexID = loadTexture("gaming.png", GL_RGB);

    // END

    unsigned int vertex, fragment;
    vertex = glCreateShader(GL_VERTEX_SHADER);
    fragment = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertex, 1, &vertexSource, nullptr);
    glShaderSource(fragment, 1, &fragmentSource, nullptr);

    int success;
    char infoLog[512];

    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    };
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    };

    unsigned int myProgram = glCreateProgram();

    glAttachShader(myProgram, vertex);
    glAttachShader(myProgram, fragment);
    glLinkProgram(myProgram);

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    /// END SETUP SHADER PROGRAM ///

    /// MATRIX SETUP ///

    glUseProgram(myProgram);

    int worldLoc = glGetUniformLocation(myProgram, "world");
    int viewLoc = glGetUniformLocation(myProgram, "view");
    int projectionLoc = glGetUniformLocation(myProgram, "projection");

    /// END MATRIX SETUP ///

    // OPENGL SETTINGS //
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    while (!glfwWindowShouldClose(window))
    {
        float t = (float)glfwGetTime();

        glm::mat4 world = glm::mat4(1.f);
        world = glm::rotate(world, glm::radians(t * 50), glm::vec3(1, 1, 1));
        world = glm::scale(world, glm::vec3(1, 1, 1));
        world = glm::translate(world, glm::vec3(0, 0, 0));

        float r = 3.f;
        glm::mat4 view = glm::lookAt(glm::vec3(2, 1, -2), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        glm::mat4 projection = glm::perspective(glm::radians(60.0f), width / (float)height, 0.1f, 100.0f);

        glUniformMatrix4fv(worldLoc, 1, GL_FALSE, glm::value_ptr(world));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // iets tekenen
        glClearColor(0.2, 0.2, 0.2, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseTexID);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, sizeof(indices), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}