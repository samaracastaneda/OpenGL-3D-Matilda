// PROYECTO: Matilda - Visualización y animación 3D
// OpenGL 3.3 + GLFW + GLAD + GLM + TinyGLTF
// Carga GLB, cámara manual, rotación, iluminación Phong y curvas Bézier.

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdint>
#include <algorithm>
#include <cmath>
#include <cfloat>
#include <cstddef>
#include <functional>
#include <glm/gtc/quaternion.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// TinyGLTF incluye STB internamente. Estas implementaciones deben
// aparecer una sola vez en todo el proyecto, en un único archivo .cpp.
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tinygltf/tiny_gltf.h>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;
const char* RUTA_MODELO = "Models/Matilda.glb"; // Ruta relativa al directorio de trabajo

// Cada vértice contiene 12 valores: posición XYZ (3), color RGBA (4),
// coordenadas UV (2) y normal XYZ (3) para calcular la iluminación.
struct VerticeGPU
{
    float x, y, z;
    float r, g, b, a;
    float u, v;
    float nx, ny, nz;
};

// CÁMARA
glm::vec3 cameraPos(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
float yaw = -90.0f;
float pitch = 0.0f;
const float cameraSpeed = 2.5f;
const float cameraTurnSpeed = 70.0f;

// MODOS: 1 = personaje, 2 = camara, 3 = ambos, 0 = manual.
int modoBezier = 1;
bool animacionActiva = true;
float tiempoAnimacion = 0.0f;
bool teclaAnterior[4] = { false, false, false, false };
bool espacioAnterior = false;
bool mostrarCurva = false;
bool mostrarPuntos = false;
bool bAnterior = false;
bool pAnterior = false;
bool iluminacionPhong = true;
bool lAnterior = false;

// Curva cubica de Bezier. t va de 0 a 1.
glm::vec3 calcularBezier(float t, const glm::vec3& p0,
    const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3)
{
    t = glm::clamp(t, 0.0f, 1.0f);
    float u = 1.0f - t;
    return (u * u * u) * p0 + (3.0f * u * u * t) * p1
        + (3.0f * u * t * t) * p2 + (t * t * t) * p3;
}

// Cuatro puntos que describen el recorrido del personaje.
glm::vec3 posicionPersonaje(float t)
{
    return calcularBezier(t,
        glm::vec3(-1.2f, 0.0f, 0.0f),
        glm::vec3(-0.8f, 0.5f, -0.5f),
        glm::vec3(0.8f, 0.5f, 0.5f),
        glm::vec3(1.2f, 0.0f, 0.0f));
}

// Recorrido de la camara frente al personaje.
glm::vec3 posicionCamaraBezier(float t)
{
    return calcularBezier(t,
        glm::vec3(-2.2f, 1.2f, 4.0f),
        glm::vec3(-1.0f, 1.8f, 3.3f),
        glm::vec3(1.0f, 1.8f, 3.3f),
        glm::vec3(2.2f, 1.2f, 4.0f));
}

void processInput(GLFWwindow* window, float deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // L: alternar iluminacion Phong sin afectar la textura.
    bool lActual = glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS;
    if (lActual && !lAnterior) {
        iluminacionPhong = !iluminacionPhong;
        std::cout << "Phong: " << (iluminacionPhong ? "ACTIVADO" : "DESACTIVADO") << std::endl;
    }
    lAnterior = lActual;

    // B y P alternan la visualizacion solo al pulsar la tecla.
    bool bActual = glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS;
    if (bActual && !bAnterior) mostrarCurva = !mostrarCurva;
    bAnterior = bActual;
    bool pActual = glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS;
    if (pActual && !pAnterior) mostrarPuntos = !mostrarPuntos;
    pAnterior = pActual;

    // Cambiar de modo con una sola pulsacion (sin repetirse cada frame).
    const int teclas[4] = { GLFW_KEY_0, GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3 };
    for (int i = 0; i < 4; ++i)
    {
        bool pulsada = glfwGetKey(window, teclas[i]) == GLFW_PRESS;
        if (pulsada && !teclaAnterior[i])
        {
            modoBezier = i;
            std::cout << "Modo seleccionado: " << i << std::endl;
        }
        teclaAnterior[i] = pulsada;
    }
    bool espacio = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
    if (espacio && !espacioAnterior)
        animacionActiva = !animacionActiva;
    espacioAnterior = espacio;

    // En modo automatico la camara se calcula en el bucle principal.
    // En los modos 0 y 1 el teclado controla la camara.
    if (modoBezier == 2 || modoBezier == 3) return;

    const float giro = cameraTurnSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)  yaw -= giro;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) yaw += giro;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)    pitch += giro;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)  pitch -= giro;

    pitch = glm::clamp(pitch, -89.0f, 89.0f);

    glm::vec3 direccion;
    direccion.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direccion.y = sin(glm::radians(pitch));
    direccion.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direccion);

    glm::vec3 derecha = glm::normalize(glm::cross(cameraFront, cameraUp));
    float paso = cameraSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        paso *= 2.0f;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cameraPos += cameraFront * paso;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cameraPos -= cameraFront * paso;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cameraPos -= derecha * paso;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cameraPos += derecha * paso;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) cameraPos -= cameraUp * paso;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) cameraPos += cameraUp * paso;

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
        yaw = -90.0f;
        pitch = 0.0f;
        cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    }
}

