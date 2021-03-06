#include <cassert>

#include <vtkTextProperty.h>
#include <vtkPropCollection.h>
#include <vtkCamera.h>
#include <vtkRenderWindow.h>

#include "config.h"
#include "vtkfigFrame.h"
#include "vtkfigFigure.h"

#include "internals/vtkfigThread.h"

namespace vtkfig
{

  /////////////////////////////////////
  /// Public API 
  Frame::Frame()
    {
      this->nvpx=1;
      this->nvpy=1;
      figures.clear();
      subframes.resize(nvpx*nvpy);
      RTCalculateViewports(nvpx, nvpy);
      
      title_subframe.viewport[0]=0;
      title_subframe.viewport[1]=0.925;
      title_subframe.viewport[2]=1.0;
      title_subframe.viewport[3]=1.0;
    }

  std::shared_ptr<Frame> Frame::New()
  {
    // Trick from:  https://stackoverflow.com/a/25069711
    struct make_shared_enabler : public Frame {};
    auto frame= std::make_shared<make_shared_enabler>();
    frame->mainthread=internals::Thread::CreateThread();
    frame->mainthread->AddFrame(frame);
    frame->SetActiveSubFrame(0);
    return frame;
  }

  
  void Frame::Show() { mainthread->Show(); step_number++;}

  void Frame::Interact() {mainthread->Show();  mainthread->Interact();step_number++;}

  void Frame::SetAutoLayout(int nfig)
  {
    int nrow=0;
    int ncol=0;
    switch(nfig)
    {
    case 1: ncol=1; nrow=1; break;
    case 2: ncol=2; nrow=1; break;
    case 3: ncol=3; nrow=1; break;
    case 4: ncol=2; nrow=2; break;
    case 5: ncol=3; nrow=2; break;
    case 6: ncol=3; nrow=2; break;
    case 7: ncol=3; nrow=3; break;
    case 8: ncol=3; nrow=3; break;
    case 9: ncol=3; nrow=3; break;
    case 10: ncol=4; nrow=3; break;
    case 11: ncol=4; nrow=3; break;
    case 12: ncol=4; nrow=3; break;
    case 13: ncol=4; nrow=4; break;
    case 14: ncol=4; nrow=4; break;
    case 15: ncol=4; nrow=4; break;
    case 16: ncol=4; nrow=4; break;
    default:
      throw std::runtime_error("Currently not more than 16 subframes in frame");
    }
    assert(nfig<=ncol*nrow);
    SetLayout(ncol,nrow);
  }


  void Frame::AddFigure(std::shared_ptr<Figure> fig, int ipos)
  {
    if (ipos>=nvpx*nvpy)
      SetAutoLayout(ipos+1);
    
    this->figures.insert(fig);
    fig->framepos=ipos;
    fig->framenum=this->number_in_frame_list;
    parameter.current_figure=fig;
    SendCommand("AddFigure", internals::Communicator::Command::FrameAddFigure);
  }

  void Frame::AddFigure(std::shared_ptr<Figure> figure)
  {
    this->AddFigure(figure, 0);
  };

  void Frame::RemoveFigure(std::shared_ptr<Figure> fig)
  {
    this->figures.erase(fig);
    parameter.current_figure=fig;
    SendCommand("RemoveFigure", internals::Communicator::Command::FrameRemoveFigure);
  }

  Frame& Frame::FindFrame(int number_in_framelist)
  {
    return internals::Thread::FindFrame(number_in_framelist);
  }

  
  // void Frame::SetFigureState(int framepos)
  // {
  //   //xxx
  // }
  
  void Frame::SetActiveSubFrame(int ipos)
  {
    if (ipos>=nvpx*nvpy)
      SetAutoLayout(ipos+1);
    
    parameter.active_subframe=ipos;
    SendCommand("ActiveSubframe", internals::Communicator::Command::FrameActiveSubFrame);
  }

  void Frame::SetActiveSubFrameCameraViewAngle(double a)
  {
    parameter.camera_view_angle=a+this->default_camera_view_angle;
    SendCommand("ViewAngle", internals::Communicator::Command::FrameActiveSubFrameCameraViewAngle);
  }

