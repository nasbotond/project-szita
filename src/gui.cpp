#include "gui.hpp"
// OpenGL Loader

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "filter.hpp"
#include "ply_writer.hpp"

/* TODO:
5. JBU upsampling vs iterative ... ?
6. implement metrics
7. report execution times of the filter algorithms
8. read d_min from txt
*/

namespace GUI 
{
    char const* selectedfolderPath;
    std::string fPath;

    static std::thread t;
    static std::thread _t;

    std::atomic<bool> is_calculated{false};
    static bool loading = false;

    // Init vtkViewers
    VtkViewer vtk_viewer_jbu;
    VtkViewer vtk_viewer_it;

    vtkSmartPointer<vtkActor> ply_actor_jbu;
    vtkSmartPointer<vtkActor> ply_actor_it;

    vtkNew<vtkNamedColors> colors;

    // Initial UI state variables set
    static bool black_background = false;

    static float spatial_sigma = 2.5;
    static float spectral_sigma = 5;
    static int window_size = 5;
    static int d_min = 200;
    static int baseline = 160;
    static int focal_length = 3740;

    static bool vtk_jb_pc_open = true;
    static bool vtk_it_pc_open = true;

    static bool box_image_open = false;
    static bool gaussian_image_open = false;
    static bool bilateral_image_open = true;
    static bool jb_image_open = false;
    static bool it_up_image_open = false;

    static bool save_to_file = false;

    cv::Mat bilateral;
    cv::Mat gaussian;
    cv::Mat box;
    cv::Mat jb;
    cv::Mat new_d;

    MatViewer viewer_bilateral;
    MatViewer viewer_gaussian;
    MatViewer viewer_box;
    MatViewer viewer_jb;
    MatViewer viewer_new_d;

    // tinyfiledialogs
    void* call_from_thread()
    {
        selectedfolderPath = tinyfd_selectFolderDialog("Select directory with data files in it", "");

        // If the dialog is open and we try to open it again, stop the program until it is closed
        if (!selectedfolderPath)
        {
            // Return to UI
            return NULL;
        }
        else
        {
            fPath = selectedfolderPath;
        }
        return NULL;
    }

    static std::string _labelPrefix(const char* const label)
    {
        float width = ImGui::CalcItemWidth();

        float x = ImGui::GetCursorPosX();
        ImGui::Text("%s", label);
        ImGui::SameLine();
        ImGui::SetCursorPosX(x + width * 0.6f + ImGui::GetStyle().ItemInnerSpacing.x);
        ImGui::SetNextItemWidth(-1);

        std::string labelID = "##";
        labelID += label;

        return labelID;
    }

    void* runFilters()
    {
        cv::Mat input = cv::imread(fPath + "view1.png", 0);
        cv::Mat input_d = cv::imread(fPath + "disp1.png", 0);
        cv::Mat input_d_low = cv::imread(fPath + "disp1_low.png", 0);
        
        gaussian_filter(input, gaussian, window_size);
        box_filter(input, box, window_size);
        bilateral_filter(input, bilateral, window_size, spatial_sigma, spectral_sigma);
        joint_bilateral_filter(input, input_d, jb, window_size, spatial_sigma, spectral_sigma);
        iterative_upsampling(input, input_d_low, new_d, window_size, spatial_sigma, spectral_sigma);        

        // std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        std::string results_suffix = "_" + std::to_string(spatial_sigma) + "_" + std::to_string(spectral_sigma) + "_" + std::to_string(window_size);

        std::string str = fPath.substr(0, fPath.length()-2);
        char ch = '/';
        size_t index = str.rfind(ch);

        if(index != std::string::npos)
        {
            results_suffix = "out_" + fPath.substr(index+1, fPath.length()-2-index) + results_suffix;
        }
        else
        {
            results_suffix = "out" + results_suffix;
        }

        std::filesystem::create_directories(fPath + results_suffix);

        cv::cvtColor(input, input, cv::COLOR_GRAY2BGR);

        PLYWriter::Disparity2PointCloud(fPath + results_suffix + "/jb", jb.rows, jb.cols, jb, 5, d_min, static_cast<double>(baseline), static_cast<double>(focal_length), input);
        PLYWriter::Disparity2PointCloud(fPath + results_suffix + "/iterative", new_d.rows, new_d.cols, new_d, 5, d_min, static_cast<double>(baseline), static_cast<double>(focal_length), input);

        cv::cvtColor(bilateral, bilateral, cv::COLOR_GRAY2BGR);
        cv::cvtColor(gaussian, gaussian, cv::COLOR_GRAY2BGR);
        cv::cvtColor(box, box, cv::COLOR_GRAY2BGR);
        cv::cvtColor(jb, jb, cv::COLOR_GRAY2BGR);
        cv::cvtColor(new_d, new_d, cv::COLOR_GRAY2BGR);

        ply_actor_jbu = getPLYActor(fPath + results_suffix + "/jb.ply", fPath + results_suffix + "/", "jb");
        ply_actor_it = getPLYActor(fPath + results_suffix + "/iterative.ply", fPath + results_suffix + "/", "iterative");

        vtk_viewer_jbu.addActor(ply_actor_jbu);
        vtk_viewer_it.addActor(ply_actor_it);

        if(save_to_file)
        {
            cv::imwrite(fPath + results_suffix + "/gaussian.png", gaussian);
            cv::imwrite(fPath + results_suffix + "/box.png", box);
            cv::imwrite(fPath + results_suffix + "/bilateral.png", bilateral);
            cv::imwrite(fPath + results_suffix + "/jb.png", jb);
            cv::imwrite(fPath + results_suffix + "/iterative_sampling_depth.png", new_d);
        }
        
        is_calculated = true;
        loading = false;
        return NULL;
    }

