#ifndef VTKFIG_TOOLS_H
#define VTKFIG_TOOLS_H
#include <vtkDelaunay2D.h>
#include <vtkProperty.h>
#include <vtkIdList.h>
#include <vtkPolyDataMapper.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>


#include "vtkSmartPointer.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyDataMapper.h"
#include "vtkLookupTable.h"
#include "vtkScalarBarActor.h"


namespace vtkfig
{
  struct RGBPoint { double x,r,g,b;};
  typedef std::vector<RGBPoint> RGBTable;
  
  vtkSmartPointer<vtkLookupTable>  BuildLookupTable(RGBTable & xrgb, int size);
  vtkSmartPointer<vtkScalarBarActor> BuildColorBar(vtkSmartPointer<vtkPolyDataMapper> mapper);
  template <class V, class IV>
    inline  void Delaunay2D(const V & points_in,  V & points, IV & cells);



  template <class V, class IV>
    inline void Delaunay2D(const V & points_in,  V & points, IV & cells)
  {
    assert(points.size()==0);
    assert(cells.size()==0);
    
    vtkSmartPointer<vtkPoints> inpoints =
      vtkSmartPointer<vtkPoints>::New();
    
    for (int i=0; i<points_in.size(); i+=2)
      inpoints->InsertNextPoint(points_in[i], points_in[i+1],0);
    
    
    
    vtkSmartPointer<vtkPolyData> aPolyData =
      vtkSmartPointer<vtkPolyData>::New();
    aPolyData->SetPoints(inpoints);
    
    
    vtkSmartPointer<vtkDelaunay2D> delaunay =
      vtkSmartPointer<vtkDelaunay2D>::New();
    delaunay->SetInputData(aPolyData);
    
    vtkSmartPointer<vtkPolyData>dgrid=delaunay->GetOutput();
    delaunay->Update();
    
    auto npoints=dgrid->GetNumberOfPoints();
    auto ncells=dgrid->GetNumberOfPolys();
    
    vtkSmartPointer<vtkPoints> vtkpoints=dgrid->GetPoints();
    for (int i=0;i<npoints;i++)
    {
      double x[3];
      vtkpoints->GetPoint(i,x);
      points.push_back(x[0]);
      points.push_back(x[1]);
    }
    
    vtkSmartPointer<vtkIdList> pts =
      vtkSmartPointer<vtkIdList>::New();
    
    for (vtkIdType i=0;i<ncells;i++)
    {
      int c[3];
      
      dgrid->GetCellPoints(i,pts);
      c[0]=pts->GetId(0);
      c[1]=pts->GetId(1);
      c[2]=pts->GetId(2);
      
      cells.push_back(c[0]);
      cells.push_back(c[1]);
      cells.push_back(c[2]);
    }
    
    
  }

}

#endif
