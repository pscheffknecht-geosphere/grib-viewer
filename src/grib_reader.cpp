#include "grib_reader.h"


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

void GribReader::loadFile(char filename[512]) {
    if (openFile(filename)) {
        fileLoaded = true;
        messageCount = getMessageCount();
        if (messageCount > 0) {
            getMessageOffsets();
            readField(0, currentField);
        }
        messageList.clear();
        for (int i = 0; i < messageCount; ++i) {
            GribMessageInfo info;
            readFieldMetadata(i, info);  // lightweight version
            info.index = i;
            messageList.push_back(info);
        }
    }
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

void GribReader::getMessageOffsets() {
    FILE* f = static_cast<FILE*>(fileHandle);
    fseek(f, 0, SEEK_SET);
    int err = 0;
    messageOffsets.clear();
    
    while (true)
    {
        long offset = ftell(f);
        codes_handle* h = codes_handle_new_from_file(nullptr, f, PRODUCT_GRIB, &err);
        if (!h) break;
        messageOffsets.push_back(offset);
        codes_handle_delete(h);
    }    
}

bool GribReader::readCode(codes_handle* h, const char* name, std::string& value) {
    if (codes_is_defined(h, name) == true) {
        size_t len = 256;
        char buffer[256];
        
        CODES_CHECK(codes_get_string(h, name, buffer, &len), 0);
        value = std::string(buffer);
        return true;
    }
    else return false;
}

bool GribReader::readCode(codes_handle* h, const char* name, long& value) {
    if (codes_is_defined(h, name) == true) {
        CODES_CHECK(codes_get_long(h, name, &value), 0);
        return true;
    }
    else return false;
}

bool GribReader::readCode(codes_handle* h, const char* name, bool& value) {
    long val = false;
    if (codes_is_defined(h, name) == true) {
        CODES_CHECK(codes_get_long(h, name, &val), 0);
        value = static_cast<bool>(val);
        return true;
    }
    else return false;
}

bool GribReader::readField(int messageIndex, GribField& field) {
    if (!fileHandle) {
        lastError = "No file open";
        return false;
    }
    
    FILE* f = static_cast<FILE*>(fileHandle);

    fseek(f, messageOffsets[messageIndex], SEEK_SET);

    int err = 0;
    codes_handle* h = codes_handle_new_from_file(nullptr, f, PRODUCT_GRIB, &err);
    if (!h) return false;
    
    // Read metadata
    if (! readCode(h, "name", field.name))
        field.name = "~";
    if (! readCode(h, "shortName", field.shortName))
        field.shortName = "~";
    if (! readCode(h, "units", field.units))
        field.units = "~";
    if (! readCode(h, "indicatorOfTypeOfLevel", field.indicatorOfTypeOfLevel))
        field.indicatorOfTypeOfLevel = "~";
    if (! readCode(h, "typeOfLevel", field.typeOfLevel))
        field.typeOfLevel = "~";
    if (! readCode(h, "typeOfFirstFixedSurface", field.typeOfFirstFixedSurface))
        field.typeOfFirstFixedSurface = "~";

    // Get dimensions
    if (! readCode(h, "Ni", field.width))
        field.discipline = -1;
    if (! readCode(h, "Nj", field.height))
        field.discipline = -1;
    if (! readCode(h, "level", field.level))
        field.discipline = -1;
    if (! readCode(h, "discipline", field.discipline))
        field.discipline = -1;
    if (! readCode(h, "parameterNumber", field.parameterNumber))
        field.parameterNumber = -1;
    if (! readCode(h, "parameterCategory", field.parameterCategory))
        field.parameterCategory = -1;
    if (! readCode(h, "indicatorOfParameter", field.indicatorOfParameter))
        field.indicatorOfParameter = -1;
    if (! readCode(h, "jScansPositively", field.jScansPositively))
        field.jScansPositively = false;
    if (! readCode(h, "perturbationNumber", field.perturbationNumber))
        field.perturbationNumber = -1;
    
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

bool GribReader::readFieldMetadata(const int messageIndex, GribMessageInfo& info) {
    if (!fileHandle) {
        lastError = "No file open";
        return false;
    }
    
    FILE* f = static_cast<FILE*>(fileHandle);

    fseek(f, messageOffsets[messageIndex], SEEK_SET);

    int err = 0;
    codes_handle* h = codes_handle_new_from_file(nullptr, f, PRODUCT_GRIB, &err);
    if (!h) return false;
    
    // Read metadata (same as in readField but without values)
    if (! readCode(h, "name", info.name))
        info.name = "~";
    if (! readCode(h, "shortName", info.shortName))
        info.shortName = "~";
    if (! readCode(h, "units", info.units))
        info.units = "~";
    if (! readCode(h, "indicatorOfTypeOfLevel", info.indicatorOfTypeOfLevel))
        info.indicatorOfTypeOfLevel = "~";
    if (! readCode(h, "typeOfLevel", info.typeOfLevel))
        info.typeOfLevel = "~";
    if (! readCode(h, "typeOfFirstFixedSurface", info.typeOfFirstFixedSurface))
        info.typeOfFirstFixedSurface = "~";

    // Get dimensions
    if (! readCode(h, "Ni", info.width))
        info.discipline = -1;
    if (! readCode(h, "Nj", info.height))
        info.discipline = -1;
    if (! readCode(h, "level", info.level))
        info.level = -1;
    if (! readCode(h, "discipline", info.discipline))
        info.discipline = -1;
    if (! readCode(h, "parameterNumber", info.parameterNumber))
        info.parameterNumber = -1;
    if (! readCode(h, "parameterCategory", info.parameterCategory))
        info.parameterCategory = -1;
    if (! readCode(h, "indicatorOfParameter", info.indicatorOfParameter))
        info.indicatorOfParameter = -1;
    if (! readCode(h, "perturbationNumber", info.perturbationNumber))
        info.perturbationNumber = -1;

    codes_handle_delete(h);
    return true;
}