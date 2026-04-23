#pragma once

#include <algorithm>
#include <cmath>

#include "gradient.h"

inline void rgbLinearToXyz(float r, float g, float b, float& X, float& Y, float& Z) {
    X = 0.4124564f * r + 0.3575761f * g + 0.1804375f * b;
    Y = 0.2126729f * r + 0.7151522f * g + 0.0721750f * b;
    Z = 0.0193339f * r + 0.1191920f * g + 0.9503041f * b;
}

inline void xyzToRgbLinear(float X, float Y, float Z, float& r, float& g, float& b) {
    r =  3.2404542f * X - 1.5371385f * Y - 0.4985314f * Z;
    g = -0.9692660f * X + 1.8760108f * Y + 0.0415560f * Z;
    b =  0.0556434f * X - 0.2040259f * Y + 1.0572252f * Z;
}

inline float labF(float t) {
    constexpr float delta = 6.0f / 29.0f;
    if (t > delta * delta * delta) return std::cbrt(t);
    return t / (3.0f * delta * delta) + 4.0f / 29.0f;
}

inline float labFinv(float t) {
    constexpr float delta = 6.0f / 29.0f;
    if (t > delta) return t * t * t;
    return 3.0f * delta * delta * (t - 4.0f / 29.0f);
}

inline void xyzToLab(float X, float Y, float Z, float& L, float& a, float& b) {
    constexpr float Xn = 0.95047f, Yn = 1.0f, Zn = 1.08883f;
    float fx = labF(X / Xn);
    float fy = labF(Y / Yn);
    float fz = labF(Z / Zn);
    L = 116.0f * fy - 16.0f;
    a = 500.0f * (fx - fy);
    b = 200.0f * (fy - fz);
}

inline void labToXyz(float L, float a, float b, float& X, float& Y, float& Z) {
    constexpr float Xn = 0.95047f, Yn = 1.0f, Zn = 1.08883f;
    float fy = (L + 16.0f) / 116.0f;
    float fx = fy + a / 500.0f;
    float fz = fy - b / 200.0f;
    X = Xn * labFinv(fx);
    Y = Yn * labFinv(fy);
    Z = Zn * labFinv(fz);
}

inline Color applyHclAdjustments(const Color& srgb,
                                 float brightness,
                                 float gamma,
                                 float vibrancy,
                                 float hueShiftDeg) {
    if (brightness == 1.0f && gamma == 1.0f && vibrancy == 1.0f && hueShiftDeg == 0.0f)
        return srgb;

    Color lin = srgb2linear(srgb);
    float X, Y, Z;
    rgbLinearToXyz(lin.r, lin.g, lin.b, X, Y, Z);

    float L, a, b;
    xyzToLab(X, Y, Z, L, a, b);

    float C = std::sqrt(a * a + b * b);
    float H = std::atan2(b, a);

    float Ln = std::clamp(L / 100.0f, 0.0f, 1.0f);
    Ln = std::pow(Ln, gamma);
    L = std::clamp(Ln * brightness, 0.0f, 1.0f) * 100.0f;

    C = std::max(0.0f, C * vibrancy);

    constexpr float pi = 3.14159265358979323846f;
    H += hueShiftDeg * pi / 180.0f;

    a = C * std::cos(H);
    b = C * std::sin(H);

    labToXyz(L, a, b, X, Y, Z);

    float r, g, bl;
    xyzToRgbLinear(X, Y, Z, r, g, bl);

    Color linOut(std::clamp(r, 0.0f, 1.0f),
                 std::clamp(g, 0.0f, 1.0f),
                 std::clamp(bl, 0.0f, 1.0f));
    return linear2srgb(linOut);
}

inline void rebuildGradientPreview(Gradient& grad,
                                   float brightness,
                                   float gamma,
                                   float vibrancy,
                                   float hueShift) {
    constexpr int previewWidth = 256;
    constexpr int previewHeight = 16;
    if (grad.previewData.size() != static_cast<size_t>(previewWidth * previewHeight))
        grad.previewData.resize(previewWidth * previewHeight);

    for (int x = 0; x < previewWidth; ++x) {
        float t = static_cast<float>(x) / (previewWidth - 1);
        Color col = applyHclAdjustments(grad.get_color(t), brightness, gamma, vibrancy, hueShift);
        for (int y = 0; y < previewHeight; ++y)
            grad.previewData[y * previewWidth + x] = col;
    }

    bool newTexture = (grad.previewTexture == 0);
    if (newTexture) glGenTextures(1, &grad.previewTexture);

    glBindTexture(GL_TEXTURE_2D, grad.previewTexture);
    if (newTexture) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, previewWidth, previewHeight, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, previewWidth, previewHeight, GL_RGB, GL_FLOAT, grad.previewData.data());
    glBindTexture(GL_TEXTURE_2D, 0);
}
