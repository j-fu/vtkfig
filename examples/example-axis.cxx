///
///   \example   examples/example-custom.cxx
///
///   Custom vtk pipeline with vtkfig::Figure
///

#include <chrono>
#include "vtkUnstructuredGrid.h"
#include "vtkPointData.h"
#include "vtkFloatArray.h"
#include "vtkActor.h"
#include "vtkOutlineFilter.h"

#include "vtkCubeAxesActor.h"
#include "vtkTextProperty.h"
#include "vtkPolyLine.h"
#include "vtkDataSetMapper.h"
#include "vtkAxisActor.h"


#include "vtkAxis.h"
#include "vtkPlotGrid.h"
#include "vtkDoubleArray.h"
#include "vtkTable.h"
#include "vtkPlotLine.h"

#include "vtkfigFrame.h"
#include "vtkfigFigure.h"
#include "vtkfigTools.h"
#include "vtkLegendBoxActor.h"

class CoordSystemActor: public vtkCubeAxesActor
{
protected:
  CoordSystemActor(): vtkCubeAxesActor() 
  {
    SetZAxisVisibility(0);
    SetYAxisVisibility(1);
    SetXAxisVisibility(1);
    SetDrawXGridlines(1);
    SetDrawXInnerGridlines(1);
    SetDrawYGridlines(1);
    GetLabelTextProperty(0)->SetFontSize(15);;
    GetTitleTextProperty(0)->SetFontSize(15);;
    GetLabelTextProperty(0)->SetColor(0,0,0);
    GetTitleTextProperty(0)->SetColor(0,0,0);
    GetLabelTextProperty(1)->SetFontSize(15);;
    GetTitleTextProperty(1)->SetFontSize(15);;
    GetLabelTextProperty(1)->SetColor(0,0,0);
    GetTitleTextProperty(1)->SetColor(0,0,0);

    GetXAxesLinesProperty()->SetColor(0, 0, 0);
    GetXAxesLinesProperty()->SetLineWidth(2);
    GetYAxesLinesProperty()->SetColor(0, 0, 0);
    GetYAxesLinesProperty()->SetLineWidth(2);

    SetAxisOrigin(0,0,0);
    SetUseAxisOrigin(0);
    GetXAxesGridlinesProperty()->SetLineWidth(0.5);
    GetXAxesGridlinesProperty()->SetColor(0, 0, 0);
    GetYAxesGridlinesProperty()->SetColor(0, 0, 0);
    SetTickLocationToOutside();

  };
public:
  static CoordSystemActor *New()    {return new CoordSystemActor();}
  void SetLogScaleX() { XAxes[0]->SetLog(true);}
  void SetLogScaleY() { YAxes[0]->SetLog(true);}
};


class myplot0: public vtkfig::Figure
{
public:
  myplot0(): vtkfig::Figure()
  {
  }
  static std::shared_ptr<myplot0> New() { return std::make_shared<myplot0>();}
  virtual std::string SubClassName() {return std::string("myplot0");}
  
  void RTBuildAllVTKPipelines(vtkSmartPointer<vtkRenderer> renderer) 
  {
    auto coordsys=vtkSmartPointer<CoordSystemActor>::New();
    coordsys->SetUse2DMode(1);
    coordsys->SetBounds(0,1,0.001,100,0,0);
//    coordsys->SetYAxisRange(0,1);

    coordsys->SetXTitle("x");
    coordsys->SetXUnits("cm");
    coordsys->SetYTitle("y");
//    coordsys->SetLogScaleY();

    // coordsys->SetScreenSize(0.1);
    // coordsys->SetLabelOffset(2);

    coordsys->SetCamera(renderer->GetActiveCamera());


    
    Figure::RTAddActor(coordsys);



    auto polyLinePoints = vtkSmartPointer<vtkPoints>::New();
    auto polyLine = vtkSmartPointer<vtkPolyLine>::New();
    int N=100;
    polyLinePoints->SetNumberOfPoints(N);
    polyLine->GetPointIds()->SetNumberOfIds(N);


    double x=0.0;
    double h=1.0/N;
    for (int i=0;i<N;i++,x+=h)
    {
      polyLinePoints->InsertPoint(i, x, exp(sin(10.0*x)), 0);
      polyLine->GetPointIds()->SetId(i,i);
    }
    
    auto polyLineGrid = vtkSmartPointer<vtkUnstructuredGrid>::New();
    polyLineGrid->Allocate(1, 1);
    polyLineGrid->InsertNextCell(polyLine->GetCellType(), polyLine->GetPointIds());
    polyLineGrid->SetPoints(polyLinePoints);

    auto polyLineMapper = vtkSmartPointer<vtkDataSetMapper>::New();
    polyLineMapper->SetInputData(polyLineGrid);

    auto polyLineActor =vtkSmartPointer<vtkActor>::New();
    polyLineActor->SetMapper(polyLineMapper);
    polyLineActor->GetProperty()->SetColor(1,0,0);
    polyLineActor->GetProperty()->SetLineWidth(2.5);
    polyLineActor->GetProperty()->BackfaceCullingOn();

    Figure::RTAddActor(polyLineActor);

    auto lbox=vtkSmartPointer<vtkLegendBoxActor>::New();
   
  }
};




