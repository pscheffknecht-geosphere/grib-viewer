#pragma once

#include "grib_reader.h"
#include <vector>

class Renderer {
public:
    Renderer();
    ~Renderer();
    
    void renderField(const GribField& field, int width, int height);
    
};
