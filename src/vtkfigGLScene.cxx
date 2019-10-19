#include "vtkfigGLScene.h"

namespace vtkfig
{
  GLScene::GLScene(): Figure()
  {
    line_data.points=vtkSmartPointer<vtkPoints>::New();
    line_data.lines=vtkSmartPointer<vtkCellArray>::New();
    line_data.colors=vtkSmartPointer<vtkUnsignedCharArray>::New();
    line_data.colors->SetNumberOfComponents(3);
    line_data.polydata=vtkSmartPointer<vtkPolyData>::New();
    line_data.polydata->SetPoints(line_data.points);
    line_data.polydata->SetLines(line_data.lines);
    line_data.polydata->GetCellData()->SetScalars(line_data.colors);
  };
  
  
  std::shared_ptr<GLScene> GLScene::New() { return std::make_shared<GLScene>();}
  
  
  void GLScene::Color(double r, double g, double b)
  {
    current_rgb[0]=(unsigned char)(r*255.0);
    current_rgb[1]=(unsigned char)(g*255.0);     
    current_rgb[2]=(unsigned char)(b*255.0);
  }
  
  void GLScene::BeginLines(void)
  {
    last_id=-1;
    current_rgb[0]=0;
    current_rgb[1]=0;
    current_rgb[2]=0;
  }
  void GLScene::Vertex2(double x, double y)
  {
    auto this_id=GLScene::line_data.points->InsertNextPoint(x,y,0.0);
    if (last_id>=0)
    {
      auto line=vtkSmartPointer<vtkLine>::New();
      line->GetPointIds()->SetId(0, last_id);
      line->GetPointIds()->SetId(1, this_id);
      GLScene::line_data.lines->InsertNextCell(line);
      GLScene::line_data.colors->InsertNextTypedTuple(current_rgb);
    }
    last_id=this_id;
  }
  void GLScene::End(void)
  {
    line_data.polydata->Modified();
  }
  
  void GLScene::Clear(void)
  {
    line_data.points->Reset();
    line_data.lines->Reset();
    line_data.colors->Reset();
    line_data.polydata->Reset();
    line_data.polydata->Modified();
  }

  // Called from render thread
  void  GLScene::RTBuildVTKPipeline()
  {
    RTCalcTransform();
    auto trans_polydata=vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    trans_polydata->SetInputData(line_data.polydata);
    trans_polydata->SetTransform(Figure::transform);
    auto mapper=vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(trans_polydata->GetOutputPort());
    auto actor=vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    Figure::RTAddActor(actor);
  }
  
}
