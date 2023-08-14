//
// Created by milana on 2.7.23..
//

#ifndef PROJECT_BASE_SHADER_H
#define PROJECT_BASE_SHADER_H

#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <rg/Error.h>

std::string readFileContents(std::string path){
    std::ifstream in(path);
    std::stringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}

class Shader {

    unsigned int m_Id;
public:
    Shader(std::string vertexShaderPath, std::string fragmentShaderPath) {

        std::string vsString = readFileContents(vertexShaderPath);
        ASSERT(!vsString.empty(), "Vertex shader source is empty");
        const char *vertexShaderSource = vsString.c_str(); // Zbog toga sto je pisan u C-u
        unsigned vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);

        int success = 0;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

        if (!success) {
            glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                      << infoLog << '\n';
        }

        std::string fsString = readFileContents(fragmentShaderPath);
        ASSERT(!fsString.empty(), "Fragment shader je prazzan\n");
        const char *fragmentShaderSource = fsString.c_str();
        unsigned fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
        glCompileShader(fragmentShader);

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                      << infoLog << '\n';
        }

        unsigned shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);

        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                      << infoLog << '\n';
        }

        //8.Brise memoriju koju su zauzimali (radi se nakon linkovanja, sam program nece biti obrisan
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        m_Id = shaderProgram;
    }

    void use() {
        ASSERT(m_Id > 0, "Use of unidentified or delete program");
        glUseProgram(m_Id);

    }

    void setUniform4f(std::string name, float x, float y, float z, float w) {
        int uniformId = glGetUniformLocation(m_Id, name.c_str());
        glUniform4f(uniformId, x, y, z, w);
    }
    void setUniform3f(std::string name, float x, float y, float z){
        int uniformId = glGetUniformLocation(m_Id, name.c_str());
        glUniform3f(uniformId, x, y, z);
    }
    void setUniform3fv(std::string name, const glm::vec3 &value){
        int uniformId = glGetUniformLocation(m_Id, name.c_str());
        glUniform3fv(uniformId, 1, &value[0]);
    }

    void setUniform1i(std::string name, int value) {
        int uniformId = glGetUniformLocation(m_Id, name.c_str());
        glUniform1i(uniformId, value);
    }

    void setUniform1f(std::string name, float value) {
        int uniformId = glGetUniformLocation(m_Id, name.c_str());
        glUniform1f(uniformId, value);
    }

    void setUniformMatrix4fv(std::string name, GLfloat* value) {
        int uniformId = glGetUniformLocation(m_Id, name.c_str());
        glUniformMatrix4fv(uniformId, 1, GL_FALSE, value);
    }
    void deleteProgram(){
        glDeleteProgram(m_Id);
        m_Id = 0;
    }
    int getShaderId(){
        return m_Id;
    }

};
#endif //PROJECT_BASE_SHADER_H