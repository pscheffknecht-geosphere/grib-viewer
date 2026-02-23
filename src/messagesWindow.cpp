#include "messagesWindow.h" 

void gribMessageListWindow(  
    bool* p_open,
    const std::vector<GribMessageInfo>& messageList,
    int& currentMessage)
{
    if (!ImGui::Begin("GRIB Messages", p_open))
    {
        ImGui::End();
        return;
    }
   
    ImGuiListClipper clipper;
    clipper.Begin(messageList.size());

    while (clipper.Step())
    {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
        {
            const auto& meta = messageList[i];
            std::string numberInfo;
            std::string memberInfo = "";
            if (meta.indicatorOfParameter >= 0)
                numberInfo = "(ioP: " + std::to_string(meta.indicatorOfParameter) + ")";
            else if (meta.parameterNumber >= 0)
                numberInfo = 
                  "(PN: "  + std::to_string(meta.parameterNumber) + 
                ", Cat: "  + std::to_string(meta.parameterCategory) + 
                ", Disc: " + std::to_string(meta.discipline) + ")";
            if (meta.perturbationNumber > 0)
                memberInfo = "[M" + std::to_string(meta.perturbationNumber)+ "] ";
            std::string label =
                "[" + std::to_string(i + 1) + "] " + memberInfo +
                meta.shortName + " | (" + meta.name + ") [" + meta.units + "] on " + 
                meta.typeOfLevel + " = " + std::to_string(meta.level) + " " + numberInfo;
            if (ImGui::Selectable(label.c_str(), currentMessage == meta.index))
                currentMessage = meta.index;
        }
    }

    ImGui::End();
}

bool compareByKey(const GribMessageInfo& a,
                  const GribMessageInfo& b,
                  SortKey key)
{
    switch (key)
    {
        case SortKey::Name: return a.name < b.name;
        case SortKey::ShortName: return a.shortName < b.shortName;
        case SortKey::IndicatorOfParameter:
            return a.indicatorOfParameter < b.indicatorOfParameter;
        case SortKey::ParameterNumber:
            return a.parameterNumber < b.parameterNumber;
        case SortKey::Level:
            return a.level < b.level;
        case SortKey::PerturbationNumber:
            return a.perturbationNumber < b.perturbationNumber;
        case SortKey::IndicatorOfTypeOfLevel:
            return a.indicatorOfParameter < b.indicatorOfParameter;
        case SortKey::ParameterCategory:
            return a.parameterCategory < b.parameterCategory;
        case SortKey::Discipline:
            return a.discipline < b.discipline;
        case SortKey::TypeOfLevel:
            return a.typeOfLevel < b.typeOfLevel;
        case SortKey::typeOfFirstFixedSurface:
            return a.typeOfFirstFixedSurface < b.typeOfFirstFixedSurface;
    }
    return false;
}

void sortWithOrder(std::vector<GribMessageInfo>& list,
                   const std::vector<SortColumn>& order)
{
    std::sort(list.begin(), list.end(),
        [&](const auto& a, const auto& b)
        {
            for (const auto& col : order)
            {
                if (compareByKey(a, b, col.key))
                    return col.ascending;

                if (compareByKey(b, a, col.key))
                    return !col.ascending;
            }
            return false;
        });
}

void gribSortWindow(bool* p_open,
                    std::vector<GribMessageInfo>& messageList)
{
    if (!ImGui::Begin("Grib Message Sorting", p_open))
    {
        ImGui::End();
        return;
    }

    static std::vector<SortColumn> sortOrder;

    const char* items[] =
    {
        "Name",
        "ShortName",
        "IndicatorOfParameter",
        "IndicatorOfTypeOfLevel",
        "ParameterNumber",
        "parameterCategory",
        "Discipline",
        "TypeOfLevel",
        "typeOfFirstFixedSurface",
        "Level",
        "PerturbationNumber"
    };

    if (ImGui::Button("Add Column"))
    {
        sortOrder.push_back({SortKey::Name, true});
    }

    ImGui::Separator();

    for (size_t i = 0; i < sortOrder.size(); ++i)
    {
        ImGui::PushID(static_cast<int>(i));

        int keyIndex = static_cast<int>(sortOrder[i].key);

        ImGui::Combo("Column", &keyIndex, items, IM_ARRAYSIZE(items));
        sortOrder[i].key = static_cast<SortKey>(keyIndex);

        ImGui::SameLine();
        ImGui::Checkbox("Asc", &sortOrder[i].ascending);

        ImGui::SameLine();
        if (ImGui::Button("Remove"))
        {
            sortOrder.erase(sortOrder.begin() + i);
            ImGui::PopID();
            break;
        }

        ImGui::PopID();
    }

    ImGui::Separator();

    if (ImGui::Button("Sort"))
    {
        sortWithOrder(messageList, sortOrder);
    }

    ImGui::End();
}