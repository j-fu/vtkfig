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

  // Generate a 10 x 10 grid of points
  for(double x = 0; x < 1; x+=0.02)
  {
    for(double y = 0; y < 1; y+=0.02)
    {
      inpoints.push_back(x + vtkMath::Random(-.025, .025));
      inpoints.push_back(y + vtkMath::Random(-.025, .025));
    }
  }
  



  std::vector<double>points;
  std::vector<int>cells;
  
  vtkfig::Delaunay2D(inpoints,points,cells);

  std::vector<double>values;
  for (int ipoint=0;ipoint<points.size(); ipoint+=2)
    values.push_back( G(points[ipoint+0],points[ipoint+1],5.0));

  auto frame=vtkfig::Frame();
  auto colors=vtkfig::RGBTable
    { 
      {0.0, 0.0, 0.0, 1.0},
      {0.5, 0.0, 1.0, 0.0},
      {1.0, 1.0, 0.0, 0.0}
    };
  auto contour=vtkfig::TriContour2D();
  contour.SetSurfaceRGBTable(colors,255);
  contour.Add(points,cells,values);
  frame.Clear();
  frame.Add(contour);
  frame.Show();
  frame.Interact();

}
