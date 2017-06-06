///
///   \example   examples/example-simplexcontour2d.cxx
///
///  Scalar function on 2D simplex grid
///

#include <chrono>

#include "vtkfigFrame.h"
#include "vtkfigTools.h"
#include "vtkfigDataSet.h"
#include "vtkfigSurfaceContour.h"
#include "vtkfigTools.h"

#include <vtkMath.h>
double scalefac=1.0;

inline double G(double x,double y, double t) 
{
  x/=scalefac;
  y/=scalefac;
  return exp(-(x*x+y*y))*sin(t+3.*x)*cos(4.*y-t);
}


int main(void)
{
  size_t nspin=vtkfig::NSpin();

  std::vector<double> inpoints;
  for(double x = -2; x < 2; x+=0.03)
  {
    for(double y = -2; y < 2; y+=0.03)
    {
      inpoints.push_back(x + vtkMath::Random(-.1, .1));
      inpoints.push_back(y + vtkMath::Random(-.1, .1));
    }
  }
  



  std::vector<double>points;
  std::vector<int>cells;
  
  vtkfig::Delaunay2D(inpoints,points,cells);


  int npoints=points.size()/2;
  std::vector<double>values(npoints);


  auto colors=vtkfig::RGBTable
    { 
      {0.0, 0.0, 0.0, 1.0},
      {0.5, 0.0, 1.0, 0.0},
      {1.0, 1.0, 0.0, 0.0}
    };

  double t=0;
  double dt=0.1;
  size_t ii=0;
  auto t0=std::chrono::system_clock::now();
  double i0=ii;

  for(auto &x: points) x*=scalefac;
  auto griddata=vtkfig::DataSet::New();
  griddata->SetSimplexGrid(2,points,cells);

  auto frame=vtkfig::Frame::New();
  auto contour=vtkfig::SurfaceContour::New();
  contour->SetData(griddata,"v");
  contour->SetSurfaceRGBTable(colors,255);
  contour->SetValueRange(-1,1);
  frame->AddFigure(contour);



  while (ii<nspin)
  {
    for (size_t ipoint=0, ival=0;ipoint<points.size(); ipoint+=2,ival++)
      values[ival]=G(points[ipoint+0],points[ipoint+1],t);

    griddata->SetPointScalar(values,"v");

    frame->Show();
     if (ii==3) 
      frame->WritePNG("example-simplexcontour2d.png");
   
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
