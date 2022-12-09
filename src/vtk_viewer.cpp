#include "vtk_viewer.hpp"

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#if defined(_MSC_VER) && _MSC_VER <= 1500 // MSVC 2008 or earlier
#include <stddef.h>
#else
#include <stdint.h>
#endif

// OpenGL Loader
#include <GL/gl3w.h> // GL3w, initialized with gl3wInit()

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

void VtkViewer::isCurrentCallbackFn(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData)
{
	bool* isCurrent = static_cast<bool*>(callData);
	*isCurrent = true;
}

void VtkViewer::processEvents()
{
	if (!ImGui::IsWindowFocused() && !ImGui::IsWindowHovered()){

		return;
	}

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigWindowsMoveFromTitleBarOnly = true; // don't drag window when clicking on image.
	ImVec2 viewportPos = ImGui::GetCursorStartPos();

	double xpos = static_cast<double>(io.MousePos[0]) - static_cast<double>(viewportPos.x);
	double ypos = static_cast<double>(io.MousePos[1]) - static_cast<double>(viewportPos.y);
	int ctrl = static_cast<int>(io.KeyCtrl);
	int shift = static_cast<int>(io.KeyShift);
	bool dclick = io.MouseDoubleClicked[0] || io.MouseDoubleClicked[1] || io.MouseDoubleClicked[2];

	interactor->SetEventInformationFlipY(xpos, ypos, ctrl, shift, dclick);

	if (ImGui::IsWindowHovered())
	{
		if (io.MouseClicked[ImGuiMouseButton_Left])
		{
			interactor->InvokeEvent(vtkCommand::LeftButtonPressEvent, nullptr);
		}
		else if (io.MouseClicked[ImGuiMouseButton_Right])
		{
			interactor->InvokeEvent(vtkCommand::RightButtonPressEvent, nullptr);
			ImGui::SetWindowFocus(); // make right-clicks bring window into focus
		}
		else if (io.MouseWheel > 0)
		{
			interactor->InvokeEvent(vtkCommand::MouseWheelForwardEvent, nullptr);
		}
		else if (io.MouseWheel < 0)
		{
			interactor->InvokeEvent(vtkCommand::MouseWheelBackwardEvent, nullptr);
		}
	}

	if (io.MouseReleased[ImGuiMouseButton_Left])
	{
		interactor->InvokeEvent(vtkCommand::LeftButtonReleaseEvent, nullptr);
	}
	else if (io.MouseReleased[ImGuiMouseButton_Right])
	{
		interactor->InvokeEvent(vtkCommand::RightButtonReleaseEvent, nullptr);
	}

	interactor->InvokeEvent(vtkCommand::MouseMoveEvent, nullptr);
}

VtkViewer::VtkViewer() 
	: viewportWidth(0), viewportHeight(0), renderWindow(nullptr), interactor(nullptr), interactorStyle(nullptr),
	renderer(nullptr), tex(0), firstRender(true)
{
		init();
}

VtkViewer::VtkViewer(const VtkViewer& vtkViewer) 
	: viewportWidth(0), viewportHeight(0), renderWindow(vtkViewer.renderWindow), interactor(vtkViewer.interactor),
	interactorStyle(vtkViewer.interactorStyle), renderer(vtkViewer.renderer), tex(vtkViewer.tex),
	firstRender(vtkViewer.firstRender) {}

VtkViewer::VtkViewer(VtkViewer&& vtkViewer) noexcept 
	: viewportWidth(0), viewportHeight(0), renderWindow(std::move(vtkViewer.renderWindow)),
	interactor(std::move(vtkViewer.interactor)), interactorStyle(std::move(vtkViewer.interactorStyle)),
	renderer(std::move(vtkViewer.renderer)), tex(vtkViewer.tex), firstRender(vtkViewer.firstRender)	{}

VtkViewer::~VtkViewer()
{
	renderer = nullptr;
	interactorStyle = nullptr;
	interactor = nullptr;
	renderWindow = nullptr;

	glDeleteTextures(1, &tex);
}

