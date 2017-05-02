#ifndef VTKFIG_FIGURE_H
#define VTKFIG_FIGURE_H

#include <memory>
#include <vector>

#include "vtkSmartPointer.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkActor.h"
#include "vtkActor2D.h"
#include "vtkMapper.h"
#include "vtkMapper2D.h"


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
    friend class TimerCallback;
  public:
    Figure();
    void SetBackground(double r, double g, double b) { bgcolor[0]=r; bgcolor[1]=g; bgcolor[2]=b;}
    bool IsEmpty();

    virtual void Build()=0;
    virtual void SetInteractor(vtkSmartPointer<vtkRenderWindowInteractor> interactor) {};


  protected:
    void AddActor(vtkSmartPointer<vtkActor> prop);
    void AddActor2D(vtkSmartPointer<vtkActor2D> prop);
    void UpdateActors()
    {
      for (auto actor: actors) {auto m=actor->GetMapper(); if (m) m->Update();}
      for (auto actor: actors2d){auto m=actor->GetMapper(); if (m) m->Update();}
    }
    void ClearActors2D() { actors2d.clear();}
    
  private:
    std::vector<vtkSmartPointer<vtkActor>> actors;
    std::vector<vtkSmartPointer<vtkActor2D>> actors2d;
    double bgcolor[3]={1,1,1};
  };
  
};

#endif
