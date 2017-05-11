///
/// \file vtkfigFrame.h
///
/// Define Frame class  providing a window for rendering.
/// 

#ifndef VTKFIG_FRAME_H
#define VTKFIG_FRAME_H

#include <memory>

#include <memory>
#include <vector>

#include "vtkSmartPointer.h"
#include "vtkRenderer.h"

namespace vtkfig
{

  class Frame;
  ///
  /// Frame: provide a thread based rendering
  /// 

  class Figure;
  class MainThread;

  class Frame
  {
  public:



    Frame(const int nrow, const int ncol);

    ~Frame();
    
    void Dump(std::string fname);
        
    void AddFigure(std::shared_ptr<Figure> figure, int irow, int icol);

    void AddFigure(std::shared_ptr<Figure> figure) {AddFigure(figure,0,0);}


    void Resize(int x, int y);

    void Reposition(int x, int y);

    std::vector<std::shared_ptr<Figure>> figures;
    




    int framenum=-1;



    static constexpr const char* keyboard_help=
R"(
--------------------------------------
Key     Action

space  Interrupt/continue calculation
r      Reset camera
w      Wireframe modus
--------------------------------------
)";

    const int nrow;

    const int ncol;


    /// File name to be passed 
    std::string fname; 

    /// window sizes
    int win_x=400;
    int win_y=400;
    
    int pos_x=0;
    int pos_y=0;
    
    /// 
    bool wireframe;

    /// 
    struct SubFrame
    {
      SubFrame(){};
      SubFrame(const double vp[4]):viewport{vp[0],vp[1],vp[2],vp[3]}{};
      double default_camera_focal_point[3]={0,0,0};
      double default_camera_position[3]={0,0,20};
      vtkSmartPointer<vtkRenderer>    renderer;
      double viewport[4]={0,0,1,1};
    };

    std::shared_ptr<MainThread> mainthread;
    
    /// Each subframe can hold several figures

    std::vector<SubFrame> subframes;

    vtkSmartPointer<vtkRenderWindow> window;
    
    int pos(const int irow, const int icol) { return irow*ncol+icol;}
    int row(const int pos) { return pos/ncol;}
    int col(const int pos) { return pos%ncol;}

    friend class  InteractorStyleTrackballCamera;
    friend class  TimerCallback;
  };
}

#endif

