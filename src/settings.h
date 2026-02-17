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
};

bool operator==(const GribViewerSettings& lhs, const GribViewerSettings& rhs);
bool operator!=(const GribViewerSettings& lhs, const GribViewerSettings& rhs);

bool cmapUpgradeNeeded(const GribViewerSettings& oldSettings, const GribViewerSettings& newSettings);