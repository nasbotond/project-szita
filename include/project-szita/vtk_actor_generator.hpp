#pragma once
#include <vtkActor.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPLYReader.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkSimplePointsWriter.h>
#include <vtkSimplePointsReader.h>

static vtkSmartPointer<vtkActor> getPLYActor(std::string filename, std::string out_dir, std::string trial)
{
    auto colors = vtkSmartPointer<vtkNamedColors>::New();

    auto reader = vtkSmartPointer<vtkPLYReader>::New();
    reader->SetFileName(filename.c_str());
    reader->Update();

    std::string out_filename = out_dir + trial + ".xyz";
    auto writer = vtkSmartPointer<vtkSimplePointsWriter>::New();
    writer->SetFileName(out_filename.c_str());
    writer->SetInputConnection(reader->GetOutputPort());
    writer->Update();

    auto reader_xyz = vtkSmartPointer<vtkSimplePointsReader>::New();
    reader_xyz->SetFileName(out_filename.c_str());
    reader_xyz->Update();

    // Visualize
    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(reader_xyz->GetOutputPort());

    auto actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    // actor->GetProperty()->SetPointSize(6);
    actor->GetProperty()->SetColor(colors->GetColor3d("Gold").GetData());

    // auto colors = vtkSmartPointer<vtkNamedColors>::New();

    // auto reader = vtkSmartPointer<vtkPLYReader>::New();
    // reader->SetFileName(filename.c_str());

    // // Visualize
    // auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    // mapper->SetInputConnection(reader->GetOutputPort());

    // auto actor = vtkSmartPointer<vtkActor>::New();
    // actor->SetMapper(mapper);
    // actor->GetProperty()->SetColor(colors->GetColor3d("DarkGray").GetData());

    return actor;
}