// ESCENA GLTF: recorrer nodos y respetar sus transformaciones.
// Cada primitiva conserva su material y su intervalo de indices.
// Las transformaciones estaticas se aplican en CPU una sola vez.
struct ParteModelo {
    GLsizei cantidad = 0;
    size_t offset = 0; // desplazamiento en indices (no bytes)
    int material = -1;
};

static glm::mat4 matrizNodo(const tinygltf::Node& n) {
    glm::mat4 m(1.0f);
    if (n.matrix.size() == 16) {
        for (int c = 0; c < 4; ++c)
            for (int r = 0; r < 4; ++r)
                m[c][r] = static_cast<float>(n.matrix[c * 4 + r]);
        return m;
    }
    if (n.translation.size() == 3)
        m = glm::translate(m, glm::vec3(n.translation[0], n.translation[1], n.translation[2]));
    if (n.rotation.size() == 4) {
        glm::quat q(static_cast<float>(n.rotation[3]), static_cast<float>(n.rotation[0]),
            static_cast<float>(n.rotation[1]), static_cast<float>(n.rotation[2]));
        m *= glm::mat4_cast(q);
    }
    if (n.scale.size() == 3)
        m = glm::scale(m, glm::vec3(n.scale[0], n.scale[1], n.scale[2]));
    return m;
}

// Este lector admite atributos float habituales de glTF.
static const unsigned char* datosAccessor(const tinygltf::Model& model,
    const tinygltf::Accessor& acc, size_t& stride) {
    if (acc.bufferView < 0 || acc.bufferView >= static_cast<int>(model.bufferViews.size()))
        return nullptr;
    const auto& view = model.bufferViews[acc.bufferView];
    if (view.buffer < 0 || view.buffer >= static_cast<int>(model.buffers.size()))
        return nullptr;
    stride = acc.ByteStride(view);
    const auto& buffer = model.buffers[view.buffer];
    size_t comienzo = view.byteOffset + acc.byteOffset;
    if (comienzo >= buffer.data.size() || stride == 0) return nullptr;
    return buffer.data.data() + comienzo;
}

