#pragma once
#include <vtkActor.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPLYReader.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkImageActor.h>
#include <vtkJPEGReader.h>
#include <vtkPNGReader.h>

static vtkSmartPointer<vtkActor> getPLYActor(std::string filename)
{
    auto colors = vtkSmartPointer<vtkNamedColors>::New();

    auto reader = vtkSmartPointer<vtkPLYReader>::New();
    reader->SetFileName(filename.c_str());

    // Visualize
    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(reader->GetOutputPort());

    auto actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    // actor->GetProperty()->SetColor(colors->GetColor3d("DarkGray").GetData());

    return actor;
}

static vtkSmartPointer<vtkActor> getImageActor(std::string filename)
{
    auto colors = vtkSmartPointer<vtkNamedColors>::New();

    auto reader = vtkSmartPointer<vtkPNGReader>::New();
    reader->SetFileName(filename.c_str());

    // Visualize
    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(reader->GetOutputPort());

    auto actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    // actor->GetProperty()->SetColor(colors->GetColor3d("DarkGray").GetData());

    return actor;
}