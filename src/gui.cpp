#include "gui.hpp"

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

    static bool noisy_image_open = false;
    static bool box_image_open = false;
    static bool gaussian_image_open = false;
    static bool bilateral_image_open = true;
    static bool jb_image_open = false;
    static bool jbu_image_open = false;
    static bool it_up_image_open = false;

    static bool save_to_file = false;
    static bool get_point_clouds = false;
    static bool calc_metrics = false;

    static bool has_point_cloud = false;

    cv::Mat noisy_input;
    cv::Mat bilateral;
    cv::Mat gaussian;
    cv::Mat box;
    cv::Mat jb;
    cv::Mat jbu;
    cv::Mat new_d;

    MatViewer viewer_noisy;
    MatViewer viewer_bilateral;
    MatViewer viewer_gaussian;
    MatViewer viewer_box;
    MatViewer viewer_jb;
    MatViewer viewer_jbu;
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

        noisy_input = input.clone();
        cv::Mat noise(input.size(), input.type());
        uchar mean = 0;
        uchar stddev = 25;
        cv::randn(noise, mean, stddev);
        noisy_input += noise;
  
        auto start1 = std::chrono::high_resolution_clock::now();
        gaussian_filter(noisy_input, gaussian, window_size);
        auto stop1 = std::chrono::high_resolution_clock::now();
        auto duration1 = std::chrono::duration_cast<std::chrono::milliseconds>(stop1 - start1);
        std::cout << "Time taken by Gaussian: " << duration1.count() << " milliseconds" << std::endl;

        auto start2 = std::chrono::high_resolution_clock::now();
        box_filter(noisy_input, box, window_size);
        auto stop2 = std::chrono::high_resolution_clock::now();
        auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(stop2 - start2);
        std::cout << "Time taken by Box: " << duration2.count() << " milliseconds" << std::endl;

        auto start3 = std::chrono::high_resolution_clock::now();
        bilateral_filter(noisy_input, bilateral, window_size, spatial_sigma, spectral_sigma);
        auto stop3 = std::chrono::high_resolution_clock::now();
        auto duration3 = std::chrono::duration_cast<std::chrono::milliseconds>(stop3 - start3);
        std::cout << "Time taken by Bilateral: " << duration3.count() << " milliseconds" << std::endl;

        auto start4 = std::chrono::high_resolution_clock::now();
        joint_bilateral_filter(input, input_d, jb, window_size, spatial_sigma, spectral_sigma);
        auto stop4 = std::chrono::high_resolution_clock::now();
        auto duration4 = std::chrono::duration_cast<std::chrono::milliseconds>(stop4 - start4);
        std::cout << "Time taken by Joint bilateral: " << duration4.count() << " milliseconds" << std::endl;

        auto start6 = std::chrono::high_resolution_clock::now();
        joint_bilateral_upsampling(input, input_d_low, jbu, window_size, spatial_sigma, spectral_sigma);
        auto stop6 = std::chrono::high_resolution_clock::now();
        auto duration6 = std::chrono::duration_cast<std::chrono::milliseconds>(stop6 - start6);
        std::cout << "Time taken by Joint bilateral upsampling: " << duration6.count() << " milliseconds" << std::endl;

        auto start5 = std::chrono::high_resolution_clock::now();
        iterative_upsampling(input, input_d_low, new_d, window_size, spatial_sigma, spectral_sigma);
        auto stop5 = std::chrono::high_resolution_clock::now();
        auto duration5 = std::chrono::duration_cast<std::chrono::milliseconds>(stop5 - start5);
        std::cout << "Time taken by Iterative upsampling: " << duration5.count() << " milliseconds" << std::endl;     

        std::string results_suffix;

        if(get_point_clouds || save_to_file || calc_metrics)
        {
            results_suffix = "_" + std::to_string(spatial_sigma) + "_" + std::to_string(spectral_sigma) + "_" + std::to_string(window_size);

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
        }

        if(get_point_clouds)
        {
            std::fstream file(fPath + "dmin.txt", std::ios::in);
            std::string line;
            if(file.is_open())
            {
                getline(file, line);
                d_min = stoi(line);
            }
            else
            {
                throw std::runtime_error("Error: failed to open file");
            }

            cv::cvtColor(input, input, cv::COLOR_GRAY2BGR);

            PLYWriter::Disparity2PointCloud(fPath + results_suffix + "/jbu", jbu.rows, jbu.cols, jbu, 5, d_min, static_cast<double>(baseline), static_cast<double>(focal_length), input);
            PLYWriter::Disparity2PointCloud(fPath + results_suffix + "/iterative", new_d.rows, new_d.cols, new_d, 5, d_min, static_cast<double>(baseline), static_cast<double>(focal_length), input);

            ply_actor_jbu = getPLYActor(fPath + results_suffix + "/jbu.ply", fPath + results_suffix + "/", "jbu");
            ply_actor_it = getPLYActor(fPath + results_suffix + "/iterative.ply", fPath + results_suffix + "/", "iterative");

            vtk_viewer_jbu.addActor(ply_actor_jbu);
            vtk_viewer_it.addActor(ply_actor_it);
            has_point_cloud = true;
        }
        else
        {
            vtk_jb_pc_open = false;
            vtk_it_pc_open = false;
        }

        if(calc_metrics)
        {
            std::string metrics_dir = fPath + results_suffix + "/metrics";
            std::filesystem::create_directories(metrics_dir);

            std::cout << "JBU metrics" << std::endl;
            Metrics::MAD(jbu, input_d, metrics_dir + "/jbu");
            Metrics::MSE(jbu, input_d, metrics_dir + "/jbu");
            Metrics::NCC(jbu, input_d);
            Metrics::MSSIM(jbu, input_d, metrics_dir + "/jbu");

            std::cout << "Iterative upsampling metrics" << std::endl;
            Metrics::MAD(new_d, input_d, metrics_dir + "/it");
            Metrics::MSE(new_d, input_d, metrics_dir + "/it");
            Metrics::NCC(new_d, input_d);
            Metrics::MSSIM(new_d, input_d, metrics_dir + "/it");
        }

        if(save_to_file)
        {
            cv::imwrite(fPath + results_suffix + "/noisy.png", noisy_input);
            cv::imwrite(fPath + results_suffix + "/gaussian.png", gaussian);
            cv::imwrite(fPath + results_suffix + "/box.png", box);
            cv::imwrite(fPath + results_suffix + "/bilateral.png", bilateral);
            cv::imwrite(fPath + results_suffix + "/jb.png", jb);
            cv::imwrite(fPath + results_suffix + "/jbu.png", jbu);
            cv::imwrite(fPath + results_suffix + "/iterative_sampling_depth.png", new_d);
        }

        cv::cvtColor(noisy_input, noisy_input, cv::COLOR_GRAY2BGR);
        cv::cvtColor(bilateral, bilateral, cv::COLOR_GRAY2BGR);
        cv::cvtColor(gaussian, gaussian, cv::COLOR_GRAY2BGR);
        cv::cvtColor(box, box, cv::COLOR_GRAY2BGR);
        cv::cvtColor(jb, jb, cv::COLOR_GRAY2BGR);
        cv::cvtColor(jbu, jbu, cv::COLOR_GRAY2BGR);
        cv::cvtColor(new_d, new_d, cv::COLOR_GRAY2BGR);
        
        is_calculated = true;
        loading = false;
        return NULL;
    }

    void initUI()
    {
        // Set background color
        std::array<unsigned char, 4> bkg{{26, 51, 77, 255}};
        colors->SetColor("BkgColor", bkg.data());

        viewer_noisy = MatViewer("Noisy input", noisy_input);
        viewer_bilateral = MatViewer("Bilateral", bilateral);
        viewer_gaussian = MatViewer("Gaussian", gaussian);
        viewer_box = MatViewer("Box", box);
        viewer_jb = MatViewer("Joint bilateral", jb);
        viewer_jbu = MatViewer("Joint bilateral upsampled", jbu);
        viewer_new_d = MatViewer("Iterative upsampled", new_d);

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

            ImGui::Checkbox(_labelPrefix("Get point clouds: ").c_str(), &get_point_clouds);

            if(get_point_clouds)
            {
                ImGui::InputInt(_labelPrefix("d_min:").c_str(), &d_min, 1, 10);

                ImGui::InputInt(_labelPrefix("Baseline (mm):").c_str(), &baseline, 1, 10);

                ImGui::InputInt(_labelPrefix("Focal length (pixels):").c_str(), &focal_length, 1, 10);
            }

            ImGui::Checkbox(_labelPrefix("Save filtered images to files: ").c_str(), &save_to_file);

            ImGui::Checkbox(_labelPrefix("Calculate and save metrics: ").c_str(), &calc_metrics);

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
                viewer_noisy.update();
                viewer_bilateral.update();
                viewer_gaussian.update();
                viewer_box.update();
                viewer_jb.update();
                viewer_jbu.update();
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
                if(ImGui::CollapsingHeader("Show filtered images"))
                {
                    // ImGui::Text("Show windows:");
                    ImGui::Checkbox(_labelPrefix("Noisy input:").c_str(), &noisy_image_open);
                    ImGui::Checkbox(_labelPrefix("Gaussian:").c_str(), &gaussian_image_open);
                    ImGui::Checkbox(_labelPrefix("Box:").c_str(), &box_image_open);
                    ImGui::Checkbox(_labelPrefix("Bilateral:").c_str(), &bilateral_image_open);
                    ImGui::Checkbox(_labelPrefix("Joint bilateral:").c_str(), &jb_image_open);
                    ImGui::Checkbox(_labelPrefix("Joint bilateral upsampled:").c_str(), &jbu_image_open);
                    ImGui::Checkbox(_labelPrefix("Iterative upsampled:").c_str(), &it_up_image_open);
                    ImGui::Text("");
                }
                // if(ImGui::CollapsingHeader("Show metrics"))
                // {
                // }
                if(has_point_cloud)
                {
                    if(ImGui::CollapsingHeader("Show point clouds"))
                    {
                        ImGui::Checkbox(_labelPrefix("JBU point cloud:").c_str(), &vtk_jb_pc_open);
                        ImGui::Checkbox(_labelPrefix("Iter. sampl. point cloud:").c_str(), &vtk_it_pc_open);
                    }
                }
                
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

        if(noisy_image_open && is_calculated)
        {
            ImGui::SetNextWindowSize(ImVec2(720, 480), ImGuiCond_FirstUseEver);
            ImGui::Begin("Noisy", &noisy_image_open, VtkViewer::NoScrollFlags()); 

            viewer_noisy.addToGUI();
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

        if(jbu_image_open && is_calculated)
        {
            ImGui::SetNextWindowSize(ImVec2(720, 480), ImGuiCond_FirstUseEver);
            ImGui::Begin("Joint bilateral upsampled", &jbu_image_open, VtkViewer::NoScrollFlags()); 

            viewer_jbu.addToGUI();
            ImGui::End();
        }

        if(it_up_image_open && is_calculated)
        {
            ImGui::SetNextWindowSize(ImVec2(720, 480), ImGuiCond_FirstUseEver);
            ImGui::Begin("Iterative upsampled", &it_up_image_open, VtkViewer::NoScrollFlags()); 

            viewer_new_d.addToGUI();
            ImGui::End();
        }
    }
}