#include "vtkfigFrame.h"
#include "vtkfigSurf2D.h"


inline double G(double x,double y, double t) 
{
  
  return exp(-(x*x+y*y))*sin(t+x)*cos(y-t);
}


int main(void)
{
  
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

  auto frame=vtkfig::Frame();
  frame.SetInteractorStyle(vtkfig::Frame::InteractorStyle::Volumetric);

  

  for (int i=0; i<Nx; i++)
    x[i] = x_low+i*dx;
  
  for (int i=0; i<Ny; i++)
    y[i] = y_low + i*dy;


  double t=0;
  double dt=0.1;
  int ii=0;
  double t0=(double)clock()/(double)CLOCKS_PER_SEC;
  double i0=ii;
  while (1)
  {
  auto surf=vtkfig::Surf2D();

  for (int i=0; i<Nx; i++)
    for (int j=0; j<Ny; j++)
      z[j*Nx+i] = G(x[i],y[j],t);
  

    surf.Add(x,y,z);
    frame.Clear();
    frame.Add(surf);
    frame.Show();

    if (ii==3) 
      frame.Dump("example-surf2d.png");

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
