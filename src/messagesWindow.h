#pragma once

#include <imgui.h>

#include "grib_reader.h"

void gribMessageListWindow(
    bool* p_open,
    const std::vector<GribMessageInfo>& messageList,
    int& currentMessage);