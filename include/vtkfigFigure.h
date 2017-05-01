#include <memory>
#include <vector>

#include "vtkSmartPointer.h"
#include "vtkPolyDataMapper.h"
#include "vtkLookupTable.h"
#include "vtkScalarBarActor.h"
#include "vtkProp.h"


namespace vtkfig
{

  /// Base class for all figures.
  ///
  /// It justs consists of a set of instances of vtkActor which
  /// contains the data  used by vtk.
  /// 
  /// Derived classes just should fill these actors by calling Figure::AddActor
  ///
  /// In this way, any vtk rendering pipeline can be used. 
  /// 

  class Figure
  {
    friend class Frame;
  public:
    Figure();
    void SetBackground(double r, double g, double b) { bgcolor[0]=r; bgcolor[1]=g; bgcolor[2]=b;}
    bool IsEmpty();

    struct RGBPoint { double x,r,g,b;};
    typedef std::vector<RGBPoint> RGBTable;
    static vtkSmartPointer<vtkLookupTable>  BuildLookupTable(RGBTable & xrgb, int size);
    static vtkSmartPointer<vtkScalarBarActor> BuildColorBar(vtkSmartPointer<vtkPolyDataMapper> mapper);

  protected:
    void AddActor(vtkSmartPointer<vtkProp> prop);
    

  private:
    std::shared_ptr<std::vector<vtkSmartPointer<vtkProp>>>actors;
    double bgcolor[3]={1,1,1};
  };

}

