#pragma once

#include <vector>
#include <imgui.h>
#include <algorithm>
#include <cmath>
#include <iostream>

#include "grib_reader.h"
#include "mpl_gradients.h"
#include "settings.h"

class Renderer {
public:
    Renderer();
    ~Renderer();
    
    void renderField(const GribField& field, int width, int height, GribViewerSettings& settings);
    
};
