#include "settings.h"

bool operator==(const GribViewerSettings& lhs, const GribViewerSettings& rhs) {

    return (
        lhs.displayZoomFactor == rhs.displayZoomFactor &&
        lhs.minVal == rhs.minVal &&
        lhs.maxVal == rhs.maxVal &&
        lhs.discreteColors == rhs.discreteColors &&
        lhs.colorCount == rhs.colorCount &&
        lhs.sqrtScale == rhs.sqrtScale &&
        lhs.useCustomMinMax == rhs.useCustomMinMax &&
        lhs.symmetricAroundZero == rhs.symmetricAroundZero &&
        lhs.oldColorBug == rhs.oldColorBug &&
        lhs.brightness == rhs.brightness &&
        lhs.gamma == rhs.gamma &&
        lhs.vibrancy == rhs.vibrancy &&
        lhs.hueShift == rhs.hueShift
    );
}

bool operator!=(const GribViewerSettings& lhs, const GribViewerSettings& rhs) {
    return ! (lhs == rhs);
}

bool cmapUpgradeNeeded(const GribViewerSettings& oldSettings, const GribViewerSettings& newSettings) {
    return (
        oldSettings.sqrtScale != newSettings.sqrtScale ||
        oldSettings.discreteColors != newSettings.discreteColors ||
        oldSettings.colorCount != newSettings.colorCount ||
        oldSettings.oldColorBug != newSettings.oldColorBug ||
        oldSettings.brightness != newSettings.brightness ||
        oldSettings.gamma != newSettings.gamma ||
        oldSettings.vibrancy != newSettings.vibrancy ||
        oldSettings.hueShift != newSettings.hueShift
    );
}