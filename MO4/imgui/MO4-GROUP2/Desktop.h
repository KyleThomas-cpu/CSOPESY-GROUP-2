#pragma once

#include "imgui.h"
#include <chrono>

class Desktop
{
public:
    Desktop();
    ~Desktop();

    void draw();

private:
    void drawClock();
    bool loadWallpaper(const char* filename);

    unsigned int wallpaperTexture = 0;
    int wallpaperWidth = 0;
    int wallpaperHeight = 0;
};