bool extraerGeometria(const tinygltf::Model& model,
    std::vector<VerticeGPU>& vertices,
    std::vector<unsigned int>& indices,
    std::vector<ParteModelo>& partes) {
    vertices.clear(); indices.clear(); partes.clear();

    auto agregarMalla = [&](int meshId, const glm::mat4& transform) {
        if (meshId < 0 || meshId >= static_cast<int>(model.meshes.size())) return;
        const auto& mesh = model.meshes[meshId];
        glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(transform)));
        for (const auto& primitive : mesh.primitives) {
            if (primitive.mode != -1 && primitive.mode != TINYGLTF_MODE_TRIANGLES) continue;
            auto posIt = primitive.attributes.find("POSITION");
            if (posIt == primitive.attributes.end()) continue;
            const auto& pos = model.accessors[posIt->second];
            if (pos.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT || pos.type != TINYGLTF_TYPE_VEC3)
                continue;
            size_t ps = 0;
            const unsigned char* pd = datosAccessor(model, pos, ps);
            if (!pd) continue;

            const tinygltf::Accessor* uv = nullptr;
            const tinygltf::Accessor* normal = nullptr;
            const unsigned char* ud = nullptr, * nd = nullptr;
            size_t us = 0, ns = 0;
            auto uvIt = primitive.attributes.find("TEXCOORD_0");
            if (uvIt != primitive.attributes.end()) {
                const auto& a = model.accessors[uvIt->second];
                if (a.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT && a.type == TINYGLTF_TYPE_VEC2) {
                    uv = &a; ud = datosAccessor(model, a, us);
                }
            }
            auto nIt = primitive.attributes.find("NORMAL");
            if (nIt != primitive.attributes.end()) {
                const auto& a = model.accessors[nIt->second];
                if (a.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT && a.type == TINYGLTF_TYPE_VEC3) {
                    normal = &a; nd = datosAccessor(model, a, ns);
                }
            }
            const unsigned int base = static_cast<unsigned int>(vertices.size());
            for (size_t i = 0; i < pos.count; ++i) {
                float xyz[3] = {}, tex[2] = {}, n[3] = {};
                std::memcpy(xyz, pd + i * ps, sizeof(xyz));
                if (uv && ud && i < uv->count) std::memcpy(tex, ud + i * us, sizeof(tex));
                if (normal && nd && i < normal->count) std::memcpy(n, nd + i * ns, sizeof(n));
                glm::vec3 p = glm::vec3(transform * glm::vec4(xyz[0], xyz[1], xyz[2], 1.0f));
                glm::vec3 vn(0.0f);
                if (normal && nd) {
                    vn = normalMat * glm::vec3(n[0], n[1], n[2]);
                    if (glm::dot(vn, vn) > 1e-12f) vn = glm::normalize(vn);
                }
                VerticeGPU v{};
                v.x = p.x; v.y = p.y; v.z = p.z;
                v.r = v.g = v.b = v.a = 1.0f;
                v.u = tex[0]; v.v = tex[1];
                v.nx = vn.x; v.ny = vn.y; v.nz = vn.z;
                vertices.push_back(v);
            }
            ParteModelo parte;
            parte.offset = indices.size();
            parte.material = primitive.material;
            if (primitive.indices >= 0) {
                const auto& a = model.accessors[primitive.indices];
                size_t is = 0;
                const unsigned char* id = datosAccessor(model, a, is);
                if (!id) continue;
                for (size_t i = 0; i < a.count; ++i) {
                    unsigned int idx = 0;
                    const unsigned char* actual = id + i * is;
                    if (a.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                        uint8_t x; std::memcpy(&x, actual, sizeof(x)); idx = x;
                    }
                    else if (a.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                        uint16_t x; std::memcpy(&x, actual, sizeof(x)); idx = x;
                    }
                    else if (a.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                        uint32_t x; std::memcpy(&x, actual, sizeof(x)); idx = x;
                    }
                    else continue;
                    indices.push_back(base + idx);
                }
            }
            else {
                for (size_t i = 0; i < pos.count; ++i) indices.push_back(base + static_cast<unsigned int>(i));
            }
            parte.cantidad = static_cast<GLsizei>(indices.size() - parte.offset);
            if (parte.cantidad > 0) partes.push_back(parte);
        }
        };

    // Recorrer la escena por sus nodos (incluye nodos hijos).
    // Se usa std::function para la recursividad de la lambda.
    std::function<void(int, const glm::mat4&)> recorrer;
    recorrer = [&](int id, const glm::mat4& padre) {
        if (id < 0 || id >= static_cast<int>(model.nodes.size())) return;
        const auto& n = model.nodes[id];
        glm::mat4 mundo = padre * matrizNodo(n);
        if (n.mesh >= 0) agregarMalla(n.mesh, mundo);
        for (int hijo : n.children) recorrer(hijo, mundo);
        };
    int escena = model.defaultScene >= 0 ? model.defaultScene : 0;
    if (escena >= 0 && escena < static_cast<int>(model.scenes.size())) {
        for (int nodo : model.scenes[escena].nodes) recorrer(nodo, glm::mat4(1.0f));
    }
    else {
        // Modelos sin escena: cargar cada malla una vez.
        for (size_t i = 0; i < model.meshes.size(); ++i)
            agregarMalla(static_cast<int>(i), glm::mat4(1.0f));
    }

    // Solo calcular normales donde no existan normales validas.
    std::vector<glm::vec3> acumuladas(vertices.size(), glm::vec3(0.0f));
    for (const auto& parte : partes) {
        for (size_t i = parte.offset; i + 2 < parte.offset + parte.cantidad; i += 3) {
            unsigned a = indices[i], b = indices[i + 1], c = indices[i + 2];
            if (a >= vertices.size() || b >= vertices.size() || c >= vertices.size()) continue;
            glm::vec3 p0(vertices[a].x, vertices[a].y, vertices[a].z);
            glm::vec3 p1(vertices[b].x, vertices[b].y, vertices[b].z);
            glm::vec3 p2(vertices[c].x, vertices[c].y, vertices[c].z);
            glm::vec3 n = glm::cross(p1 - p0, p2 - p0);
            acumuladas[a] += n; acumuladas[b] += n; acumuladas[c] += n;
        }
    }
    for (size_t i = 0; i < vertices.size(); ++i) {
        auto& v = vertices[i];
        glm::vec3 n(v.nx, v.ny, v.nz);
        if (glm::dot(n, n) < 1e-12f) n = acumuladas[i];
        if (glm::dot(n, n) < 1e-12f) n = glm::vec3(0, 1, 0);
        n = glm::normalize(n);
        v.nx = n.x; v.ny = n.y; v.nz = n.z;
    }
    std::cout << "Vertices: " << vertices.size() << " | Indices: " << indices.size()
        << " | Partes: " << partes.size() << std::endl;
    return !vertices.empty() && !indices.empty() && !partes.empty();
}

