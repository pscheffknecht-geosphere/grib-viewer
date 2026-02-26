#pragma once

#include <imgui.h>
#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "grib_reader.h"
#include "mpl_gradients.h"
#include "renderer.h"
#include "settings.h"

#include "messagesWindow.h"
#include "visualizationSettingsWindow.h"

void showMainwindow(Renderer& renderer, char filename[512], GribReader& reader,
                    ImVec2& yScanDirectionA, ImVec2& yScanDirectionB, GLFWwindow* window,
                    ImGuiIO& io);

