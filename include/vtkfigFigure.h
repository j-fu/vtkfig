#ifndef VTKFIG_FIGURE_H
#define VTKFIG_FIGURE_H

#include <memory>
#include <vector>

#include "vtkSmartPointer.h"
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


  protected:
    void AddActor(vtkSmartPointer<vtkProp> prop);
    

  private:
    std::shared_ptr<std::vector<vtkSmartPointer<vtkProp>>>actors;
    double bgcolor[3]={1,1,1};
  };

}

#endif
