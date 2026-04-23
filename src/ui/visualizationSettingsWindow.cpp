#include "visualizationSettingsWindow.h"

void visualizationSettingsWindow(GribViewerSettings& settings, GribField& field) {
    // Visualization
    ImGui::Begin("Visualization Settings");
    {
        ImGui::Text("Visualization:");
        ImGui::SliderInt("Display Zoom Factor", &settings.displayZoomFactor, 1, 10);
        ImGui::Checkbox("Use custom min and max", &settings.useCustomMinMax);
        if (settings.useCustomMinMax) {
            settings.symmetricAroundZero = false;
            ImGui::InputFloat("Minimum", &settings.minVal, 0.f, 0.f, "%.8f");
            ImGui::InputFloat("Maximum", &settings.maxVal, 0.f, 0.f, "%.8f");
        }
        ImGui::Checkbox("Set symmetric around zero", &settings.symmetricAroundZero);
        ImGui::Checkbox("Use sqrt scaling", &settings.sqrtScale);
        ImGui::Checkbox("Use discrete colors", &settings.discreteColors);
        ImGui::InputInt("Color count (for discrete)", (int*)&settings.colorCount);
        ImGui::Checkbox("Old color bug (for aesthetics)", &settings.oldColorBug);
        if (settings.symmetricAroundZero) {
            settings.useCustomMinMax = false;
            float absMax = std::max(std::abs(field.min_value), std::abs(field.max_value));
            settings.minVal = -absMax;
            settings.maxVal = absMax;
        }

        ImGui::Separator();
        ImGui::Text("HCL color adjustments:");
        auto resetOnDoubleClick = [](float* v, float def) {
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                *v = def;
        };
        ImGui::SliderFloat("Brightness", &settings.brightness, 0.0f, 2.0f, "%.2f");
        resetOnDoubleClick(&settings.brightness, 1.0f);
        ImGui::SliderFloat("Gamma", &settings.gamma, 0.1f, 3.0f, "%.2f");
        resetOnDoubleClick(&settings.gamma, 1.0f);
        ImGui::SliderFloat("Vibrancy", &settings.vibrancy, 0.0f, 2.0f, "%.2f");
        resetOnDoubleClick(&settings.vibrancy, 1.0f);
        ImGui::SliderFloat("Hue shift", &settings.hueShift, -180.0f, 180.0f, "%.1f deg");
        resetOnDoubleClick(&settings.hueShift, 0.0f);
        if (ImGui::Button("Reset HCL")) {
            settings.brightness = 1.0f;
            settings.gamma = 1.0f;
            settings.vibrancy = 1.0f;
            settings.hueShift = 0.0f;
        }
    }
}