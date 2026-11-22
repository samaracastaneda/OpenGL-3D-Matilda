#include "Shader.h"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <utility>

GLuint Shader::compilar(GLenum tipo, const char* fuente) {
    GLuint shader = glCreateShader(tipo);
    glShaderSource(shader, 1, &fuente, nullptr);
    glCompileShader(shader);
    GLint correcto = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &correcto);
    if (correcto != GL_TRUE) {
        char mensaje[4096] = {};
        glGetShaderInfoLog(shader, sizeof(mensaje), nullptr, mensaje);
        std::cerr << "Error compilando shader " << (tipo == GL_VERTEX_SHADER ? "vertex" : "fragment") << ":\n" << mensaje << '\n';
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

Shader::Shader(const char* vertexSource, const char* fragmentSource) {
    GLuint vertice = compilar(GL_VERTEX_SHADER, vertexSource);
    if (!vertice) return;
    GLuint fragmento = compilar(GL_FRAGMENT_SHADER, fragmentSource);
    if (!fragmento) { glDeleteShader(vertice); return; }
    GLuint programa = glCreateProgram();
    glAttachShader(programa, vertice);
    glAttachShader(programa, fragmento);
    glLinkProgram(programa);
    glDeleteShader(vertice);
    glDeleteShader(fragmento);
    GLint correcto = GL_FALSE;
    glGetProgramiv(programa, GL_LINK_STATUS, &correcto);
    if (correcto != GL_TRUE) {
        char mensaje[4096] = {};
        glGetProgramInfoLog(programa, sizeof(mensaje), nullptr, mensaje);
        std::cerr << "Error enlazando programa de shaders:\n" << mensaje << '\n';
        glDeleteProgram(programa);
        return;
    }
    id_ = programa;
}

Shader::~Shader() { if (id_) glDeleteProgram(id_); }
Shader::Shader(Shader&& otro) noexcept : id_(std::exchange(otro.id_, 0)) {}
Shader& Shader::operator=(Shader&& otro) noexcept {
    if (this != &otro) {
        if (id_) glDeleteProgram(id_);
        id_ = std::exchange(otro.id_, 0);
    }
    return *this;
}
void Shader::usar() const { glUseProgram(id_); }
GLint Shader::ubicacion(const char* nombre) const { return glGetUniformLocation(id_, nombre); }
void Shader::setBool(const char* n, bool v) const { glUniform1i(ubicacion(n), v ? 1 : 0); }
void Shader::setInt(const char* n, int v) const { glUniform1i(ubicacion(n), v); }
void Shader::setVec3(const char* n, const glm::vec3& v) const { glUniform3fv(ubicacion(n), 1, glm::value_ptr(v)); }
void Shader::setVec4(const char* n, const glm::vec4& v) const { glUniform4fv(ubicacion(n), 1, glm::value_ptr(v)); }
void Shader::setMat3(const char* n, const glm::mat3& v) const { glUniformMatrix3fv(ubicacion(n), 1, GL_FALSE, glm::value_ptr(v)); }
void Shader::setMat4(const char* n, const glm::mat4& v) const { glUniformMatrix4fv(ubicacion(n), 1, GL_FALSE, glm::value_ptr(v)); }
