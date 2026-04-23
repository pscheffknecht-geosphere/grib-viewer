#include "mainWindow.h"

#include <cstring>
#include <string>

static int findSortedPos(const std::vector<GribMessageInfo>& list, size_t globalIndex) {
    for (int i = 0; i < static_cast<int>(list.size()); ++i) {
        if (list[i].globalIndex == globalIndex) return i;
    }
    return -1;
}

static void splitMultiSelect(const char* selected, std::vector<std::string>& out) {
    out.clear();
    if (!selected) return;
    const char* start = selected;
    for (const char* p = selected;; ++p) {
        if (*p == '|' || *p == '\0') {
            if (p > start) out.emplace_back(start, p - start);
            if (*p == '\0') break;
            start = p + 1;
        }
    }
}

void showMainwindow(Renderer& renderer, char filename[512], GribCollection& collection,
                    ImVec2& yScanDirectionA, ImVec2& yScanDirectionB, GLFWwindow* window,
                    ImGuiIO& io) {
    (void)yScanDirectionA;
    (void)yScanDirectionB;
    static int currentMessage = 0;  // globalIndex
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

    static bool genGradientPreview = true;
    static float previewBrightness = -1.0f;
    static float previewGamma = -1.0f;
    static float previewVibrancy = -1.0f;
    static float previewHueShift = 0.0f;
    if (previewBrightness != settings.brightness ||
        previewGamma != settings.gamma ||
        previewVibrancy != settings.vibrancy ||
        previewHueShift != settings.hueShift) {
        genGradientPreview = true;
        previewBrightness = settings.brightness;
        previewGamma = settings.gamma;
        previewVibrancy = settings.vibrancy;
        previewHueShift = settings.hueShift;
    }
    if (genGradientPreview) {
        for ( auto& grad : mpl_gradients) {
            rebuildGradientPreview(grad, settings.brightness, settings.gamma,
                                   settings.vibrancy, settings.hueShift);
        }
        genGradientPreview = false;
    }

    glfwPollEvents();
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

    ImGui::Begin("DockSpaceHost", nullptr, window_flags | ImGuiWindowFlags_MenuBar);

    ImGui::PopStyleVar(2);

    bool doOpen = false;
    bool doExport = false;
    bool doCopy = false;

    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_O)) doOpen = true;
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_E)) doExport = true;
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_C)) doCopy = true;

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open", "Ctrl+O")) doOpen = true;
            if (ImGui::MenuItem("Export Image", "Ctrl+E", false, !imgData.empty())) doExport = true;
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Copy Image", "Ctrl+C", false, !imgData.empty())) doCopy = true;
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    if (doOpen) {
        const char* filters[] = { "*.grib", "*.grib2", "*.grb", "*.grb2", "*" };
        const char* selected = tinyfd_openFileDialog(
            "Open GRIB File(s)", "", 5, filters, "GRIB Files", 1);
        if (selected) {
            std::vector<std::string> paths;
            splitMultiSelect(selected, paths);
            if (!paths.empty()) {
                strncpy(filename, paths.front().c_str(), 511);
                filename[511] = '\0';
                collection.beginLoad(paths);
                currentMessage = 0;
                previousMessage = -1;
                needNewTexture = true;
                updateCbarTexture = true;
            }
        }
    }
    if (doExport && !imgData.empty()) {
        const char* filters[] = { "*.png" };
        const char* selected = tinyfd_saveFileDialog(
            "Export Image", "export.png", 1, filters, "PNG Images");
        if (selected) {
            if (exportImagePng(selected, displayWidth, displayHeight, imgData, collection.currentField.jScansPositively)) {
                std::cout << "Exported image to " << selected << std::endl;
            } else {
                std::cerr << "Failed to export image to " << selected << std::endl;
            }
        }
    }
    if (doCopy && !imgData.empty()) {
        if (copyImageToClipboard(displayWidth, displayHeight, imgData, collection.currentField.jScansPositively)) {
            std::cout << "Image copied to clipboard" << std::endl;
        } else {
            std::cerr << "Failed to copy image to clipboard" << std::endl;
        }
    }

    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

    ImGui::End();

    ImGui::Begin("GRIB Viewer");

    ImGui::Separator();
    static bool updateImg = false;
    if (collection.fileLoaded && !collection.messageList.empty()) {
        const int msgCount = static_cast<int>(collection.messageList.size());
        if (currentMessage >= msgCount) currentMessage = msgCount - 1;
        if (currentMessage < 0) currentMessage = 0;

        if (ImGui::Button("Show Messages Window")) {
            showMessageListWindow = !showMessageListWindow;
        }
        bool showGribSortWindow = true;
        gribSortWindow(&showGribSortWindow, collection.messageList);
        if (showMessageListWindow) {
            gribMessageListWindow(&showMessageListWindow, collection.messageList, currentMessage);
        }

        // Up/Down: walk the sorted list. Left/Right: walk steps within current field identity.
        if (!ImGui::GetIO().WantTextInput) {
            if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) || ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
                int pos = findSortedPos(collection.messageList,
                                        static_cast<size_t>(currentMessage));
                if (pos >= 0) {
                    if (ImGui::IsKeyPressed(ImGuiKey_DownArrow) && pos < msgCount - 1)
                        currentMessage = static_cast<int>(collection.messageList[pos + 1].globalIndex);
                    else if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) && pos > 0)
                        currentMessage = static_cast<int>(collection.messageList[pos - 1].globalIndex);
                }
            }
            if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) || ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
                const auto& cur = collection.messageList[
                    findSortedPos(collection.messageList,
                                  static_cast<size_t>(currentMessage))];
                const auto* steps = collection.stepsFor(fieldIdentityOf(cur));
                if (steps && !steps->empty()) {
                    int stepPos = -1;
                    for (int i = 0; i < static_cast<int>(steps->size()); ++i) {
                        if ((*steps)[i] == static_cast<size_t>(currentMessage)) {
                            stepPos = i;
                            break;
                        }
                    }
                    if (stepPos >= 0) {
                        if (ImGui::IsKeyPressed(ImGuiKey_RightArrow) &&
                            stepPos < static_cast<int>(steps->size()) - 1)
                            currentMessage = static_cast<int>((*steps)[stepPos + 1]);
                        else if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) && stepPos > 0)
                            currentMessage = static_cast<int>((*steps)[stepPos - 1]);
                    }
                }
            }
        }

        ImGui::Text("Message: %d / %d", currentMessage + 1, msgCount);
        ImGui::SliderInt("##message", &currentMessage, 0, msgCount - 1);

        // Forecast step slider within the current field identity
        int sortedPos = findSortedPos(collection.messageList,
                                      static_cast<size_t>(currentMessage));
        if (sortedPos >= 0) {
            const auto& cur = collection.messageList[sortedPos];
            const auto* steps = collection.stepsFor(fieldIdentityOf(cur));
            if (steps && steps->size() > 1) {
                int stepPos = 0;
                for (int i = 0; i < static_cast<int>(steps->size()); ++i) {
                    if ((*steps)[i] == static_cast<size_t>(currentMessage)) {
                        stepPos = i;
                        break;
                    }
                }
                int oldStepPos = stepPos;
                ImGui::Text("Forecast step: %ld %s (%d of %d for this field)",
                            cur.step, cur.stepUnits.c_str(),
                            stepPos + 1, static_cast<int>(steps->size()));
                ImGui::SliderInt("##step", &stepPos, 0,
                                 static_cast<int>(steps->size()) - 1);
                if (stepPos != oldStepPos) {
                    currentMessage = static_cast<int>((*steps)[stepPos]);
                }
            }
        }

        if (currentMessage != previousMessage) {
            collection.readField(static_cast<size_t>(currentMessage), collection.currentField);
            previousMessage = currentMessage;
            updateImg = true;
        }

        ImGui::Separator();

        const auto& cur = collection.messageList[
            findSortedPos(collection.messageList, static_cast<size_t>(currentMessage))];

        ImGui::Text("Field: %s (%s) (indicatorOfParameter: %ld) on typeOfLevel %s",
                    collection.currentField.name.c_str(), collection.currentField.shortName.c_str(),
                    collection.currentField.indicatorOfParameter,
                    collection.currentField.indicatorOfTypeOfLevel.c_str());
        ImGui::Text("    parameterNumber = %ld, category = %ld, discipline = %ld",
                    collection.currentField.parameterNumber, collection.currentField.parameterCategory,
                    collection.currentField.discipline);
        ImGui::Text("Level: %ld", collection.currentField.level);
        ImGui::Text("    Type of level: %s (typeOfFirstFixedSurface: %s)",
                    collection.currentField.typeOfLevel.c_str(),
                    collection.currentField.typeOfFirstFixedSurface.c_str());
        ImGui::Text("Units: %s", collection.currentField.units.c_str());
        ImGui::Text("Dimensions: %ld x %ld", collection.currentField.width, collection.currentField.height);
        ImGui::Text("Value range: %.6f to %.6f", collection.currentField.min_value,
                    collection.currentField.max_value);
        ImGui::Text("Plot value range: %.6f to %.6f", settings.minVal, settings.maxVal);

        if (cur.fileIdx < collection.filenames.size()) {
            ImGui::Text("Source: %s", collection.filenames[cur.fileIdx].c_str());
        }
        ImGui::Text("Forecast step: %ld %s   Valid: %ld %04ld",
                    cur.step, cur.stepUnits.c_str(), cur.validityDate, cur.validityTime);

        if (!collection.duplicateWarnings.empty()) {
            ImGui::Separator();
            ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.2f, 1.0f),
                               "Warnings (%zu):", collection.duplicateWarnings.size());
            for (const auto& w : collection.duplicateWarnings) {
                ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.2f, 1.0f), "  %s", w.c_str());
            }
        }

        ImGui::Separator();

        visualizationSettingsWindow(settings, collection.currentField);
        ImGui::End();
        static size_t currentGradient = 0;
        static size_t previousGradient = 0;
        if (ImGui::BeginCombo("Gradient", mpl_gradient_names[currentGradient].c_str(), ImGuiComboFlags_HeightLarge)) {
            for (size_t i = 0; i < mpl_gradient_names.size(); ++i) {
                bool is_selected = (currentGradient == i);

                if (ImGui::Selectable(mpl_gradient_names[i].c_str(), is_selected)) {
                    currentGradient = i;
                    settings.gradient = mpl_gradients[i];
                }

                ImGui::SameLine(200);

                ImGui::Image((ImTextureID)(intptr_t)mpl_gradients[i].previewTexture, ImVec2(400, 18));

                if (is_selected) ImGui::SetItemDefaultFocus();
            }

            ImGui::EndCombo();
        }
        ImGui::SameLine();
        if (ImGui::Button("Reverse")) {
            const std::string& name = mpl_gradient_names[currentGradient];
            std::string target = (name.size() >= 2 &&
                                  name.compare(name.size() - 2, 2, "_r") == 0)
                                 ? name.substr(0, name.size() - 2)
                                 : name + "_r";
            for (size_t i = 0; i < mpl_gradient_names.size(); ++i) {
                if (mpl_gradient_names[i] == target) {
                    currentGradient = i;
                    settings.gradient = mpl_gradients[i];
                    break;
                }
            }
        }
        if (currentGradient != previousGradient) {
            needNewTexture = true;
            updateCbarTexture = true;
            previousGradient = currentGradient;
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
            displayWidth = collection.currentField.width;
            displayHeight = collection.currentField.height;
            imgData.resize(displayHeight * displayWidth);
            glDeleteTextures(1, &fieldTexture);
            fieldTexture = 0;
            fieldTexture = renderer.createTexture(displayWidth, displayHeight);
            needNewTexture = false;
            updateImg = true;
        }

        if (updateImg) {
            renderer.renderField(collection.currentField, displayWidth, displayHeight, settings,
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
        ImGui::Begin("Field Display", nullptr, ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar);
        ImGui::Image((ImTextureID)(intptr_t)fieldTexture,
                     ImVec2(displayWidth * settings.displayZoomFactor,
                            displayHeight * settings.displayZoomFactor),
                     collection.currentField.uv1, collection.currentField.uv2);
        ImGui::End();
        ImGui::EndChild();
    } else {
        ImGui::Text("No GRIB file loaded.");
        ImGui::Text("Please specify a GRIB file path and click 'Load'");
        ImGui::Text("Or run with: ./GribViewer <path-to-grib-file> [more-files...]");
    }

    ImGui::End();

    static bool loadingPopupOpen = false;
    if (collection.loading && !loadingPopupOpen) {
        ImGui::OpenPopup("Loading GRIB files");
        loadingPopupOpen = true;
    }
    if (ImGui::BeginPopupModal("Loading GRIB files", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
        size_t total = collection.pendingPaths.size();
        size_t done = collection.loadCursor;
        size_t currentIdx = collection.loading && done < total ? done : total;
        ImGui::Text("Loading file %zu of %zu", currentIdx + (collection.loading ? 1 : 0), total);
        if (collection.loading && done < total) {
            ImGui::TextUnformatted(collection.pendingPaths[done].c_str());
        }
        float frac = total > 0 ? static_cast<float>(done) / static_cast<float>(total) : 0.0f;
        ImGui::ProgressBar(frac, ImVec2(400, 0));
        if (!collection.loading) {
            ImGui::CloseCurrentPopup();
            loadingPopupOpen = false;
        }
        ImGui::EndPopup();
    }

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

    if (collection.loading) {
        collection.loadNext();
    }
}
