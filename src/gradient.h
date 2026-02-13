#pragma once

#include <cmath>
#include <vector>

struct Color {
    float r, g, b;
    Color() : r(0), g(0), b(0) {}
    Color(float r, float g, float b) : r(r), g(g), b(b) {}
    Color(float s) : r(s), g(s), b(s) {}
    ~Color() {}
};

inline Color linear2srgb(const Color& c) {
    return Color(
        c.r <= 0.0031308f ? 12.92f * c.r : 1.055f * std::pow(c.r, 1.0f / 2.4f) - 0.055f,
        c.g <= 0.0031308f ? 12.92f * c.g : 1.055f * std::pow(c.g, 1.0f / 2.4f) - 0.055f,
        c.b <= 0.0031308f ? 12.92f * c.b : 1.055f * std::pow(c.b, 1.0f / 2.4f) - 0.055f
    );
}

inline Color srgb2linear(const Color& c) {
    return Color(
        c.r <= 0.04045f ? c.r / 12.92f : std::pow((c.r + 0.055f) / 1.055f, 2.4f),
        c.g <= 0.04045f ? c.g / 12.92f : std::pow((c.g + 0.055f) / 1.055f, 2.4f),
        c.b <= 0.04045f ? c.b / 12.92f : std::pow((c.b + 0.055f) / 1.055f, 2.4f)
    );
}

class Gradient {
    public:
    std::vector<Color> colors;
    std::vector<float> positions; // 0 to 1
    Gradient() {
        // Default gradient: black to white
        colors.push_back(Color(0.0f));
        colors.push_back(Color(1.0f));
        positions.push_back(0.0f);
        positions.push_back(1.0f);
    }   
    Gradient(const std::vector<Color>& colors, const std::vector<float>& positions) : colors(colors), positions(positions) {}
    ~Gradient() {}
    Color get_color(float t) const {
        if (t <= positions.front()) return colors.front();
        if (t >= positions.back()) return colors.back();
        for (size_t i = 0; i < positions.size() - 1; ++i) {
            if (t >= positions[i] && t <= positions[i + 1]) {
                float local_t = (t - positions[i]) / (positions[i + 1] - positions[i]);
                return linear2srgb(Color(
                    colors[i].r + local_t * (colors[i + 1].r - colors[i].r),
                    colors[i].g + local_t * (colors[i + 1].g - colors[i].g),
                    colors[i].b + local_t * (colors[i + 1].b - colors[i].b)
                ));
            }
        }
        return Color(1.f, 0.f, 1.f); // Should never reach here
    }
};

