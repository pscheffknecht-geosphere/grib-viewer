#pragma once

#include <eccodes.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <imgui.h>

#include <string>
#include <vector>
#include <memory>

#include <imgui.h>

struct GribField {
    std::string name;
    std::string shortName;
    std::string units;
    std::string indicatorOfTypeOfLevel;
    std::string typeOfLevel;
    std::string typeOfFirstFixedSurface;
    long level;
    long width;
    long height;
    long indicatorOfParameter;
    long parameterNumber;
    long discipline;
    long parameterCategory;
    long perturbationNumber;
    std::vector<double> values;
    double min_value;
    double max_value;
    bool jScansPositively = true;
    ImVec2 uv1;
    ImVec2 uv2;

    GribField() : width(0), height(0), min_value(0.0), max_value(0.0) {processScanDirections();}
    void processScanDirections() {
        if (jScansPositively) {
            uv1 = ImVec2(0, 1);
            uv2 = ImVec2(1, 0);
        } else {
            uv1 = ImVec2(0, 0);
            uv2 = ImVec2(1, 1);
        }
    }
};

struct GribMessageInfo
{
    int index;
    std::string name;
    std::string shortName;
    std::string units;
    std::string indicatorOfTypeOfLevel;
    std::string typeOfLevel;
    std::string typeOfFirstFixedSurface;
    long level;
    long width;
    long height;
    long indicatorOfParameter;
    long parameterNumber;
    long discipline;
    long parameterCategory;
    long perturbationNumber;
};

enum class SortKey
{
    Name,
    ShortName,
    IndicatorOfParameter,
    IndicatorOfTypeOfLevel,
    ParameterNumber,
    ParameterCategory,
    Discipline,
    TypeOfLevel,
    typeOfFirstFixedSurface,
    Level,
    PerturbationNumber
};

struct SortColumn
{
    SortKey key;
    bool ascending = true;
};

class GribReader {
public:
    bool fileLoaded = false;
    int messageCount = 0;
    GribField currentField;
    std::vector<GribMessageInfo> messageList;
    GribReader();
    ~GribReader();
    
    bool openFile(const std::string& filename);
    void loadFile(char filename[512]);
    void close();
    
    int getMessageCount() const;
    void getMessageOffsets();
    bool get_value(const codes_handle* h, const char* value_name, const int* value, const int& len);
    bool readField(int messageIndex, GribField& field);
    bool readFieldMetadata(const int messageIndex, GribMessageInfo& field);

    bool readCode(codes_handle* h, const char* name, std::string& value);
    bool readCode(codes_handle* h, const char* name, long& value);
    bool readCode(codes_handle* h, const char* name, bool& value);
    void readValue(size_t& len, codes_handle* h, char buffer[256], GribField& field);

    const std::string& getLastError() const { return lastError; }
    
private:
    std::string filename;
    std::string lastError;
    std::vector<long> messageOffsets;
    void* fileHandle;
};