    void initUI()
    {
        // Set background color
        std::array<unsigned char, 4> bkg{{26, 51, 77, 255}};
        colors->SetColor("BkgColor", bkg.data());

        viewer_bilateral = MatViewer("Bilateral", bilateral);
        viewer_gaussian = MatViewer("Gaussian", gaussian);
        viewer_box = MatViewer("Box", box);
        viewer_jb = MatViewer("Joint Bilateral", jb);
        viewer_new_d = MatViewer("Iterative Upsampling", new_d);

        vtk_viewer_jbu.getRenderer()->SetBackground(colors->GetColor3d("BkgColor").GetData());
        vtk_viewer_it.getRenderer()->SetBackground(colors->GetColor3d("BkgColor").GetData());
    }

    void generateUI()
    {        
        // ImGui::ShowDemoWindow();
        {            
            ImGui::Begin("Menu");

            if(ImGui::Button("Select directory"))
            {
                selectedfolderPath = NULL;
                fPath = "";

                // If the dialog is open and we try to open it again, stop the program until it is closed
				if(t.joinable())
				{
					t.join();
					t = std::thread(&call_from_thread);
				}
				else
				{
					t = std::thread(&call_from_thread);
				}
            }

            ImGui::Text("%s", fPath.c_str());

            ImGui::InputInt(_labelPrefix("Window size:").c_str(), &window_size, 1, 2);

            ImGui::InputFloat(_labelPrefix("Spatial sigma:").c_str(), &spatial_sigma, 0.001f, 0.01f, "%.1f");

            ImGui::InputFloat(_labelPrefix("Spectral sigma:").c_str(), &spectral_sigma, 0.01f, 0.1f, "%.1f");

            ImGui::InputInt(_labelPrefix("d_min:").c_str(), &d_min, 1, 10);

            ImGui::InputInt(_labelPrefix("Baseline (mm):").c_str(), &baseline, 1, 10);

            ImGui::InputInt(_labelPrefix("Focal length (pixels):").c_str(), &focal_length, 1, 10);

            ImGui::Checkbox(_labelPrefix("Save results to files: ").c_str(), &save_to_file);

            if(ImGui::Button("Calculate"))
            {     
                if(t.joinable())
				{
					t.join();
				}

                if(fPath.length() != 0)
                {
                    loading = true;
                    is_calculated = false;

                    if(_t.joinable())
                    {
                        _t.join();
                        _t = std::thread(&runFilters);
                    }
                    else
                    {
                        _t = std::thread(&runFilters);
                    }
                }
                else
                {
                    ImGui::OpenPopup("No data, no problem...?");
                }
            }
            if(!is_calculated && loading)
            {
                ImGui::SameLine();
                ImGui::Text("Calculating... %c", "|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3]);
            }
            ImGui::Text("");

            // Always center this window when appearing
            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            if(ImGui::BeginPopupModal("No data, no problem...?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("Please select a folder containing data files!\n\n");
                ImGui::Separator();

                if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
                ImGui::SetItemDefaultFocus();
                ImGui::EndPopup();
            }

            if(is_calculated)
            {
                if(_t.joinable())
                {
                    _t.join();
                }
                if(t.joinable())
                {
                    t.join();
                }
                viewer_bilateral.update();
                viewer_gaussian.update();
                viewer_box.update();
                viewer_jb.update();
                viewer_new_d.update();

                auto renderer_pc = vtk_viewer_jbu.getRenderer();                
                auto renderer_oriented_pc = vtk_viewer_it.getRenderer();

                ImGui::Checkbox(_labelPrefix("Black background:").c_str(), &black_background);
                if(black_background)
                {
                    // Set background to black
                    renderer_pc->SetBackground(0, 0, 0);
                    renderer_oriented_pc->SetBackground(0, 0, 0);
                }

                if(!black_background)
                {
                    renderer_pc->SetBackground(colors->GetColor3d("BkgColor").GetData());
                    renderer_oriented_pc->SetBackground(colors->GetColor3d("BkgColor").GetData());
                }
            
                static float vtk2BkgAlpha = 0.2f;
                ImGui::SliderFloat(_labelPrefix("Background alpha:").c_str(), &vtk2BkgAlpha, 0.0f, 1.0f);
                renderer_pc->SetBackgroundAlpha(vtk2BkgAlpha);
                renderer_oriented_pc->SetBackgroundAlpha(vtk2BkgAlpha);

                ImGui::Text("");
                ImGui::Text("Show windows:");
                ImGui::Checkbox(_labelPrefix("JBU point cloud:").c_str(), &vtk_jb_pc_open);
                ImGui::Checkbox(_labelPrefix("Iter. Sampl. point cloud:").c_str(), &vtk_it_pc_open);
                ImGui::Checkbox(_labelPrefix("Gaussian:").c_str(), &gaussian_image_open);
                ImGui::Checkbox(_labelPrefix("Box:").c_str(), &box_image_open);
                ImGui::Checkbox(_labelPrefix("Bilateral:").c_str(), &bilateral_image_open);
                ImGui::Checkbox(_labelPrefix("Joint bilateral:").c_str(), &jb_image_open);
                ImGui::Checkbox(_labelPrefix("Iterative Upsampling:").c_str(), &it_up_image_open);
                ImGui::Text("");
            }

            ImGui::Text("\nApplication average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            ImGui::End();
        }

        if(vtk_jb_pc_open && is_calculated)
        {
            ImGui::SetNextWindowSize(ImVec2(720, 480), ImGuiCond_FirstUseEver);
            ImGui::Begin("JBU", &vtk_jb_pc_open, VtkViewer::NoScrollFlags());            
            
            vtk_viewer_jbu.render();
            ImGui::End();
        }

        if(vtk_it_pc_open && is_calculated)
        {
            ImGui::SetNextWindowSize(ImVec2(720, 480), ImGuiCond_FirstUseEver);
            ImGui::Begin("Iter. Sampl.", &vtk_it_pc_open, VtkViewer::NoScrollFlags());

            vtk_viewer_it.render();
            ImGui::End();
        }

        if(bilateral_image_open && is_calculated)
        {
            ImGui::SetNextWindowSize(ImVec2(720, 480), ImGuiCond_FirstUseEver);
            ImGui::Begin("Bilateral", &bilateral_image_open, VtkViewer::NoScrollFlags()); 

            viewer_bilateral.addToGUI();
            ImGui::End();
        }

        if(box_image_open && is_calculated)
        {
            ImGui::SetNextWindowSize(ImVec2(720, 480), ImGuiCond_FirstUseEver);
            ImGui::Begin("Box", &box_image_open, VtkViewer::NoScrollFlags()); 

            viewer_box.addToGUI();
            ImGui::End();
        }

        if(gaussian_image_open && is_calculated)
        {
            ImGui::SetNextWindowSize(ImVec2(720, 480), ImGuiCond_FirstUseEver);
            ImGui::Begin("Gaussian", &gaussian_image_open, VtkViewer::NoScrollFlags()); 

            viewer_gaussian.addToGUI();
            ImGui::End();
        }

        if(jb_image_open && is_calculated)
        {
            ImGui::SetNextWindowSize(ImVec2(720, 480), ImGuiCond_FirstUseEver);
            ImGui::Begin("Joint bilateral", &jb_image_open, VtkViewer::NoScrollFlags()); 

            viewer_jb.addToGUI();
            ImGui::End();
        }

        if(it_up_image_open && is_calculated)
        {
            ImGui::SetNextWindowSize(ImVec2(720, 480), ImGuiCond_FirstUseEver);
            ImGui::Begin("Iterative Upsample", &it_up_image_open, VtkViewer::NoScrollFlags()); 

            viewer_new_d.addToGUI();
            ImGui::End();
        }
    }
}