///
///   \example   examples/example-simplexquiver2d.cxx
///
///  Vector function on 2D simplex grid
///

#include "vtkMath.h"
#include "vtkfigFrame.h"
#include "vtkfigScalarView.h"
#include "vtkfigVectorView.h"
#include "vtkfigTools.h"

double xscale=1;
double yscale=1;

inline double G(double x,double y, double t) 
{
  x=x/xscale;
  y=y/yscale;
  return exp(-(x*x+y*y))*sin(t+x)*cos(y-t);
}

inline double dGdx(double x,double y, double t) 
{
  x=x/xscale;
  y=y/yscale;
  return xscale*cos(y-t)*exp(-(x*x+y*y))*(cos(t+x)-2*x*sin(t+x));
}


inline double dGdy(double x,double y, double t) 
{
  x=x/xscale;
  y=y/yscale;
  return yscale*sin(t+x)*exp(-(x*x+y*y))*(-2*y*cos(y-t) -sin(y-t));
}


int main(void)
{
  size_t nspin=vtkfig::NSpin();
  
  std::vector<double> inpoints;
  for(double x = -2; x < 2; x+=0.03)
  {
    for(double y = -2; y < 2; y+=0.03)
    {
      inpoints.push_back(xscale*(x + vtkMath::Random(-.1, .1)));
      inpoints.push_back(yscale*(y + vtkMath::Random(-.1, .1)));
    }
  }
  
  
  
  
  std::vector<double>points;
  std::vector<int>cells;
  
  vtkfig::Delaunay2D(inpoints,points,cells);
  
  
  int npoints=points.size()/2;

  std::vector<double>values(npoints);


  
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
  
  std::vector<double> z(npoints);
  std::vector<double> u(npoints);
  std::vector<double> v(npoints);
  
  
  double t=0;
  double dt=0.1;
  size_t ii=0;
  double t0=(double)clock()/(double)CLOCKS_PER_SEC;
  double i0=ii;
  
  auto griddata=vtkfig::DataSet::New();
  griddata->SetSimplexGrid(2,points,cells);
  griddata->SetPointScalar(z ,"v");
  griddata->SetPointVector(u,v ,"grad");
  
  auto contour=vtkfig::ScalarView::New();
  contour->SetData(griddata,"v");
  contour->SetSurfaceRGBTable(colors,255);
  contour->ShowIsolines(false);
  
  auto quiver=vtkfig::VectorView::New();
  quiver->SetData(griddata,"grad");
  quiver->SetQuiverGrid(15,15);

  contour->KeepXYAspect(false);
  quiver->KeepXYAspect(false);

  
  frame->AddFigure(contour);
  frame->AddFigure(quiver);
  
  while (ii<nspin)
  {
    
    for (size_t ipoint=0, ival=0;ipoint<points.size(); ipoint+=2,ival++)
    {
      z[ival]=G(points[ipoint+0],points[ipoint+1],t);
      u[ival]=dGdx(points[ipoint+0],points[ipoint+1],t);
      v[ival]=dGdy(points[ipoint+0],points[ipoint+1],t);
    }
    griddata->SetPointScalar(z ,"v");
    griddata->SetPointVector(u,v ,"grad");

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
