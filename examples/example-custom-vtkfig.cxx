///
///   \example  example-custom-vtkfig.cxx
///
///   Custom vtk pipeline with vtkfig::Figure.
///
///  Example showing creation of custom scenes using  vtkfig::Frame and vtkfig::Figure base class.
///

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

  size_t nspin=vtkfig::NSpin();

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
    


  

  for (int i=0; i<Nx; i++)
    x[i] = x_low+i*dx;
  
  for (int i=0; i<Ny; i++)
    y[i] = y_low + i*dy;


  double t=0;
  double dt=0.1;
  size_t ii=0;
  auto t0=std::chrono::system_clock::now();
  double i0=ii;

  auto figure=vtkfig::Figure::New();
  figure->SetViewVolume(0,1,0,1,0,1);
  int i,j,k;
  
  
  auto gridfunc=vtkSmartPointer<vtkStructuredGrid>::New();
  auto points=vtkSmartPointer<vtkPoints>::New();
  auto colors=vtkSmartPointer<vtkFloatArray>::New();
  
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

  vtkSmartPointer<vtkLookupTable> lut = vtkfig::internal::BuildLookupTable(rgbcolors,255);
  
  gridfunc->GetPointData()->SetScalars(colors);
  
  
  // filter to geometry primitive
  auto geometry = vtkSmartPointer<vtkStructuredGridGeometryFilter>::New();
  geometry->SetInputDataObject(gridfunc);
  
  
  // map gridfunction
  auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInputConnection(geometry->GetOutputPort());
  
  
  // create plot surface actor
  auto surfplot = vtkSmartPointer<vtkActor>::New();
  surfplot->SetMapper(mapper);
  mapper->SetLookupTable(lut);
  mapper->UseLookupTableScalarRangeOn();
  
  
  // create outline
  auto outlinefilter = vtkSmartPointer<vtkOutlineFilter>::New();
  outlinefilter->SetInputConnection(geometry->GetOutputPort());
  
  auto outlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  outlineMapper->SetInputConnection(outlinefilter->GetOutputPort());
  auto outline = vtkSmartPointer<vtkActor>::New();
  outline->SetMapper(outlineMapper);
  outline->GetProperty()->SetColor(0, 0, 0);
  
  
  

  // renderer
  figure->AddActor(surfplot);
  figure->AddActor(outline);
  figure->AddActor2D(vtkfig::internal::BuildColorBar(mapper));
  frame->AddFigure(figure);
  
  
  
  
  while (ii <nspin)
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
