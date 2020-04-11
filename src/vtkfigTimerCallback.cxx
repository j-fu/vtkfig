#include <vtkRenderWindow.h>
#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>

#include "internals/vtkfigThread.h"
#include "internals/vtkfigTimerCallback.h"

namespace vtkfig
{
  namespace internals
  {
      
    void TimerCallback::Execute( vtkObject *vtkNotUsed(caller), unsigned long eventId, void *vtkNotUsed(callData))
    {
      
      if (this->mainthread->communication_blocked) return;
      
      if (eventId ==  vtkCommand::TimerEvent)
      {
        if (this->mainthread->cmd==Communicator::Command::Empty)
          return;
        
        // Lock mutex
        if (this->mainthread->running_multithreaded)
          std::unique_lock<std::mutex> lock(this->mainthread->mutex);
        
        // Command dispatch
        switch(this->mainthread->cmd)
          {
            // Add frame to main thread
          case Communicator::Command::RenderThreadAddFrame:
          {
            this->mainthread->RTAddFrame(*mainthread,this->mainthread->iframe);
          }
          break;
          
          // Remove frame from mainthread
          case Communicator::Command::RenderThreadRemoveFrame:
          {
            
            auto &frame=*mainthread->framemap[mainthread->iframe];
            for (auto & subframe: frame.subframes)
              subframe.renderer->RemoveAllViewProps();
            
            frame.window->Finalize();
            std::unique_lock<std::mutex> lock(this->mainthread->mutex);
            mainthread->framemap.erase(frame.number_in_frame_list);
          }
          break;
          
          
          
          case Communicator::Command::RenderThreadShow:
          case Communicator::Command::RenderThreadShowAndBlock:
          {
            // Add actors from figures to renderer
            for (auto & framepair: mainthread->framemap)
              framepair.second->RTAddFigures();
            
            if (mainthread->cmd==Communicator::Command::RenderThreadShowAndBlock)
              mainthread->communication_blocked=true;
            
            for (auto & framepair: mainthread->framemap)
            {
              auto &frame=*framepair.second;

              if(frame.mainthread->communication_blocked)
                frame.title_actor->SetText(6,"-|-");
              else
                frame.title_actor->SetText(6,"---");

              frame.title_actor->Modified();

              // for (auto & figure: framepair.second->figures)
              //   figure->RTUpdateActors();
              frame.window->Render();
              if (frame.videowriter && ! frame.mainthread->communication_blocked)
              {
                this->Interactor->Render();
                auto imgfilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
                imgfilter->SetInput(frame.window);
                imgfilter->Update();
                frame.videowriter->SetInputConnection(imgfilter->GetOutputPort());
                this->Interactor->Render();
                frame.videowriter->Write();
              }
            }
            //this->Interactor->Render();
          }
          break;
        
          // Write picture to file
          case Communicator::Command::FrameDump:
          {
            auto &frame=*mainthread->framemap[mainthread->iframe];
            auto imgfilter = vtkSmartPointer<vtkWindowToImageFilter>::New();
            auto pngwriter = vtkSmartPointer<vtkPNGWriter>::New();
          
            pngwriter->SetInputConnection(imgfilter->GetOutputPort());
            pngwriter->SetFileName(frame.parameter.filename.c_str());
          
            imgfilter->SetInput(frame.window);
            imgfilter->Update();
          
            this->Interactor->Render();
            pngwriter->Write();
          }
          break;

          // Start video
          case Communicator::Command::StartVideo:
          {
            auto &frame=*mainthread->framemap[mainthread->iframe];

            frame.videowriter =  vtkSmartPointer<vtkOggTheoraWriter>::New();
            frame.videowriter->SetFileName(frame.parameter.filename.c_str());
            frame.videowriter->Start();
          }
          break;

          case Communicator::Command::StopVideo:
          {
            auto &frame=*mainthread->framemap[mainthread->iframe];
            frame.videowriter->End();
            frame.videowriter=0;
          }
          break;

          case Communicator::Command::FrameRemoveFigure:
          {
            auto &frame=*mainthread->framemap[mainthread->iframe];
            frame.RTRemoveFigure(frame.parameter.current_figure);
          }
          break;
          
          case Communicator::Command::FrameClear:
          {
            auto &frame=*mainthread->framemap[mainthread->iframe];
            for (auto & figure: frame.figures)
            {
              frame.RTRemoveFigure(figure);
            }
            frame.figures.clear();
          }
          break;
          

          case Communicator::Command::FrameLayout:
          {
            auto &frame=*mainthread->framemap[mainthread->iframe];
            frame.RTSetLayout(frame.parameter.nvpx,frame.parameter.nvpy); 
            frame.RTResetRenderers(false);
          }


          case Communicator::Command::FrameRefreshState:
          {                
            auto &frame=*mainthread->framemap[mainthread->iframe];
            for (auto & figure: frame.figures)
            {
              if (figure->framepos==frame.parameter.figurepos)
              {
                figure->RTRefreshState();
              }
            }
          }
          
          
          // Set frame size
          case Communicator::Command::FrameSize:
          {
            auto &frame=*mainthread->framemap[mainthread->iframe];
            frame.window->SetSize(frame.parameter.winsize_x, frame.parameter.winsize_y);
          }
          break;
        
          // Set active subframe
          case Communicator::Command::FrameActiveSubFrame:
          {
            auto &frame=*mainthread->framemap[mainthread->iframe];
            frame.RTSetActiveSubFrame(frame.parameter.active_subframe,false);
          }
          break;

          // Set active subframe
          case Communicator::Command::FrameActiveSubFrameCameraViewAngle:
          {
            auto &frame=*mainthread->framemap[mainthread->iframe];
            auto& subframe=frame.subframes[frame.parameter.active_subframe];
            frame.RTSetActiveSubFrameCameraViewAngle(subframe,frame.parameter.camera_view_angle);
          }
          break;
        
          // Set active subframe
          case Communicator::Command::FrameActiveSubFrameCameraPosition:
          {
            auto &frame=*mainthread->framemap[mainthread->iframe];
            auto& subframe=frame.subframes[frame.parameter.active_subframe];
            frame.RTSetActiveSubFrameCameraPosition(subframe,frame.parameter.camera_position);
          }
          break;
        
          // Set active subframe
          case Communicator::Command::FrameActiveSubFrameCameraFocalPoint:
          {
            auto &frame=*mainthread->framemap[mainthread->iframe];
            auto& subframe=frame.subframes[frame.parameter.active_subframe];
            frame.RTSetActiveSubFrameCameraFocalPoint(subframe,frame.parameter.camera_focal_point);
          }
          break;
        
          case Communicator::Command::FrameSingleView:
          {
            auto &frame=*mainthread->framemap[mainthread->iframe];
            frame.RTSetSingleView(frame.parameter.single_subframe_view);
          }
          break;
        


          // Set frame position
          case Communicator::Command::FramePosition:
          {
            auto &frame=*mainthread->framemap[mainthread->iframe];
            frame.window->SetPosition(frame.parameter.winposition_x, frame.parameter.winposition_y);
          }
          break;

          // Set frame title
          case Communicator::Command::FrameTitle:
          {
            auto &frame=*mainthread->framemap[mainthread->iframe];
            frame.RTInit();
            frame.title_actor->SetText(7,frame.parameter.frametitle.c_str());
            frame.title_actor->Modified();
            
          }
          break;

          // Set window title
          case Communicator::Command::WindowTitle:
          {
            auto &frame=*mainthread->framemap[mainthread->iframe];
            frame.window->SetWindowName(frame.parameter.wintitle.c_str());
          }
          break;

          // Link camera to other
          case Communicator::Command::FrameLinkCamera:
          {
            auto &frame=*mainthread->framemap[mainthread->iframe];
            auto renderer=mainthread->framemap[mainthread->iframe]->subframes[frame.parameter.camlinkthisframepos].renderer;
            auto lrenderer=mainthread->framemap[frame.parameter.camlinkframenum]->subframes[frame.parameter.camlinkframepos].renderer;
            renderer->SetActiveCamera(lrenderer->GetActiveCamera());
          }
          break;

          // Terminate
          case Communicator::Command::RenderThreadTerminate:
          {

            if (mainthread->debug_level>0)
              std::cout << "vtkfig: Start termination" << std::endl;            
            
            mainthread->interactor->TerminateApp();
            return;
          }
          break;
        
          default:;
          }
        
          // Clear command 
          mainthread->cmd=Communicator::Command::Empty;
        
          // Notify that command was exeuted
          if (mainthread->running_multithreaded)
            // 
            mainthread->condition_variable.notify_all();
          else
            // dirty trick to unblock event loop.
            // hopefully works if multitreading does not
            mainthread->interactor->TerminateApp();
        
        } // if eventid
      } // Execute
  }
}
