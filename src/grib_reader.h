#pragma once

#include <eccodes.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <imgui.h>

#include <string>
#include <vector>
#include <memory>

struct GribField {
    std::string name;
    std::string shortName;
    std::string units;
    std::string indicatorOfTypeOfLevel;
    long level;
    long width;
    long height;
    long indicatorOfParameter;
    long parameterNumber;
    long discipline;
    long parameterCategory;
    std::vector<double> values;
    double min_value;
    double max_value;
    bool jScansPositively = true;
    
    GribField() : width(0), height(0), min_value(0.0), max_value(0.0) {}
};

class GribReader {
public:
    GribReader();
    ~GribReader();
    
    bool openFile(const std::string& filename);
    void close();
    
    int getMessageCount() const;
    bool get_value(const codes_handle* h, const char* value_name, const int* value, const int& len);
    bool readField(int messageIndex, GribField& field);

    bool readCode(codes_handle* h, const char* name, std::string& value);
    bool readCode(codes_handle* h, const char* name, long& value);
    bool readCode(codes_handle* h, const char* name, bool& value);
    void readValue(size_t& len, codes_handle* h, char buffer[256], GribField& field);

    const std::string& getLastError() const { return lastError; }
    
private:
    std::string filename;
    std::string lastError;
    void* fileHandle;
};
