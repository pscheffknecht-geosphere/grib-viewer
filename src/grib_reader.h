#pragma once

#include <eccodes.h>
#include <algorithm>
#include <cmath>
#include <iostream>

#include <string>
#include <vector>
#include <memory>

struct GribField {
    std::string name;
    std::string shortName;
    std::string units;
    std::string indicatorOfTypeOfLevel;
    size_t level;
    size_t width;
    size_t height;
    size_t indicatorOfParameter;
    std::vector<double> values;
    double min_value;
    double max_value;
    
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

    bool readCode(codes_handle* h, char* name, std::string& value);
    bool readCode(codes_handle* h, char* name, long& value);
    void readValue(size_t& len, codes_handle* h, char buffer[256], GribField& field);

    const std::string& getLastError() const { return lastError; }
    
private:
    std::string filename;
    std::string lastError;
    void* fileHandle;
};