// Texturas por imagen: cada material apunta a una textura de glTF,
// que a su vez apunta a una imagen. Si no existe se utiliza blanco.
GLuint crearTextura(const tinygltf::Image* imagen) {
    GLuint id = 0;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    const unsigned char blanco[4] = { 255,255,255,255 };
    if (imagen && !imagen->image.empty() && imagen->bits == 8 &&
        imagen->width > 0 && imagen->height > 0) {
        GLenum formato = GL_RGBA;
        if (imagen->component == 1) formato = GL_RED;
        else if (imagen->component == 2) formato = GL_RG;
        else if (imagen->component == 3) formato = GL_RGB;
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, formato, imagen->width, imagen->height,
            0, formato, GL_UNSIGNED_BYTE, imagen->image.data());
    }
    else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, blanco);
    }
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    return id;
}

// SHADERS
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aTexCoord;
uniform mat4 uMVP;
uniform mat4 uModel;
uniform mat3 uNormalMatrix;
out vec4 VertexColor;
out vec2 TexCoord;
out vec3 FragPos;
out vec3 Normal;
void main()
{
    gl_Position = uMVP * vec4(aPos, 1.0);
    FragPos = vec3(uModel * vec4(aPos, 1.0));
    Normal = normalize(uNormalMatrix * aNormal);
    VertexColor = aColor;
    TexCoord = aTexCoord;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
in vec4 VertexColor;
in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;
uniform sampler2D texture1;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform bool usarPhong;
uniform vec4 colorMaterial;
out vec4 FragColor;
void main()
{
    vec4 base = texture(texture1, TexCoord) * VertexColor * colorMaterial;
    if (!usarPhong) { FragColor = base; return; }
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 ambient = 0.35 * lightColor;
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = 0.65 * diff * lightColor;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 16.0);
    vec3 specular = 0.10 * spec * lightColor;
    vec3 iluminado = (ambient + diffuse) * base.rgb + specular;
    FragColor = vec4(iluminado, base.a);
}
)";

bool comprobarShader(GLuint shader, const char* nombre)
{
    GLint ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (ok == GL_FALSE)
    {
        char log[2048];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::cout << "Error compilando " << nombre << ":\n" << log << std::endl;
    }
    return ok == GL_TRUE;
}

bool comprobarPrograma(GLuint programa)
{
    GLint ok = GL_FALSE;
    glGetProgramiv(programa, GL_LINK_STATUS, &ok);
    if (ok == GL_FALSE)
    {
        char log[2048];
        glGetProgramInfoLog(programa, sizeof(log), nullptr, log);
        std::cout << "Error enlazando shaders:\n" << log << std::endl;
    }
    return ok == GL_TRUE;
}

