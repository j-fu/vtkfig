///
///   \example   examples/example-simplexquiver2d.cxx
///
///  Vector function on 3D simplex grid
///

#include "vtkMath.h"
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
  
  std::vector<double> inpoints;
  
  
  for(double x = -2; x < 2; x+=0.2)
    for(double y = -2; y < 2; y+=0.2)
      for(double z = -2; z < 2; z+=0.2)
      {
        inpoints.push_back(x + vtkMath::Random(-.1, .1));
        inpoints.push_back(y + vtkMath::Random(-.1, .1));
        inpoints.push_back(z + vtkMath::Random(-.1, .1));
      }
  
  
  
  
  
  std::vector<double>points;
  std::vector<int>cells;
  
  vtkfig::Delaunay3D(inpoints,points,cells);
  
  
  int npoints=points.size()/3;

  std::vector<double> u(npoints);
  std::vector<double> v(npoints);
  std::vector<double> w(npoints);
  std::vector<double>val(npoints);
  
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
  griddata->SetSimplexGrid(3,points,cells);
  griddata->SetPointScalar(val ,"val");
  griddata->SetPointVector(u,v,w ,"grad");
  
  auto contour=vtkfig::SurfaceContour::New();
  contour->SetData(griddata,"val");
  contour->SetSurfaceRGBTable(colors,255);
  contour->ShowIsolines(false);
  contour->SetValueRange(-1,1);
  
  auto quiver=vtkfig::Quiver::New();
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
    
    for (size_t ipoint=0, ival=0;ipoint<points.size(); ipoint+=3,ival++)
    {
      val[ival]=G(points[ipoint+0],points[ipoint+1],points[ipoint+2],t);
      u[ival]=dGdx(points[ipoint+0],points[ipoint+1],points[ipoint+2],t);
      v[ival]=dGdy(points[ipoint+0],points[ipoint+1],points[ipoint+2],t);
      w[ival]=dGdz(points[ipoint+0],points[ipoint+1],points[ipoint+2],t);
    }
    griddata->SetPointScalar(val ,"val");
    griddata->SetPointVector(u,v ,w,"grad");

    frame->Show();

    if (ii==3) 
      frame->WritePNG("example-simplexquiver2d.png");

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
