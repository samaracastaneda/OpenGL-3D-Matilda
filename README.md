# OpenGL-3D-Matilda

**Estudiante:** Samara Stacey Castañeda Gómez  
**Código estudiantil:** 6014442  
**Universidad:** Universidad Militar Nueva Granada  
**Programa:** Ingeniería en Multimedia  
**Asignatura:** Computación Gráfica  
**Proyecto:** Visualización y animación de un modelo 3D con OpenGL  
**Año:** 2026

---

## Descripción del proyecto

Este proyecto consiste en la visualización y animación de un modelo tridimensional llamado **Matilda**, desarrollado en C++ mediante OpenGL.

Su propósito es aplicar conceptos de gráficos por computadora, como las transformaciones 3D, el manejo de cámaras, la iluminación y las animaciones mediante curvas Bézier, permitiendo interactuar con el modelo dentro de un entorno tridimensional.

## Herramientas y bibliotecas utilizadas

- **C++:** lenguaje de programación utilizado para desarrollar el proyecto.
- **OpenGL 3.3:** renderizado de los gráficos tridimensionales.
- **GLFW:** creación de la ventana y manejo de entradas del teclado.
- **GLAD:** carga de las funciones de OpenGL.
- **GLM:** operaciones matemáticas con vectores, matrices y transformaciones.
- **TinyGLTF:** carga y procesamiento del modelo 3D en formato `.glb`.
- **GLSL:** lenguaje utilizado para programar los shaders.
- **Visual Studio:** entorno de desarrollo utilizado.

## Funcionalidades principales

### Carga y visualización del modelo 3D

Se utiliza TinyGLTF para cargar el modelo Matilda desde el archivo `Models/Matilda.glb`. Se procesan sus vértices, índices, normales, materiales y texturas para representarlo mediante OpenGL.

### Transformaciones y cámara

Se emplean matrices de modelo, vista y proyección para posicionar y visualizar los elementos de la escena. La cámara puede controlarse manualmente mediante el teclado o seguir una trayectoria automática.

### Iluminación Blinn-Phong

Se implementa un modelo de iluminación con componentes ambientales, difusas y especulares, que permite apreciar mejor el volumen y las superficies del personaje. La iluminación puede activarse o desactivarse durante la ejecución sin eliminar las texturas del modelo.

### Animación mediante curvas Bézier

Se utilizan curvas Bézier cúbicas para generar trayectorias suaves. El programa permite mover el personaje, la cámara o ambos, además de visualizar la trayectoria del personaje y sus puntos de control.

### Interacción mediante el teclado

Se incorporan controles para desplazar la cámara, seleccionar modos de animación, pausar movimientos y activar o desactivar elementos de visualización.

## Arquitectura del software

El proyecto utiliza una arquitectura modular sencilla, donde `Main.cpp` coordina la carga del modelo, las cámaras, las animaciones y el renderizado, mientras que la clase `Shader`, definida en `Shader.h` e implementada en `Shader.cpp`, se encarga de administrar los programas de shaders de OpenGL.

Esta organización permite separar las responsabilidades de cada archivo y facilita el mantenimiento del código.

### Componentes principales

| Componente | Función |
|---|---|
| **Carga del modelo** | TinyGLTF lee el archivo `.glb` y obtiene la información del modelo tridimensional. |
| **Procesamiento de geometría** | Organiza vértices, índices, normales y materiales para su representación. |
| **Configuración gráfica** | GLFW y GLAD permiten crear la ventana e inicializar OpenGL. |
| **Buffers gráficos** | VAO, VBO y EBO almacenan y organizan los datos utilizados durante el renderizado. |
| **Clase Shader** | Administra la compilación, el enlazado y la utilización de los programas de shaders. |
| **Shaders e iluminación** | Los shaders GLSL procesan la geometría, las texturas y la iluminación Blinn-Phong. |
| **Cámara y transformaciones** | GLM permite calcular las matrices de modelo, vista y proyección. |
| **Animaciones Bézier** | Calcula trayectorias para controlar el movimiento del personaje y la cámara. |
| **Interacción** | Procesa las entradas del teclado y actualiza las opciones de visualización. |

### Diagrama de arquitectura

```mermaid
flowchart TD
    A["Main.cpp - Programa principal"]
    B["Carga del modelo GLB - TinyGLTF"]
    C["Procesamiento de geometría y materiales"]
    D["Inicialización gráfica - GLFW y GLAD"]
    E["Buffers VAO, VBO y EBO"]
    F["Bucle principal"]
    G["Controles del teclado"]
    H["Cámara y animaciones Bézier - GLM"]
    I["Clase Shader - Shader.h y Shader.cpp"]
    J["Shaders GLSL e iluminación Blinn-Phong"]
    K["Renderizado de la escena - OpenGL"]
    L["Liberación de recursos"]

    A --> B
    B --> C
    C --> D
    D --> E
    E --> I
    I --> J
    J --> F
    F --> G
    F --> H
    F --> I
    G --> H
    H --> K
    J --> K
    K --> F
    F --> L
```

### Flujo de ejecución

1. **Carga del modelo:** se lee el archivo GLB, se extrae su geometría y se calculan el centro y la escala del personaje.
2. **Inicialización gráfica:** se crea la ventana, se configura OpenGL y se preparan los buffers y las texturas.
3. **Configuración de shaders:** la clase `Shader` administra los programas utilizados para representar el modelo y los elementos auxiliares.
4. **Actualización:** en cada fotograma se procesan las entradas del teclado y se calculan las posiciones de la cámara y del personaje.
5. **Renderizado:** se aplican las transformaciones, texturas e iluminación para dibujar el modelo y, cuando están activadas, la curva Bézier y sus puntos de control.
6. **Finalización:** al cerrar la ventana, se liberan los recursos gráficos utilizados.

