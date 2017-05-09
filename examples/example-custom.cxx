#include <chrono>
#include "vtkStructuredGrid.h"
#include "vtkPointData.h"
#include "vtkFloatArray.h"
#include "vtkActor.h"
#include "vtkOutlineFilter.h"
#include "vtkStructuredGridGeometryFilter.h"

#include "vtkfigFrame.h"
#include "vtkfigFigure.h"
#include "vtkfigTools.h"

inline double G(double x,double y, double t) 
{
  
  return exp(-(x*x+y*y))*sin(t+x)*cos(y-t);
}


int main(void)
{
  cout <<
R"(
Example showing creation of custom scenes using 
vtkfigFrame and vtkfigFigure base class.
)";
  cout.sync_with_stdio(true);
  
  const int Nx = 200;
  const int Ny = 250;
  
  std::vector<double> x(Nx);
  std::vector<double> y(Ny);
  std::vector<double> z(Nx*Ny);
  
  const double x_low = -2.5;
  const double x_upp = 1.5;
  const double y_low = -2.5;
  const double y_upp = 4;
  const double dx = (x_upp-x_low)/(Nx-1);
  const double dy = (y_upp-y_low)/(Ny-1);

  auto frame=vtkfig::Frame::New();
  frame->SetInteractorStyle(vtkfig::Frame::InteractorStyle::Volumetric);

  

  for (int i=0; i<Nx; i++)
    x[i] = x_low+i*dx;
  
  for (int i=0; i<Ny; i++)
    y[i] = y_low + i*dy;


  double t=0;
  double dt=0.1;
  int ii=0;
  auto t0=std::chrono::system_clock::now();
  double i0=ii;

  auto figure=vtkfig::Figure::New();

  int i,j,k;
  double Lxy, Lz;
  if (x[Nx-1]-x[0] > y[Ny-1]-y[0])
    Lxy = x[Nx-1]-x[0];
  else
    Lxy = y[Ny-1]-y[0];
  
  double z_low = 10000, z_upp = -10000;
  
  vtkSmartPointer<vtkStructuredGrid> 	    gridfunc=vtkSmartPointer<vtkStructuredGrid>::New();
  vtkSmartPointer<vtkPoints> points=vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkFloatArray> colors=vtkSmartPointer<vtkFloatArray>::New();
  
  gridfunc->SetDimensions(Nx, Ny, 1);
  
  points = vtkSmartPointer<vtkPoints>::New();
  for (j = 0; j < Ny; j++)
  {
    for (i = 0; i < Nx; i++)
    {
      points->InsertNextPoint(x[i], y[j], 0);
    }
  }
  gridfunc->SetPoints(points);
  
  colors = vtkSmartPointer<vtkFloatArray>::New();
  colors->SetNumberOfComponents(1);
  colors->SetNumberOfTuples(Nx*Ny);
  k = 0;
  for (j = 0; j < Ny; j++)
    for (i = 0; i < Nx; i++)
    {
      colors->InsertComponent(k, 0, 0);
      k++;
    }
  
  auto rgbcolors=vtkfig::RGBTable
    { 
      {0.0, 0.0, 0.0, 1.0},
      {0.5, 0.0, 1.0, 0.0},
      {1.0, 1.0, 0.0, 0.0}
    };

  vtkSmartPointer<vtkLookupTable> lut = vtkfig::BuildLookupTable(rgbcolors,255);
  
  gridfunc->GetPointData()->SetScalars(colors);
  
  
  // filter to geometry primitive
  vtkSmartPointer<vtkStructuredGridGeometryFilter>geometry =
    vtkSmartPointer<vtkStructuredGridGeometryFilter>::New();
  geometry->SetInputDataObject(gridfunc);
  
  
  // map gridfunction
  vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInputConnection(geometry->GetOutputPort());
  
  
  // create plot surface actor
  vtkSmartPointer<vtkActor> surfplot = vtkSmartPointer<vtkActor>::New();
  surfplot->SetMapper(mapper);
  mapper->SetLookupTable(lut);
  mapper->UseLookupTableScalarRangeOn();
  
  
  // create outline
  vtkSmartPointer<vtkOutlineFilter> outlinefilter = vtkSmartPointer<vtkOutlineFilter>::New();
  outlinefilter->SetInputConnection(geometry->GetOutputPort());
  
  vtkSmartPointer<vtkPolyDataMapper>outlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  outlineMapper->SetInputConnection(outlinefilter->GetOutputPort());
  vtkSmartPointer<vtkActor> outline = vtkSmartPointer<vtkActor>::New();
  outline->SetMapper(outlineMapper);
  outline->GetProperty()->SetColor(0, 0, 0);
  
  
  // renderer
  figure->RTAddActor(surfplot);
  figure->RTAddActor(outline);
  figure->RTAddActor2D(vtkfig::BuildColorBar(mapper));
  
  frame->AddFigure(figure);
  
  
  
  
  while (1)
  {
    
    for (int i=0; i<Nx; i++)
      for (int j=0; j<Ny; j++)
        z[j*Nx+i] = G(x[i],y[j],t);
    
    float vmax=-10000;
    float vmin=10000;
    for (int j = 0; j < Ny; j++)
    {
      for (int i = 0; i < Nx; i++)
      {
        int k=j*Nx+i;
        float v=z[k];
        vmin=std::min(v,vmin);
        vmax=std::max(v,vmax);
        double  p[3];
        points->GetPoint(k,p);
        p[2]=v;
        points->SetPoint(k,p);
        colors->InsertComponent(k, 0,v);
      }
    }
    
    points->Modified();
    colors->Modified();
    gridfunc->Modified();
    double Lz = vmax-vmin;
    lut->SetTableRange(vmin,vmax);
    lut->Modified();
    
    frame->Show();
    

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
