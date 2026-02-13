#include "renderer.h"


Renderer::Renderer() {
}

Renderer::~Renderer() {
}

Color valueToColor(double value, double min_val, double max_val, const Gradient& gradient) {
    // Normalize value to 0-1
    double normalized = (value - min_val) / (max_val - min_val);
    normalized = std::clamp(normalized, 0.0, 1.0);
    
    // Color c = value >= -999999999. ? Color(.7f, 0.f, 0.f) : gradient.get_color(static_cast<float>(normalized));
    Color c = gradient.get_color(static_cast<float>(normalized));
    return c;
}


void Renderer::renderField(const GribField& field, int displayWidth, int displayHeight, GribViewerSettings& settings) {
    if (field.values.empty() || field.width == 0 || field.height == 0) {
        ImGui::Text("No data to display");
        return;
    }
    
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
    
    //Gradient gradient = YlGnBu;

    // Calculate pixel size
    float pixel_width = static_cast<float>(displayWidth) / field.width;
    float pixel_height = static_cast<float>(displayHeight) / field.height;
    
    // const Gradient gradient = mpl_gradients::rainbow;
    
    // Draw each grid cell
    for (size_t y = 0; y < field.height; ++y) {
        for (size_t x = 0; x < field.width; ++x) {
            size_t idx = (field.height - 1 - y) * field.width + x;
            if (idx >= field.values.size()) continue;
            
            double value = field.values[idx];
            Color color = valueToColor(value, field.min_value, field.max_value, settings.gradient);
            // if (x % 50 == 0 && y % 50 == 0) {
            //     std::cout << "Value: " << value << " Color: (" << color.r << ", " << color.g << ", " << color.b << ")\n";
            // }
            ImVec2 p_min(canvas_pos.x + x * pixel_width, 
                        canvas_pos.y + y * pixel_height);
            ImVec2 p_max(canvas_pos.x + (x + 1) * pixel_width, 
                        canvas_pos.y + (y + 1) * pixel_height);
            
            ImU32 col = ImGui::ColorConvertFloat4ToU32(ImVec4(color.r, color.g, color.b, 1.0f));
            draw_list->AddRectFilled(p_min, p_max, col);
        }
    }
    
    // Move cursor forward
    ImGui::Dummy(ImVec2(static_cast<float>(displayWidth), static_cast<float>(displayHeight)));
}
