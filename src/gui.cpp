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
    VtkViewer vtk_viewer_pc;
    VtkViewer vtk_viewer_oriented_pc;

    vtkSmartPointer<vtkActor> ply_actor_pc;
    vtkSmartPointer<vtkActor> ply_actor_oriented_pc;

    vtkNew<vtkNamedColors> colors;

    // Initial UI state variables set
    static bool black_background = false;

    static float naive_gain = 0.003f;
    static float madg_beta = 0.18f;
    static float madg_zeta = 0.0001f;

    static bool vtk_pc_open = false;
    static bool vtk_oriented_pc_open = true;

    static bool save_to_file = false;

    // static bool show_style_editor = false;
    // static bool show_demo_window = false;

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
        std::ofstream ply_pc;
        std::ofstream ply_pc_oriented;

        std::string results_suffix = "_" +std::to_string(freq) + "_" + std::to_string(naive_gain) + "_" + std::to_string(madg_beta) + "_" + std::to_string(start_index) + "_" + std::to_string(end_index);

        std::string str = fPath.substr(0, fPath.length()-2);
        char ch = '/';
        size_t index = str.rfind(ch);

        if (index != std::string::npos)
        {
            results_suffix = "out_" + fPath.substr(index+1, fPath.length()-2-index) + results_suffix;
        }
        else
        {
            results_suffix = "out" + results_suffix;
        }

        std::filesystem::create_directories(fPath + results_suffix);

        ply_pc.open(fPath + results_suffix + "/pc.ply");
        ply_pc_oriented.open(fPath + results_suffix + "/pc_oriented.ply");

        // run filters here
        // for (int i = start_index; i < end_index; ++i)
        // {
        //     ply_pc << enu_madg_mag.q_1 << "," << enu_madg_mag.q_2 << "," << enu_madg_mag.q_3 << "," << enu_madg_mag.q_4 << "\n";
        //     ply_pc_oriented << enu_naive_mag.q_1 << "," << enu_naive_mag.q_2 << "," << enu_naive_mag.q_3 << "," << enu_naive_mag.q_4 << "\n";

        // }

        ply_pc.close();
        ply_pc_oriented.close();

        vtk_viewer_pc.removeActor(ply_actor_pc);
        vtk_viewer_oriented_pc.removeActor(ply_actor_oriented_pc);        

        ply_actor_pc = getPLYActor("../data/7_dp.ply");
        ply_actor_oriented_pc = getPLYActor("../data/7_gt.ply");

        vtk_viewer_pc.addActor();
        vtk_viewer_oriented_pc.addActor();

        // update actors here
        // vtk_viewer_madg.updateActors(ply_actor_pc, gravity_vectors_madg.at(0));        
        // vtk_viewer_naive.updateActors(ply_actor_oriented_pc, gravity_vectors_naive.at(0));

        if(!save_to_file)
        {
            std::filesystem::remove_all(fPath + results_suffix);
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

        // Initialize actors
        // ply_actor_pc = getPLYActor({0, 0, 0});
        // ply_actor_oriented_pc = getPLYActor({0, 0, 0});

        // Add actors to vtkViewer instances
        vtk_viewer_pc.getRenderer()->SetBackground(colors->GetColor3d("BkgColor").GetData());
        // vtk_viewer_pc.addActor(ply_actor_pc);

        vtk_viewer_oriented_pc.getRenderer()->SetBackground(colors->GetColor3d("BkgColor").GetData());
        // vtk_viewer_oriented_pc.addActor(ply_actor_oriented_pc);
    }

    void generateUI()
    {
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

            ImGui::InputFloat(_labelPrefix("Naive filter gain:").c_str(), &naive_gain, 0.001f, 0.01f, "%.5f");

            ImGui::InputFloat(_labelPrefix("Madg. filter beta:").c_str(), &madg_beta, 0.01f, 0.1f, "%.5f");

            ImGui::InputFloat(_labelPrefix("Madg. filter zeta:").c_str(), &madg_zeta, 0.0001f, 0.001f, "%.5f");

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

                auto renderer_pc = vtk_viewer_pc.getRenderer();                
                auto renderer_oriented_pc = vtk_viewer_oriented_pc.getRenderer();

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

                // update actors
                // vtk_viewer_madg.updateActors(actors_madg, gravity_vectors_madg.at(vector_index));
                // vtk_viewer_gt.updateActors(actors_gt, gravity_vectors_gt.at(vector_index));
                
                ImGui::PopButtonRepeat();

                ImGui::Text("");
                ImGui::Text("Show PC:");
                ImGui::Checkbox(_labelPrefix("PC:").c_str(), &vtk_pc_open);
                ImGui::Checkbox(_labelPrefix("Oriented PC:").c_str(), &vtk_oriented_pc_open);
                ImGui::Text("");
            }

            ImGui::Text("\nApplication average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            ImGui::End();
        }

        if(vtk_pc_open && is_calculated)
        {
            ImGui::SetNextWindowSize(ImVec2(720, 480), ImGuiCond_FirstUseEver);
            ImGui::Begin("PC", &vtk_pc_open, VtkViewer::NoScrollFlags());
            
            vtk_viewer_pc.render();
            ImGui::End();
        }

        if(vtk_oriented_pc_open && is_calculated)
        {
            ImGui::SetNextWindowSize(ImVec2(720, 480), ImGuiCond_FirstUseEver);
            ImGui::Begin("Oriented PC", &vtk_oriented_pc_open, VtkViewer::NoScrollFlags());
            
            vtk_viewer_oriented_pc.render();
            ImGui::End();
        }
    }
}