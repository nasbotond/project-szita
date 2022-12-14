# project-szita

## Description
This project is an implementation of the bilateral filter, joint bilateral filter and two upsampling algorithms: joint bilateral upsampling and iterative upsampling. Using the upsampled disparity maps, the point clouds are reconstructed including an oriented point cloud. Twelve different color and depth image pairs were used. The project is accompanyed by a GUI for visualizing the filtered images and point clouds.

The code for this project is written in C++ and utilizes OpenCV for image manipulation and visualization and VTK for build-in point cloud visualization. The GUI is written using OpenGL, GLFW and ImGui. The solution is built using CMake, allowing easy cross-platform use.

## Requirements
- Working C++ compiler (C++ 17 or greater)
- CMake (version >= 2.8)
- OpenCV (version >= 4.0.0)

## Usage
- `./filter`
