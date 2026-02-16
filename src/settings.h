#pragma once

#include "gradient.h"
#include "mpl_gradients.h"

struct GribViewerSettings {
    int displayZoomFactor = 1;
    Gradient gradient = mpl_gradients[0];
    float minVal = 0.;
    float maxVal = 1.;
    bool useCustomMinMax = false;
};

bool operator==(GribViewerSettings& lhs, GribViewerSettings& rhs);
bool operator!=(GribViewerSettings& lhs, GribViewerSettings& rhs);