///
///   \example   examples/example-gridview.cxx
///
/// Multiple frames. 
///
/// Here the problem is that we get interaction only in the first frame...
///
#include <chrono>
#include "vtkfigFrame.h"
#include "vtkfigDataSet.h"
#include "vtkfigSurfaceContour.h"
#include "vtkfigXYPlot.h"
#include "vtkfigTools.h"

inline double GU(double x,double y, double t) 
{
  
  return exp(-(x*x+y*y))*sin(t+x)*cos(y-t);
}

inline double GV(double x,double y, double t) 
{
  
  return exp(-(x*x+y*y))*sin(1.0-(2*t+2*x))*cos(1.0-(2*y-2*t));
}


int main(void)
{
  cout.sync_with_stdio(true);
  size_t nspin=vtkfig::NSpin();

  const int Nx = 200;
  const int Ny = 250;
  
  std::vector<double> x(Nx);
  std::vector<double> y(Ny);
  std::vector<double> u(Nx*Ny);
  std::vector<double> v(Nx*Ny);
  
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
  size_t ii=0;
  auto t0=std::chrono::system_clock::now();
  double i0=ii;

  auto frame1=vtkfig::Frame::New();
  auto frame2=vtkfig::Frame::New();
  auto frame3=vtkfig::Frame::New();

  frame1->SetSize(400,400);
  frame2->SetSize(400,400);
  frame2->SetPosition(500,0);
  frame3->SetSize(400,400);
  frame3->SetPosition(1000,0);
  frame2->LinkCamera(frame1);


  auto griddata=vtkfig::DataSet::New();
  griddata->SetRectilinearGrid(x,y);
  griddata->SetPointScalar(u ,"u");
  griddata->SetPointScalar(v ,"v");


  auto contour_u=vtkfig::SurfaceContour::New();
  contour_u->SetData(griddata,"u");
  contour_u->SetSurfaceRGBTable(colors,255);
  frame1->AddFigure(contour_u);

  auto contour_v=vtkfig::SurfaceContour::New();
  contour_v->SetData(griddata,"v");
  contour_v->SetSurfaceRGBTable(colors,255);
  frame2->AddFigure(contour_v);

  auto xyplot=vtkfig::XYPlot::New();
  xyplot->SetYRange(-0.5,0.5);
  frame3->AddFigure(xyplot);

  while (ii<nspin)
  {
    for (int i=0; i<Nx; i++)
      for (int j=0; j<Ny; j++)
      {
        double f=GU(x[i],y[j],t);
        u[j*Nx+i] = f;
        if (i==Nx/2) fy[j]=f;
        if (j==Ny/2) fx[i]=f;

        f=GV(x[i],y[j],t);
        v[j*Nx+i] = f;
      }

    griddata->SetPointScalar(u ,"u");
    griddata->SetPointScalar(v ,"v");

    xyplot->Clear();
    xyplot->LineColorRGB(0,0,1);
    xyplot->LineType("-");
    xyplot->Legend("y=0.5");
    xyplot->AddPlot(x, fx);
    xyplot->LineColorRGB(1,0,0);
    xyplot->LineType("-");
    xyplot->Legend("x=0.5");
    xyplot->AddPlot(y, fy);
    frame1->Show();
    frame2->Show();
    frame3->Show();


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


