#pragma once

// OpenGL Loader
#include <GL/gl3w.h> // GL3w, initialized with gl3wInit() below

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

// ImGui + imgui-vtk
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "gui.hpp"

namespace GUI
{
	void setup();
	void begin();
    void render();
    void destroy();
    static void glfw_error_callback(int error, const char* description);
}