  ///
  ///  Set view angle for camera in active frame
  ///
  void Frame::SetActiveSubFrameCameraFocalPoint(double x, double y, double z)
  {
    parameter.camera_focal_point[0]=x+this->default_camera_focal_point[0];
    parameter.camera_focal_point[1]=y+this->default_camera_focal_point[1];
    parameter.camera_focal_point[2]=z+this->default_camera_focal_point[2];
    SendCommand("CameraFocalPoint", internals::Communicator::Command::FrameActiveSubFrameCameraFocalPoint);
  }

  ///
  ///  Set view angle for camera in active frame
  ///
  void Frame::SetActiveSubFrameCameraPosition(double x, double y, double z)
  {
    parameter.camera_position[0]=x+this->default_camera_position[0];
    parameter.camera_position[1]=y+this->default_camera_position[1];
    parameter.camera_position[2]=z+this->default_camera_position[2];
    SendCommand("CameraPosition", internals::Communicator::Command::FrameActiveSubFrameCameraPosition);
  }



  void Frame::LinkCamera(int ivp, Frame& frame, int livp)
  {
    parameter.camlinkthisframepos=ivp;
    parameter.camlinkframepos=livp;
    parameter.camlinkframenum=frame.number_in_frame_list;
    SendCommand("LinkCamera", internals::Communicator::Command::FrameLinkCamera);
  }
  
  void Frame::RefreshState(int figurepos)
  {
    parameter.figurepos=figurepos;
    SendCommand("RefreshState", internals::Communicator::Command::FrameRefreshState);
  }

  void Frame::WritePNG(std::string fname)
  {
    parameter.filename=fname;
    SendCommand("Dump", internals::Communicator::Command::FrameDump);
  }

  void Frame::StartVideo(std::string fname)
  {
    parameter.filename=fname;
    SendCommand("StartVideo", internals::Communicator::Command::StartVideo);
  }

  void Frame::StopVideo()
  {
    SendCommand("StopVideo", internals::Communicator::Command::StopVideo);
  }



  void Frame::SetSize(int x, int y)
  {
    parameter.winsize_x=x;
    parameter.winsize_y=y;
    SendCommand("Size", internals::Communicator::Command::FrameSize);
  }


  void Frame::SetPosition(int x, int y)
  {
    parameter.winposition_x=x;
    parameter.winposition_y=y;
    SendCommand("Position", internals::Communicator::Command::FramePosition);
  }

  void Frame::SetWindowTitle(const std::string title)
  {
    parameter.wintitle=title;
    SendCommand("WindowTitle", internals::Communicator::Command::WindowTitle);
  }

  void Frame::SetFrameTitle(const std::string title)
  {
    parameter.frametitle=title;
    SendCommand("FrameTitle", internals::Communicator::Command::FrameTitle);
  }


  void Frame::SetLayout(int xnvpx, int xnvpy)
  {
    parameter.nvpx=xnvpx;
    parameter.nvpy=xnvpy;
    SendCommand("Layout", internals::Communicator::Command::FrameLayout);
  }


  void Frame::SetSingleSubFrameView(bool view)
  {
    parameter.single_subframe_view=view;
    SendCommand("SingleView", internals::Communicator::Command::FrameSingleView);
  }

  
  void Frame::SendCommand(std::string source, internals::Communicator::Command comm)
  {
    mainthread->SendCommand(number_in_frame_list, source, comm);
  }

  void Frame::Unmap()
  {
    if (number_in_frame_list>=0)
    {
      mainthread->RemoveFrame(number_in_frame_list);
      number_in_frame_list=-1;
    }
  }
  
  Frame::~Frame()
  {
    Unmap();      
    if (mainthread->debug_level>0)
      std::cout << "vtkfig: delete frame" << std::endl;
  }
  
  
  void Frame::Clear(void)
  {
    auto figures=this->figures;
    for (auto figure: figures)
      this->RemoveFigure(figure);
  }


