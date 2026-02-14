#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <array>

struct TriangleRenderer {

    GLuint vao{}, vbo{}, program{};

    static constexpr auto vertexShaderSource = R"(
    #version 330 core

    layout (location = 0) in vec2 aPos;

    uniform float aspect;
    uniform mat4 rotationMatrix;

    void main()
    {
        vec4 rotatedPos = rotationMatrix * vec4(aPos, 0.0, 1.0);
        rotatedPos.x *= aspect;
        gl_Position = rotatedPos;
    }
)";

    static constexpr auto fragmentShaderSource = R"(
    #version 330 core

    out vec4 FragColor;

    void main()
    {
        FragColor = vec4(1.0, 0.4, 0.2, 1.0);
    }
)";

    static constexpr auto triangleVertices = std::array{
        -0.5f, -0.5f, 0.0f,  // Bottom left
        0.5f, -0.5f, 0.0f,  // Bottom right
        0.0f,  0.5f, 0.0f     // Top
    };

    bool setup() {
        auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);

        if (!tryCompileShaderWithLog(vertexShader)) {
            return false;
        }

        auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);

        if (!tryCompileShaderWithLog(fragmentShader)) {
            return false;
        }

        program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);

        if (!tryLinkProgramWithLog(program)) {
            return false;
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices),
                     triangleVertices.data(),
                     GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                              3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);

        return true;
    }

    void render(int windowWidth, int windowHeight, float rotationAngle = 0.0f) {
        glUseProgram(program);

        float aspect = static_cast<float>(windowHeight) / windowWidth;
        auto aspectLoc = glGetUniformLocation(program, "aspect");

        glUniform1f(aspectLoc, aspect);

        auto rotationMatrix = glm::rotate(glm::mat4(1.0f),
                                          rotationAngle,
                                          glm::vec3(0, 0, 1));

        auto rotationLoc = glGetUniformLocation(program, "rotationMatrix");
        glUniformMatrix4fv(rotationLoc, 1, GL_FALSE, glm::value_ptr(rotationMatrix));

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    void cleanup() {
        glDeleteProgram(program);
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
    }

    static bool tryCompileShaderWithLog(GLuint shaderID) {
        glCompileShader(shaderID);
        GLint success = 0;
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
        if (success == GL_FALSE) {
            char log[1024];
            glGetShaderInfoLog(shaderID, sizeof(log), nullptr, log);
            std::cout << "cannot compile shader\n";
            return false;
        }
        return true;
    }

    static bool tryLinkProgramWithLog(GLuint programID) {
        glLinkProgram(programID);
        GLint success = 0;
        glGetProgramiv(programID, GL_LINK_STATUS, &success);
        if (success == GL_FALSE) {
            char log[1024];
            glGetProgramInfoLog(programID, sizeof(log), nullptr, log);
            std::cout << "cannot link shader\n";
            return false;
        }
        return true;
    }
};
