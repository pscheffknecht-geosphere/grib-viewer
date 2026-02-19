#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>

#include "grib_reader.h"
#include "renderer.h"
#include "settings.h"
#include "mpl_gradients.h"
#include "messagesWindow.h"

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
    GribField currentField;
    std::vector<GribMessageInfo> messageList;
    bool fileLoaded = false;
    char filename[512] = "";
    int currentMessage = 0;
    int previousMessage = -1;
    int messageCount = 0;

    // stuff for the main image
    GLuint fieldTexture;
    bool needNewTexture = true;
    int displayWidth = 0;
    int displayHeight = 0;
    std::vector<Color> imgData;
    ImVec2 yScanDirectionA = ImVec2(0, 0);
    ImVec2 yScanDirectionB = ImVec2(1, 1);

    // add a color bar
    bool updateCbarTexture = true;
    int cbarHeight = 20;
    int cbarWidth = 500;
    GLuint cbarTexture = renderer.createTexture(cbarWidth, cbarHeight);
    std::vector<Color> cbarData;
    cbarData.resize(cbarWidth * cbarHeight);

    // sub windows
    bool showMessageListWindow = false;

    // Load file from command line if provided
    std::vector<long> messageOffsets;
    if (argc > 1) {
        strcpy(filename, argv[1]);
        if (reader.openFile(filename)) {
            fileLoaded = true;
            messageCount = reader.getMessageCount();
            if (messageCount > 0) {
                reader.getMessageOffsets();
                reader.readField(0, currentField);
                if (currentField.jScansPositively == 1) {
                    yScanDirectionA = ImVec2(0, 1);
                    yScanDirectionB = ImVec2(1, 0);
                }
                else {
                    yScanDirectionA = ImVec2(0, 0);
                    yScanDirectionB = ImVec2(1, 1);
                }
            }
            messageList.clear();
            for (int i = 0; i < messageCount; ++i)
            {
                GribMessageInfo info;
                reader.readFieldMetadata(i, info);  // lightweight version
                messageList.push_back(info);
            }
        }
    }

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        static GribViewerSettings settings;
        static GribViewerSettings settings_old;

        glfwPollEvents();
        // ImGuiID dockspace_id = ImGui::GetID("GRIB Viewer");
        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuiWindowFlags window_flags =
            ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        ImGui::Begin("DockSpaceHost", nullptr, window_flags);

        ImGui::PopStyleVar(2);

        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

        ImGui::End();
        // Main window
        // ImGui::SetNextWindowPos(ImVec2(0, 0));
        // ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("GRIB Viewer"); /*, nullptr, 
                     ImGuiWindowFlags_NoTitleBar | 
                     ImGuiWindowFlags_NoResize | 
                     ImGuiWindowFlags_NoMove); */

        // File selection
        ImGui::Text("GRIB File:");
        ImGui::SameLine();
        ImGui::InputText("##filename", filename, sizeof(filename));
        ImGui::SameLine();
        
        if (ImGui::Button("Load")) {
            reader.close();
            if (reader.openFile(filename)) {
                fileLoaded = true;
                messageCount = reader.getMessageCount();
                currentMessage = 0;
                if (messageCount > 0) {
                    reader.readField(currentMessage, currentField);
                    if (currentField.jScansPositively == 1) {
                        yScanDirectionA = ImVec2(0, 1);
                        yScanDirectionB = ImVec2(1, 0);
                    }
                    else {
                        yScanDirectionA = ImVec2(0, 0);
                        yScanDirectionB = ImVec2(1, 1);
                    }
                    messageList.clear();
                    for (int i = 0; i < messageCount; ++i)
                    {
                        GribMessageInfo info;
                        reader.readFieldMetadata(i, info);  // lightweight version
                        messageList.push_back(info);
                    }
                }
            } else {
                fileLoaded = false;
                std::cerr << "Error: " << reader.getLastError() << std::endl;
            }
        }

        ImGui::Separator();
        static bool updateImg = false;
        if (fileLoaded && messageCount > 0) {
            if (ImGui::Button("Show Messages Window")) {
                showMessageListWindow = !showMessageListWindow;
            }
            if (showMessageListWindow) {
                gribMessageListWindow(&showMessageListWindow, messageList, currentMessage);
            }
            // Message selection
            ImGui::Text("Message: %d / %d", currentMessage + 1, messageCount);
            ImGui::SliderInt("##message", &currentMessage, 0, messageCount - 1);
            if (currentMessage != previousMessage) {
                reader.readField(currentMessage, currentField);
                previousMessage = currentMessage;
                updateImg = true;
            }

            ImGui::Separator();

            // Field information
            ImGui::Text("Field: %s (%s) (indicatorOfParameter: %ld) on typeOfLevel %s", currentField.name.c_str(), currentField.shortName.c_str(),
                        currentField.indicatorOfParameter, currentField.indicatorOfTypeOfLevel.c_str());
            ImGui::Text("    parameterNumber = %ld, category = %ld, discipline = %ld", currentField.parameterNumber, 
                currentField.parameterCategory, currentField.discipline);
            ImGui::Text("Level: %ld", currentField.level);
            ImGui::Text("    Type of level: %s (typeOfFirstFixedSurface: %s)", currentField.typeOfLevel.c_str(), currentField.typeOfFirstFixedSurface.c_str());
            ImGui::Text("Units: %s", currentField.units.c_str());
            ImGui::Text("Dimensions: %ld x %ld", currentField.width, currentField.height);
            ImGui::Text("Value range: %.6f to %.6f", currentField.min_value, currentField.max_value);

            ImGui::Separator();

            // Visualization
            ImGui::Begin("Visualization Settings");
            {
                ImGui::Text("Visualization:");
                ImGui::SliderInt("Display Zoom Factor", &settings.displayZoomFactor, 1, 10);
                ImGui::Checkbox("Use custom min and max", &settings.useCustomMinMax);
                ImGui::InputFloat("Minimum", &settings.minVal, 0.f, 0.f, "%.8f");
                ImGui::InputFloat("Maximum", &settings.maxVal, 0.f, 0.f, "%.8f");
                ImGui::Checkbox("Use sqrt scaling", &settings.sqrtScale);
                ImGui::Checkbox("Use discrete colors", &settings.discreteColors);
                ImGui::InputInt("Color count (for discrete)", (int*)&settings.colorCount);
            }
            ImGui::End();
            static int currentGradient = 0;
            static int previousGradient = 0;
            if (ImGui::BeginCombo("Gradient", mpl_gradient_names[currentGradient].c_str()))
            {
                for (size_t i = 0; i < mpl_gradient_names.size(); ++i)
                {
                    bool is_selected = (currentGradient == i);

                    if (ImGui::Selectable(mpl_gradient_names[i].c_str(), is_selected))
                    {
                        currentGradient = i;
                        settings.gradient = mpl_gradients[i];
                    }

                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }

                ImGui::EndCombo();
                if (currentGradient != previousGradient) {
                    needNewTexture = true;
                    updateCbarTexture = true;
                    previousGradient = currentGradient;
                }
            }


            ImGui::BeginChild("Visualization", ImVec2(0, 0), true);
            
            if (settings != settings_old) {
                needNewTexture = true;
                if (cmapUpgradeNeeded(settings_old, settings)) {
                   updateCbarTexture = true;
                }
                settings_old = settings;
            }
            
            if (needNewTexture) {
                displayWidth = currentField.width * settings.displayZoomFactor;
                displayHeight = currentField.height * settings.displayZoomFactor; 
                imgData.resize(displayHeight * displayWidth);
                glDeleteTextures(1, &fieldTexture);
                fieldTexture = 0;
                fieldTexture = renderer.createTexture(displayWidth, displayHeight);
                needNewTexture = false;
                updateImg = true;
            }

            if (updateImg) {
                renderer.renderField(currentField, displayWidth, displayHeight, settings, imgData);
                renderer.updateTexture(fieldTexture, displayWidth, displayHeight, imgData);
                updateImg = false;
            }

            if (updateCbarTexture) {
                renderer.updateCbar(cbarTexture, cbarWidth, cbarHeight, cbarData, settings);
                renderer.updateTexture(cbarTexture, cbarWidth, cbarHeight, cbarData);
                updateCbarTexture = false;
            }
            ImGui::Image((ImTextureID)(intptr_t)cbarTexture, ImVec2(cbarWidth, cbarHeight));
            ImGui::Image((ImTextureID)(intptr_t)fieldTexture, ImVec2(displayWidth, displayHeight), yScanDirectionA, yScanDirectionB);

            
            ImGui::EndChild();
        } else {
            ImGui::Text("No GRIB file loaded.");
            ImGui::Text("Please specify a GRIB file path and click 'Load'");
            ImGui::Text("Or run with: ./GribViewer <path-to-grib-file>");
        }

        ImGui::End();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
