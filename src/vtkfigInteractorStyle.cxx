#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkContextScene.h>
#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>

#include "internals/vtkfigThread.h"
#include "internals/vtkfigInteractorStyle.h"

namespace vtkfig
{
  namespace internals
  {
    void InteractorStyle::SetFrame(Frame* xframe)
    {
      this->frame=xframe;
    }
    
    void InteractorStyle::OnConfigure()
    {
      for (auto & figure: frame->figures)
      {
        for (auto & actor: figure->ctxactors) 
          actor->GetScene()->SetDirty(true);
      }
      vtkInteractorStyleTrackballCamera::OnConfigure();        
    }
    
    /// Overwrite left button down
    void InteractorStyle::OnLeftButtonDown()
    {
      
      
      /// If edit mode is active,
      /// catch mouse position, otherwisw pass to base class
      if (this->edit_mode)
      {
        this->left_button_down=true;
        lastx=this->Interactor->GetEventPosition()[0];
        lasty=this->Interactor->GetEventPosition()[1];
      }
      else
        vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
    }
    
    /// Overwrite left button up
    void InteractorStyle::OnLeftButtonUp()
    {
      for (auto &figure: frame->figures)
      {
        
        if (frame->subframes[figure->framepos].renderer==this->CurrentRenderer)
          figure->RTShowActive();
        else
          figure->RTShowInActive();
        frame->window->Render();
      }
      
      /// If button was down, release, else pass to base class
      if (this->left_button_down)
        this->left_button_down=false;
      else
        vtkInteractorStyleTrackballCamera::OnLeftButtonUp();
    }
    
    /// Overwrite  mouse move
    void InteractorStyle::OnMouseMove()
    {
      
      
      /// If button is down, act, else pass to base class
      if (this->left_button_down)
      {
        // Calculate difference between old and
        // new mouse position
        int thisx=this->Interactor->GetEventPosition()[0];
        int thisy=this->Interactor->GetEventPosition()[1];
        int dx=thisx-lastx;
        int dy=thisy-lasty;
        
        // Pass to all edited figures
        for (auto figure:this->edited_figures) 
          figure->RTProcessMove(dx,dy);
        
        // Render changed figure
        this->Interactor->Render();
        
        // Set new old position
        lastx=thisx;
        lasty=thisy;
      }
      else
        vtkInteractorStyleTrackballCamera::OnMouseMove();
    }
    
