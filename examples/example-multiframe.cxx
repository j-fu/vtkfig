// This example does not work
// as vtk is not thread safe.
//
// we need to modify the architecture:
// vtkfig::Thread() instead of vtkfig::ServerConnection
// vtkfigFrame(thread)


#include <chrono>
// For Multithreading with X11
//extern "C" void XInitThreads(void); 


#include "vtkfigFrame.h"
#include "vtkfigSurf2D.h"
#include "vtkfigXYPlot.h"


inline double G(double x,double y, double t) 
{
  
  return exp(-(x*x+y*y))*sin(t+x)*cos(y-t);
}


int main(void)
{

  cout.sync_with_stdio(true);

  const int Nx = 200;
  const int Ny = 250;
  
  std::vector<double> x(Nx);
  std::vector<double> y(Ny);
  std::vector<double> z(Nx*Ny);
  
  std::vector<double> fx(Nx);
  std::vector<double> fy(Ny);
  
  const double x_low = -2.5;
  const double x_upp = 1.5;
  const double y_low = -2.5;
  const double y_upp = 4;
  const double dx = (x_upp-x_low)/(Nx-1);
  const double dy = (y_upp-y_low)/(Ny-1);

  auto colors=vtkfig::RGBTable
    { 
      {0.0, 0.0, 0.0, 1.0},
      {0.5, 0.0, 1.0, 0.0},
      {1.0, 1.0, 0.0, 0.0}
    };

  for (int i=0; i<Nx; i++)
    x[i] = x_low+i*dx;
  
  for (int i=0; i<Ny; i++)
    y[i] = y_low + i*dy;

  double t=0;
  double dt=0.1;
  int ii=0;
  auto t0=std::chrono::system_clock::now();
  double i0=ii;


  auto frame1=vtkfig::Frame::New();
  auto frame2=vtkfig::Frame::New();


  frame1->Size(400,400);
  frame2->Size(400,400);
  frame2->Position(500,0);
  auto surf=vtkfig::Surf2D::New();
  surf->SetRGBTable(colors,255);
  surf->SetGrid(x,y);
  frame1->AddFigure(surf);
  auto xyplot=vtkfig::XYPlot::New();
  frame2->AddFigure(xyplot);

  while (1)
  {
    for (int i=0; i<Nx; i++)
      for (int j=0; j<Ny; j++)
      {
        double f=G(x[i],y[j],t);
        z[j*Nx+i] = f;
        if (i==Nx/2) fy[j]=f;
        if (j==Ny/2) fx[i]=f;
      }

    surf->UpdateValues(z);
    xyplot->Clear();
    xyplot->LineColorRGB(0,0,1);
    xyplot->LineType("-");
    xyplot->AddPlot(x, fx);
    xyplot->LineColorRGB(1,0,0);
    xyplot->LineType("-");
    xyplot->AddPlot(y, fy);
    frame1->Show();
   

    if (ii==3) 
    {
      frame1->Dump("example-multiframe1.png");
      frame2->Dump("example-multiframe2.png");
    }
    t+=dt;
    auto t1=std::chrono::system_clock::now();
    double dt=std::chrono::duration_cast<std::chrono::duration<double>>(t1-t0).count();
    double i1=ii;
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
