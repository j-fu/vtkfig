///
///   \example   examples/example-rectquiver2d.cxx
///
///  Vector function on 2D rectilinear grid
///

#include "vtkfigFrame.h"
#include "vtkfigSurfaceContour.h"
#include "vtkfigQuiver.h"
#include "vtkfigTools.h"

inline double G(double x,double y, double z, double t) 
{
  
  return exp(-(x*x+y*y+z*z))*sin(t+x)*cos(y-t)*cos(z-2*t);
}

inline double dGdx(double x,double y, double z, double t) 
{
  return cos(y-t)*cos(z-2*t)*exp(-(x*x+y*y+z*z))*(cos(t+x)-2*x*sin(t+x));
}

inline double dGdy(double x,double y, double z, double t) 
{
  return sin(t+x)*cos(z-2*t)*exp(-(x*x+y*y+z*z))*(-2*y*cos(y-t) -sin(y-t));
}

inline double dGdz(double x,double y, double z, double t) 
{
  return sin(t+x)*cos(y-t)*exp(-(x*x+y*y+z*z))*(-2*z*cos(z-2*t) -sin(z-2*t));
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


  std::vector<double> u(Nx*Ny*Nz);
  std::vector<double> v(Nx*Ny*Nz);
  std::vector<double> w(Nx*Ny*Nz);
  std::vector<double> val(Nx*Ny*Nz);





  auto colors=vtkfig::RGBTable
    { 
      {0.0, 0.3, 0.3, 1.0},
      {0.5, 0.3, 1.0, 0.3},
      {1.0, 1.0, 0.3, 0.3}
    };
  auto qcolors=vtkfig::RGBTable
    { 
      {0.0, 0.0, 0.0, 0.0},
      {1.0, 0.0, 0.0, 0.0}
    };
  

  auto frame=vtkfig::Frame::New();

  auto griddata=vtkfig::DataSet::New();
  griddata->SetRectilinearGrid(x,y,z);
  griddata->SetPointScalar(val ,"val");
  griddata->SetPointVector(u,v , w, "grad");

  auto contour=vtkfig::SurfaceContour::New();
  contour->SetData(griddata,"val");
  contour->SetSurfaceRGBTable(colors,255);
  contour->ShowIsolines(false);
  contour->SetValueRange(-1,1);
  
  auto quiver=vtkfig::Quiver::New();
  quiver->SetQuiverArrowScale(2);
  quiver->SetData(griddata,"grad");
  quiver->SetQuiverGrid(10,10,10);




  frame->AddFigure(contour);
  frame->AddFigure(quiver);



  double t=0;
  double dt=0.1;
  size_t ii=0;
  double t0=(double)clock()/(double)CLOCKS_PER_SEC;
  double i0=ii;
  while (ii<nspin)
  {

    for (int i=0; i<Nx; i++)
      for (int j=0; j<Ny; j++)
        for (int k=0; k<Nz; k++)
      {
        int idx=k*Nx*Ny+j*Nx+i;
        val[idx] =  G(x[i],y[j],z[k],t);
        u[idx] = dGdx(x[i],y[j],z[k],t);
        v[idx] = dGdy(x[i],y[j],z[k],t);
        w[idx] = dGdz(x[i],y[j],z[k],t);
      }
    griddata->SetPointScalar(val ,"val");
    griddata->SetPointVector(u,v ,w,"grad");
    
    frame->Show();

    if (ii==3) 
      frame->WritePNG("example-rectquiver3d.png");

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