  /// part of frame init from render thread
  void Frame::RTInit()
  {
    if (!this->title_actor)
    {
      this->title_actor= vtkSmartPointer<vtkCornerAnnotation>::New();
      auto textprop=this->title_actor->GetTextProperty();
      textprop->ItalicOff();
      textprop->BoldOn();
      textprop->SetFontSize(10);
      textprop->SetFontFamilyToCourier();
      textprop->SetColor(0,0,0);
    }


  }
  
  /// Set subframe layout
  void Frame::RTSetLayout(int xnvpx, int xnvpy)
  {
    this->nvpx=xnvpx;
    this->nvpy=xnvpy;
    subframes.resize(this->nvpx*this->nvpy);
    if (single_subframe_view)
      RTSetSingleViewport(nvpx,nvpy);
    else
      RTCalculateViewports(nvpx, nvpy);

  }


  /// Hide subframe
  void Frame::RTHideSubframe(SubFrame &subframe)
  {
    subframe.hidden=true;
  }
  
  /// Unhide subframe
  void Frame::RTUnHideSubframe(SubFrame &subframe)
  {
    subframe.hidden=false;
  }
  
  /// Set visible subframe
  void Frame::RTSetActiveSubFrame(int isub, bool hide_old)
  {
    if (!single_subframe_view) return;
    if (hide_old)  this->RTHideSubframe(this->subframes[this->active_subframe]);
    this->active_subframe=(isub+this->subframes.size())%(this->subframes.size());
    this->RTUnHideSubframe(this->subframes[this->active_subframe]);
    this->RTResetRenderers(false);
  }

  /// Calculate viewports for multi subframe view
  void Frame::RTCalculateViewports(int xnvpx, int xnvpy)
  {
    double dy= 0.925/(double)xnvpy;
    double dx= 1.0/(double)xnvpx;
    double y=0.0;
    for (int ivpy=0;ivpy<xnvpy;ivpy++, y+=dy)
    {
      double x=0.0;
      for (int ivpx=0 ;ivpx<xnvpx;ivpx++, x+=dx)
      {
        int ipos=pos(ivpx,ivpy);
        
        assert(this->ivpx(ipos)==ivpx);
        assert(this->ivpy(ipos)==ivpy);
        
        auto & subframe=subframes[ipos];
        subframe.viewport[0]=x;
        subframe.viewport[1]=y;
        subframe.viewport[2]=x+dx;
        subframe.viewport[3]=y+dy;
      }
    }
  }

  /// Calculate viewports for single subframe view
  void Frame::RTSetSingleViewport(int xnvpx, int xnvpy)
  {
    for (int ivpy=0;ivpy<xnvpy;ivpy++)
      for (int ivpx=0 ;ivpx<xnvpx;ivpx++)
      {
        size_t ipos=pos(ivpx,ivpy);
        auto & subframe=subframes[ipos];
        subframe.viewport[0]=0;
        subframe.viewport[1]=0;
        subframe.viewport[2]=1;
        subframe.viewport[3]=0.925;
        if (ipos==this->active_subframe)
          subframe.hidden=false;
        else
          subframe.hidden=true;

      }

  }

  
  /// reset camera to default position
  void Frame::RTResetCamera(SubFrame& subframe)
  {
    subframe.renderer->GetActiveCamera()->SetPosition(this->default_camera_position);
    subframe.renderer->GetActiveCamera()->SetFocalPoint(this->default_camera_focal_point);
    subframe.renderer->GetActiveCamera()->OrthogonalizeViewUp();
    subframe.renderer->GetActiveCamera()->SetRoll(0);
//    subframe.renderer->GetActiveCamera()->SetObliqueAngles(45,90);
//    subframe.renderer->GetActiveCamera()->Zoom(subframe.default_camera_zoom);
    subframe.renderer->GetActiveCamera()->SetViewAngle(this->default_camera_view_angle);
    subframe.renderer->ResetCamera(); // without this, we get an empty fig and need to click in
    // it calculates some bounding box etc.
  }

