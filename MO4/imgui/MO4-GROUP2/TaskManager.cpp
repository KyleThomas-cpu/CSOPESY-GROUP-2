#include "TaskManager.h"
#include "imgui.h"

void TaskManager::draw(bool* show)
{
    ImGui::SetNextWindowSize(ImVec2(800, 400),ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Task Manager", show))
    {
        ImGui::End();
        return;
    }
    if (ImGui::BeginTabBar("TaskManagerTabs"))
    {
        if (ImGui::BeginTabItem("Processes"))
        {
            ImGui::Text("Apps and background processes");
            ImGui::Separator();
            if (ImGui::BeginTable("ProcessesTable",4,ImGuiTableFlags_Borders |ImGuiTableFlags_RowBg |ImGuiTableFlags_Resizable |ImGuiTableFlags_ScrollY))
            {
                ImGui::TableSetupColumn("Name");
                ImGui::TableSetupColumn("Status");
                ImGui::TableSetupColumn("CPU");
                ImGui::TableSetupColumn("Memory");
                ImGui::TableHeadersRow();
                struct Process
                {
                    const char* name;
                    const char* status;
                    const char* cpu;
                    const char* memory;
                };
                const Process processes[] =
                {
                    { "Chrome", " ", "6.7%", "1423 MB" },
                    { "MATLAB", " ", "2.3%", "322 MB" },
                    { "Steam", " ", "4.9%", "652 MB" },
                    { "Settings", " ", "0.5%", "45 MB" },
                    { "Task Manager", " ", "0.6%", "67 MB" }
                };
                for (const Process&process : processes)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(process.name);

                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted(process.status);

                    ImGui::TableSetColumnIndex(2);
                    ImGui::TextUnformatted(process.cpu);

                    ImGui::TableSetColumnIndex(3);
                    ImGui::TextUnformatted(process.memory);
                }
                ImGui::EndTable();
            }
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Performance"))
        {
            ImGui::Text("CPU");
            ImGui::Text("Utilization: 15%%");
            ImGui::ProgressBar(0.15f, ImVec2(-1, 0));

            ImGui::Spacing();
            
            ImGui::Text("Memory");
            ImGui::Text("In Use: 62%%");
            ImGui::ProgressBar(0.62f, ImVec2(-1, 0));

            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}