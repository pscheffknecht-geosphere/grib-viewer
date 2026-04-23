#pragma once

#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "grib_reader.h"

struct FieldIdentity {
    long indicatorOfParameter;
    std::string indicatorOfTypeOfLevel;
    long parameterNumber;
    long parameterCategory;
    long discipline;
    std::string typeOfFirstFixedSurface;
    std::string typeOfLevel;
    long level;

    bool operator==(const FieldIdentity& o) const {
        return indicatorOfParameter == o.indicatorOfParameter &&
               indicatorOfTypeOfLevel == o.indicatorOfTypeOfLevel &&
               parameterNumber == o.parameterNumber &&
               parameterCategory == o.parameterCategory &&
               discipline == o.discipline &&
               typeOfFirstFixedSurface == o.typeOfFirstFixedSurface &&
               typeOfLevel == o.typeOfLevel &&
               level == o.level;
    }
};

struct FieldIdentityHash {
    size_t operator()(const FieldIdentity& f) const {
        auto hLong = std::hash<long>{};
        auto hStr = std::hash<std::string>{};
        size_t h = 1469598103934665603ull;
        auto mix = [&h](size_t v) { h ^= v + 0x9e3779b97f4a7c15ull + (h << 6) + (h >> 2); };
        mix(hLong(f.indicatorOfParameter));
        mix(hStr(f.indicatorOfTypeOfLevel));
        mix(hLong(f.parameterNumber));
        mix(hLong(f.parameterCategory));
        mix(hLong(f.discipline));
        mix(hStr(f.typeOfFirstFixedSurface));
        mix(hStr(f.typeOfLevel));
        mix(hLong(f.level));
        return h;
    }
};

FieldIdentity fieldIdentityOf(const GribMessageInfo& m);

class GribCollection {
public:
    bool fileLoaded = false;
    bool loading = false;
    size_t loadCursor = 0;
    std::vector<std::string> pendingPaths;

    std::vector<std::string> filenames;
    std::vector<std::unique_ptr<GribReader>> readers;
    std::vector<GribMessageInfo> messageList;
    std::vector<std::pair<size_t, int>> lookupByGlobal;
    std::unordered_map<FieldIdentity, std::vector<size_t>, FieldIdentityHash> identityIndex;
    std::vector<std::string> duplicateWarnings;

    GribField currentField;
    size_t currentGlobalIndex = 0;

    void clear();
    void beginLoad(const std::vector<std::string>& paths);
    bool loadNext();
    bool readField(size_t globalIndex, GribField& field);

    size_t messageCount() const { return messageList.size(); }

    const std::vector<size_t>* stepsFor(const FieldIdentity& id) const;

private:
    void finalizeLoad();
};
