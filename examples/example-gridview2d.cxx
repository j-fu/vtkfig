/**
\example example-gridview2d.cxx
2D Grid visualization  with vtkfig::GridView
*/

#include <chrono>
#include <vtkMath.h>
#include "vtkfigFrame.h"
#include "vtkfigDataSet.h"
#include "vtkfigGridView.h"
#include "vtkfigXYPlot.h"
#include "vtkfigTools.h"

int main(void)
{
  cout.sync_with_stdio(true);

  const int Nx = 20;
  const int Ny = 25;
  
  std::vector<double> x(Nx);
  std::vector<double> y(Ny);
  
  const double x_low = -2.5;
  const double x_upp = 1.5;
  const double y_low = -2.5;
  const double y_upp = 4;
  const double dx = (x_upp-x_low)/(Nx-1);
  const double dy = (y_upp-y_low)/(Ny-1);


  for (int i=0; i<Nx; i++)
    x[i] = x_low+i*dx;
  
  for (int i=0; i<Ny; i++)
    y[i] = y_low + i*dy;


  std::vector<double> inpoints;



  for(double x = -2; x < 2; x+=0.2)
  {
    for(double y = -2; y < 2; y+=0.2)
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

  std::shared_ptr<vtkfig::Frame> frame=vtkfig::Frame::New();
  std::shared_ptr<vtkfig::DataSet> ugriddata=vtkfig::DataSet::New();


  frame->SetSize(800,400);

  ugriddata->SetSimplexGrid(2,points,cells);


  std::shared_ptr<vtkfig::DataSet> rgriddata=vtkfig::DataSet::New();
  rgriddata->SetRectilinearGrid(x,y);
  std::shared_ptr<vtkfig::GridView> rgview=vtkfig::GridView::New();
  rgview->SetData(rgriddata);
  std::shared_ptr<vtkfig::GridView> ugview=vtkfig::GridView::New();
  ugview->SetData(ugriddata);
  ugview->SetTitle("Unstructured Grid");
  rgview->SetTitle("Rectilinear Grid");
  frame->AddFigure(rgview,0);
  frame->AddFigure(ugview,1);
  frame->Interact();
}
