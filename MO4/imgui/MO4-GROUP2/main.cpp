#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Desktop.h"
#include "Taskbar.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>
#include "stb_image.h"
#include "../imgui.h"
#define STB_IMAGE_IMPLEMENTATION

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

GLuint loadImg(const char* filename)
{
    int width;
    int height;
    int channels;

    unsigned char* data = stbi_load(
        filename,
        &width,
        &height,
        &channels,
        4
    );

    if (data == nullptr)
    {
        printf("Failed to load: %s\n", filename);
        return 0;
    }
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA,
        width, height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, data
    );
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);

    printf("Loaded %s -> texture %u\n", filename, textureID);
    return textureID;
}


int main(int, char**)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    const char* glsl_version = nullptr;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
    GLFWwindow* window = glfwCreateWindow((int)(1280 * main_scale), (int)(800 * main_scale),
                                          "CSOPESY MO4 GROUP 2", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    bool show_demo_window = true;
    bool show_another_window = false;


    bool show_start = false;
    bool show_task_manager = false;

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    {
        Desktop desktop;   
        Taskbar taskbar(main_scale);


        taskbar.addIcon("Button 1", "assets/start.png", [&show_start]{show_start = true;});
        taskbar.addIcon("Button 2", "assets/taskmanager.png", [&show_task_manager]{show_task_manager = true;});
        taskbar.addIcon("Button 3", "assets/button3.png", []{printf("Button 3 clicked.\n");});

        GLuint pfp = loadImg("assets/pfp.png");
        GLuint chrome = loadImg("assets/chrome.png");
        GLuint matlab = loadImg("assets/matlab.png");
        GLuint settings = loadImg("assets/settings.png");
        GLuint steam = loadImg("assets/steam.png");
        GLuint taskmanager = loadImg("assets/taskmanager.png");

        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
            if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
            {
                ImGui_ImplGlfw_Sleep(10);
                continue;
            }

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            desktop.draw();
            taskbar.draw();
            taskbar.setPowerOffCallback([window]() {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            });

            if (show_task_manager) {
                ImGui::Begin("Task Manager", &show_task_manager);
                ImGui::Text("Placeholder...");
                ImGui::End();
            }

            if (show_start) {
                const float taskbarH = 60.0f * main_scale;

                ImGui::SetNextWindowPos(ImVec2(0, io.DisplaySize.y - taskbarH), ImGuiCond_Always, ImVec2(0.0f, 1.0f));
                ImGui::SetNextWindowSize(ImVec2(240, 460));
                ImGui::Begin("Start", &show_start, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

                const float containerX = ImGui::GetContentRegionAvail().x;

                ImGui::BeginChild("container", ImVec2(250, 65), false);
                ImGui::SetCursorPos(ImVec2(0, (50 - ImGui::GetTextLineHeight()) * 0.5f));
                ImGui::Text("Collado Nazareno");
                ImGui::SameLine();

                ImGui::SetCursorPos(ImVec2(containerX - 50, 0));
                ImGui::Image(
                    (ImTextureID)(intptr_t)pfp,
                    ImVec2(50, 50)
                );

                ImVec2 pos = ImGui::GetWindowPos();

                ImGui::GetWindowDrawList()->AddLine(
                    ImVec2(pos.x + 0.0f,   pos.y + 60.0f),
                    ImVec2(pos.x + 240.0f, pos.y + 60.0f),
                    IM_COL32(108, 108, 108, 255),
                    1.0f
                );

                ImGui::EndChild();
                struct App {
                    const char* name;
                    ImTextureID texture;
                };

                App apps[] = {
                    { "Chrome",       (ImTextureID)(intptr_t)chrome },
                    { "Matlab",       (ImTextureID)(intptr_t)matlab },
                    { "Settings",     (ImTextureID)(intptr_t)settings },
                    { "Steam",        (ImTextureID)(intptr_t)steam },
                    { "Task Manager", (ImTextureID)(intptr_t)taskmanager },
                };

                ImGui::BeginChild("appContainer", ImVec2(240, 330), false);
                ImGui::Text("Applications");
                ImGui::NewLine();
                for (const App& app : apps) {
                    const float y = ImGui::GetCursorPosY();

                    ImGui::Image(app.texture, ImVec2(50, 50));
                    ImGui::SameLine();
                    ImGui::SetCursorPosY(y + (50.0f - ImGui::GetTextLineHeight()) * 0.5f);
                    ImGui::Text("%s", app.name);

                    ImGui::SetCursorPosY(y + 50.0f);
                    ImGui::Dummy(ImVec2(0, 6.0f));
                }
                ImGui::EndChild();


                ImGui::BeginChild("user control", ImVec2(240, 20), false);

                ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.35f, 0.35f, 0.35f, 1.0f));
                ImGui::Button("Log off", ImVec2(55, 20));
                ImGui::SameLine();
                ImGui::Button("Switch User", ImVec2(80, 20));
                ImGui::SameLine();
                ImGui::Button("Shut Down", ImVec2(80, 20));
                ImGui::PopStyleColor(3);

                ImGui::EndChild();

                ImGui::End();
            }

            {
                static float f = 0.0f;
                static int counter = 0;

                ImGui::Begin("Hello, world!");
                ImGui::Text("This is some useful text.");
                ImGui::Checkbox("Demo Window", &show_demo_window);
                ImGui::Checkbox("Another Window", &show_another_window);
                ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
                ImGui::ColorEdit3("clear color", (float*)&clear_color);

                if (ImGui::Button("Button"))
                    counter++;
                ImGui::SameLine();
                ImGui::Text("counter = %d", counter);

                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
                ImGui::End();
            }

            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
                         clear_color.z * clear_color.w, clear_color.w);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);
        }
        glDeleteTextures(1, &pfp);
    }   // ~Desktop() runs here


    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}