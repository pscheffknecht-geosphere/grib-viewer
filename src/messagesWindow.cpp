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

            std::string label =
                "[" + std::to_string(i + 1) + "] " +
                meta.shortName + " | " +
                meta.typeOfLevel + " | " +
                std::to_string(meta.level) + " " +
                meta.units;

            if (ImGui::Selectable(label.c_str(), currentMessage == i))
                currentMessage = i;
        }
    }

    ImGui::End();
}