int main()
{
    // 1. Leer el archivo GLB.
    tinygltf::TinyGLTF loader;
    tinygltf::Model gltfModel;
    std::string error, warning;

    bool loaded = loader.LoadBinaryFromFile(
        &gltfModel, &error, &warning, RUTA_MODELO);

    if (!warning.empty()) std::cout << "Advertencia GLB: " << warning << std::endl;
    if (!error.empty()) std::cout << "Error GLB: " << error << std::endl;
    if (!loaded)
    {
        std::cout << "No se pudo cargar: " << RUTA_MODELO << std::endl;
        std::cout << "Revisa la ruta y el nombre exacto del archivo.\n";
        return -1;
    }

    std::cout << "Modelo GLB cargado correctamente.\n";
    std::cout << "Mallas: " << gltfModel.meshes.size() << std::endl;

    std::vector<VerticeGPU> vertices;
    std::vector<unsigned int> indices;
    std::vector<ParteModelo> partes;
    if (!extraerGeometria(gltfModel, vertices, indices, partes))
    {
        std::cout << "No se encontro geometria compatible en el modelo.\n";
        return -1;
    }

    // 2. Calcular el centro y el tamaño para centrar y ajustar el modelo.
    glm::vec3 minimo(FLT_MAX), maximo(-FLT_MAX);
    for (const auto& v : vertices)
    {
        minimo.x = std::min(minimo.x, v.x);
        minimo.y = std::min(minimo.y, v.y);
        minimo.z = std::min(minimo.z, v.z);
        maximo.x = std::max(maximo.x, v.x);
        maximo.y = std::max(maximo.y, v.y);
        maximo.z = std::max(maximo.z, v.z);
    }
    glm::vec3 centro = (minimo + maximo) * 0.5f;
    glm::vec3 tamano = maximo - minimo;
    float mayorDimension = std::max(tamano.x, std::max(tamano.y, tamano.z));
    if (mayorDimension <= 0.0001f) mayorDimension = 1.0f;
    float escala = 2.0f / mayorDimension;

    // 3. Inicializar GLFW y crear ventana.
    if (!glfwInit())
    {
        std::cout << "ERROR: No se pudo inicializar GLFW.\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(
        SCR_WIDTH, SCR_HEIGHT, "OpenGL - Modelo GLB", nullptr, nullptr);
    if (!window)
    {
        std::cout << "ERROR: No se pudo crear la ventana.\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "ERROR: No se pudo inicializar GLAD.\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // 4. Compilar y enlazar shaders.
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    if (!comprobarShader(vertexShader, "VERTEX SHADER")) return -1;

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    if (!comprobarShader(fragmentShader, "FRAGMENT SHADER")) return -1;

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    if (!comprobarPrograma(shaderProgram)) return -1;

    // 5. Crear VAO, VBO y EBO con los datos extraídos del GLB.
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
        vertices.size() * sizeof(VerticeGPU), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    const GLsizei stride = sizeof(VerticeGPU);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
        (void*)offsetof(VerticeGPU, x));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride,
        (void*)offsetof(VerticeGPU, r));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride,
        (void*)offsetof(VerticeGPU, u));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride,
        (void*)offsetof(VerticeGPU, nx));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    // Geometria auxiliar: trayectoria Bezier y cuatro puntos de control.
    // Se renderiza con un shader sencillo de color uniforme.
    const char* curvaVS = R"GLSL(#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 uVP;
void main() { gl_Position = uVP * vec4(aPos, 1.0); }
)GLSL";
    const char* curvaFS = R"GLSL(#version 330 core
