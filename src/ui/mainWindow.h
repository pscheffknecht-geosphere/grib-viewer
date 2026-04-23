#pragma once

#include <imgui.h>
#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "grib_reader.h"
#include "grib_collection.h"
#include "mpl_gradients.h"
#include "renderer.h"
#include "settings.h"
#include "color_adjust.h"

#include <tinyfiledialogs.h>

#include "messagesWindow.h"
#include "visualizationSettingsWindow.h"
#include "export_image.h"

void showMainwindow(Renderer& renderer, char filename[512], GribCollection& collection,
                    ImVec2& yScanDirectionA, ImVec2& yScanDirectionB, GLFWwindow* window,
                    ImGuiIO& io);

