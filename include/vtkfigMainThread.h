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
    friend class TimerCallback;
    friend class InteractorStyleTrackballCamera;


  public:
    ~MainThread();
    MainThread();



  private:

    static MainThread * CreateMainThread();
    void Show();
    void Interact();
    void Terminate(void);
    void AddFrame(Frame *frame);
    void RemoveFrame(Frame *frame);
    void Start();


    std::map<int,Frame*> framemap;
    static MainThread *mainthread;


    static void RenderThread(MainThread*);
    static void CommunicatorThread(MainThread*);

    bool connection_open=false;
    void OpenConnection(int port, int wtime);
    vtkSmartPointer<Communicator> communicator;
    int debug_level=0;

    static void RTAddFrame(MainThread* mt, int iframe);
    std::shared_ptr<std::thread> thread;
    int lastframenum=0;



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
