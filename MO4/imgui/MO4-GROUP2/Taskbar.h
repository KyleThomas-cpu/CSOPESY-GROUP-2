#pragma once
#include "imgui.h"

#include <functional>
#include <string>
#include <vector>

struct TaskbarIcon 
{
    std::string name;
    unsigned int texture = 0;
    std::function<void()> onClick;
};

class Taskbar
{
    public:
        explicit Taskbar(float scale = 1.0f);
        ~Taskbar();
        void draw();
        bool addIcon(const std::string& name, const char* imagePath,
        std::function<void()> onClick);
        void setPowerOffCallback(std::function<void()> cb) { onPowerOff = std::move(cb); }
    
    private:
        void drawSystemTray();
        unsigned int loadTexture(const char* filename);
        float scale;
        std::function<void()> onPowerOff;
        std::vector<TaskbarIcon> taskbarIcons;
};