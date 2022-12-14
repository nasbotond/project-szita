#include "main.hpp"
// #include "filter.hpp"

int main(int argc, char* argv[])
{
    GUI::begin();
	return 0;
}

/**
 * @brief Average RMSE calculation
 *
 */
// int main(int argc, char* argv[])
// {
//     std::string sPath = "/Users/botond/development/ELTE-IK_22_23_1/3D_sensing_and_sensor_fusion/assignments/szita_metrics/";
//     int window_size = 5;

//     std::vector<std::vector<float>> grand_row_jbu_mad( 20 , std::vector<float> (9, 0));
//     std::vector<std::vector<float>> grand_row_iu_mad( 20 , std::vector<float> (9, 0));

//     std::vector<std::vector<float>> grand_row_jbu_mse( 20 , std::vector<float> (9, 0));
//     std::vector<std::vector<float>> grand_row_iu_mse( 20 , std::vector<float> (9, 0));

//     std::vector<std::vector<float>> grand_row_jbu_ncc( 20 , std::vector<float> (9, 0));
//     std::vector<std::vector<float>> grand_row_iu_ncc( 20 , std::vector<float> (9, 0));

//     std::vector<std::vector<float>> grand_row_jbu_mssim( 20 , std::vector<float> (9, 0));
//     std::vector<std::vector<float>> grand_row_iu_mssim( 20 , std::vector<float> (9, 0));

//     for(const auto & entry : std::filesystem::directory_iterator(sPath))
//     {
//         std::string fileName = entry.path().filename().string();

//         if(entry.is_directory())
//         {
//             std::cout << fileName << '\n';
//             std::ofstream jbu_mad;
//             std::ofstream jbu_mse;
//             std::ofstream jbu_ncc;
//             std::ofstream jbu_mssim;

//             std::ofstream iu_mad;
//             std::ofstream iu_mse;
//             std::ofstream iu_ncc;
//             std::ofstream iu_mssim;

//             jbu_mad.open(sPath + fileName + "/jbu_mad.csv");
//             jbu_mse.open(sPath + fileName + "/jbu_mse.csv");
//             jbu_ncc.open(sPath + fileName + "/jbu_ncc.csv");
//             jbu_mssim.open(sPath + fileName + "/jbu_mssim.csv");

//             iu_mad.open(sPath + fileName + "/iu_mad.csv");
//             iu_mse.open(sPath + fileName + "/iu_mse.csv");
//             iu_ncc.open(sPath + fileName + "/iu_ncc.csv");
//             iu_mssim.open(sPath + fileName + "/iu_mssim.csv");

//             jbu_mad << " ";
//             jbu_mse << " ";
//             jbu_ncc << " ";
//             jbu_mssim << " ";

//             iu_mad << " ";
//             iu_mse << " ";
//             iu_ncc << " ";
//             iu_mssim << " ";

//             for(float spectral_sigma = 1.0; spectral_sigma <= 5.0; spectral_sigma=spectral_sigma+0.5) // columns
//             {
//                 jbu_mad << "," << spectral_sigma;
//                 jbu_mse << "," << spectral_sigma;
//                 jbu_ncc << "," << spectral_sigma;
//                 jbu_mssim << "," << spectral_sigma;

//                 iu_mad << "," << spectral_sigma;
//                 iu_mse << "," << spectral_sigma;
//                 iu_ncc << "," << spectral_sigma;
//                 iu_mssim << "," << spectral_sigma;
//             }
//             jbu_mad << "\n";
//             jbu_mse << "\n";
//             jbu_ncc << "\n";
//             jbu_mssim << "\n";

//             iu_mad << "\n";
//             iu_mse << "\n";
//             iu_ncc << "\n";
//             iu_mssim << "\n";

//             int grand_row = 0;
//             int grand_col = 0;

//             cv::Mat jbu;
//             cv::Mat new_d;

//             cv::Mat input = cv::imread(sPath + fileName + "/view1.png", 0);
//             cv::Mat input_d = cv::imread(sPath + fileName + "/disp1.png", 0);
//             cv::Mat input_d_low = cv::imread(sPath + fileName + "/disp1_low.png", 0);

//             // Madgwick mag
//             for(float spatial_sigma = 1.0; spatial_sigma <= 20.0; spatial_sigma=spatial_sigma+1.0) // rows
//             {
//                 jbu_mad << spatial_sigma;
//                 jbu_mse << spatial_sigma;
//                 jbu_ncc << spatial_sigma;
//                 jbu_mssim << spatial_sigma;

//                 iu_mad << spatial_sigma;
//                 iu_mse << spatial_sigma;
//                 iu_ncc << spatial_sigma;
//                 iu_mssim << spatial_sigma;

//                 for(float spectral_sigma = 1.0; spectral_sigma <= 5.0; spectral_sigma=spectral_sigma+0.5) // columns
//                 {
//                     joint_bilateral_upsampling(input, input_d_low, jbu, window_size, spatial_sigma, spectral_sigma);

//                     iterative_upsampling(input, input_d_low, new_d, window_size, spatial_sigma, spectral_sigma);

//                     std::cout << "JBU metrics" << std::endl;
//                     float mad_jbu = Metrics::MAD(jbu, input_d, "/jbu");
//                     float mse_jbu = Metrics::MSE(jbu, input_d, "/jbu");
//                     float ncc_jbu = Metrics::NCC(jbu, input_d);
//                     float mssim_jbu = Metrics::MSSIM(jbu, input_d, "/jbu");

