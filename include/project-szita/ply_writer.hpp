#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <string> 
#include <fstream>
#include <sstream>
#include <vector>

namespace PLYWriter
{
    // estimate normal vectors for oriented point cloud
    static void getNormalVectors(cv::Mat& points, cv::Mat& normals, const int& window_size)
    {
        int hw = window_size/2; // window size of 5, hw is half window size

        // for each point
        for(int i = hw; i < points.rows - hw; ++i)
        {
            for(int j = hw; j < points.cols - hw; ++j)
            {
                // get window of points
                std::vector<cv::Point3f> pts;
                for(int a = -hw; a <= hw; ++a)
                {
                    for(int b = -hw; b <= hw; ++b)
                    {
                    cv::Point3f point;
                    point.x = static_cast<float>(points.at<cv::Vec3f>(i+a, j+b)[0]);
                    point.y = static_cast<float>(points.at<cv::Vec3f>(i+a, j+b)[1]);
                    point.z = static_cast<float>(points.at<cv::Vec3f>(i+a, j+b)[2]);

                    pts.push_back(point);
                    }
                }
                
                int num = pts.size();

                // find center of gravity
                cv::Point3d t(0.0, 0.0, 0.0);

                for (int idx = 0; idx < num; idx++)
                {
                    t.x += pts.at(idx).x;
                    t.y += pts.at(idx).y;
                    t.z += pts.at(idx).z;
                }

                t.x = t.x / num;
                t.y = t.y / num;
                t.z = t.z / num;

                // X*l = 0 (homogenous equation)
                cv::Mat X(num, 3, CV_32F);

                // get matrix X (plane matrix)
                for (int idx = 0; idx < num; idx++)
                {
                    cv::Point3d pt = pts.at(idx);
                    X.at<float>(idx, 0) = pt.x - t.x;
                    X.at<float>(idx, 1) = pt.y - t.y;
                    X.at<float>(idx, 2) = pt.z - t.z;
                }

                // normal vector l -> eigenvector of X^T X corresponding to the least eigenvalues
                cv::Mat mtx = X.t() * X;
                cv::Mat evals, evecs;

                cv::eigen(mtx, evals, evecs);

                float nx = evecs.at<float>(2, 0);
                float ny = evecs.at<float>(2, 1);
                float nz = evecs.at<float>(2, 2);

                // check if we need to flip any plane normals towards viewpoint
                float vp_x = 0 - static_cast<float>(points.at<cv::Vec3f>(i, j)[0]);
                float vp_y = 0 - static_cast<float>(points.at<cv::Vec3f>(i, j)[1]);
                float vp_z = 0 - static_cast<float>(points.at<cv::Vec3f>(i, j)[2]);

                // dot product between the (viewpoint - point) and the plane normal
                float cos_theta = (vp_x * nx + vp_y * ny + vp_z * nz);

                normals.at<cv::Vec3f>(i, j) = cos_theta < 0 ? cv::Vec3f(-nx, -ny, -nz) : cv::Vec3f(nx, ny, nz);
            }
        }
    }

    // create and write .ply file for point clouds
    static void writePLY(const std::string& output_file, cv::Mat points, cv::Mat normals, cv::Mat colors)
    {
        int rows = points.rows;
        int cols = points.cols;

        // int triangleSize = 3; // size of triangles for triangulated surface

        std::stringstream out3d;
        out3d << output_file << ".ply";
        std::ofstream outfile(out3d.str());
        
        // header
        outfile <<"ply\n";
        outfile <<"format ascii 1.0\n";
        outfile <<"element vertex "<< rows*cols <<std::endl;
        outfile <<"property float x\n";
        outfile <<"property float y\n";
        outfile <<"property float z\n";
        outfile <<"property float nx\n";
        outfile <<"property float ny\n";
        outfile <<"property float nz\n";
        outfile <<"property uchar red\n";
        outfile <<"property uchar green\n";
        outfile <<"property uchar blue\n";
        // outfile <<"element face " << 2*((rows-1)/triangleSize)*((cols-1)/triangleSize) << std::endl;
        outfile <<"element face " << 0 << std::endl;
        outfile <<"property list uchar int vertex_index\n";
        outfile <<"end_header\n";
        
        // write point vertices, normals, and colors
        for (int r = 0; r < rows; r++)
        {
            for(int c = 0; c < cols; c++)
            {
                cv::Vec3f point = points.at<cv::Vec3f>(r, c);
                cv::Vec3f normal = normals.at<cv::Vec3f>(r, c);
                cv::Vec3b color = colors.at<cv::Vec3b>(r, c);

                outfile << point.val[0] << " " << point.val[1] << " " << point.val[2] << " " << normal.val[0] << " " << normal.val[1] << " " << normal.val[2] << " " << (int)color.val[0] << " " << (int)color.val[1] << " " << (int)color.val[2] <<std::endl;        
            }
        }

        // // determine and write faces
        // for (int r = triangleSize; r <= triangleSize*((rows-1)/triangleSize); r=r+triangleSize)
        // {
        //     for(int c = 0; c < triangleSize*((cols-1)/triangleSize); c=c+triangleSize)
        //     {
        //         outfile << "3 " << r*cols+c << " " << (r*cols+c)-triangleSize*cols << " " << (r*cols+c)-triangleSize*cols+triangleSize << std::endl;
        //         outfile << "3 " << r*cols+c << " " << (r*cols+c)-triangleSize*cols+triangleSize << " " << (r*cols+c)+triangleSize << std::endl; 
        //     }
        // }

        outfile.close();
    }

    // calculate 3D point from disparity map
    static void Disparity2PointCloud(
        const std::string& output_file,
        int height, int width, cv::Mat& disparities,
        const int& window_size,
        const int& dmin, const double& baseline, const double& focal_length, cv::Mat& image_color)
    {
        cv::Mat pointsMat = cv::Mat(height - window_size, width - window_size, CV_32FC3, cv::Scalar(0.,0.,0.));
        cv::Mat colorsMat = cv::Mat(height - window_size, width - window_size, CV_8UC3, cv::Scalar(0,0,0));
        cv::Mat normalsMat = cv::Mat(height - window_size, width - window_size, CV_32FC3, cv::Scalar(0.,0.,0.));

        for (int i = 0; i < height - window_size; ++i)
        {
            std::cout << "Reconstructing 3D point cloud from disparities... " << std::ceil(((i) / static_cast<double>(height - window_size + 1)) * 100) << "%\r" << std::flush;
            for (int j = 0; j < width - window_size; ++j)
            {
                if(disparities.at<uchar>(i, j) + dmin == 0) continue;

                const double u1 = j-(width/2);
                const double d = static_cast<double>(disparities.at<uchar>(i, j)) + dmin;
                const double u2 = u1-d; // u1+d
                const double v = i-(height/2);

                const double Z = (baseline*focal_length)/d;
                const double X = -1*(baseline*(u1+u2))/(2*d);
                const double Y = baseline*(v)/d;

                pointsMat.at<cv::Vec3f>(i, j) = cv::Vec3f(-X, Y, Z);

                // set color of point
                cv::Vec3b color = image_color.at<cv::Vec3b>(i, j);
                colorsMat.at<cv::Vec3b>(i, j) = cv::Vec3b(color.val[2], color.val[1], color.val[0]);
            }
        }

        std::cout << "Reconstructing 3D point cloud from disparities... Done.\r" << std::flush;
        std::cout << std::endl;

        // estimate normal vectors
        getNormalVectors(pointsMat, normalsMat, 5);

        // write 3D point cloud file
        writePLY(output_file, pointsMat, normalsMat, colorsMat);
    }
}