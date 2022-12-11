#pragma once

#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

#include "vtk_viewer.hpp"

#include <vtkSmartPointer.h>
#include <vtkActor.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkAxesActor.h>

#include "vtk_actor_generator.hpp"
#include "tinyfiledialogs.h"

#include "mat_viewer.hpp"

namespace GUI
{
    void initUI();
    void generateUI();
    static std::string _labelPrefix(const char* const label);
	void* runFilters();
    void* call_from_thread();
}