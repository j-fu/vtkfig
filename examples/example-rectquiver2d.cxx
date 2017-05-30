#include "vtkfigFrame.h"
#include "vtkfigSurfaceContour.h"
#include "vtkfigQuiver.h"
#include "vtkfigTools.h"

inline double G(double x,double y, double t) 
{
  
  return exp(-(x*x+y*y))*sin(t+x)*cos(y-t);
}

inline double dGdx(double x,double y, double t) 
{
  return cos(y-t)*exp(-(x*x+y*y))*(cos(t+x)-2*x*sin(t+x));
}

inline double dGdy(double x,double y, double t) 
{
  return sin(t+x)*exp(-(x*x+y*y))*(-2*y*cos(y-t) -sin(y-t));
}



int main(void)
{
  size_t nspin=vtkfig::NSpin();
  const int Nx = 20;
  const int Ny = 25;
  
  std::vector<double> x(Nx);
  std::vector<double> y(Ny);
  std::vector<double> z(Nx*Ny);
  std::vector<double> u(Nx*Ny);
  std::vector<double> v(Nx*Ny);

  
  const double x_low = -2.5;
  const double x_upp = 1.5;
  const double y_low = -2.5;
  const double y_upp = 4;
  const double dx = (x_upp-x_low)/(Nx-1);
  const double dy = (y_upp-y_low)/(Ny-1);

  auto frame=vtkfig::Frame::New();

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
  
  for (int i=0; i<Nx; i++)
    x[i] = x_low+i*dx;
  
  for (int i=0; i<Ny; i++)
    y[i] = y_low + i*dy;


  double t=0;
  double dt=0.1;
  size_t ii=0;
  double t0=(double)clock()/(double)CLOCKS_PER_SEC;
  double i0=ii;

  auto griddata=vtkfig::RectilinearGridData::New();
  griddata->SetGrid(x,y);
  griddata->SetPointScalar(z ,"v");
  griddata->SetPointVector(u,v ,"grad");

  auto contour=vtkfig::SurfaceContour::New();
  contour->SetData(griddata,"v");
  contour->SetSurfaceRGBTable(colors,255);
  contour->ShowIsolines(false);
  
  auto quiver=vtkfig::Quiver::New();
  quiver->SetQuiverArrowScale(0.3);
  quiver->SetData(griddata,"grad");

  frame->AddFigure(contour);
  frame->AddFigure(quiver);

  while (ii<nspin)
  {

    for (int i=0; i<Nx; i++)
      for (int j=0; j<Ny; j++)
      {
        z[j*Nx+i] = G(x[i],y[j],t);
        u[j*Nx+i] = dGdx(x[i],y[j],t);
        v[j*Nx+i] = dGdy(x[i],y[j],t);
      }

    griddata->SetPointScalar(z ,"v");
    griddata->SetPointVector(u,v ,"grad");

    frame->Show();

    if (ii==3) 
      frame->Dump("example-quiver2d.png");

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
