#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <iostream>

#include "triangle.h"

struct Rotator {
    float currentAngle;
    bool isRotating;
    double lastTime;

    Rotator()
        : currentAngle(0.0f), isRotating(false), lastTime(glfwGetTime()) {}

    void Update() {
        auto currentTime = glfwGetTime();
        auto deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;

        if (isRotating) {
            currentAngle += deltaTime;
        }
    }
};

struct Framebuffer {
    GLuint fbo{};
    GLuint texture{};
    int width{};
    int height{};

    bool setup(int w, int h) {
        width = w;
        height = h;

        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                     GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, texture, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "framebuffer not complete\n";
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            return false;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return true;
    }

    void resize(int w, int h) {
        if (w == width && h == height)
            return;
        width = w;
        height = h;
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                     GL_UNSIGNED_BYTE, nullptr);
    }

    void bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, width, height);
    }

    static void unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void cleanup() {
        glDeleteTextures(1, &texture);
        glDeleteFramebuffers(1, &fbo);
    }
};

int main() {

    if (!glfwInit()) {
        std::cout << "glfw error\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    auto *window = glfwCreateWindow(800, 600, "ImGui Classic", nullptr, nullptr);
    if (!window) {
        std::cout << "cannot create window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGL()) {
        std::cout << "cannot init glad\n";
        glfwTerminate();
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    auto monitor = glfwGetPrimaryMonitor();
    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(monitor);

    auto &style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);

    auto &io = ImGui::GetIO();
    io.FontGlobalScale = main_scale * 1.3f;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    constexpr int fbo_w = 300, fbo_h = 250;

    TriangleRenderer main_triangle;
    if (!main_triangle.setup()) {
        glfwTerminate();
        return -1;
    }

    TriangleRenderer about_triangle;
    if (!about_triangle.setup()) {
        glfwTerminate();
        return -1;
    }

    Framebuffer main_fbo;
    if (!main_fbo.setup(fbo_w, fbo_h)) {
        glfwTerminate();
        return -1;
    }

    Framebuffer about_fbo;
    if (!about_fbo.setup(fbo_w, fbo_h)) {
        glfwTerminate();
        return -1;
    }

    Rotator main_rotator, about_rotator;
    bool show_about = true;

    ImVec2 main_img_size(fbo_w, fbo_h);
    ImVec2 about_img_size(fbo_w, fbo_h);

    while (!glfwWindowShouldClose(window)) {

        main_rotator.Update();
        about_rotator.Update();

        // Resize FBOs to match last frame's available space
        int mw = std::max((int)main_img_size.x, 1);
        int mh = std::max((int)main_img_size.y, 1);
        main_fbo.resize(mw, mh);

        int aw = std::max((int)about_img_size.x, 1);
        int ah = std::max((int)about_img_size.y, 1);
        about_fbo.resize(aw, ah);

        // Render main triangle to FBO
        main_fbo.bind();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        main_triangle.render(mw, mh, main_rotator.currentAngle);

        // Render about triangle to FBO
        about_fbo.bind();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        about_triangle.render(aw, ah, about_rotator.currentAngle);

        Framebuffer::unbind();

        // Main screen
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Main panel
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(350, 350), ImGuiCond_FirstUseEver);
        ImGui::Begin("Main");

        ImGui::Text("Main window text");
        if (ImGui::Button("Main window button")) {
            main_rotator.isRotating = !main_rotator.isRotating;
        }

        main_img_size = ImGui::GetContentRegionAvail();
        ImGui::Image((ImTextureID)(intptr_t)main_fbo.texture,
                     main_img_size,
                     ImVec2(0, 1), ImVec2(1, 0));

        ImGui::End();

        // About panel
        if (show_about) {
            ImGui::SetNextWindowPos(ImVec2(400, 10), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(350, 350), ImGuiCond_FirstUseEver);
            ImGui::Begin("About", &show_about);

            ImGui::Text("ImGUI + OpenGL Demo ver 0.0.1");
            if (ImGui::Button("About window button")) {
                about_rotator.isRotating = !about_rotator.isRotating;
            }

            about_img_size = ImGui::GetContentRegionAvail();
            ImGui::Image((ImTextureID)(intptr_t)about_fbo.texture,
                         about_img_size,
                         ImVec2(0, 1), ImVec2(1, 0));

            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }
    }

    main_fbo.cleanup();
    about_fbo.cleanup();
    main_triangle.cleanup();
    about_triangle.cleanup();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
