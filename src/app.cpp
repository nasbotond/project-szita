#include "app.hpp"

namespace GUI 
{
    GLFWwindow* window = nullptr;
    ImGuiIO io;

    static void glfw_error_callback(int error, const char* description)
    {
        fprintf(stderr, "Glfw Error %d: %s\n", error, description);
    }

	void destroy() 
    {
		std::cout << "Closing application";

        // Cleanup
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        // Close GLFW
		if(window)
        {
            glfwDestroyWindow(window);
        } 
        glfwTerminate();

	}

	void setup() 
    {
        // ---- OpenGL stuff ----

        // Setup window
        glfwSetErrorCallback(glfw_error_callback);
        
        if (!glfwInit())
        {
            return;
        }

        // Use GL 3.2 (All Platforms)
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

        // Decide GLSL version
        #ifdef __APPLE__
        // GLSL 150
        const char* glsl_version = "#version 150";
        #else
        // GLSL 130
        const char* glsl_version = "#version 130";
        #endif

        // Create window with graphics context
        window = glfwCreateWindow(1360, 720, "Filter Visualization", NULL, NULL);
        if(window == NULL)
        {
            return;
        }
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); // Enable vsync

        // Initialize OpenGL loader
        if(gl3wInit() != 0)
        {
            fprintf(stderr, "Failed to initialize OpenGL loader!\n");
            return;
        }

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        ImGuiStyle* style = &ImGui::GetStyle();
        style->WindowRounding = 4;
        style->FrameRounding = 4;
        style->GrabRounding = 4;

        // Setup Platform/renderer_madg backends
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);
	}

	void render() 
    {
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(720, 480), ImGuiCond_FirstUseEver);

		generateUI(); // initialized in gui.hpp and implemented in gui.cpp

		ImGui::Render();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
	}

	void begin()
	{
		setup();

        initUI(); // initialized in gui.hpp and implemented in gui.cpp

		while(!glfwWindowShouldClose(window))
		{
			render();
			glfwPollEvents();
		}

		destroy();
	}
}