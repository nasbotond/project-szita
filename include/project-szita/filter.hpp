#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

cv::Mat create_gaussian_kernel(int window_size, const float spatial_sigma = 2.5) 
{
    cv::Mat kernel(cv::Size(window_size, window_size), CV_32FC1);

    int half_window_size = window_size / 2;

    // see: lecture_03_slides.pdf, Slide 13
    // const double k = 2.5;
    const float k = spatial_sigma;
    const float r_max = std::sqrt(2.0 * half_window_size * half_window_size);
    const float sigma = r_max / k;

    // sum is for normalization 
    float sum = 0.0;

    for (int x = -window_size / 2; x <= window_size / 2; x++) 
    {
        for (int y = -window_size / 2; y <= window_size / 2; y++) 
        {
            float val = exp(-(x * x + y * y) / (2 * sigma * sigma));
            kernel.at<float>(x + window_size / 2, y + window_size / 2) = val;
            sum += val;
        }
    }

    // normalising the Kernel 
    for (int i = 0; i < window_size; ++i)
    {
        for (int j = 0; j < window_size; ++j)
        {
            kernel.at<float>(i, j) /= sum;
        }
    }
    // note that this is a naive implementation
    // there are alternative (better) ways
    // e.g. 
    // - perform analytic normalisation (what's the integral of the gaussian? :))
    // - you could store and compute values as uchar directly in stead of float
    // - computing it as a separable kernel [ exp(x + y) = exp(x) * exp(y) ] ...
    // - ...

    return kernel;
}

void box_filter(const cv::Mat& input, cv::Mat& output, const int window_size = 5) 
{
    const auto width = input.cols;
    const auto height = input.rows;
    output = cv::Mat::zeros(height, width, CV_8U);

    // TEMPORARY CODE
    // for (int r = 0; r < height; ++r) 
    // {
    //     for (int c = 0; c < width; ++c) 
    //     {
    //         output.at<uchar>(r, c) = 0;
    //     }
    // }

    for (int r = window_size / 2; r < height - window_size / 2; ++r) 
    {
        for (int c = window_size / 2; c < width - window_size / 2; ++c) 
        {

            // box filter
            int sum = 0;
            for (int i = -window_size / 2; i <= window_size / 2; ++i) 
            {
                for (int j = -window_size / 2; j <= window_size / 2; ++j) 
                {
                    sum += input.at<uchar>(r + i, c + j);
                }
            }
            output.at<uchar>(r, c) = sum / (window_size * window_size);
        }
    }
}

void gaussian_filter(const cv::Mat& input, cv::Mat& output, const int window_size = 5) 
{
    const auto width = input.cols;
    const auto height = input.rows;

    cv::Mat gaussianKernel = create_gaussian_kernel(window_size);
    output = cv::Mat::zeros(height, width, CV_8U);

    // TEMPORARY CODE
    // for(int r = 0; r < height; ++r) 
    // {
    //     for(int c = 0; c < width; ++c) 
    //     {
    //         output.at<uchar>(r, c) = 0;
    //     }
    // }

    for(int r = window_size / 2; r < height - window_size / 2; ++r) 
    {
        for(int c = window_size / 2; c < width - window_size / 2; ++c) 
        {
            int sum = 0;
            for(int i = -window_size / 2; i <= window_size / 2; ++i) 
            {
                for(int j = -window_size / 2; j <= window_size / 2; ++j) 
                {
                    sum += input.at<uchar>(r + i, c + j) * gaussianKernel.at<float>(i + window_size / 2, j + window_size / 2);
                }
            }
            output.at<uchar>(r, c) = sum;
        }
    }
}