//                     std::cout << "Iterative upsampling metrics" << std::endl;
//                     float mad_iu = Metrics::MAD(new_d, input_d, "/it");
//                     float mse_iu = Metrics::MSE(new_d, input_d, "/it");
//                     float ncc_iu = Metrics::NCC(new_d, input_d);
//                     float mssim_iu = Metrics::MSSIM(new_d, input_d, "/it");

//                     jbu_mad << "," << mad_jbu;
//                     jbu_mse << "," << mse_jbu;
//                     jbu_ncc << "," << ncc_jbu;
//                     jbu_mssim << "," << mssim_jbu;

//                     iu_mad << "," << mad_iu;
//                     iu_mse << "," << mse_iu;
//                     iu_ncc << "," << ncc_iu;
//                     iu_mssim << "," << mssim_iu;

//                     grand_row_jbu_mad.at(grand_row).at(grand_col) += mad_jbu;
//                     grand_row_jbu_mse.at(grand_row).at(grand_col) += mse_jbu;
//                     grand_row_jbu_ncc.at(grand_row).at(grand_col) += ncc_jbu;
//                     grand_row_jbu_mssim.at(grand_row).at(grand_col) += mssim_jbu;

//                     grand_row_iu_mad.at(grand_row).at(grand_col) += mad_iu;
//                     grand_row_iu_mse.at(grand_row).at(grand_col) += mse_iu;
//                     grand_row_iu_ncc.at(grand_row).at(grand_col) += ncc_iu;
//                     grand_row_iu_mssim.at(grand_row).at(grand_col) += mssim_iu;

//                     grand_col++;
//                 }
//                 jbu_mad << "\n";
//                 jbu_mse << "\n";
//                 jbu_ncc << "\n";
//                 jbu_mssim << "\n";

//                 iu_mad << "\n";
//                 iu_mse << "\n";
//                 iu_ncc << "\n";
//                 iu_mssim << "\n";

//                 grand_row++;
//                 grand_col = 0;
//             }
//             jbu_mad.close();
//             jbu_mse.close();
//             jbu_ncc.close();
//             jbu_mssim.close();

//             iu_mad.close();
//             iu_mse.close();
//             iu_ncc.close();
//             iu_mssim.close();
//         }
//     }

//     std::ofstream jmad;
//     std::ofstream jmse;
//     std::ofstream jncc;
//     std::ofstream jmssim;

//     std::ofstream imad;
//     std::ofstream imse;
//     std::ofstream incc;
//     std::ofstream imssim;

//     jmad.open("../data/jmad_labelled.csv");
//     jmse.open("../data/jmse_labelled.csv");
//     jncc.open("../data/jncc_labelled.csv");
//     jmssim.open("../data/jmssim_labelled.csv");

//     imad.open("../data/imad_labelled.csv");
//     imse.open("../data/imse_labelled.csv");
//     incc.open("../data/incc_labelled.csv");
//     imssim.open("../data/imssim_labelled.csv");

//     float divide_by = 12.0;

//     jmad << " ";
//     jmse << " ";
//     jncc << " ";
//     jmssim << " ";

//     imad << " ";
//     imse << " ";
//     incc << " ";
//     imssim << " ";

//     for(float spectral_sigma = 1.0; spectral_sigma <= 5.0; spectral_sigma=spectral_sigma+0.5) // columns
//     {
//         jmad << "," << spectral_sigma;
//         jmse << "," << spectral_sigma;
//         jncc << "," << spectral_sigma;
//         jmssim << "," << spectral_sigma;

//         imad << "," << spectral_sigma;
//         imse << "," << spectral_sigma;
//         incc << "," << spectral_sigma;
//         imssim << "," << spectral_sigma;
//     }
//     jmad << "\n";
//     jmse << "\n";
//     jncc << "\n";
//     jmssim << "\n";

//     imad << "\n";
//     imse << "\n";
//     incc << "\n";
//     imssim << "\n";

//     for(int i = 0; i < 20; ++i)
//     {
//         jmad << (i+1.0);
//         jmse << (i+1.0);
//         jncc << (i+1.0);
//         jmssim << (i+1.0);

//         imad << (i+1.0);
//         imse << (i+1.0);
//         incc << (i+1.0);
//         imssim << (i+1.0);

//         for(int j = 0; j < 9; ++j)
//         {
//             jmad << "," << grand_row_jbu_mad.at(i).at(j)/divide_by;
//             jmse << "," << grand_row_jbu_mse.at(i).at(j)/divide_by;
//             jncc << "," << grand_row_jbu_ncc.at(i).at(j)/divide_by;
//             jmssim << "," << grand_row_jbu_mssim.at(i).at(j)/divide_by;

//             imad << "," << grand_row_iu_mad.at(i).at(j)/divide_by;
//             imse << "," << grand_row_iu_mse.at(i).at(j)/divide_by;
//             incc << "," << grand_row_iu_ncc.at(i).at(j)/divide_by;
//             imssim << "," << grand_row_iu_mssim.at(i).at(j)/divide_by;
//         }

//         jmad << "\n";
//         jmse << "\n";
//         jncc << "\n";
//         jmssim << "\n";

//         imad << "\n";
//         imse << "\n";
//         incc << "\n";
//         imssim << "\n";
//     }

//     jmad.close();
//     jmse.close();
//     jncc.close();
//     jmssim.close();

//     imad.close();
//     imse.close();
//     incc.close();
//     imssim.close();

//     return 0;
// }