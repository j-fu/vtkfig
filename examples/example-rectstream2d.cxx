///
///   \example   examples/example-rectquiver2d.cxx
///
///  Vector function on 2D rectilinear grid
///

#include "vtkfigFrame.h"
#include "vtkfigQuiver.h"
#include "vtkfigStream.h"
#include "vtkfigTools.h"


inline double UX(double x, double y, double t)
{
  return -y*pow(x*x+y*y,-0.5);
}

inline double UY(double x, double y, double t)
{
  return x*pow(x*x+y*y,-0.5);

}


int main(void)
{
  size_t nspin=vtkfig::NSpin();
  const int Nx = 50;
  const int Ny = 50;
  
  std::vector<double> x(Nx);
  std::vector<double> y(Ny);
  std::vector<double> u(Nx*Ny);
  std::vector<double> v(Nx*Ny);

  
  const double x_low = -1.0;
  const double x_upp = 1.0;
  const double y_low = -1.0;
  const double y_upp = 1.0;
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

  auto griddata=vtkfig::DataSet::New();
  griddata->SetRectilinearGrid(x,y);
  griddata->SetPointVector(u,v ,"flow");
  std::vector<double> seeds=
  {
    0,0.1,
    0,0.3,
    0,0.5,
    0,0.7,
    0,0.9
  };

  
  auto quiver=vtkfig::Quiver::New();
  quiver->SetData(griddata,"flow");
  quiver->SetQuiverGrid(5,5);

  auto stream=vtkfig::Stream::New();
  stream->SetData(griddata,"flow");

  stream->SetSeedPoints(seeds);



  frame->AddFigure(quiver);
  frame->AddFigure(stream);

 

  while (ii<nspin)
  {

    for (int i=0; i<Nx; i++)
      for (int j=0; j<Ny; j++)
      {
        u[j*Nx+i] = UX(x[i],y[j],t);
        v[j*Nx+i] = UY(x[i],y[j],t);
      }

    griddata->SetPointVector(u,v ,"flow");

    frame->Show();

    if (ii==3) 
      frame->WritePNG("example-rectstream3.png");

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
