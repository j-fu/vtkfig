#include <memory>
#include <mutex>
#include <thread>
#include <memory>
#include <map>
#include <condition_variable>

#include "vtkfigCommunicator.h"


namespace vtkfig
{

  class Frame;

  class  MainThread: public std::enable_shared_from_this<MainThread>
  {
    static void RenderThread(std::shared_ptr<MainThread>);
    static void CommunicatorThread(std::shared_ptr<MainThread>);
    bool connection_open=false;
    void OpenConnection(int port, int wtime);
    vtkSmartPointer<Communicator> communicator;

  public:
    static void RTAddFrame(std::shared_ptr<MainThread>, int iframe);
    std::shared_ptr<std::thread> thread;
    std::map<int, std::shared_ptr<Frame>> framemap;
    int lastframenum=0;
    std::shared_ptr<Frame> GetFrame(int key) {return framemap[key];}


    MainThread();

    static std::shared_ptr<MainThread> New(){return std::make_shared<MainThread>();}

    std::shared_ptr<Frame>  AddFrame(int nrow, int ncol);
    std::shared_ptr<Frame>  AddFrame() { return AddFrame(1,1);}

    void RemoveFrame(std::shared_ptr<Frame> frame);
    void Start();
    void Terminate(void);



    /// mutex to organize communication
    std::mutex mtx; 

    /// condition variable signalizing finished command
    std::condition_variable cv; 
        
    /// Thread state
    bool thread_alive=false;
    
    /// space down state ?
    bool communication_blocked=false;


    /// 
    int iframe=-1;

    enum class Command
    {
      None=0,
        Show=100,
        Dump,            
        Resize,            
        AddFigure,            
        AddFrame,            
        Reposition,
        Clear,            
        Terminate,
        SetBackgroundColor          
    };

    /// Communication command
    Command cmd; 


    void Show();

    void Interact();

    void SendCommand(int iframe, const std::string from,Command cmd);
    
    ~MainThread()
    {
      Terminate();
      this->thread->join();
    }
  };
}
