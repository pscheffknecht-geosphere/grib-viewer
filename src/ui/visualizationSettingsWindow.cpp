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
    }
}