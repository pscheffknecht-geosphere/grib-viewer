#include "renderer.h"


Renderer::Renderer() {
}

Renderer::~Renderer() {
}

GLuint Renderer::createTexture(const int displayWidth, const int displayHeight) {
    GLuint tex = 0;

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    // Filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Wrapping (important to define explicitly)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Allocate storage (NO DATA YET)
    glTexImage2D(
        GL_TEXTURE_2D,
        0,                  // mip level
        GL_RGB32F,          // internal format (float texture)
        displayWidth,
        displayHeight,
        0,
        GL_RGB,             // format of incoming data
        GL_FLOAT,           // type
        nullptr             // no data yet
    );

    glBindTexture(GL_TEXTURE_2D, 0);

    return tex;
}

void Renderer::updateTexture(GLuint texture,
                             int width,
                             int height,
                             const std::vector<Color>& data)
{
    if (data.size() != static_cast<size_t>(width * height))
        return; // or assert

    glBindTexture(GL_TEXTURE_2D, texture);

    glTexSubImage2D(
        GL_TEXTURE_2D,
        0,              // mip level
        0, 0,           // xoffset, yoffset
        width,
        height,
        GL_RGB,
        GL_FLOAT,
        data.data()
    );

    glBindTexture(GL_TEXTURE_2D, 0);
}

Color valueToColor(double value, double min_val, double max_val, const Gradient& gradient) {
    // Normalize value to 0-1
    double normalized = (value - min_val) / (max_val - min_val);
    normalized = std::clamp(normalized, 0.0, 1.0);
    
    // Color c = value >= -999999999. ? Color(.7f, 0.f, 0.f) : gradient.get_color(static_cast<float>(normalized));
    Color c = gradient.get_color(static_cast<float>(normalized));
    return c;
}


void Renderer::renderField(const GribField& field, int displayWidth, int displayHeight, 
    GribViewerSettings& settings, std::vector<Color>& imgData) {
    if (field.values.empty() || field.width == 0 || field.height == 0) {
        ImGui::Text("No data to display");
        return;
    }
    
    // Fill each pixel
    # pragma omp parallel for
    for (size_t y = 0; y < displayHeight; ++y) {
        for (size_t x = 0; x < displayWidth; ++x) {
            
            int fieldPosX = x * field.width / displayWidth;
            int fieldPosY = y * field.height / displayHeight;
            size_t idxField = field.width * fieldPosY + fieldPosX;
            size_t idxImg = y * displayWidth + x;

            double value = field.values[idxField];
            imgData[idxImg] = valueToColor(value, field.min_value, field.max_value, settings.gradient);
            
        }
    }
}
