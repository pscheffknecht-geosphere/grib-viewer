#include "mainWindow.h"

void showMainwindow(Renderer& renderer, char filename[512], GribReader& reader,
                    ImVec2& yScanDirectionA, ImVec2& yScanDirectionB, GLFWwindow* window,
                    ImGuiIO& io) {
    static int currentMessage = 0;
    static int previousMessage = -1;

    // stuff for the main image
    static GLuint fieldTexture;
    static bool needNewTexture = true;
    static int displayWidth = 0;
    static int displayHeight = 0;
    static std::vector<Color> imgData;

    // add a color bar
    static bool updateCbarTexture = true;
    static int cbarHeight = 20;
    static int cbarWidth = 500;
    static GLuint cbarTexture = renderer.createTexture(cbarWidth, cbarHeight);
    static std::vector<Color> cbarData;
    cbarData.resize(cbarWidth * cbarHeight);

    // sub windows
    static bool showMessageListWindow = true;
    static GribViewerSettings settings;
    static GribViewerSettings settings_old;

    glfwPollEvents();
    // ImGuiID dockspace_id = ImGui::GetID("GRIB Viewer");
    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

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
        reader.loadFile(filename);
    }

    ImGui::Separator();
    static bool updateImg = false;
    if (reader.fileLoaded && reader.messageCount > 0) {
        if (ImGui::Button("Show Messages Window")) {
            showMessageListWindow = !showMessageListWindow;
        }
        bool showGribSortWindow = true;
        gribSortWindow(&showGribSortWindow, reader.messageList);
        if (showMessageListWindow) {
            gribMessageListWindow(&showMessageListWindow, reader.messageList, currentMessage);
        }
        // Message selection
        ImGui::Text("Message: %d / %d", currentMessage + 1, reader.messageCount);
        ImGui::SliderInt("##message", &currentMessage, 0, reader.messageCount - 1);
        if (currentMessage != previousMessage) {
            reader.readField(currentMessage, reader.currentField);
            previousMessage = currentMessage;
            updateImg = true;
        }

        ImGui::Separator();

        // Field information
        ImGui::Text("Field: %s (%s) (indicatorOfParameter: %ld) on typeOfLevel %s",
                    reader.currentField.name.c_str(), reader.currentField.shortName.c_str(),
                    reader.currentField.indicatorOfParameter,
                    reader.currentField.indicatorOfTypeOfLevel.c_str());
        ImGui::Text("    parameterNumber = %ld, category = %ld, discipline = %ld",
                    reader.currentField.parameterNumber, reader.currentField.parameterCategory,
                    reader.currentField.discipline);
        ImGui::Text("Level: %ld", reader.currentField.level);
        ImGui::Text("    Type of level: %s (typeOfFirstFixedSurface: %s)",
                    reader.currentField.typeOfLevel.c_str(),
                    reader.currentField.typeOfFirstFixedSurface.c_str());
        ImGui::Text("Units: %s", reader.currentField.units.c_str());
        ImGui::Text("Dimensions: %ld x %ld", reader.currentField.width, reader.currentField.height);
        ImGui::Text("Value range: %.6f to %.6f", reader.currentField.min_value,
                    reader.currentField.max_value);

        ImGui::Separator();

        visualizationSettingsWindow(settings);
        ImGui::End();
        static size_t currentGradient = 0;
        static size_t previousGradient = 0;
        if (ImGui::BeginCombo("Gradient", mpl_gradient_names[currentGradient].c_str())) {
            for (size_t i = 0; i < mpl_gradient_names.size(); ++i) {
                bool is_selected = (currentGradient == i);

                if (ImGui::Selectable(mpl_gradient_names[i].c_str(), is_selected)) {
                    currentGradient = i;
                    settings.gradient = mpl_gradients[i];
                }

                if (is_selected) ImGui::SetItemDefaultFocus();
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
            displayWidth = reader.currentField.width;
            displayHeight = reader.currentField.height;
            imgData.resize(displayHeight * displayWidth);
            glDeleteTextures(1, &fieldTexture);
            fieldTexture = 0;
            fieldTexture = renderer.createTexture(displayWidth, displayHeight);
            needNewTexture = false;
            updateImg = true;
        }

        if (updateImg) {
            renderer.renderField(reader.currentField, displayWidth, displayHeight, settings,
                                 imgData);
            renderer.updateTexture(fieldTexture, displayWidth, displayHeight, imgData);
            updateImg = false;
        }

        if (updateCbarTexture) {
            renderer.updateCbar(cbarTexture, cbarWidth, cbarHeight, cbarData, settings);
            renderer.updateTexture(cbarTexture, cbarWidth, cbarHeight, cbarData);
            updateCbarTexture = false;
        }
        ImGui::Image((ImTextureID)(intptr_t)cbarTexture, ImVec2(cbarWidth, cbarHeight));
        ImGui::Image((ImTextureID)(intptr_t)fieldTexture,
                     ImVec2(displayWidth * settings.displayZoomFactor,
                            displayHeight * settings.displayZoomFactor),
                     reader.currentField.uv1, reader.currentField.uv2);

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
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
    glfwSwapBuffers(window);
}
