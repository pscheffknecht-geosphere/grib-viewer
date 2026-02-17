#pragma once

#include "gradient.h"
#include "mpl_gradients.h"

struct GribViewerSettings {
    int displayZoomFactor = 1;
    Gradient gradient = mpl_gradients[0];
    float minVal = 0.;
    float maxVal = 1.;
    bool discreteColors = false;
    uint32_t colorCount = 25;
    bool sqrtScale = false;
    bool useCustomMinMax = false;
    GribViewerSettings operator=(const GribViewerSettings& other) {
        displayZoomFactor = other.displayZoomFactor;
        gradient = other.gradient;
        minVal = other.minVal;
        maxVal = other.maxVal;
        discreteColors = other.discreteColors;
        colorCount = other.colorCount;
        sqrtScale = other.sqrtScale;
        useCustomMinMax = other.useCustomMinMax;
        return *this;
    }
};

bool operator==(const GribViewerSettings& lhs, const GribViewerSettings& rhs);
bool operator!=(const GribViewerSettings& lhs, const GribViewerSettings& rhs);

bool cmapUpgradeNeeded(const GribViewerSettings& oldSettings, const GribViewerSettings& newSettings);