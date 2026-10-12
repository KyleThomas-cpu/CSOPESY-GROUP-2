#include "FileExplorer.h"
#include "imgui.h"
 
void FileExplorer::draw(bool* show)
{
    ImGui::SetNextWindowSize(ImVec2(1000, 600), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("File Explorer", show))
    {
        ImGui::End();
        return;
    }
 
    struct Item
    {
        const char* name;
        const char* modified;
        const char* type;
        const char* size;
    };
 
    struct Folder
    {
        const char* name;
        const char* path;
        const Item* items;
        int count;
    };
 
    static const Item desktopItems[] =
    {
        { "EAF", "10/09/2026 9:27 AM", "File folder", "" },
        { "IMPORTANT FILES", "08/08/2026 12:04 AM", "File folder", "" },
        { "Arknights", "13/08/2026 7:54 PM", "Shortcut", "2 KB" },
        { "Discord", "08/10/2026 7:12 PM", "Shortcut", "3 KB" },
        { "Deadlock", "24/08/2025 4:16 PM", "Internet Shortcut",    "1 KB" },
        { "Umamusume Pretty Derby", "02/07/2025 3:25 PM", "Internet Shortcut",    "1 KB" },
    };
    static const Item documentsItems[] =
    {
        { "CCICOMP ME Reviewer", "07/10/2024 7:36 AM", "Chrome PDF Document", "121 KB" },
        { "BM IA", "07/04/2024 12:10 pm", "Microsoft Excel Worksheet", "944 KB" },
        { "SYDNEY SPHERE", "21/02/2024 8:57 pm", "SketchUp 2023", "159 KB" },
        { "AA Practice Test 1", "26/01/2024 1:24 pm", "Chrome PDF Document", "306 KB" },
        { "AUSTRALIAN UNNIVERSITIES - Shortcut", "28/06/2023 6:54 pm", "Shortcut", "2 KB" },
        { "BabysFirstProgram - Shortcut", "28/06/2023 6:54 pm", "Shortcut", "1 KB" },
    };
    static const Item downloadsItems[] =
    {
        { "installer.exe", "10/10/2026 07:22 PM", "Application", "84,512 KB" },
        { "wallpaper.jpg", "10/09/2026 03:18 PM", "JPG File", "1,963 KB" },
        { "lecture3.zip", "10/06/2026 09:50 AM", "ZIP File", "22,340 KB" },
    };
    static const Item picturesItems[] =
    {
        { "MIKU BACKGROUND", "05/05/2025 7:48 AM", "JPG File", "232 KB" },
        { "pfp.png", "10/02/2026 05:30 PM", "PNG File", "237 KB" },
        { "group_photo.jpg", "09/30/2026 12:12 PM", "JPG File", "3,102 KB" },
    };
    static const Item musicItems[] =
    {
        { "Playlist 1", "09/25/2026 08:00 PM", "Folder", "" },
        { "song.mp3", "09/20/2026 10:10 AM", "MP3 File", "4,512 KB" },
    };
 
    static const Folder folders[] =
    {
        { "Desktop",   "This PC > Desktop",   desktopItems,   (int)(sizeof(desktopItems)   / sizeof(Item)) },
        { "Documents", "This PC > Documents", documentsItems, (int)(sizeof(documentsItems) / sizeof(Item)) },
        { "Downloads", "This PC > Downloads", downloadsItems, (int)(sizeof(downloadsItems) / sizeof(Item)) },
        { "Pictures",  "This PC > Pictures",  picturesItems,  (int)(sizeof(picturesItems)  / sizeof(Item)) },
        { "Music",     "This PC > Music",     musicItems,     (int)(sizeof(musicItems)     / sizeof(Item)) },
    };
    const int folderCount = (int)(sizeof(folders) / sizeof(Folder));
 
    const Folder& current = folders[selectedFolder];
 
    ImGui::Text("%s", current.path);
    ImGui::Separator();
 
    ImGui::BeginChild("FE_Sidebar", ImVec2(150, 0), ImGuiChildFlags_Borders);
    ImGui::TextDisabled("Quick Access");
    ImGui::Separator();
    for (int i = 0; i < folderCount; ++i)
    {
        if (ImGui::Selectable(folders[i].name, selectedFolder == i))
        {
            selectedFolder = i;
            selectedFile = -1;
        }
    }
    ImGui::EndChild();
 
    ImGui::SameLine();
 
    ImGui::BeginChild("FE_Files", ImVec2(0, 0));
    if (ImGui::BeginTable("FilesTable", 4,
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
        ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY))
    {
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Date modified");
        ImGui::TableSetupColumn("Type");
        ImGui::TableSetupColumn("Size");
        ImGui::TableHeadersRow();
 
        for (int i = 0; i < current.count; ++i)
        {
            const Item& item = current.items[i];
 
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::PushID(i);
            if (ImGui::Selectable(item.name, selectedFile == i, ImGuiSelectableFlags_SpanAllColumns))
                selectedFile = i;
            ImGui::PopID();
 
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(item.modified);
 
            ImGui::TableSetColumnIndex(2);
            ImGui::TextUnformatted(item.type);
 
            ImGui::TableSetColumnIndex(3);
            ImGui::TextUnformatted(item.size);
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();
 
    ImGui::End();
}
 