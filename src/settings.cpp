#include "settings.h"

bool operator==(GribViewerSettings& lhs, GribViewerSettings& rhs) {
    return (
        lhs.displayZoomFactor == rhs.displayZoomFactor &&
        lhs.minVal == rhs.minVal &&
        lhs.maxVal == rhs.maxVal &&
        lhs.useCustomMinMax == rhs.useCustomMinMax
    );
}

bool operator!=(GribViewerSettings& lhs, GribViewerSettings& rhs) {
    return ! (lhs == rhs);
}