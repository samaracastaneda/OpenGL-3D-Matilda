#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

class Shader {
public:
    Shader(const char* vertexSource, const char* fragmentSource);
    ~Shader();
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    bool valido() const { return id_ != 0; }
    void usar() const;
    void setBool(const char* nombre, bool valor) const;
    void setInt(const char* nombre, int valor) const;
    void setVec3(const char* nombre, const glm::vec3& valor) const;
    void setVec4(const char* nombre, const glm::vec4& valor) const;
    void setMat3(const char* nombre, const glm::mat3& valor) const;
    void setMat4(const char* nombre, const glm::mat4& valor) const;

private:
    GLuint id_ = 0;
    static GLuint compilar(GLenum tipo, const char* fuente);
    GLint ubicacion(const char* nombre) const;
};
