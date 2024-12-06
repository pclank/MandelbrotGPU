#pragma once

#include "Timer.hpp"
#include <string>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

/// <summary>
/// GUI wrapper that handles all imgui related calls
/// </summary>
class GUI
{
public:
    GUI(GLFWwindow* pWindow, Timer& timer);

    /// <summary>
    /// Initialize our GUI wrapper
    /// </summary>
    void Init();

    /// <summary>
    /// Render our GUI with updated reference data
    /// </summary>
    void Render();

    /// <summary>
    /// Perform GUI cleanup
    /// </summary>
    void Cleanup();

    /// <summary>
    /// Resets all input flags
    /// </summary>
    void ResetInputFlags();

    /// <summary>
    /// Update mouse position in GUI
    /// </summary>
    /// <param name="xpos"></param>
    /// <param name="ypos"></param>
    inline void MousePositionUpdate(double xpos, double ypos)
    {
        mouse_prev_xpos = mouse_xpos;
        mouse_prev_ypos = mouse_ypos;
        mouse_xpos = xpos;
        mouse_ypos = ypos;
    }

    double mouse_xpos;
    double mouse_ypos;
    double mouse_prev_xpos;
    double mouse_prev_ypos;
    bool cursor_enabled;
    bool clicked;
    bool reset_pressed;
    bool gui_enabled;

private:
    GLFWwindow* p_window;
    Timer& m_timer;
};