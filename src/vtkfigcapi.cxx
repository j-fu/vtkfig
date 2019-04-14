#include "vtkfigcapi.h"
#include "vtkfigFrame.h"
#include "vtkfigDataSet.h"
#include "vtkfigScalarView.h"

template<typename T>
class adapter
{
  const int n;
  T* d;
public:
  adapter(T*d, int n): d(d), n(n){};
  T&operator[](int i)const { return d[i];}
  int size() const { return n;}
};


extern "C"
{
  struct vtkfigFrame_struct 
  {
    std::shared_ptr<vtkfig::Frame> frame;
  };
  
  
  vtkfigFrame* vtkfigCreateFrame(void)
  {
    vtkfigFrame* frame=reinterpret_cast<vtkfigFrame*>(malloc(sizeof(vtkfigFrame)));
    frame->frame=vtkfig::Frame::New();
    return frame;
  }
  
  void vtkfigDestroyFrame(vtkfigFrame *frame)
  {
    frame->frame=nullptr;
    free(frame);
  }


  struct vtkfigDataSet_struct 
  {
    std::shared_ptr<vtkfig::DataSet> dataset;
  };
  
  vtkfigDataSet* vtkfigCreateDataSet(void)
  {
    vtkfigDataSet* dataset=reinterpret_cast<vtkfigDataSet*>(malloc(sizeof(vtkfigDataSet)));
    dataset->dataset=vtkfig::DataSet::New();
    return dataset;
  }
  
  void vtkfigDestroyDataSet(vtkfigDataSet *dataset)
  {
    dataset->dataset=nullptr;
    free(dataset);
  }

  struct vtkfigScalarView_struct 
  {
    std::shared_ptr<vtkfig::ScalarView> scalarview;
  };
  
  vtkfigScalarView* vtkfigCreateScalarView(void)
  {
    vtkfigScalarView* scalarview=reinterpret_cast<vtkfigScalarView*>(malloc(sizeof(vtkfigScalarView)));
    scalarview->scalarview=vtkfig::ScalarView::New();

    auto colors=vtkfig::RGBTable
      { 
        {0.0, 0.0, 0.0, 1.0},
        {0.5, 0.0, 1.0, 0.0},
        {1.0, 1.0, 0.0, 0.0}
      };
    scalarview->scalarview->SetSurfaceRGBTable(colors,255);
    return scalarview;
  }
  
  void vtkfigDestroyScalarView(vtkfigScalarView *scalarview)
  {
    scalarview->scalarview=nullptr;
    free(scalarview);
  }


  
    
  void vtkfigSetRectilinearGrid(vtkfigDataSet*dataset,double *x, int nx, double *y, int ny)
  {
    auto X=adapter<double>(x,nx);
    auto Y=adapter<double>(y,ny);
    dataset->dataset->SetRectilinearGrid(X,Y);
  }

  void vtkfigSetPointScalar(vtkfigDataSet*dataset,double *x, int nx, char *name)
  {
    auto X=adapter<double>(x,nx);
    dataset->dataset->SetPointScalar(X,name);
  }

  void vtkfigSetData(vtkfigScalarView*scalarview, vtkfigDataSet*dataset, char *name)
  {
      scalarview->scalarview->SetData(dataset->dataset,"V");
  }

  void vtkfigAddScalarView(vtkfigFrame*frame, vtkfigScalarView *scalarview)
  {
    frame->frame->AddFigure(scalarview->scalarview);
  }

  void vtkfigShow(vtkfigFrame*frame)
  {
    frame->frame->Show();
  }

  void vtkfigInteract(vtkfigFrame*frame)
  {
    frame->frame->Interact();
  }
  
}