    ///  Overwrite keyboard callback
    void InteractorStyle::OnChar() 
    {
      // Get the key pressed
      std::string key = this->Interactor->GetKeySym();
      
      //cout << key << endl;
      
      // disable some standard vtk keys
      if(key== "f")  {}
      
      // ctrl-q -> exit
      else if(key == "q")
      {
        if (this->Interactor->GetControlKey())
        {
          this->Interactor->TerminateApp();
#if __APPLE__ || __MINGW32__
          _exit(0);
#else
          std::quick_exit(0);
#endif
        }
      }
      
      // Reset Camera
      else if(key == "r")
      {
        for (auto & subframe: this->frame->subframes)
        {
          if (subframe.renderer==this->CurrentRenderer)
          {
            frame->RTResetCamera(subframe);
          }
        }
        this->Interactor->Render();
      }
      
      // Toggle wireframe
      else if(key == "w")
      {
        for (auto &figure: this->frame->figures)
          if (this->frame->subframes[figure->framepos].renderer==this->CurrentRenderer)
          {
            figure->state.wireframe=!figure->state.wireframe;
            if (figure->state.wireframe)
              for (auto & actor: figure->actors)  actor->GetProperty()->SetRepresentationToWireframe();
            else
              for (auto&  actor: figure->actors)  actor->GetProperty()->SetRepresentationToSurface();
            this->Interactor->Render();
          }
        
      }
      
      // Write output to png
      else if (key=="p")
      {
        
        // Generate file name using current time
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::tm * ttm = localtime(&time);
        char time_str[] = "yyyy-mm-ddTHH:MM:SS ";
        strftime(time_str, strlen(time_str), "%Y-%m-%dT%H:%M:%S", ttm);
        auto fname=frame->parameter.wintitle+"-"+time_str+".png";
        
        auto imgfilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
        imgfilter->SetInput(frame->window);
        imgfilter->Update();
        
        auto pngwriter = vtkSmartPointer<vtkPNGWriter>::New();
        pngwriter->SetInputConnection(imgfilter->GetOutputPort());
        pngwriter->SetFileName(fname.c_str());
          
        this->Interactor->Render();
        pngwriter->Write();

        cout << "Frame written to "<< fname << endl;
      }
      
      // Keys which toggle editing
      else if (key == "x" || key== "y" || key== "z" || key== "l" || key== "a")
      {      
        if (!this->edit_mode)
        {
          
          this->edit_mode=true;
          /// Create list of currently edited figures. These
          /// are those the same subframe which means that they have
          /// the same renderer
          for (auto &figure: frame->figures)
          {
            
            if (
              frame->subframes[figure->framepos].renderer==this->CurrentRenderer
              // &&(
              //   (key=="a" && figure->SubClassName()=="Quiver")
              //   ||
              //   (key=="x" && figure->SubClassName()=="SurfaceContour")
              //   ||
              //   (key=="y" && figure->SubClassName()=="SurfaceContour")
              //   ||
              //   (key=="z" && figure->SubClassName()=="SurfaceContour")
              //   ||
              //   (key=="l" && figure->SubClassName()=="SurfaceContour")
              //  )
              )
              this->edited_figures.push_back(figure);
          }
        } 
          
        /// Pass key to edited figures
        for (auto figure : this->edited_figures)
          figure->RTProcessKey(key);
        this->Interactor->Render();
      }


      // Emulate mouse move for all edited figures
      else if(key == "Left" || key== "Down")
      {
        if (this->edit_mode)
        {
          for (auto figure : this->edited_figures)
            figure->RTProcessMove(-1,-1);
          this->Interactor->Render();
        }
      }

      // Emulate mouse move for all edited figures
      else if(key == "Right" || key== "Up")
      {
        if (this->edit_mode)
        {
          for (auto figure : this->edited_figures)
            figure->RTProcessMove(1,1);
          this->Interactor->Render();
        }
      }

      // These are interaction keys in edit mode
      else if(key == "Return" || key=="Delete")
      {
        if (this->edit_mode)
        {
          for (auto figure : this->edited_figures)
            figure->RTProcessKey(key);
          this->Interactor->Render();
        }
      }

      // Escape ends edit mode
      else if(key == "Escape")
      {
        this->edit_mode=false;
        for (auto figure : this->edited_figures)
        {
          figure->RTProcessKey(key);
          figure->RTMessage("");
        }
        this->Interactor->Render();
        this->edited_figures.resize(0);
        this->Interactor->Render();
      }
        
      // Toggle some states independent of edit mode
      else if(key == "I" || key== "L" || key == "E" || key == "S" || key == "slash" || key == "B" || key == "C" || key == "O"|| key == "A")
      {
        for (auto &figure: frame->figures)
          if (frame->subframes[figure->framepos].renderer==this->CurrentRenderer)
            figure->RTProcessKey(key);

        this->Interactor->Render();
      }

      // Block/unblock calculation
      else if (key=="space")
      {
        frame->mainthread->communication_blocked=!frame->mainthread->communication_blocked;
        if(frame->mainthread->communication_blocked)
          frame->title_actor->SetText(6,"-|-");
        else
          frame->title_actor->SetText(6,"---");
        frame->title_actor->Modified();
        frame->window->Render();
      }

      // Block/unblock calculation
      else if (key=="BackSpace")
      {
        frame->mainthread->communication_blocked=!frame->mainthread->communication_blocked;
        if (!frame->mainthread->communication_blocked)
          frame->step_number=std::max(frame->step_number-2,0);

      }
        
      else if (key=="asterisk")
      {
        if (!frame->single_subframe_view)
          for(size_t i=0;i<this->frame->subframes.size();i++)
          {
            if (this->frame->subframes[i].renderer==this->CurrentRenderer)
              this->frame->active_subframe=i;
          }
          
        frame->single_subframe_view=!frame->single_subframe_view;
        frame->RTSetSingleView(frame->single_subframe_view);
        this->Interactor->Render();
      }


      else if (frame->single_subframe_view && key=="Next")
      {
        frame->RTSetActiveSubFrame(frame->active_subframe-1,true);
        this->Interactor->Render();
      }

      else if (frame->single_subframe_view && key=="Prior")
      {
        frame->RTSetActiveSubFrame(frame->active_subframe+1,true);
        this->Interactor->Render();
      }


        
      // Print help string
      else if(key == "h" or key == "question")
      {
        cout << Frame::KeyboardHelp;
      }

      // Pass other keys to base
      else
      {
        vtkInteractorStyleTrackballCamera::OnChar();
      }
    }
  };
}
