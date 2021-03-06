///
///  \example   example-rectcontour3d.cxx
///
///  Scalar function on 3D rectilinear grid
///

#include <chrono>
#include "vtkfigFrame.h"
#include "vtkfigDataSet.h"
#include "vtkfigScalarView.h"
#include "vtkfigTools.h"

inline double G(double x,double y, double z, double t) 
{
  
  return exp(-0.5*(x*x+y*y+z*z))*sin(t+x)*cos(y-t)*sin(0.5*z-0.5*t);
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

  for (int i=0; i<Nx; i++)
    x[i] = x_low+i*dx;
  
  for (int i=0; i<Ny; i++)
    y[i] = y_low + i*dy;

  for (int i=0; i<Nz; i++)
    z[i] = z_low + i*dz;





  auto colors=vtkfig::RGBTable
    { 
      {0.0, 0.0, 0.0, 1.0},
      {0.5, 0.0, 1.0, 0.0},
      {1.0, 1.0, 0.0, 0.0}
    };


  auto frame=vtkfig::Frame::New();
  auto dataset=vtkfig::DataSet::New();

  dataset->SetRectilinearGrid(x,y,z);
  dataset->SetPointScalar(v ,"V");

  auto  contour=vtkfig::ScalarView::New();
  contour->SetData(dataset,"V");
  contour->SetSurfaceRGBTable(colors,255);
  contour->SetValueRange(-1,1);
  frame->AddFigure(contour);



  double t=0;
  double dt=0.1;
  size_t ii=0;
  auto t0=std::chrono::system_clock::now();
  double i0=ii;

  while (ii<nspin)
  {

    for (int i=0; i<Nx; i++)
      for (int j=0; j<Ny; j++)
        for (int k=0; k<Nz; k++)
      {
        v[k*Nx*Ny+j*Nx+i] = G(x[i],y[j],z[k],t);
      }

    dataset->SetPointScalar(v ,"V");

    frame->Show();

    if (ii==3) 
      frame->WritePNG("example-rectcontour3d.png");

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
