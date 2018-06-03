#include <stdexcept>
#include <iostream>
#include <cstdio>
#include <chrono>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Primitives.hpp"
#include "draw.hpp"

int main(void)
{
  try {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

    if (!glfwInit()) {
      throw std::runtime_error("Error initializing glfw");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    int width = 800;
    int height = 800;
    GLFWwindow* window = glfwCreateWindow(
      width, height, "3D Game Engine", nullptr, nullptr
    );

    if (!window) {
      glfwTerminate();
      throw std::runtime_error("Error creating glfw window");
    }

    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK) {
      throw std::runtime_error("Error initializing glew");
    }

    // enable vsync
    glfwSwapInterval(1);

    printf(
      "OpenGL %s, GLSL %s\n",
      glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION)
    );

    const std::vector<float>& bufferData = {
      0, 0, 0,
      1, 0, 0,
      0, 1, 0
    };
    BufferView* bufferView = createBufferView(bufferData);
    Accessor accessor = {
      bufferView,
      0,
      3,
      Accessor::Type::Vec3,
      Accessor::ComponentType::Float,
      false
    };
    MeshPrimitive::AttributeMap attributeMap = {
      { "POSITION", 0 }
    };
    MeshPrimitive mesh = {
      MeshPrimitive::Mode::Triangles,
      { { "POSITION", &accessor } }
    };

    mesh.loadToGpu(attributeMap);

    auto shaderProgram = new ShaderProgram();
    shaderProgram->initFromFiles("dist/simple.vert", "dist/simple.frag");

    while (!glfwWindowShouldClose(window))
    {
      glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glEnable(GL_DEPTH_TEST);

      // DRAW
      draw(*shaderProgram, mesh);

      glfwSwapBuffers(window);
      glfwPollEvents();
      //glfwWaitEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
  }
  catch (const std::exception& err) {
    std::cerr << "An error occured: " << err.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
