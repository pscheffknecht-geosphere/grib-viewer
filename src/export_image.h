#pragma once

#include <string>
#include <vector>
#include "gradient.h"

bool exportImagePng(const std::string& filename,
                    int width, int height,
                    const std::vector<Color>& imgData,
                    bool flipVertically = false);

bool copyImageToClipboard(int width, int height,
                          const std::vector<Color>& imgData,
                          bool flipVertically = false);