void bilateral_filter(const cv::Mat& input, cv::Mat& output, const int window_size = 5, const float spatial_sigma = 2.5, const float spectral_sigma = 5.0)
{
    const auto width = input.cols;
    const auto height = input.rows;

    cv::Mat gaussianKernel = create_gaussian_kernel(window_size, spatial_sigma);
    output = cv::Mat::zeros(height, width, CV_8U);

    // TEMPORARY CODE
    // for(int r = 0; r < height; ++r)
    // {
    //     for(int c = 0; c < width; ++c) 
    //     {
    //         output.at<uchar>(r, c) = 0;
    //     }
    // }

    auto d = [](float a, float b) 
    {
        return std::abs(a - b);
    };

    auto p = [](float val, float sigma) 
    {
        // const float sigma = 5;
        const float sigmaSq = sigma * sigma;
        const float normalization = std::sqrt(2*M_PI) * sigma;
        return (1 / normalization) * std::exp(-val / (2 * sigmaSq));
    };

    for(int r = window_size / 2; r < height - window_size / 2; ++r)
    {
        for(int c = window_size / 2; c < width - window_size / 2; ++c)
        {
            float sum_w = 0;
            float sum = 0;

            for(int i = -window_size / 2; i <= window_size / 2; ++i) 
            {
                for(int j = -window_size / 2; j <= window_size / 2; ++j) 
                {
                    float range_difference = d(input.at<uchar>(r, c), input.at<uchar>(r + i, c + j));

                    float w = p(range_difference, spectral_sigma) * gaussianKernel.at<float>(i + window_size / 2, j + window_size / 2);

                    sum += input.at<uchar>(r + i, c + j) * w;
                    sum_w += w;
                }
            }
            output.at<uchar>(r, c) = sum / sum_w;
        }
    }
}

void joint_bilateral_filter(const cv::Mat& input_color, const cv::Mat& input_depth, cv::Mat& output, const int window_size = 5, const float spatial_sigma = 2.5, const float spectral_sigma = 5.0) 
{
    const auto width = input_color.cols;
    const auto height = input_color.rows;

    cv::Mat gaussianKernel = create_gaussian_kernel(window_size, spatial_sigma);
    output = cv::Mat::zeros(height, width, CV_8U);

    // TEMPORARY CODE
    // for(int r = 0; r < height; ++r)
    // {
    //     for(int c = 0; c < width; ++c)
    //     {
    //         output.at<uchar>(r, c) = 0;
    //     }
    // }

    auto d = [](float a, float b) 
    {
        return std::abs(a - b);
    };

    auto p = [](float val, float sigma) 
    {
        // const float sigma = 5;
        const float sigmaSq = sigma * sigma;
        const float normalization = std::sqrt(2*M_PI) * sigma;
        return (1 / normalization) * std::exp(-val / (2 * sigmaSq));
    };

    for(int r = window_size / 2; r < height - window_size / 2; ++r) 
    {
        for(int c = window_size / 2; c < width - window_size / 2; ++c) 
        {
            float sum_w = 0;
            float sum = 0;

            for(int i = -window_size / 2; i <= window_size / 2; ++i) 
            {
                for(int j = -window_size / 2; j <= window_size / 2; ++j) 
                {
                    float range_difference = d(input_color.at<uchar>(r, c), input_color.at<uchar>(r + i, c + j));

                    float w = p(range_difference, spectral_sigma) * gaussianKernel.at<float>(i + window_size / 2, j + window_size / 2);

                    sum += input_depth.at<uchar>(r + i, c + j) * w;
                    sum_w += w;
                }
            }
            output.at<uchar>(r, c) = sum / sum_w;
        }
    }
}

void iterative_upsampling(const cv::Mat& input_color, const cv::Mat& input_depth, cv::Mat& depth, const int window_size = 5.0, const float spatial_sigma = 2.5, const float spectral_sigma = 5.0)
{
	int uf = log2(input_color.rows / input_depth.rows); // upsample factor
    std::cout << "uf: " << uf << std::endl;

	depth = input_depth.clone();
	cv::Mat guidance = input_color.clone();

	for (int i = 0; i < uf; ++i)
	{
		cv::resize(depth, depth, depth.size() * 2);
		cv::resize(guidance, guidance, depth.size());
        cv::Mat out_depth;
		joint_bilateral_filter(guidance, depth, out_depth, window_size, spatial_sigma, spectral_sigma);
        depth = out_depth.clone();
	}
    
	cv::resize(depth, depth, input_color.size());
    cv::Mat out_depth;
	joint_bilateral_filter(input_color, depth, out_depth, window_size, spatial_sigma, spectral_sigma);
    depth = out_depth.clone();
}

// void joint_bilateral_upsampling()
// {

// }