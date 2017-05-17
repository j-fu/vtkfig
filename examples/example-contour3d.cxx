#include "vtkfigFrame.h"
#include "vtkfigRectContour.h"
#include "vtkfigTools.h"


inline double G(double x,double y, double z, double t) 
{
  
  return exp(-(x*x+y*y+z*z))*sin(t+x)*cos(y-t)*sin(0.5*z-0.5*t);
}


int main(void)
{
  
  size_t nspin=vtkfig::NSpin();

  const int Nx = 40;
  const int Ny = 40;
  const int Nz = 40;
  
  std::vector<double> x(Nx);
  std::vector<double> y(Ny);
  std::vector<double> z(Nz);

  std::vector<double> v(Nx*Ny*Nz);

  
  const double x_low = -2.5;
  const double x_upp = 1.5;

  const double y_low = -2.5;
  const double y_upp = 4;

  const double z_low = -2.5;
  const double z_upp = 4;

  const double dx = (x_upp-x_low)/(Nx-1);
  const double dy = (y_upp-y_low)/(Ny-1);
  const double dz = (z_upp-z_low)/(Nz-1);

  vtkfig::Frame frame;


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

  for (int i=0; i<Nz; i++)
    z[i] = z_low + i*dz;


  double t=0;
  double dt=0.01;
  int ii=0;
  double t0=(double)clock()/(double)CLOCKS_PER_SEC;
  double i0=ii;
  vtkfig::RectContour contour;
  contour.SetContourRGBTable(colors,255);
  contour.SetGrid(x,y,z);
  frame.AddFigure(contour);

  while (ii<nspin)
  {

    for (int i=0; i<Nx; i++)
      for (int j=0; j<Ny; j++)
        for (int k=0; k<Nz; k++)
      {
        v[k*Nx*Ny+j*Nx+i] = G(x[i],y[j],z[k],t);
      }

    contour.UpdateValues(v);
    frame.Show();

    if (ii==3) 
      frame.Dump("example-contour3d.png");

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
