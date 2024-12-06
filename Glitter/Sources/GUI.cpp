#include "GUI.hpp"
#include <vector>

GUI::GUI(GLFWwindow* pWindow, Timer& timer)
    :
    p_window(pWindow),
    m_timer(timer)
{
    cursor_enabled = true;
    clicked = false;
    reset_pressed = false;
    gui_enabled = true;
    animationSpeed = 1.0f;
    animationTime = 0.0f;
}

void GUI::Init()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(p_window, true);
    //ImGui_ImplOpenGL3_Init("#version 130");
    //ImGui_ImplOpenGL3_Init("#version 330");
    ImGui_ImplOpenGL3_Init();

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
}

void GUI::Render()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Control Window");
    ImGui::Text("DeltaTime: %f", m_timer.GetDeltaTime());
    ImGui::Text("FPS: %.2f", m_timer.GetFPS());
    ImGui::Separator();
    ImGui::Text("Animation stuff");
    ImGui::Text("Animation time: %.2f", animationTime);
    ImGui::Text("Animation speed: %.1f", animationSpeed);
    ImGui::Separator();
    ImGui::Text("Mouse cursor stuff:");
    ImGui::Text("Cursor_x: %f", mouse_xpos);
    ImGui::Text("Cursor_y: %f", mouse_ypos);
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUI::Cleanup()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void GUI::ResetInputFlags()
{
    // TODO: Add all!
    //clicked = false;
}