uniform vec3 uColor;
out vec4 FragColor;
void main() { FragColor = vec4(uColor, 1.0); }
)GLSL";
    GLuint curvaVert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(curvaVert, 1, &curvaVS, nullptr);
    glCompileShader(curvaVert);
    if (!comprobarShader(curvaVert, "vertex shader curva")) return -1;
    GLuint curvaFrag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(curvaFrag, 1, &curvaFS, nullptr);
    glCompileShader(curvaFrag);
    if (!comprobarShader(curvaFrag, "fragment shader curva")) return -1;
    GLuint curvaProgram = glCreateProgram();
    glAttachShader(curvaProgram, curvaVert);
    glAttachShader(curvaProgram, curvaFrag);
    glLinkProgram(curvaProgram);
    glDeleteShader(curvaVert);
    glDeleteShader(curvaFrag);
    if (!comprobarPrograma(curvaProgram)) return -1;
    GLint curvaVPLoc = glGetUniformLocation(curvaProgram, "uVP");
    GLint curvaColorLoc = glGetUniformLocation(curvaProgram, "uColor");

    // La trayectoria se proyecta sobre el suelo (Y=-1.02).
    const glm::vec3 puntosControl[4] = {
        {-1.2f, -1.02f, 0.0f},
        {-0.8f, -1.02f, -0.5f},
        { 0.8f, -1.02f, 0.5f},
        { 1.2f, -1.02f, 0.0f}
    };
    const int segmentosCurva = 120;
    std::vector<glm::vec3> verticesCurva;
    for (int i = 0; i <= segmentosCurva; ++i)
    {
        glm::vec3 punto = posicionPersonaje(float(i) / segmentosCurva);
        punto.y = -1.02f;
        verticesCurva.push_back(punto);
    }

    GLuint curvaVAO = 0, curvaVBO = 0, puntosVAO = 0, puntosVBO = 0;
    glGenVertexArrays(1, &curvaVAO);
    glGenBuffers(1, &curvaVBO);
    glBindVertexArray(curvaVAO);
    glBindBuffer(GL_ARRAY_BUFFER, curvaVBO);
    glBufferData(GL_ARRAY_BUFFER, verticesCurva.size() * sizeof(glm::vec3),
        verticesCurva.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    glGenVertexArrays(1, &puntosVAO);
    glGenBuffers(1, &puntosVBO);
    glBindVertexArray(puntosVAO);
    glBindBuffer(GL_ARRAY_BUFFER, puntosVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(puntosControl), puntosControl, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // 6. Crear una textura por imagen (y una textura blanca de reserva).
    GLuint texturaBlanca = crearTextura(nullptr);
    std::vector<GLuint> texturasImagen;
    for (const auto& imagen : gltfModel.images)
        texturasImagen.push_back(crearTextura(&imagen));

    GLint mvpLoc = glGetUniformLocation(shaderProgram, "uMVP");
    GLint texLoc = glGetUniformLocation(shaderProgram, "texture1");
    GLint modelLoc = glGetUniformLocation(shaderProgram, "uModel");
    GLint normalLoc = glGetUniformLocation(shaderProgram, "uNormalMatrix");
    GLint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
    GLint lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
    GLint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
    GLint phongLoc = glGetUniformLocation(shaderProgram, "usarPhong");
    GLint materialLoc = glGetUniformLocation(shaderProgram, "colorMaterial");

    std::cout << "\n=== CONTROLES ===\n"
        << "W / S: avanzar y retroceder\n"
        << "A / D: izquierda y derecha\n"
        << "Q / E: bajar y subir\n"
        << "Flechas: girar la camara\n"
        << "Shift: moverse mas rapido\n"
        << "R: reiniciar camara\n"
        << "0: camara manual y modelo quieto\n"
        << "1: mover personaje por Bezier (camara manual)\n"
        << "2: mover camara por Bezier (personaje quieto)\n"
        << "3: mover ambos por Bezier\n"
        << "ESPACIO: pausar / reanudar trayectoria\n"
        << "B: mostrar / ocultar curva Bezier\n"
        << "P: mostrar / ocultar puntos de control\n"
        << "L: activar / desactivar iluminacion Phong\n"
        << "ESC: salir\n" << std::endl;

    float ultimoFrame = static_cast<float>(glfwGetTime());

    // 7. Bucle de renderizado.
    while (!glfwWindowShouldClose(window))
    {
        float tiempoActual = static_cast<float>(glfwGetTime());
        float deltaTime = tiempoActual - ultimoFrame;
        ultimoFrame = tiempoActual;
        processInput(window, deltaTime);
        if (animacionActiva) tiempoAnimacion += deltaTime;
        // Ida y regreso suaves, periodo aproximado de 12 segundos.
        float fase = 0.5f + 0.5f * sin(tiempoAnimacion * 0.52f);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        if (height == 0) height = 1;
        glViewport(0, 0, width, height);

        glClearColor(0.60f, 0.33f, 0.42f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(
            glm::radians(60.0f), (float)width / (float)height, 0.1f, 100.0f);
        glm::vec3 posicionModelo(0.0f);
        if (modoBezier == 1 || modoBezier == 3)
            posicionModelo = posicionPersonaje(fase);

        glm::vec3 camaraRender = cameraPos;
        glm::vec3 objetivoCamara = cameraPos + cameraFront;
        if (modoBezier == 2 || modoBezier == 3)
        {
            camaraRender = posicionCamaraBezier(fase);
            objetivoCamara = posicionModelo;
        }
        glm::mat4 view = glm::lookAt(camaraRender, objetivoCamara, cameraUp);

        // Centrar y escalar el modelo; después se rota lentamente sobre Y.
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, posicionModelo);
        // Rotacion lenta opcional; en modo 0 queda quieto.
        if (modoBezier != 0)
            model = glm::rotate(model, tiempoAnimacion * glm::radians(15.0f),
                glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(escala));
        model = glm::translate(model, -centro);

        glm::mat4 mvp = projection * view * model;
        glUseProgram(shaderProgram);
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
        glUniformMatrix3fv(normalLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        glUniform3f(lightPosLoc, 2.0f, 4.0f, 3.0f);
        glUniform3f(lightColorLoc, 1.0f, 0.96f, 0.90f);
        glUniform3fv(viewPosLoc, 1, glm::value_ptr(camaraRender));
        glUniform1i(texLoc, 0);
        glUniform1i(phongLoc, iluminacionPhong ? 1 : 0);
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(VAO);
        for (const auto& parte : partes) {
            GLuint textura = texturaBlanca;
            glm::vec4 color(1.0f);
            if (parte.material >= 0 && parte.material < static_cast<int>(gltfModel.materials.size())) {
                const auto& material = gltfModel.materials[parte.material];
                const auto& pbr = material.pbrMetallicRoughness;
                if (pbr.baseColorFactor.size() == 4)
                    color = glm::vec4(pbr.baseColorFactor[0], pbr.baseColorFactor[1],
                        pbr.baseColorFactor[2], pbr.baseColorFactor[3]);
                int texIndex = pbr.baseColorTexture.index;
                if (texIndex >= 0 && texIndex < static_cast<int>(gltfModel.textures.size())) {
                    int imagen = gltfModel.textures[texIndex].source;
                    if (imagen >= 0 && imagen < static_cast<int>(texturasImagen.size()))
                        textura = texturasImagen[imagen];
                }
            }
            glUniform4fv(materialLoc, 1, glm::value_ptr(color));
            glBindTexture(GL_TEXTURE_2D, textura);
            glDrawElements(GL_TRIANGLES, parte.cantidad, GL_UNSIGNED_INT,
                reinterpret_cast<const void*>(parte.offset * sizeof(unsigned int)));
        }
        glBindVertexArray(0);

        // Dibujar la curva en coordenadas del mundo, sin transformacion del GLB.
        // El personaje se traslada usando estos mismos puntos.
        if (mostrarCurva || mostrarPuntos)
        {
            glUseProgram(curvaProgram);
            glm::mat4 vp = projection * view;
            glUniformMatrix4fv(curvaVPLoc, 1, GL_FALSE, glm::value_ptr(vp));
            if (mostrarCurva)
            {
                glUniform3f(curvaColorLoc, 0.15f, 1.0f, 0.65f);
                glBindVertexArray(curvaVAO);
                glLineWidth(2.0f);
                glDrawArrays(GL_LINE_STRIP, 0, segmentosCurva + 1);
            }
            if (mostrarPuntos)
            {
                glUniform3f(curvaColorLoc, 1.0f, 0.75f, 0.18f);
                glBindVertexArray(puntosVAO);
                glPointSize(10.0f);
                glDrawArrays(GL_POINTS, 0, 4);
            }
            glBindVertexArray(0);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 8. Liberar recursos de OpenGL.
    glDeleteVertexArrays(1, &curvaVAO);
    glDeleteVertexArrays(1, &puntosVAO);
    glDeleteBuffers(1, &curvaVBO);
    glDeleteBuffers(1, &puntosVBO);
    glDeleteProgram(curvaProgram);
    for (GLuint id : texturasImagen) glDeleteTextures(1, &id);
    glDeleteTextures(1, &texturaBlanca);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}