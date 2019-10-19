///
///   \example  example-scene2d.cxx
///

#include "vtkfigFrame.h"
#include "vtkfigGLScene.h"
#include "vtkfigTools.h"



int main(void)
{
  auto scene=vtkfig::GLScene::New();
  scene->SetViewVolume(-1,1,-1,1,-1,1);
  auto frame=vtkfig::Frame::New();
  frame->AddFigure(scene);

  for (double t=0;;t+=0.01)
  {
    scene->Clear();
    scene->BeginLines();
    for (double x=0;x<3.0; x+=0.01)
    {
      double xt=sin(t-x)*sin(t-x);
      scene->Color(xt,0.0,1.0-xt);
      scene->Vertex2(cos(3.0*(x-t)),sin(5.0*(x-t)));
    }
    scene->End();
    frame->Show();
  }
  frame->Interact();
}


