#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <string> 
#include <fstream>
#include <sstream>
#include <vector>

namespace Metrics
{
    // Mean Absolute Difference
    void MAD(const cv::Mat& disp_est, const cv::Mat& disp_gt, const std::string& output_file)
    {
        cv::Mat mad;
        cv::absdiff(disp_est, disp_gt, mad);
        // cv::normalize(mad, mad, 255, 0, cv::NORM_MINMAX);

        std::cout << "MAD mean: " << cv::mean(mad) << std::endl;
        cv::imwrite(output_file + "_mad.png", mad);
    }

    // Mean Squared Error
    void MSE(const cv::Mat& disp_est, const cv::Mat& disp_gt, const std::string& output_file)
    {
        int height = disp_est.rows;
        int width = disp_est.cols;
        cv::Mat ssd = cv::Mat::zeros(height, width, CV_8UC1);

        for (int i = 0; i < height; ++i)
        {
            for (int j = 0; j < width; ++j)
            {
                ssd.at<uchar>(i, j) = pow((disp_gt.at<uchar>(i, j) - disp_est.at<uchar>(i, j)), 2);
            }
        }
        // cv::normalize(ssd, ssd, 255, 0, cv::NORM_MINMAX);

        std::cout << "MSE mean: " << cv::mean(ssd) << std::endl;
        cv::imwrite(output_file + "_sad.png", ssd);
    }

    // Normalized Cross Correlation
    void NCC(const cv::Mat& disp_est, const cv::Mat& disp_gt)
    {
        int height = disp_est.rows;
        int width = disp_est.cols;
        cv::Mat ncc = cv::Mat::zeros(height, width, CV_32FC1);

        cv::Mat float_gt;
        cv::Mat float_est;
        disp_gt.convertTo(float_gt, CV_32FC1);
        disp_est.convertTo(float_est, CV_32FC1);

        cv::matchTemplate(float_gt, float_est, ncc, cv::TM_CCORR_NORMED);

        std::cout << "NCC mean: " << cv::mean(ncc) << std::endl;
    }

    // OpenCV Implementation of Structural Similarity Measure
    void MSSIM(const cv::Mat& disp_est, const cv::Mat& disp_gt, const std::string& output_file)
    {
        const double C1 = 6.5025, C2 = 58.5225;

        int d = CV_32F;
        cv::Mat I1, I2;
        disp_est.convertTo(I1, d); // cannot calculate on one byte large values
        disp_gt.convertTo(I2, d);
        cv::Mat I2_2 = I2.mul(I2); // I2^2
        cv::Mat I1_2 = I1.mul(I1); // I1^2
        cv::Mat I1_I2 = I1.mul(I2); // I1 * I2

        cv::Mat mu1, mu2;
        cv::GaussianBlur(I1, mu1, cv::Size(11, 11), 1.5);
        cv::GaussianBlur(I2, mu2, cv::Size(11, 11), 1.5);
        cv::Mat mu1_2 = mu1.mul(mu1);
        cv::Mat mu2_2 = mu2.mul(mu2);
        cv::Mat mu1_mu2 = mu1.mul(mu2);
        cv::Mat sigma1_2, sigma2_2, sigma12;
        cv::GaussianBlur(I1_2, sigma1_2, cv::Size(11, 11), 1.5);
        sigma1_2 -= mu1_2;
        cv::GaussianBlur(I2_2, sigma2_2, cv::Size(11, 11), 1.5);
        sigma2_2 -= mu2_2;
        cv::GaussianBlur(I1_I2, sigma12, cv::Size(11, 11), 1.5);
        sigma12 -= mu1_mu2;

        cv::Mat t1, t2, t3;
        t1 = 2 * mu1_mu2 + C1;
        t2 = 2 * sigma12 + C2;
        t3 = t1.mul(t2); // t3 = ((2*mu1_mu2 + C1).*(2*sigma12 + C2))
        t1 = mu1_2 + mu2_2 + C1;
        t2 = sigma1_2 + sigma2_2 + C2;
        t1 = t1.mul(t2); // t1 =((mu1_2 + mu2_2 + C1).*(sigma1_2 + sigma2_2 + C2))

        cv::Mat ssim_map;
        cv::divide(t3, t1, ssim_map); // ssim_map =  t3./t1;
        // cv::Scalar mssim = cv::mean(ssim_map); // mssim = average of ssim map
        std::cout << "SSIM mean: " << cv::mean(ssim_map) << std::endl;

        double minVal; double maxVal;
        cv::minMaxLoc(ssim_map, &minVal, &maxVal);

        cv::normalize(ssim_map, ssim_map, 255./(maxVal-minVal), 0, cv::NORM_MINMAX);
        cv::imwrite(output_file + "_ssim.png", ssim_map);
    }
}