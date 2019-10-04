/**
\example example-gridview3d.cxx
3D Grid visualization with vtkfig::GridView
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
  const int Nz = 25;
  
  std::vector<double> x(Nx);
  std::vector<double> y(Ny);
  std::vector<double> z(Nz);
  
  const double x_low = -2.5;
  const double x_upp = 1.5;
  const double y_low = -2.5;
  const double y_upp = 4;
  const double z_low = -2.5;
  const double z_upp = 3;
  const double dx = (x_upp-x_low)/(Nx-1);
  const double dy = (y_upp-y_low)/(Ny-1);
  const double dz = (z_upp-z_low)/(Nz-1);


  for (int i=0; i<Nx; i++)
    x[i] = x_low+i*dx;
  
  for (int i=0; i<Ny; i++)
    y[i] = y_low + i*dy;

  for (int i=0; i<Nz; i++)
    z[i] = z_low + i*dz;


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
  auto ugriddata=vtkfig::DataSet::New();

  frame->SetSize(800,400);

  ugriddata->SetSimplexGrid(3,points,cells);

  auto rgriddata=vtkfig::DataSet::New();
  rgriddata->SetRectilinearGrid(x,y,z);
  auto rgview=vtkfig::GridView::New();
  rgview->SetData(rgriddata);
  auto ugview=vtkfig::GridView::New();
  ugview->SetData(ugriddata);
  ugview->SetTitle("Unstructured Grid");
  rgview->SetTitle("Rectilinear Grid");
  frame->AddFigure(rgview,0);
  frame->AddFigure(ugview,1);
  frame->Interact();
}