class myplot: public vtkfig::Figure
{
public:
  myplot(): vtkfig::Figure()
  {
  }
  static std::shared_ptr<myplot> New() { return std::make_shared<myplot>();}
  virtual std::string SubClassName() {return std::string("myplot");}
  
  void RTBuildAllVTKPipelines(vtkSmartPointer<vtkRenderer> renderer) 
  {
    auto ctxactor= vtkSmartPointer<vtkContextActor>::New(); 
    auto XAxis=vtkSmartPointer<vtkAxis>::New();

    double P00x=0.1; double P00y=0.1;
    double P01x=0.1; double P01y=0.9;
    double P10x=0.9; double P10y=0.1;
    double P11x=0.9; double P11y=0.9;
    renderer->NormalizedDisplayToDisplay(P00x,P00y);
    renderer->NormalizedDisplayToDisplay(P10x,P10y);
    renderer->NormalizedDisplayToDisplay(P01x,P01y);
    renderer->NormalizedDisplayToDisplay(P11x,P11y);

    XAxis->SetPoint1(P00x,P00y);
    XAxis->SetPoint2(P10x,P10y);
    cout << P10x << " " << P10y << endl;
    XAxis->SetPosition(vtkAxis::BOTTOM);
    XAxis->SetRange(0,10);
    XAxis->SetAxisVisible(1);
    XAxis->SetGridVisible(1);
    XAxis->SetScene(ctxactor->GetScene());
    XAxis->Update();


    auto YAxis=vtkSmartPointer<vtkAxis>::New();
    YAxis->SetPoint1(P00x,P00y);
    YAxis->SetPoint2(P01x,P01y);
    YAxis->SetPosition(vtkAxis::LEFT);
    YAxis->SetRange(0.01,10);
//    YAxis->LogScaleOn();
//    YAxis->SetNumberOfTicks(2);
    YAxis->SetAxisVisible(1);
    YAxis->SetGridVisible(1);
    YAxis->SetScene(ctxactor->GetScene());
    YAxis->Update();

//    XAxis->GetProperty()->SetColor(0,0,0);

    auto Grid=vtkSmartPointer<vtkPlotGrid>::New();
    Grid->SetXAxis(XAxis);
    Grid->SetYAxis(YAxis);
    Grid->Update();
  
    cout << ctxactor->GetScene()->GetViewWidth() << " "<< ctxactor->GetScene()->GetViewHeight() << endl;
    cout << ctxactor->GetScene()->GetSceneWidth() << " "<< ctxactor->GetScene()->GetSceneHeight() << endl;


    
    vtkSmartPointer<vtkDoubleArray> X;
    vtkSmartPointer<vtkDoubleArray> Y;
    vtkSmartPointer<vtkTable> table;
    vtkSmartPointer<vtkPlotLine> line;

    X=vtkSmartPointer<vtkDoubleArray>::New();
    Y=vtkSmartPointer<vtkDoubleArray>::New();
    X->SetName("X");
    Y->SetName("Y");
    table=vtkSmartPointer<vtkTable>::New();
    table->AddColumn(X);
    table->AddColumn(Y);

    int N=100;
    table->SetNumberOfRows(N);
    double x=0.0;
    double h=1.0/N;
    for (int i=0;i<N;i++,x+=h)
    {
      table->SetValue(i,0,10*x);
      table->SetValue(i,1,exp(sin(10.0*x)));
    }


    line=vtkSmartPointer<vtkPlotLine>::New();
    line->SetInputData(table, 0, 1);
    line->SetXAxis(XAxis);
    line->SetYAxis(YAxis);
    line->SetPolyLine(true);
    line->SetColor(1,0,0);
    line->Update();



    ctxactor->GetScene()->AddItem(Grid);
    ctxactor->GetScene()->AddItem(XAxis);
    ctxactor->GetScene()->AddItem(YAxis);
    ctxactor->GetScene()->AddItem(line);
    Figure::RTAddContextActor(ctxactor);
    



  }
};


int main(void)
{


  auto frame=vtkfig::Frame::New();
  auto plot=myplot0::New();


  frame->AddFigure(plot);
  frame->Interact();

 
  
#if 0  
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
#endif
}
