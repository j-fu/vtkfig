///
///   \example  example-multifig.cxx
///
/// Multiple figures in one frame
///
#include <chrono>
#include "vtkfigFrame.h"
#include "vtkfigDataSet.h"
#include "vtkfigScalarView.h"
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

  std::shared_ptr<vtkfig::Frame> frame=vtkfig::Frame::New();
  std::shared_ptr<vtkfig::DataSet> dataset=vtkfig::DataSet::New();

  frame->SetLayout(2,2);
  frame->LinkCamera(1,frame,0);

  frame->SetSize(800,400);
  frame->SetWindowTitle("vtkfig");
  frame->SetFrameTitle("Multiple figures in one frame");

  dataset->SetRectilinearGrid(x,y);
  dataset->SetPointScalar(u ,"u");
  dataset->SetPointScalar(v ,"v");



  std::shared_ptr<vtkfig::ScalarView> contour_u=vtkfig::ScalarView::New();
  contour_u->SetData(dataset,"u");
  contour_u->SetSurfaceRGBTable(colors,255);
  frame->AddFigure(contour_u,0);

  std::shared_ptr<vtkfig::ScalarView> contour_v=vtkfig::ScalarView::New();
  contour_v->SetData(dataset,"v");
  contour_v->SetSurfaceRGBTable(colors,255);
  frame->AddFigure(contour_v,1);

  
  std::shared_ptr<vtkfig::XYPlot> xyplot=vtkfig::XYPlot::New();
  xyplot->SetYRange(-0.5,0.5);
  frame->AddFigure(xyplot,2);

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

    dataset->SetPointScalar(u ,"u");
    dataset->SetPointScalar(v ,"v");

    xyplot->Clear();
    xyplot->SetPlotColor(0,0,1);
    xyplot->SetPlotLineType("-");
    xyplot->SetPlotLegend("y=0.5");
    xyplot->AddPlot(x, fx);
    xyplot->SetPlotColor(1,0,0);
    xyplot->SetPlotLineType("-");
    xyplot->SetPlotLegend("x=0.5");
    xyplot->AddPlot(y, fy);
    frame->Show();

    if (ii==3) 
      frame->WritePNG("example-multifig.png");

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