## Implementación de la clase Shader

Para mejorar la organización del proyecto, se implementó una clase `Shader` que permite administrar los programas de shaders utilizados en OpenGL.

Anteriormente, la compilación y el enlazado de los shaders se realizaban directamente en `Main.cpp`. Con esta modificación, estas tareas se encuentran separadas del código principal, lo que facilita su comprensión, mantenimiento y reutilización.

### Organización de los archivos

- **Shader.h:** contiene la declaración de la clase `Shader` y los métodos disponibles.
- **Shader.cpp:** implementa las funciones necesarias para compilar, enlazar y utilizar los programas de shaders.
- **Main.cpp:** controla la carga del modelo 3D, las transformaciones, las cámaras, la iluminación, las curvas de Bézier y el renderizado de la escena.

### Funcionamiento de la clase

La clase `Shader` se encarga de administrar los programas utilizados por la GPU para procesar los vértices y determinar la apariencia de los fragmentos.

Sus principales responsabilidades son:

1. Compilar el Vertex Shader y el Fragment Shader.
2. Enlazar ambos shaders en un programa de OpenGL.
3. Comprobar posibles errores de compilación o enlazado.
4. Activar el programa correspondiente durante el renderizado.
5. Enviar variables `uniform`, como matrices de transformación, posiciones, colores y parámetros de iluminación.

### Aplicación en Matilda

El proyecto utiliza dos programas de shaders con funciones diferentes.

**Shader del modelo 3D**

Procesa los vértices del personaje, aplica las transformaciones necesarias y calcula su apariencia mediante las texturas, los materiales y la iluminación Blinn-Phong.

La iluminación puede activarse o desactivarse con la tecla `L`, conservando las texturas del modelo.

**Shader de las curvas de Bézier**

Se utiliza para representar visualmente la trayectoria del personaje y sus puntos de control.

Este programa emplea una transformación de cámara y proyección, junto con un color uniforme, para dibujar los elementos auxiliares de la escena.

La curva se puede mostrar con la tecla `B` y los puntos de control con la tecla `P`.

### Ventajas de esta implementación

La separación de los shaders en una clase permite reducir el código repetido dentro de `Main.cpp` y organizar mejor las responsabilidades de cada archivo.

Además, facilita la incorporación de nuevos programas de shaders sin tener que repetir todo el proceso de compilación y enlazado.

Esta modificación mantiene el funcionamiento original del proyecto, incluyendo la visualización del modelo GLB, la iluminación Blinn-Phong, los controles de cámara y las animaciones mediante curvas de Bézier.

## Controles del programa

### Movimiento de cámara

| Tecla | Función |
|---|---|
| `W` / `S` | Avanzar y retroceder |
| `A` / `D` | Mover a izquierda y derecha |
| `Q` / `E` | Bajar y subir |
| `Flechas` | Girar la cámara |
| `Shift` | Aumentar la velocidad de movimiento |
| `R` | Reiniciar la cámara |

Los controles manuales de la cámara están disponibles en los modos `0` y `1`.

### Modos de animación

| Tecla | Función |
|---|---|
| `0` | Cámara manual y modelo quieto |
| `1` | Mover el personaje mediante Bézier, con cámara manual |
| `2` | Mover la cámara mediante Bézier, con personaje quieto |
| `3` | Mover el personaje y la cámara mediante Bézier |
| `Espacio` | Pausar o reanudar la trayectoria |

### Visualización y otros controles

| Tecla | Función |
|---|---|
| `B` | Mostrar u ocultar la curva Bézier |
| `P` | Mostrar u ocultar los puntos de control |
| `L` | Activar o desactivar la iluminación Blinn-Phong |
| `ESC` | Cerrar el programa |

## Estructura del proyecto

- **Main.cpp:** contiene la carga del modelo GLB, la configuración de OpenGL, las transformaciones, las cámaras, la iluminación, los controles y las animaciones.
- **Shader.h:** declara la clase `Shader` y los métodos utilizados para administrar los programas de shaders.
- **Shader.cpp:** implementa la compilación, el enlazado, la activación y la configuración de variables de los shaders.
- **Models/:** contiene el archivo `Matilda.glb`, utilizado para representar el personaje.
- **Libraries/:** incluye las bibliotecas y dependencias necesarias para el funcionamiento del proyecto.
- **Archivos de Visual Studio:** permiten abrir, configurar y compilar el proyecto.

## Ejecución del proyecto

El proyecto fue desarrollado en **Visual Studio**, utilizando C++ y OpenGL 3.3.

Para ejecutarlo:

1. Descargar o clonar el repositorio.
2. Abrir la solución correspondiente en Visual Studio.
3. Verificar las rutas de las bibliotecas y del archivo `Models/Matilda.glb`.
4. Seleccionar la configuración **Debug | x64**, si está disponible.
5. Compilar y ejecutar el programa.

Al iniciar, se carga el modelo Matilda y se habilitan los controles del teclado para interactuar con la escena y sus animaciones.

**Nota:** Al ejecutar el proyecto en otro computador, puede ser necesario ajustar las rutas de las bibliotecas o del modelo 3D. El archivo GLB se carga mediante una ruta relativa al directorio de trabajo.

## Objetivo de aprendizaje

El desarrollo de este proyecto permitió aplicar conocimientos de gráficos por computadora mediante la integración de modelos tridimensionales, transformaciones, iluminación, manejo de cámaras y animaciones con curvas Bézier utilizando OpenGL.

Además, la implementación de la clase `Shader` permitió mejorar la organización del código mediante la separación de responsabilidades, facilitando la comprensión de la arquitectura y la administración de los programas gráficos.
