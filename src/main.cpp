#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>
#include <string>



// ui stuff
#include "ui/mainWindow.h"


static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main(int argc, char** argv) {
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Create window
    GLFWwindow* window = glfwCreateWindow(1280, 720, "GRIB Viewer", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // <-- ADD THIS
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // optional

    // Setup style
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // --- Backgrounds (Midnight-based layers) ---
    colors[ImGuiCol_WindowBg]        = ImVec4(0.02f, 0.18f, 0.22f, 1.0f);
    colors[ImGuiCol_DockingEmptyBg]  = ImVec4(0.015f, 0.15f, 0.18f, 1.0f);
    colors[ImGuiCol_ChildBg]         = ImVec4(0.03f, 0.22f, 0.27f, 1.0f);
    colors[ImGuiCol_PopupBg]         = ImVec4(0.03f, 0.22f, 0.27f, 1.0f);

    // --- Title bars (desaturated Midnight, not accent color) ---
    colors[ImGuiCol_TitleBg]         = ImVec4(0.04f, 0.25f, 0.30f, 1.0f);
    colors[ImGuiCol_TitleBgActive]   = ImVec4(0.06f, 0.32f, 0.38f, 1.0f);
    colors[ImGuiCol_TitleBgCollapsed]= ImVec4(0.04f, 0.25f, 0.30f, 0.7f);

    // --- Tabs ---
    colors[ImGuiCol_Tab]             = ImVec4(0.05f, 0.22f, 0.26f, 1.0f);
    colors[ImGuiCol_TabHovered]      = ImVec4(0.10f, 0.35f, 0.40f, 1.0f);
    colors[ImGuiCol_TabActive]       = ImVec4(0.08f, 0.40f, 0.50f, 1.0f);
    colors[ImGuiCol_TabUnfocused]    = ImVec4(0.05f, 0.22f, 0.26f, 0.7f);

    // --- Lime accent (interaction only) ---
    colors[ImGuiCol_Button]          = ImVec4(0.60f, 0.65f, 0.20f, 1.0f);
    colors[ImGuiCol_ButtonHovered]   = ImVec4(0.75f, 0.81f, 0.25f, 1.0f);
    colors[ImGuiCol_ButtonActive]    = ImVec4(0.50f, 0.55f, 0.18f, 1.0f);

    colors[ImGuiCol_SliderGrab]      = ImVec4(0.75f, 0.81f, 0.25f, 1.0f);
    colors[ImGuiCol_SliderGrabActive]= ImVec4(0.85f, 0.90f, 0.30f, 1.0f);
    colors[ImGuiCol_CheckMark]       = ImVec4(0.75f, 0.81f, 0.25f, 1.0f);

    // --- Text ---
    colors[ImGuiCol_Text]            = ImVec4(0.95f, 0.95f, 0.95f, 1.0f);
    colors[ImGuiCol_TextDisabled]    = ImVec4(0.65f, 0.75f, 0.75f, 1.0f);
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Application state
    GribReader reader;
    Renderer renderer;
    char filename[512] = "";


    // Load file from command line if provided
    std::vector<long> messageOffsets;
    static ImVec2 yScanDirectionA = ImVec2(0, 0);
    static ImVec2 yScanDirectionB = ImVec2(1, 1);

    if (argc > 1) {
        strcpy(filename, argv[1]);
        reader.loadFile(filename, yScanDirectionA, yScanDirectionB);
        }

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        showMainwindow(renderer, filename, reader, yScanDirectionA, yScanDirectionB, window, io);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