  void Frame::RTSetActiveSubFrameCameraViewAngle(SubFrame & subframe, double a)
  {
    subframe.renderer->GetActiveCamera()->SetViewAngle(a);
  }

  void Frame::RTSetActiveSubFrameCameraFocalPoint(SubFrame & subframe, double a[3])
  {
    subframe.renderer->GetActiveCamera()->SetFocalPoint(a);
  }

  void Frame::RTSetActiveSubFrameCameraPosition(SubFrame & subframe, double a[3])
  {
    subframe.renderer->GetActiveCamera()->SetPosition(a);
  }

  /// reset all renderers
  void Frame::RTResetRenderers(bool from_scratch)
  {
    if (from_scratch)
    {
       title_subframe.renderer = vtkSmartPointer<vtkRenderer>::New();
       title_subframe.renderer->SetBackground(1., 1., 1.);
       RTResetCamera(title_subframe);
       title_subframe.renderer->SetViewport(title_subframe.viewport);
       this->window->AddRenderer(title_subframe.renderer);
    }

    for (auto & subframe : this->subframes)
    {
      if (subframe.renderer)
      {
//        subframe.renderer->Clear(); crashes somehow, but seems to be not relevant
        if (!subframe.hidden)
          this->window->RemoveRenderer(subframe.renderer);
      }
      else
      {
        subframe.renderer = vtkSmartPointer<vtkRenderer>::New();
        subframe.renderer->SetBackground(1., 1., 1.);
        RTResetCamera(subframe);
      }
      subframe.renderer->SetViewport(subframe.viewport);
      this->window->AddRenderer(subframe.renderer);
      if (subframe.hidden)
        this->window->RemoveRenderer(subframe.renderer);

    }
  }

  void Frame::RTSetSingleView(bool single_view)
  {
    single_subframe_view=single_view;
    if (single_view)
    {
     this->RTSetSingleViewport(this->nvpx, this->nvpy);
      for (size_t i=0;i<this->subframes.size();i++)
        if (i!=this->active_subframe)
          this->RTHideSubframe(this->subframes[i]);
    }
    else
    {
      this->RTCalculateViewports(this->nvpx, this->nvpy);
      for (size_t i=0;i<this->subframes.size();i++)
        this->RTUnHideSubframe(this->subframes[i]);
    }
    this->RTResetRenderers(false);
    RTAddFigures();
  }
  
  void Frame::RTRemoveFigure(std::shared_ptr<Figure> figure)
  {
    if (figure==nullptr) return;
    auto &subframe=this->subframes[figure->framepos];

    // It needs to be called here. We need to see how this can
    // be handled in an intuitive way.
    // RTResetCamera(subframe);


    auto &renderer=subframe.renderer;

    for (auto & actor: figure->actors) 
      renderer->RemoveActor(actor);

    for (auto & actor: figure->ctxactors) 
      renderer->RemoveActor(actor);
    
    for (auto & actor: figure->actors2d) 
      renderer->RemoveActor(actor);
    
  }
  

  void Frame::RTAddFigures()
  {
    for (auto & figure: this->figures)
    {
      if (!this->subframes[figure->framepos].hidden)
      {
        auto &renderer=this->subframes[figure->framepos].renderer;
        
        if (
          figure->IsEmpty()  
          || renderer->GetActors()->GetNumberOfItems()==0
          )
        {
          figure->RTBuildAllVTKPipelines(renderer);
          
          for (auto & actor: figure->actors) 
            renderer->AddActor(actor);
          
          for (auto & actor: figure->ctxactors) 
            renderer->AddActor(actor);

          for (auto & actor: figure->actors2d) 
            renderer->AddActor(actor);
          
        }
        
        figure->RTPreRender();
        figure->RTUpdateActors();
      
        renderer->SetBackground(figure->bgcolor[0],
                                figure->bgcolor[1],
                                figure->bgcolor[2]);
        
      }
    }
  }



  vtkSmartPointer<vtkRenderer> Frame::GetRenderer(std::shared_ptr<Figure> fig)
    {
      
     return this->subframes[fig->framepos].renderer;
    }


}
