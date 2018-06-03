#include <stdexcept>
#include <iostream>
#include <cstdio>
#include <chrono>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "AssetManager.hpp"
#include "Primitives.hpp"
#include "draw.hpp"

int main(int argc, char** argv)
{
  if (argc != 2) {
    printf("Usage: %s <asset-path>", argv[0]);
    return 1;
  }
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

    MeshPrimitive::AttributeMap attributeMap = {
      { "POSITION", 0 }
    };

    std::string assetPath = argv[1];
    AssetManager assets;
    int assetId = assets.loadAsset(assetPath);

    assets.gpuLoadAll(attributeMap);

    auto shaderProgram = new ShaderProgram();
    shaderProgram->initFromFiles("dist/simple.vert", "dist/simple.frag");

    while (!glfwWindowShouldClose(window))
    {
      glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glEnable(GL_DEPTH_TEST);

      // DRAW
      draw(*shaderProgram, assets.getMesh(assetId, 0)->primitives[0]);

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
