///
///   \example   examples/example-simplexcontour3d.cxx
///
///  Scalar function on 3D simplex grid
///

#include <chrono>

#include "vtkfigFrame.h"
#include "vtkfigTools.h"
#include "vtkfigDataSet.h"
#include "vtkfigSurfaceContour.h"
#include "vtkfigBoundary.h"
#include "vtkfigTools.h"

#include <vtkMath.h>

inline double G(double x,double y, double z, double t) 
{
  
  return exp(-0.01*(x*x+y*y+z*z))*sin(t+x)*cos(y-t)*sin(0.5*z-0.5*t);
}


int main(void)
{
  size_t nspin=vtkfig::NSpin();
  std::vector<double> inpoints;

  
  for(double x = -2; x < 2; x+=0.2)
    for(double y = -2; y < 2; y+=0.2)
      for(double z = -2; z < 2; z+=0.2)
      {
        inpoints.push_back(x + vtkMath::Random(-.1, .1));
        inpoints.push_back(y + vtkMath::Random(-.1, .1));
        inpoints.push_back(z + vtkMath::Random(-.1, .1));
      }
  
  
  
  
  
  std::vector<double>points;
  std::vector<int>cells;
  
  vtkfig::Delaunay3D(inpoints,points,cells);
  

  int npoints=points.size()/3;
  std::vector<double>values(npoints);


  auto colors=vtkfig::RGBTable
    { 
      {0.0, 0.0, 0.0, 1.0},
      {0.5, 0.0, 1.0, 0.0},
      {1.0, 1.0, 0.0, 0.0}
    };

  auto frame=vtkfig::Frame::New();
  auto boundary=vtkfig::Boundary::New();

  auto griddata=vtkfig::DataSet::New();
  griddata->SetSimplexGrid(3,points,cells);
  griddata->SetPointScalar(values,"V");

  auto contour=vtkfig::SurfaceContour::New();
  contour->SetData(griddata,"V");
  contour->SetSurfaceRGBTable(colors,255);
  contour->SetValueRange(-1,1);

  boundary->SetData(griddata);

  frame->AddFigure(contour);
  frame->AddFigure(boundary);



  double t=0;
  double dt=0.1;
  size_t ii=0;
  auto t0=std::chrono::system_clock::now();
  double i0=ii;

  while (ii<nspin)
  {
    for (size_t ipoint=0, ival=0;ipoint<points.size(); ipoint+=3,ival++)
      values[ival]=G(points[ipoint+0],points[ipoint+1],points[ipoint+2],t);


    griddata->SetPointScalar(values,"V");

    frame->Show();
     if (ii==3) 
      frame->WritePNG("example-simplexcontour3d.png");
   
    t+=dt;
    auto t1=std::chrono::system_clock::now();
    double xdt=std::chrono::duration_cast<std::chrono::duration<double>>(t1-t0).count();
    double i1=ii;
    if (xdt>4.0)
    {
      printf("Frame rate: %.2f fps\n",(double)(i1-i0)/4.0);
      t0=std::chrono::system_clock::now();
      i0=ii;
    }
    ii++;

  }
}
