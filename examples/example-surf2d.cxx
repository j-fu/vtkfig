#include <chrono>
#include "vtkfigFrame.h"
#include "vtkfigSurf2D.h"
#include "vtkfigMainThread.h"


inline double G(double x,double y, double t) 
{
  
  return exp(-(x*x+y*y))*sin(t+x)*cos(y-t);
}


int main(const int argc, const char *argv[])
{
  cout.sync_with_stdio(true);
  
  const int Nx = 200;
  const int Ny = 250;
  
  std::vector<double> x(Nx);
  std::vector<double> y(Ny);
  std::vector<double> z(Nx*Ny);
  
  const double x_low = -2.5;
  const double x_upp = 1.5;
  const double y_low = -2.5;
  const double y_upp = 4;
  const double dx = (x_upp-x_low)/(Nx-1);
  const double dy = (y_upp-y_low)/(Ny-1);

  auto mainthread=vtkfig::MainThread::New();
  auto frame=mainthread->AddFrame();


  

  for (int i=0; i<Nx; i++)
    x[i] = x_low+i*dx;
  
  for (int i=0; i<Ny; i++)
    y[i] = y_low + i*dy;


  double t=0;
  double dt=0.1;
  int ii=0;
  auto t0=std::chrono::system_clock::now();
  double i0=ii;

  auto surf=vtkfig::Surf2D::New();
  surf->SetGrid(x,y);

  auto colors=vtkfig::RGBTable
    { 
      {0.0, 0.0, 0.0, 1.0},
      {0.5, 0.0, 1.0, 0.0},
      {1.0, 1.0, 0.0, 0.0}
    };
  surf->SetRGBTable(colors,255);

  frame->AddFigure(surf);

  while (1)
  {
    
    for (int i=0; i<Nx; i++)
      for (int j=0; j<Ny; j++)
        z[j*Nx+i] = G(x[i],y[j],t);
    
    surf->UpdateValues(z);
    
    mainthread->Show();
    
    if (ii==3) 
      frame->Dump("example-surf2d.png");

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
