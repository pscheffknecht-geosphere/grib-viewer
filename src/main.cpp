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

    // Setup style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Application state
    GribReader reader;
    Renderer renderer;
    GribField currentField;
    bool fileLoaded = false;
    char filename[512] = "";
    int currentMessage = 0;
    int previousMessage = -1;
    int messageCount = 0;

    GLuint fieldTexture;
    bool needNewTexture = true;
    int displayWidth = 0;
    int displayHeight = 0;
    std::vector<Color> imgData;
    ImVec2 yScanDirectionA = ImVec2(0, 0);
    ImVec2 yScanDirectionB = ImVec2(1, 1);

    // Load file from command line if provided
    if (argc > 1) {
        strcpy(filename, argv[1]);
        if (reader.openFile(filename)) {
            fileLoaded = true;
            messageCount = reader.getMessageCount();
            if (messageCount > 0) {
                reader.readField(0, currentField);
            }
        }
    }

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        static GribViewerSettings settings;
        glfwPollEvents();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Main window
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("GRIB Viewer", nullptr, 
                     ImGuiWindowFlags_NoTitleBar | 
                     ImGuiWindowFlags_NoResize | 
                     ImGuiWindowFlags_NoMove);

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
                    if (currentField.yScansNegatively) {
                        yScanDirectionA = ImVec2(0, 1);
                        yScanDirectionB = ImVec2(1, 0);
                    }
                    else {
                        yScanDirectionA = ImVec2(0, 0);
                        yScanDirectionB = ImVec2(1, 1);

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
            ImGui::Text("Field: %s (%s) (indicatorOfParameter: %d) on typeOfLevel %s", currentField.name.c_str(), currentField.shortName.c_str(),
                        currentField.indicatorOfParameter, currentField.indicatorOfTypeOfLevel.c_str());
            ImGui::Text("Level: %d", currentField.level);
            ImGui::Text("Units: %s", currentField.units.c_str());
            ImGui::Text("Dimensions: %d x %d", currentField.width, currentField.height);
            ImGui::Text("Value range: %.6f to %.6f", currentField.min_value, currentField.max_value);

            ImGui::Separator();

            // Visualization
            ImGui::Text("Visualization:");
            static int previousZoomFactor = 1;
            ImGui::SliderInt("Display Zoom Factor", &settings.displayZoomFactor, 1, 10);
            if (settings.displayZoomFactor != previousZoomFactor) {
                needNewTexture = true;
                previousZoomFactor = settings.displayZoomFactor;
            }
            static int currentGradient = 0;
            static int previousGradient = 0;
            if (ImGui::BeginCombo("Gradient", mpl_gradient_names[currentGradient].c_str()))
            {
                for (int i = 0; i < mpl_gradient_names.size(); ++i)
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
                    previousGradient = currentGradient;
                }
            }
            ImGui::BeginChild("Visualization", ImVec2(0, 0), true);
            
            
            if (needNewTexture) {
                displayWidth = currentField.width * settings.displayZoomFactor;
                displayHeight = displayWidth * currentField.height / currentField.width; 
                imgData.resize(displayHeight * displayWidth);
                fieldTexture = renderer.createTexture(displayWidth, displayHeight);
                needNewTexture = false;
                updateImg = true;
            }

            if (updateImg) {
                renderer.renderField(currentField, displayWidth, displayHeight, settings, imgData);
                renderer.updateTexture(fieldTexture, displayWidth, displayHeight, imgData);
                updateImg = false;
            }
            ImGui::Image((ImTextureID)(intptr_t)fieldTexture, ImVec2(displayWidth, displayHeight)); //, ImVec2(0,1), ImVec2(0, 1)); //yScanDirection); // uv1);
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
