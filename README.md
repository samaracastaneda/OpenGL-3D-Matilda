# OpenGL-3D-Matilda

**Estudiante:** Samara Stacey Castañeda Gómez  
**Código estudiantil:** 6014442  
**Universidad:** Universidad Militar Nueva Granada  
**Programa:** Ingeniería en Multimedia  
**Asignatura:** Computación Gráfica  
**Proyecto:** Visualización y animación de un modelo 3D con OpenGL  
**Año:** 2026

## Descripción del proyecto

Este proyecto consiste en la visualización y animación de un modelo tridimensional llamado **Matilda**, desarrollado en C++ mediante OpenGL.

Su propósito es aplicar conceptos de gráficos por computadora, como las transformaciones 3D, el manejo de cámaras, la iluminación y las animaciones mediante curvas Bézier, permitiendo interactuar con el modelo dentro de un entorno tridimensional.

## Herramientas y bibliotecas utilizadas

- **C++:** lenguaje de programación utilizado para desarrollar el proyecto.
- **OpenGL:** renderizado de los gráficos tridimensionales.
- **GLFW:** creación de la ventana y manejo de entradas del teclado.
- **GLAD:** carga de las funciones de OpenGL.
- **GLM:** operaciones matemáticas con vectores, matrices y transformaciones.
- **TinyGLTF:** carga del modelo 3D en formato `.glb`.
- **Visual Studio:** entorno de desarrollo utilizado.

## Funcionalidades principales

### Carga y visualización del modelo 3D

Se utiliza TinyGLTF para cargar el modelo Matilda desde un archivo `.glb`, procesando la información necesaria para representarlo mediante OpenGL.

### Transformaciones y cámara

Se emplean matrices de modelo, vista y proyección para posicionar y visualizar los elementos de la escena. La cámara puede controlarse manualmente mediante el teclado.

### Iluminación Blinn-Phong

Se implementa un modelo de iluminación basado en componentes ambientales, difusas y especulares, que permite apreciar mejor el volumen y las superficies del personaje.

### Animación mediante curvas Bézier

Se utilizan curvas Bézier para generar trayectorias suaves. El programa permite mover el personaje, la cámara o ambos, además de visualizar la curva y sus puntos de control.

### Interacción mediante el teclado

Se incorporan diferentes controles para desplazar la cámara, seleccionar modos de animación, pausar movimientos y activar o desactivar elementos de visualización.

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

### Modos de animación

| Tecla | Función |
|---|---|
| `0` | Cámara manual y modelo quieto |
| `1` | Mover el personaje mediante Bézier |
| `2` | Mover la cámara mediante Bézier |
| `3` | Mover ambos mediante Bézier |
| `Espacio` | Pausar o reanudar la trayectoria |

### Visualización y otros controles

| Tecla | Función |
|---|---|
| `B` | Mostrar u ocultar la curva Bézier |
| `P` | Mostrar u ocultar los puntos de control |
| `L` | Activar o desactivar la iluminación |
| `ESC` | Cerrar el programa |

## Estructura del proyecto

- `Main.cpp`: contiene la configuración de OpenGL, la carga del modelo, los shaders, la iluminación, las transformaciones, los controles y las animaciones.
- `Models/`: contiene los archivos del modelo tridimensional de Matilda.
- `Libraries/`: incluye las bibliotecas y dependencias utilizadas.
- Archivos de solución y proyecto de Visual Studio: permiten abrir y compilar el programa.

## Ejecución del proyecto

El proyecto fue desarrollado en **Visual Studio**, utilizando C++ y OpenGL.

Para ejecutarlo:

1. Descargar o clonar el repositorio.
2. Abrir la solución correspondiente en Visual Studio.
3. Verificar las rutas de las bibliotecas y del archivo `.glb`.
4. Seleccionar la configuración **Debug | x64**.
5. Compilar y ejecutar el programa.

Al iniciar, se carga el modelo Matilda y se habilitan los controles del teclado para interactuar con la escena y sus animaciones.

**Nota:** Al ejecutar el proyecto en otro computador, puede ser necesario ajustar las rutas de las bibliotecas o del modelo 3D.

## Objetivo de aprendizaje

El desarrollo de este proyecto permitió aplicar conocimientos de gráficos por computadora mediante la integración de modelos tridimensionales, transformaciones, iluminación, manejo de cámaras y animaciones con curvas Bézier utilizando OpenGL.
