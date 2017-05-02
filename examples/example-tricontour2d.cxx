#include "vtkfigFrame.h"
#include "vtkfigTools.h"
#include "vtkfigTriContour2D.h"




#include <vtkMath.h>

inline double G(double x,double y, double t) 
{
  
  return exp(-(x*x+y*y))*sin(t+3.*x)*cos(4.*y-t);
}


int main(void)
{
  
  std::vector<double> inpoints;

  for(double x = 0; x < 1; x+=0.01)
  {
    for(double y = 0; y < 1; y+=0.01)
    {
      inpoints.push_back(x + vtkMath::Random(-.025, .025));
      inpoints.push_back(y + vtkMath::Random(-.025, .025));
    }
  }
  



  std::vector<double>points;
  std::vector<int>cells;
  
  vtkfig::Delaunay2D(inpoints,points,cells);


  int npoints=points.size()/2;
  std::vector<double>values(npoints);

  auto frame=vtkfig::Frame::New();
  auto colors=vtkfig::RGBTable
    { 
      {0.0, 0.0, 0.0, 1.0},
      {0.5, 0.0, 1.0, 0.0},
      {1.0, 1.0, 0.0, 0.0}
    };

  double t=0;
  double dt=0.1;
  int ii=0;
  double t0=(double)clock()/(double)CLOCKS_PER_SEC;
  double i0=ii;

  auto contour=vtkfig::TriContour2D::New();
  contour->SetSurfaceRGBTable(colors,255);
  contour->SetGrid(points,cells);
  frame->AddFigure(contour);

  while (1)
  {
    for (int ipoint=0, ival=0;ipoint<points.size(); ipoint+=2,ival++)
      values[ival]=G(points[ipoint+0],points[ipoint+1],t);

    contour->UpdateValues(values);
    frame->Show();
     if (ii==3) 
      frame->Dump("example-tricontour2d.png");
   
    t+=dt;
    double t1=(double)clock()/(double)CLOCKS_PER_SEC;
    double i1=ii;
    if (t1-t0>4.0)
    {
      printf("Frame rate: %.2f fps\n",(double)(i1-i0)/4.0);
      t0=(double)clock()/(double)CLOCKS_PER_SEC;
      i0=ii;
    }
    ii++;

  }
}
