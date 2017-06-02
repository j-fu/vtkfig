#include "vtkSmartPointer.h"
#include "vtkOpenGLRenderWindow.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include <vtkVersion.h>

int main()
{

  std::cout << vtkVersion::GetVTKSourceVersion() << std::endl;

  
  auto window=vtkSmartPointer<vtkRenderWindow>::New();
  auto renderer=vtkSmartPointer<vtkRenderer>::New();
  window->AddRenderer(renderer);
  auto interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
  interactor->SetRenderWindow(window);
  interactor->Initialize();

  int vmaj, vmin;
  
  auto oglwin=vtkOpenGLRenderWindow::SafeDownCast(window);
  cout << "OpenGL supported: "<<  oglwin->SupportsOpenGL()  << endl;

#if VTK_MAJOR_VERSION>6 

  cout << "         Backend: " << oglwin->GetRenderingBackend() << endl;
  oglwin->GetOpenGLVersion(vmaj, vmin);
  cout << "  OpenGL version: " << vmaj << "." << vmin << endl;
//  cout << "                " << oglwin->GetOpenGLSupportMessage() << endl;


  cout << "Capabilities: " <<endl<< oglwin->ReportCapabilities() << endl;
#endif
  return 0;
}
