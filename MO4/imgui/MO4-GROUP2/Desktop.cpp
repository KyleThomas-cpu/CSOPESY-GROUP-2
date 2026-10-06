#include "Desktop.h"

#include "backends/imgui_impl_opengl3_loader.h"
#include <ctime>
#include <cstdio>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
// hello
Desktop::Desktop()
{
    if (!loadWallpaper("assets/wallpaper.jpg") &&
        !loadWallpaper("Debug/assets/wallpaper.jpg"))
    {
        printf("FAILED TO LOAD WALLPAPER: %s\n", stbi_failure_reason());
    }
    else
    {
        printf("WALLPAPER LOADED SUCCESSFULLY\n");
    }
}

Desktop::~Desktop()
{
    if (wallpaperTexture != 0)
    {
        glDeleteTextures(1, &wallpaperTexture);
    }
}

bool Desktop::loadWallpaper(const char* filename)
{
    int channels;

    unsigned char* data = stbi_load(
        filename,
        &wallpaperWidth,
        &wallpaperHeight,
        &channels,
        4
    );

    if (!data)
    {
        return false;
    }

    glGenTextures(1, &wallpaperTexture);
    glBindTexture(GL_TEXTURE_2D, wallpaperTexture);

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
        wallpaperWidth,
        wallpaperHeight,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        data
    );

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);

    return true;
}

void Desktop::draw()
{
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

    ImGui::Begin(
        "Desktop",
        nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus
    );

    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();

    if (wallpaperTexture != 0)
    {
        ImGui::GetWindowDrawList()->AddImage(
            (ImTextureID)(intptr_t)wallpaperTexture,
            windowPos,
            ImVec2(
                windowPos.x + windowSize.x,
                windowPos.y + windowSize.y
            )
        );
    }

    drawClock();

    ImGui::End();
}

void Desktop::drawClock()
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    struct tm timeinfo;
    localtime_s(&timeinfo, &time);

    char buffer[64];
    strftime(
        buffer,
        sizeof(buffer),
        "%I:%M:%S %p",
        &timeinfo
    );

    ImVec2 textSize = ImGui::CalcTextSize(buffer);

    ImVec2 pos = ImVec2(
        ImGui::GetIO().DisplaySize.x - textSize.x - 20,
        20
    );

    ImGui::GetWindowDrawList()->AddText(
        pos,
        IM_COL32(255, 255, 255, 255),
        buffer
    );
}