VtkViewer& VtkViewer::operator=(const VtkViewer& vtkViewer)
{
	viewportWidth = vtkViewer.viewportWidth;
	viewportHeight = vtkViewer.viewportHeight;
	renderWindow = vtkViewer.renderWindow;
	interactor = vtkViewer.interactor;
	interactorStyle = vtkViewer.interactorStyle;
	renderer = vtkViewer.renderer;
	tex = vtkViewer.tex;
	firstRender = vtkViewer.firstRender;
	return *this;
}

void VtkViewer::init()
{
	renderer = vtkSmartPointer<vtkRenderer>::New();
	renderer->ResetCamera();
	renderer->SetBackground(DEFAULT_BACKGROUND);
	renderer->SetBackgroundAlpha(DEFAULT_ALPHA);
	// renderer->GetActiveCamera()->SetPosition(1, -5, 1);
    // renderer->GetActiveCamera()->SetFocalPoint(0, 0, 0);
    // renderer->GetActiveCamera()->SetViewUp(0, 0, 1);  

	interactorStyle = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
	interactorStyle->SetDefaultRenderer(renderer);

	interactor = vtkSmartPointer<vtkGenericRenderWindowInteractor>::New();
	interactor->SetInteractorStyle(interactorStyle);
	interactor->EnableRenderOff();

	int viewportSize[2] = {static_cast<int>(viewportWidth), static_cast<int>(viewportHeight)};

	renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
	renderWindow->SetSize(viewportSize);

	vtkSmartPointer<vtkCallbackCommand> isCurrentCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	isCurrentCallback->SetCallback(&isCurrentCallbackFn);
	renderWindow->AddObserver(vtkCommand::WindowIsCurrentEvent, isCurrentCallback);

	renderWindow->SwapBuffersOn();

	renderWindow->SetOffScreenRendering(true);
	renderWindow->SetFrameBlitModeToNoBlit();

	renderWindow->AddRenderer(renderer);
	renderWindow->SetInteractor(interactor);

	if(!renderer || !interactorStyle || !renderWindow || !interactor)
	{
		throw VtkViewerError("Couldn't initialize VtkViewer");
	}
}

void VtkViewer::render()
{
	render(ImGui::GetContentRegionAvail());
}
void VtkViewer::render(const ImVec2 size)
{
	setViewportSize(size);

	renderWindow->Render();
	renderWindow->WaitForCompletion();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
	ImGui::BeginChild("##Viewport", size, true, VtkViewer::NoScrollFlags());
	ImGui::Image(reinterpret_cast<void*>(tex), ImGui::GetContentRegionAvail(), ImVec2(0, 1), ImVec2(1, 0));
	processEvents();
	ImGui::EndChild();
	ImGui::PopStyleVar();
}

void VtkViewer::addActor(const vtkSmartPointer<vtkProp>& actor)
{
	renderer->AddActor(actor);
}

void VtkViewer::addActors(const vtkSmartPointer<vtkPropCollection>& actors)
{
	actors->InitTraversal();
	vtkProp* actor;
	vtkCollectionSimpleIterator sit;
	for (actors->InitTraversal(sit); (actor = actors->GetNextProp(sit));)
	{
		renderer->AddActor(actor);
	}
}

void VtkViewer::removeActor(const vtkSmartPointer<vtkProp>& actor)
{
	renderer->RemoveActor(actor);
}

