#pragma once

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
    bool readField(int messageIndex, GribField& field);
    
    const std::string& getLastError() const { return lastError; }
    
private:
    std::string filename;
    std::string lastError;
    void* fileHandle;
};
