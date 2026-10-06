#include "Taskbar.h"
#include "backends/imgui_impl_opengl3_loader.h"
#include <chrono>
#include <cstdio>
#include <ctime>
#include "stb_image.h"

Taskbar::Taskbar(float scale) : scale(scale){}

Taskbar::~Taskbar()
{
    for (auto& icon : taskbarIcons)
    {
        if (icon.texture != 0)
            glDeleteTextures(1, &icon.texture);
    }
}

unsigned int Taskbar::loadTexture(const char* filename)
{
    int w, h, channels;
    unsigned char* data = stbi_load(
        filename,
        &w,
        &h,
        &channels,
        4
    );
    if(!data){
        printf("Failed to load icon %s: %s\n", filename, stbi_failure_reason());
        return 0;
    }
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_LINEAR
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_LINEAR
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_S,
        GL_CLAMP_TO_EDGE
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_T,
        GL_CLAMP_TO_EDGE
    );

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        w,
        h,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        data
    );

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);

    return true;

}

bool Taskbar::addIcon(const std::string& name, const char* imagePath, std::function<void()> onClick){
    unsigned int tex = loadTexture(imagePath);
    if(tex==0) return false;

    TaskbarIcon icon;
    icon.name = name;
    icon.texture = tex;
    icon.onClick = std::move(onClick);
    taskbarIcons.push_back(std::move(icon));
    return true;
}

void Taskbar::draw()
{
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    float taskbarHeight = 60.0f * scale;

    // Position at bottom of screen
    ImGui::SetNextWindowPos(ImVec2(0, displaySize.y - taskbarHeight));
    ImGui::SetNextWindowSize(ImVec2(displaySize.x, taskbarHeight));

    ImGui::Begin("Taskbar", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse);

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10 * scale, 0));

    bool first = true;
    for (auto& icon : taskbarIcons)
    {
        if (!icon.texture)
            continue;

        if (!first)
            ImGui::SameLine();
        first = false;

        if (ImGui::ImageButton(icon.name.c_str(),
                               (ImTextureID)(intptr_t)icon.texture,
                               ImVec2(40 * scale, 40 * scale)))
        {
            if (icon.onClick)
                icon.onClick();
        }

        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", icon.name.c_str());
    }

    ImGui::PopStyleVar();

    drawSystemTray();

    ImGui::End();
}

void Taskbar::drawSystemTray()
{
    const char* label = "Power Off";
    ImVec2 btnSize(
        ImGui::CalcTextSize(label).x + 24 * scale,
        40 * scale
    );

    // Right-aligned, vertically centered
    ImGui::SameLine(ImGui::GetWindowWidth() - btnSize.x - 20 * scale);
    ImGui::SetCursorPosY((ImGui::GetWindowHeight() - btnSize.y) * 0.5f);

    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.75f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.90f, 0.25f, 0.25f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.60f, 0.10f, 0.10f, 1.0f));

    if (ImGui::Button(label, btnSize))
        ImGui::OpenPopup("Confirm Power Off");

    ImGui::PopStyleColor(3);

    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Shut down");

    // Confirmation dialog, centered on screen
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(),
                            ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Confirm Power Off", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Are you sure you want to shut down?");
        ImGui::Separator();

        if (ImGui::Button("Yes", ImVec2(120 * scale, 0)))
        {
            if (onPowerOff)
                onPowerOff();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120 * scale, 0)))
            ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }
}