vtkSmartPointer<vtkTransform> VtkViewer::getNewTransform(const std::vector<double>& vector)
{
	// Generate a random start and end point
    double start_point[3] = {0, 0, 0};
    // double end_point[3] = {0.0, 0.0, 1.0};
    float x = -sin(vector.at(1));
    float y = cos(vector.at(1))*sin(vector.at(0));
    // Negate z to make arrow point down (towards the earth)
    float z = cos(vector.at(1))*cos(vector.at(0));

    double end_point[3] = {x, y, z};
    vtkNew<vtkMinimalStandardRandomSequence> rng;

    // Compute a basis
    double normalized_x[3];
    double normalized_y[3];
    double normalized_z[3];

    // // The X axis is a vector from start to end
    vtkMath::Subtract(end_point, start_point, normalized_x);
    double length = vtkMath::Norm(normalized_x);
    vtkMath::Normalize(normalized_x);

    // The Z axis is an arbitrary vector cross X
    double arbitrary[3];
    for (auto i = 0; i < 3; ++i)
    {
        rng->Next();
        arbitrary[i] = rng->GetRangeValue(-10, 10);
    }
    vtkMath::Cross(normalized_x, arbitrary, normalized_z);
    vtkMath::Normalize(normalized_z);

    // The Y axis is Z cross X
    vtkMath::Cross(normalized_z, normalized_x, normalized_y);
    vtkNew<vtkMatrix4x4> matrix;

    // Create the direction cosine matrix
    matrix->Identity();
    for (auto i = 0; i < 3; i++)
    {
        matrix->SetElement(i, 0, normalized_x[i]);
        matrix->SetElement(i, 1, normalized_y[i]);
        matrix->SetElement(i, 2, normalized_z[i]);
    }

    // Apply the transforms
    auto transform = vtkSmartPointer<vtkTransform>::New();
    // vtkNew<vtkTransform> transform;
    transform->Translate(start_point);
    transform->Concatenate(matrix);
    transform->Scale(length, length, length);

	return transform;
}

vtkSmartPointer<vtkPlaneSource> VtkViewer::getNewPlaneSource(const std::vector<double>& vector)
{
	vtkSmartPointer<vtkPlaneSource> plane_source = vtkSmartPointer<vtkPlaneSource>::New();
	plane_source->SetOrigin(0.0, 0.0, 0.0);
	plane_source->SetPoint1(0.5*cos(vector.at(1))*cos(vector.at(2)), 0.5*(cos(vector.at(2))*sin(vector.at(1))*sin(vector.at(0)) - cos(vector.at(0))*sin(vector.at(2))), 0.5*(cos(vector.at(0))*cos(vector.at(2))*sin(vector.at(1)) + sin(vector.at(0))*sin(vector.at(2))));
	plane_source->SetPoint2(0.25*cos(vector.at(1))*sin(vector.at(2)), 0.25*(cos(vector.at(0))*cos(vector.at(2)) + sin(vector.at(1))*sin(vector.at(0))*sin(vector.at(2))), 0.25*(cos(vector.at(0))*sin(vector.at(1))*sin(vector.at(2)) - cos(vector.at(2))*sin(vector.at(0))));
	plane_source->SetCenter(0.0, 0.0, 0.0);
	plane_source->Update();
	return plane_source;
}

void VtkViewer::updateActors(const vtkSmartPointer<vtkActorCollection>& actors, const std::vector<double>& vector)
{
	actors->InitTraversal();
	vtkActor* actor;
	vtkCollectionSimpleIterator sit;
	for (actors->InitTraversal(sit); (actor = actors->GetNextActor(sit));)
	{
		// Determine if arrow or plane by color
		double* pro = actor->GetProperty()->GetColor();
		double color = *pro;

		if(color == 1)
		{
			actor->SetUserMatrix(getNewTransform(vector)->GetMatrix());
		}
		else
		{
			actor->GetMapper()->SetInputDataObject(getNewPlaneSource(vector)->GetOutput());
		}		
	}
}

void VtkViewer::setViewportSize(const ImVec2 newSize)
{
	if (((viewportWidth == newSize.x && viewportHeight == newSize.y) || viewportWidth <= 0 || viewportHeight <= 0) && !firstRender)
	{
		return;
	}

	viewportWidth = static_cast<unsigned int>(newSize.x);
	viewportHeight = static_cast<unsigned int>(newSize.y);

	int viewportSize[] = {static_cast<int>(newSize.x), static_cast<int>(newSize.y)};

	// Free old buffers
	glDeleteTextures(1, &tex);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewportWidth, viewportHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	glBindTexture(GL_TEXTURE_2D, 0);

	renderWindow->InitializeFromCurrentContext();
	renderWindow->SetSize(viewportSize);
	interactor->SetSize(viewportSize);

	auto vtkfbo = renderWindow->GetDisplayFramebuffer();
	vtkfbo->Bind();
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
	vtkfbo->UnBind();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	firstRender = false;
}
