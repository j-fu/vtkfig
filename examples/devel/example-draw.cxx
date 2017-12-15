// After https://www.vtk.org/Wiki/VTK/Examples/Cxx/Images/DrawOnAnImage

#include <vtkSmartPointer.h>
 
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkJPEGReader.h>
#include <vtkImageData.h>
#include <vtkImageMapper3D.h>
#include <vtkImageViewer2.h>
#include <vtkImageCanvasSource2D.h>
#include <vtkImageBlend.h>
 
int main ( int argc, char* argv[] )
{
 

  // Draw a circle in the center of the image
  vtkSmartPointer<vtkImageCanvasSource2D> drawing = 
    vtkSmartPointer<vtkImageCanvasSource2D>::New();
  drawing->SetNumberOfScalarComponents(3);
  drawing->SetScalarTypeToUnsignedChar();
  drawing->SetExtent(0,1000,0,1000,0,1000);
  drawing->SetDrawColor(0.0, 0.0, 0.0);
  drawing->FillBox(0,1000,0,1000);
  drawing->SetDrawColor(255.0, 255.0, 255.0);
  drawing->DrawCircle(500,500,100);
 
  
  // Display the result
  vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = 
    vtkSmartPointer<vtkRenderWindowInteractor>::New();
  
  vtkSmartPointer<vtkImageViewer2> imageViewer = 
    vtkSmartPointer<vtkImageViewer2>::New();
  imageViewer->SetInputConnection(drawing->GetOutputPort());
  imageViewer->SetSize(640, 512);
  imageViewer->SetupInteractor(renderWindowInteractor);
  imageViewer->GetRenderer()->ResetCamera();
  imageViewer->GetRenderer()->SetBackground(1,0,0); //red
 
  renderWindowInteractor->Initialize();
  renderWindowInteractor->Start();
 
  return EXIT_SUCCESS;
}
