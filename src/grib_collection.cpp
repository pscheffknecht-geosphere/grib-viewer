#include "grib_collection.h"

#include <algorithm>
#include <cstring>
#include <set>
#include <sstream>

FieldIdentity fieldIdentityOf(const GribMessageInfo& m) {
    return FieldIdentity{
        m.indicatorOfParameter,
        m.indicatorOfTypeOfLevel,
        m.parameterNumber,
        m.parameterCategory,
        m.discipline,
        m.typeOfFirstFixedSurface,
        m.typeOfLevel,
        m.level,
    };
}

void GribCollection::clear() {
    fileLoaded = false;
    loading = false;
    loadCursor = 0;
    pendingPaths.clear();
    filenames.clear();
    readers.clear();
    messageList.clear();
    lookupByGlobal.clear();
    identityIndex.clear();
    duplicateWarnings.clear();
    currentField = GribField{};
    currentGlobalIndex = 0;
}

void GribCollection::beginLoad(const std::vector<std::string>& paths) {
    clear();
    if (paths.empty()) return;
    pendingPaths = paths;
    filenames = paths;
    readers.reserve(paths.size());
    loadCursor = 0;
    loading = true;
}

bool GribCollection::loadNext() {
    if (!loading) return false;
    if (loadCursor >= pendingPaths.size()) {
        finalizeLoad();
        return false;
    }
    size_t fi = loadCursor;
    auto reader = std::make_unique<GribReader>();
    char buf[512];
    std::strncpy(buf, pendingPaths[fi].c_str(), sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    reader->loadFile(buf);
    if (reader->fileLoaded) {
        for (auto& info : reader->messageList) {
            info.fileIdx = fi;
            info.globalIndex = messageList.size();
            lookupByGlobal.push_back({fi, info.indexInFile});
            messageList.push_back(info);
        }
    }
    readers.push_back(std::move(reader));
    ++loadCursor;
    if (loadCursor >= pendingPaths.size()) {
        finalizeLoad();
        return false;
    }
    return true;
}

void GribCollection::finalizeLoad() {
    loading = false;
    fileLoaded = !messageList.empty();

    identityIndex.clear();
    for (size_t i = 0; i < messageList.size(); ++i) {
        identityIndex[fieldIdentityOf(messageList[i])].push_back(i);
    }
    for (auto& [id, globals] : identityIndex) {
        std::sort(globals.begin(), globals.end(),
                  [&](size_t a, size_t b) {
                      const auto& ma = messageList[a];
                      const auto& mb = messageList[b];
                      if (ma.step != mb.step) return ma.step < mb.step;
                      if (ma.validityDate != mb.validityDate) return ma.validityDate < mb.validityDate;
                      if (ma.validityTime != mb.validityTime) return ma.validityTime < mb.validityTime;
                      return ma.fileIdx < mb.fileIdx;
                  });

        std::set<std::tuple<long, long, long>> seen;
        for (size_t gidx : globals) {
            const auto& m = messageList[gidx];
            auto key = std::make_tuple(m.step, m.validityDate, m.validityTime);
            if (!seen.insert(key).second) {
                std::ostringstream os;
                os << "Duplicate (field, step) entries: shortName=" << m.shortName
                   << " level=" << m.level
                   << " step=" << m.step
                   << " — loaded files overlap";
                duplicateWarnings.push_back(os.str());
                break;
            }
        }
    }

    if (fileLoaded) {
        readField(0, currentField);
    }
}

bool GribCollection::readField(size_t globalIndex, GribField& field) {
    if (globalIndex >= lookupByGlobal.size()) return false;
    auto [fileIdx, indexInFile] = lookupByGlobal[globalIndex];
    if (fileIdx >= readers.size()) return false;
    bool ok = readers[fileIdx]->readField(indexInFile, field);
    if (ok) currentGlobalIndex = globalIndex;
    return ok;
}

const std::vector<size_t>* GribCollection::stepsFor(const FieldIdentity& id) const {
    auto it = identityIndex.find(id);
    if (it == identityIndex.end()) return nullptr;
    return &it->second;
}
