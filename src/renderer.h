#pragma once

#include <vector>
#include <imgui.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <GL/gl.h>

#include "grib_reader.h"
#include "mpl_gradients.h"
#include "settings.h"

class Renderer {
public:
    Renderer();
    ~Renderer();
    GLuint createTexture(const int displayWidth, const int displayHeight);
    void updateTexture(GLuint texture,
                             int width,
                             int height,
                             const std::vector<Color>& data);
    
void updateCbar(GLuint texture, const int width, const int height, std::vector<Color>& data,
    GribViewerSettings& settings);
    
    void renderField(const GribField& field, int displayWidth, int displayHeight, 
    GribViewerSettings& settings, std::vector<Color>& imgData);

};
