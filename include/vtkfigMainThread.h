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
  class  MainThread
  {
    friend class Frame;
    friend class Client;
    friend  class TimerCallback;
    friend class  InteractorStyleTrackballCamera;


  public:
    ~MainThread();
    MainThread();



  private:


    static MainThread * CreateMainThread();
    std::map<int, std::shared_ptr<Frame>> framemap;
    static MainThread *mainthread;
    void Show();
    void Interact();
    void Terminate(void);


    static void RenderThread(MainThread*);
    static void CommunicatorThread(MainThread*);
    bool connection_open=false;
    void OpenConnection(int port, int wtime);
    vtkSmartPointer<Communicator> communicator;
    int debug_level=0;

    static void RTAddFrame(MainThread* mt, int iframe);
    std::shared_ptr<std::thread> thread;
    int lastframenum=0;
    


    static std::shared_ptr<MainThread> New(){return std::make_shared<MainThread>();}

    void AddFrame(std::shared_ptr<Frame>);

    void RemoveFrame(std::shared_ptr<Frame> frame);
    void Start();



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


    /// Communication command
    Communicator::Command cmd; 




    void SendCommand(int iframe, const std::string from,Communicator::Command cmd);

    static void DeleteMainThread();




  };
}
