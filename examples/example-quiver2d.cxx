#include "vtkfigFrame.h"
#include "vtkfigRectContour.h"
#include "vtkfigQuiver2D.h"
#include "vtkfigTools.h"

inline double G(double x,double y, double t) 
{
  
  return exp(-(x*x+y*y))*sin(t+x)*cos(y-t);
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
  int ii=0;
  double t0=(double)clock()/(double)CLOCKS_PER_SEC;
  double i0=ii;
  auto contour=vtkfig::RectContour::New();
  contour->SetSurfaceRGBTable(colors,255);
  contour->ShowIsocontours(false);
  
  auto quiver=vtkfig::Quiver2D::New();
  quiver->ShowColorbar(false);
  quiver->SetRGBTable(qcolors, 2);
  quiver->SetArrowScale(0.5);

  contour->SetGrid(x,y);
  frame->AddFigure(contour);

  quiver->SetGrid(x,y);
  frame->AddFigure(quiver);

  while (ii<nspin)
  {

    for (int i=0; i<Nx; i++)
      for (int j=0; j<Ny; j++)
      {
        z[j*Nx+i] = G(x[i],y[j],t);
      }

    for (int i=0; i<Nx; i++)
      for (int j=0; j<Ny; j++)
      {
        int ij=j*Nx+i;

        if (i==0) 
          u[ij]=(z[ij+1]-z[ij])/(x[i+1]-x[i]);
        else if (i==(Nx-1))
          u[ij]=(z[ij]-z[ij-1])/(x[i]-x[i-1]);
        else
          u[ij]=(z[ij+1]-z[ij-1])/(x[i+1]-x[i-1]);

        if (j==0) 
          v[ij]=(z[ij+Nx]-z[ij])/(y[j+1]-y[j]);
        else if (j==(Ny-1))
          v[ij]=(z[ij]-z[ij-Nx])/(y[j]-y[j-1]);
        else
          v[ij]=(z[ij+Nx]-z[ij-Nx])/(y[j+1]-y[j-1]);

      }

    contour->UpdateValues(z);
    quiver->UpdateValues(u,v);

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
