#pragma once

#include <imgui.h>
#include "../settings.h"
#include "../grib_reader.h"

void visualizationSettingsWindow(GribViewerSettings& settings,
    GribField& currentField);
