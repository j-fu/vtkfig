/**
    \file vtkfigGLScene.h

    Provide user API  class vtkfig::GLScene derived from vtkfig::Figure 
    for drawing with OpenGL 1.1 like API.
*/

#ifndef VTKFIG_GLSCENE_H
#define VTKFIG_GLSCENE_H

#include <vtkPointData.h>
#include <vtkLine.h>

#include "vtkfigFigure.h"


namespace vtkfig
{
  ///
  /// Drawing with OpenGL 1.1 like API. So far kind  more or less a demo.
  ///
  class GLScene: public Figure
  {
  public:
    static std::shared_ptr<GLScene> New();
    void Color(double r, double g, double b);
    void BeginLines(void);
    void Vertex2(double x, double y);
    void End(void);
    void Clear(void);

    
    GLScene();
    ~GLScene(){};
    
  private:
    void  RTBuildVTKPipeline()  override final;
    struct
    {
      vtkSmartPointer<vtkPolyData> polydata;
      vtkSmartPointer<vtkPolyDataMapper> mapper;
      vtkSmartPointer<vtkActor> actor;
      vtkSmartPointer<vtkPoints> points;
      vtkSmartPointer<vtkCellArray> lines;
      vtkSmartPointer<vtkUnsignedCharArray> colors;
    } line_data;
    unsigned char current_rgb[3];
    vtkIdType last_id;
  };
  
}

#endif
