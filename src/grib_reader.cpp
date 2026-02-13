#include "grib_reader.h"
#include <eccodes.h>
#include <algorithm>
#include <cmath>
#include <iostream>

GribReader::GribReader() : fileHandle(nullptr) {
}

GribReader::~GribReader() {
    close();
}

bool GribReader::openFile(const std::string& fname) {
    close();
    
    filename = fname;
    FILE* f = fopen(filename.c_str(), "rb");
    if (!f) {
        lastError = "Cannot open file: " + filename;
        return false;
    }
    
    fileHandle = f;
    return true;
}

void GribReader::close() {
    if (fileHandle) {
        fclose(static_cast<FILE*>(fileHandle));
        fileHandle = nullptr;
    }
}

int GribReader::getMessageCount() const {
    if (!fileHandle) return 0;
    
    FILE* f = static_cast<FILE*>(fileHandle);
    long currentPos = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    int count = 0;
    int err = 0;
    codes_handle* h = nullptr;
    
    while ((h = codes_handle_new_from_file(nullptr, f, PRODUCT_GRIB, &err)) != nullptr) {
        count++;
        codes_handle_delete(h);
    }
    
    fseek(f, currentPos, SEEK_SET);
    return count;
}

bool GribReader::readField(int messageIndex, GribField& field) {
    if (!fileHandle) {
        lastError = "No file open";
        return false;
    }
    
    FILE* f = static_cast<FILE*>(fileHandle);
    fseek(f, 0, SEEK_SET);
    
    int err = 0;
    codes_handle* h = nullptr;
    
    // Skip to the requested message
    for (int i = 0; i <= messageIndex; i++) {
        if (h) codes_handle_delete(h);
        h = codes_handle_new_from_file(nullptr, f, PRODUCT_GRIB, &err);
        if (!h) {
            lastError = "Cannot read message " + std::to_string(messageIndex);
            return false;
        }
    }
    
    // Read metadata
    size_t len = 256;
    char buffer[256];
    
    CODES_CHECK(codes_get_string(h, "name", buffer, &len), 0);
    field.name = std::string(buffer);
    
    len = 256;
    CODES_CHECK(codes_get_string(h, "shortName", buffer, &len), 0);
    field.shortName = std::string(buffer);
    
    len = 256;
    CODES_CHECK(codes_get_string(h, "units", buffer, &len), 0);
    field.units = std::string(buffer);

    len = 256;
    CODES_CHECK(codes_get_string(h, "indicatorOfTypeOfLevel", buffer, &len), 0);
    field.indicatorOfTypeOfLevel = std::string(buffer);

    // Get dimensions
    long nx, ny, level, indicatorOfParameter;
    CODES_CHECK(codes_get_long(h, "Ni", &nx), 0);
    CODES_CHECK(codes_get_long(h, "Nj", &ny), 0);
    CODES_CHECK(codes_get_long(h, "level", &level), 0);
    CODES_CHECK(codes_get_long(h, "indicatorOfParameter", &indicatorOfParameter), 0);
    field.width = static_cast<size_t>(nx);
    field.height = static_cast<size_t>(ny);
    field.level = static_cast<size_t>(level);
    field.indicatorOfParameter = static_cast<size_t>(indicatorOfParameter);
    
    // Get values
    size_t values_len = 0;
    CODES_CHECK(codes_get_size(h, "values", &values_len), 0);
    
    field.values.resize(values_len);
    CODES_CHECK(codes_get_double_array(h, "values", field.values.data(), &values_len), 0);
    
    // Calculate min/max
    field.min_value = *std::min_element(field.values.begin(), field.values.end());
    field.max_value = *std::max_element(field.values.begin(), field.values.end());
    
    codes_handle_delete(h);
    return true;
}
