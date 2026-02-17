#include "settings.h"

bool operator==(const GribViewerSettings& lhs, const GribViewerSettings& rhs) {

    return (
        lhs.displayZoomFactor == rhs.displayZoomFactor &&
        lhs.minVal == rhs.minVal &&
        lhs.maxVal == rhs.maxVal &&
        lhs.discreteColors == rhs.discreteColors &&
        lhs.colorCount == rhs.colorCount &&
        lhs.sqrtScale == rhs.sqrtScale &&
        lhs.useCustomMinMax == rhs.useCustomMinMax
    );
}

bool operator!=(const GribViewerSettings& lhs, const GribViewerSettings& rhs) {
    return ! (lhs == rhs);
}

bool cmapUpgradeNeeded(const GribViewerSettings& oldSettings, const GribViewerSettings& newSettings) {
    return (
        oldSettings.sqrtScale != newSettings.sqrtScale ||
        oldSettings.discreteColors != newSettings.discreteColors ||
        oldSettings.colorCount != newSettings.colorCount
    );
}