///
///   \example   examples/example-custom.cxx
///
///   Custom vtk pipeline with vtkfig::Figure
///

#include <chrono>
#include <vtkContext2D.h>
#include <vtkContext3D.h>
#include <vtkContextItem.h>
#include <vtkContextActor.h>
#include <vtkContextScene.h>
#include <vtkBrush.h>

#include "vtkfigFrame.h"
#include "vtkfigFigure.h"
#include "vtkfigTools.h"



class MyDrawing : public vtkContextItem
{
public:
  static MyDrawing *New();
  vtkTypeMacro(MyDrawing, vtkContextItem);
  double delta=0.0;
  virtual bool Paint(vtkContext2D* painter)
  {
    painter->GetBrush()->SetColor(255, 0, 0);
    painter->DrawRect(20+delta,20+delta,40,40);
    auto p3d=painter->GetContext3D();
    p3d->DrawLine(vtkVector3f{0,0,0},vtkVector3f{delta,delta,delta} );
    return true;
  }
};

vtkStandardNewMacro(MyDrawing);
int main(void)
{

  cout <<
R"(
Example showing creation of custom scenes using 
vtkfigFrame and vtkfigFigure base class.
)";

  size_t nspin=vtkfig::NSpin();

  cout.sync_with_stdio(true);
  

  auto frame=vtkfig::Frame::New();
  auto figure=vtkfig::Figure::New();
  figure->SetViewVolume(0,1,0,1,0,1);
  
  auto cactor=vtkContextActor::New();
  auto drawing=MyDrawing::New();
  cactor->GetScene()->AddItem(drawing);
  figure->RTAddContextActor(cactor);
  frame->AddFigure(figure);


  auto t0=std::chrono::system_clock::now();

  
  
  int ii=0;
  int i0=0;
  while (ii <nspin)
  {
    auto t1=std::chrono::system_clock::now();
    double dt=std::chrono::duration_cast<std::chrono::duration<double>>(t1-t0).count();
    int i1=ii;
    drawing->delta+=0.5;
    frame->Show();
    if (dt>4.0)
    {
      printf("Frame rate: %.2f fps\n",(double)(i1-i0)/4.0);
      t0=std::chrono::system_clock::now();
      i0=ii;
      fflush(stdout);
    }
    ii++;
  }

}
