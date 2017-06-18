#ifndef VTKFIG_TOOLS_H
#define VTKFIG_TOOLS_H
#include <vtkDelaunay2D.h>
#include <vtkDelaunay3D.h>
#include <vtkProperty.h>
#include <vtkIdList.h>
#include <vtkPolyDataMapper.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>


#include "vtkSmartPointer.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyDataMapper.h"
#include "vtkLookupTable.h"
#include "vtkScalarBarActor.h"

#include "vtkfigCommunicator.h"

namespace vtkfig
{
  /// RGB point for color tables
  struct RGBPoint { double x,r,g,b;};

  typedef std::vector<RGBPoint> RGBTable;

  void SendRGBTable(vtkSmartPointer<internals::Communicator> communicator, RGBTable & rgbtab);
  
  void ReceiveRGBTable(vtkSmartPointer<internals::Communicator> communicator, RGBTable & rgbtab);
  
  size_t NSpin();

  vtkSmartPointer<vtkLookupTable>  BuildLookupTable(RGBTable & xrgb, size_t size);
  
  vtkSmartPointer<vtkScalarBarActor> BuildColorBar(vtkSmartPointer<vtkPolyDataMapper> mapper, int irank=0);

  template <class V, class IV> inline  void Delaunay2D(const V & points_in,  V & points, IV & cells);

  template <class V, class IV> inline  void Delaunay3D(const V & points_in,  V & points, IV & cells);

  void PrintPoints(vtkSmartPointer<vtkPoints> pts, std::ostream & os);

  void PrintArray(vtkSmartPointer<vtkDataArray> data, std::ostream & os);




  template <class V, class IV> inline void Delaunay2D(const V & points_in,  V & points, IV & cells)
  {
    assert(points.size()==0);
    assert(cells.size()==0);
    
    auto inpoints =vtkSmartPointer<vtkPoints>::New();
    
    for (int i=0; i<points_in.size(); i+=2)
      inpoints->InsertNextPoint(points_in[i], points_in[i+1],0);
    
    
    auto aPolyData=   vtkSmartPointer<vtkPolyData>::New();
    aPolyData->SetPoints(inpoints);
    
    
    auto delaunay = vtkSmartPointer<vtkDelaunay2D>::New();
    delaunay->SetInputData(aPolyData);
    
    auto dgrid=delaunay->GetOutput();
    delaunay->Update();
    
    auto npoints=dgrid->GetNumberOfPoints();
    auto ncells=dgrid->GetNumberOfPolys();
    
    auto  vtkpoints=dgrid->GetPoints();
    for (int i=0;i<npoints;i++)
    {
      double x[3];
      vtkpoints->GetPoint(i,x);
      points.push_back(x[0]);
      points.push_back(x[1]);
    }
    
    auto pts =vtkSmartPointer<vtkIdList>::New();
    
    for (vtkIdType i=0;i<ncells;i++)
    {
      dgrid->GetCellPoints(i,pts);
      cells.push_back(pts->GetId(0));
      cells.push_back(pts->GetId(1));
      cells.push_back(pts->GetId(2));
    }
    
    
  }


  template <class V, class IV>
    inline void Delaunay3D(const V & points_in,  V & points, IV & cells)
  {
    assert(points.size()==0);
    assert(cells.size()==0);
    
    auto inpoints =  vtkSmartPointer<vtkPoints>::New();
    
    for (int i=0; i<points_in.size(); i+=3)
      inpoints->InsertNextPoint(points_in[i], points_in[i+1], points_in[i+2]);
    
    
    
    auto aPolyData = vtkSmartPointer<vtkPolyData>::New();
    aPolyData->SetPoints(inpoints);
    
    
    auto delaunay = vtkSmartPointer<vtkDelaunay3D>::New();
    delaunay->SetInputData(aPolyData);
    
    auto dgrid=delaunay->GetOutput();
    delaunay->Update();
    
    auto npoints=dgrid->GetNumberOfPoints();
    auto ncells=dgrid->GetNumberOfCells();
    
    auto  vtkpoints=dgrid->GetPoints();
    for (int i=0;i<npoints;i++)
    {
      double x[3];
      vtkpoints->GetPoint(i,x);
      points.push_back(x[0]);
      points.push_back(x[1]);
      points.push_back(x[2]);
    }
    
    auto pts =  vtkSmartPointer<vtkIdList>::New();
    
    for (vtkIdType i=0;i<ncells;i++)
    {
      dgrid->GetCellPoints(i,pts);
      cells.push_back(pts->GetId(0));
      cells.push_back(pts->GetId(1));
      cells.push_back(pts->GetId(2));
      cells.push_back(pts->GetId(3));
    }
    
    
  }


}

#endif
