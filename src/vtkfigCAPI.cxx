#include "vtkfigCAPI.h"
#include "vtkfigFrame.h"
#include "vtkfigTools.h"
#include "vtkfigDataSet.h"
#include "vtkfigScalarView.h"
#include "vtkfigVectorView.h"
#include "vtkfigGridView.h"
#include "vtkfigXYPlot.h"


#include <vtkSmartPointer.h>
#include <vtkRenderingOpenGLConfigure.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkVersion.h>

#include "config.h"


template<typename T>
class vector_adapter
{
  const int n;
  T* d;
public:
  vector_adapter(T*d, int n): d(d), n(n){};
  T&operator[](int i)const { return d[i];}
  int size() const { return n;}
};


extern "C"
{
  ////////////////////////////////////////////////////////////////////////
  struct vtkfigFrame_struct 
  {
     vtkfig::Frame *cxxobj;
  };
  
  vtkfigFrame* vtkfigCreateFrame(void)
  {
    vtkfigFrame* frame=reinterpret_cast<vtkfigFrame*>(malloc(sizeof(vtkfigFrame)));
    frame->cxxobj=new vtkfig::Frame();
    return frame;
  }
  
  void vtkfigDestroyFrame(vtkfigFrame *frame)
  {
    delete frame->cxxobj;
    free(frame);
  }


  ////////////////////////////////////////////////////////////////////////
  struct vtkfigDataSet_struct 
  {
    vtkfig::DataSet* cxxobj;
  };
  
  vtkfigDataSet* vtkfigCreateDataSet(void)
  {
    vtkfigDataSet* dataset=reinterpret_cast<vtkfigDataSet*>(malloc(sizeof(vtkfigDataSet)));
    dataset->cxxobj=new vtkfig::DataSet();
    return dataset;
  }
  
  void vtkfigDestroyDataSet(vtkfigDataSet *dataset)
  {
    delete dataset->cxxobj;
    free(dataset);
  }

  ////////////////////////////////////////////////////////////////////////
  struct vtkfigScalarView_struct 
  {
    vtkfig::ScalarView* cxxobj;
  };
  
  vtkfigScalarView* vtkfigCreateScalarView(void)
  {
    vtkfigScalarView* scalarview=reinterpret_cast<vtkfigScalarView*>(malloc(sizeof(vtkfigScalarView)));
    scalarview->cxxobj=new vtkfig::ScalarView();
    
    auto colors=vtkfig::RGBTable
      { 
        {0.0, 0.0, 0.0, 1.0},
        {0.5, 0.0, 1.0, 0.0},
        {1.0, 1.0, 0.0, 0.0}
      };
    scalarview->cxxobj->SetSurfaceRGBTable(colors,255);
    return scalarview;
  }
  
  void vtkfigDestroyScalarView(vtkfigScalarView *scalarview)
  {
    delete scalarview->cxxobj;
    free(scalarview);
  }
  
////////////////////////////////////////////////////////////////////////
  struct vtkfigGridView_struct 
  {
    vtkfig::GridView* cxxobj;
  };
  
  vtkfigGridView* vtkfigCreateGridView(void)
  {
    vtkfigGridView* gridview=reinterpret_cast<vtkfigGridView*>(malloc(sizeof(vtkfigGridView)));
    gridview->cxxobj=new vtkfig::GridView();
    return gridview;
  }
  
  void vtkfigDestroyGridView(vtkfigGridView *gridview)
  {
    delete gridview->cxxobj;
    free(gridview);
  }


////////////////////////////////////////////////////////////////////////
  struct vtkfigVectorView_struct 
  {
    vtkfig::VectorView* cxxobj;
  };
  
  vtkfigVectorView* vtkfigCreateVectorView(void)
  {
    vtkfigVectorView* vectorview=reinterpret_cast<vtkfigVectorView*>(malloc(sizeof(vtkfigVectorView)));
    vectorview->cxxobj=new vtkfig::VectorView();
    return vectorview;
  }
  
  void vtkfigDestroyVectorView(vtkfigVectorView *vectorview)
  {
    delete vectorview->cxxobj;
    free(vectorview);
  }

////////////////////////////////////////////////////////////////////////
  struct vtkfigXYPlot_struct 
  {
    vtkfig::XYPlot* cxxobj;
  };
  
  vtkfigXYPlot* vtkfigCreateXYPlot(void)
  {
    vtkfigXYPlot* xyplot=reinterpret_cast<vtkfigXYPlot*>(malloc(sizeof(vtkfigXYPlot)));
    xyplot->cxxobj=new vtkfig::XYPlot();
    return xyplot;
  }
  
  void vtkfigDestroyXYPlot(vtkfigXYPlot *xyplot)
  {
    delete xyplot->cxxobj;
    free(xyplot);
  }

  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////

  void vtkfigAddScalarView(vtkfigFrame*frame, vtkfigScalarView *scalarview)
  {
    frame->cxxobj->AddFigure(*(scalarview->cxxobj));
  }

  void vtkfigRemoveScalarView(vtkfigFrame*frame, vtkfigScalarView *scalarview)
  {
    frame->cxxobj->RemoveFigure(*(scalarview->cxxobj));
  }

  void vtkfigAddScalarViewAt(vtkfigFrame*frame, vtkfigScalarView *scalarview, int ipos)
  {
    frame->cxxobj->AddFigure(*(scalarview->cxxobj),ipos);
  }

  void vtkfigAddVectorView(vtkfigFrame*frame, vtkfigVectorView *vectorview)
  {
    frame->cxxobj->AddFigure(*(vectorview->cxxobj));
  }
  void vtkfigRemoveVectorView(vtkfigFrame*frame, vtkfigVectorView *vectorview)
  {
    frame->cxxobj->RemoveFigure(*(vectorview->cxxobj));
  }

  void vtkfigAddVectorViewAt(vtkfigFrame*frame, vtkfigVectorView *vectorview, int ipos)
  {
    frame->cxxobj->AddFigure(*(vectorview->cxxobj),ipos);
  }


  void vtkfigAddGridView(vtkfigFrame*frame, vtkfigGridView *gridview)
  {
    frame->cxxobj->AddFigure(*(gridview->cxxobj));
  }

  void vtkfigRemoveGridView(vtkfigFrame*frame, vtkfigGridView *gridview)
  {
    frame->cxxobj->RemoveFigure(*(gridview->cxxobj));
  }

  void vtkfigAddGridViewAt(vtkfigFrame*frame, vtkfigGridView *gridview, int ipos)
  {
    frame->cxxobj->AddFigure(*(gridview->cxxobj),ipos);
  }

  void vtkfigAddXYPlot(vtkfigFrame*frame, vtkfigXYPlot *xyplot)
  {
    frame->cxxobj->AddFigure(*(xyplot->cxxobj));
  }

  void vtkfigRemoveXYPlot(vtkfigFrame*frame, vtkfigXYPlot *xyplot)
  {
    frame->cxxobj->RemoveFigure(*(xyplot->cxxobj));
  }

  void vtkfigAddXYPlotAt(vtkfigFrame*frame, vtkfigXYPlot *xyplot, int ipos)
  {
    frame->cxxobj->AddFigure(*(xyplot->cxxobj),ipos);
  }



  void vtkfigWritePNG(vtkfigFrame*frame, char*fname)
  {
    frame->cxxobj->WritePNG(fname);
  }
  void vtkfigStartVideo(vtkfigFrame*frame, char*fname)
  {
    frame->cxxobj->StartVideo(fname);
  }
  void vtkfigStopVideo(vtkfigFrame*frame)
  {
    frame->cxxobj->StopVideo();
  }
  void vtkfigSetSize(vtkfigFrame*frame,int x, int y)
  {
    frame->cxxobj->SetSize(x,y);
  }
  void vtkfigSetLayout(vtkfigFrame*frame,int x, int y)
  {
    frame->cxxobj->SetLayout(x,y);
  }
  void vtkfigSetWindowTitle(vtkfigFrame*frame, char*title)
  {
    frame->cxxobj->SetWindowTitle(title);
  }
  void vtkfigSetFrameTitle(vtkfigFrame*frame, char*title)
  {
    frame->cxxobj->SetFrameTitle(title);
  }
  void vtkfigSetPosition(vtkfigFrame*frame,int x, int y)
  {
    frame->cxxobj->SetPosition(x,y);
  }

  void vtkfigShow(vtkfigFrame*frame)
  {
    frame->cxxobj->Show();
  }

  void vtkfigInteract(vtkfigFrame*frame)
  {
    frame->cxxobj->Interact();
  }



  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////

  
    
  void vtkfigSetRectilinearGrid(vtkfigDataSet*dataset,double *x, int nx, double *y, int ny)
  {
    auto X=vector_adapter<double>(x,nx);
    auto Y=vector_adapter<double>(y,ny);
    dataset->cxxobj->SetRectilinearGrid(X,Y);
  }

  void vtkfigSetPointScalar(vtkfigDataSet*dataset,double *x, int nx, char *name)
  {
    auto X=vector_adapter<double>(x,nx);
    dataset->cxxobj->SetPointScalar(X,name);
  }

  void vtkfigWriteVTK(vtkfigDataSet*dataset,char* filename, char* filetype)
  {
    dataset->cxxobj->WriteVTK(filename,filetype);
  }
  
  void vtkfigSetData(vtkfigScalarView*scalarview, vtkfigDataSet*dataset, char *name)
  {
    scalarview->cxxobj->SetData(*(dataset->cxxobj),"V");
  }


  void vtkfigPrintOpenGLInfo()
  {
    vtkfig::PrintOpenGLInfo();
  }
  
  ///////////////////////////////////////////////////////////////////
  void vtkfigAddPlot(vtkfigXYPlot*xyplot, double *x, int nx, double *y, int ny)
  {
    auto X=vector_adapter<double>(x,nx);
    auto Y=vector_adapter<double>(y,ny);
    xyplot->cxxobj->AddPlot(X,Y);
  }

  void vtkfigSetTitle(vtkfigXYPlot*xyplot,char *t)
  {
    xyplot->cxxobj->SetTitle(t);
  }
  void vtkfigShowLegend(vtkfigXYPlot*xyplot,int b)
  {
    xyplot->cxxobj->ShowLegend(b);
  }
  void vtkfigShowGrid(vtkfigXYPlot*xyplot,int b)
  {
    xyplot->cxxobj->ShowGrid(b);
  }

  void vtkfigAdjustLabels(vtkfigXYPlot*xyplot,int b)
  {
    xyplot->cxxobj->AdjustLabels(b);
  }
  void vtkfigSetLegendPosition(vtkfigXYPlot*xyplot,double x, double y)
  {
    xyplot->cxxobj->SetLegendPosition(x,y);
  }
  void vtkfigSetLegendSize(vtkfigXYPlot*xyplot,double w, double h)
  {
    xyplot->cxxobj->SetLegendSize(w,h);
  }

  void vtkfigClear(vtkfigXYPlot*xyplot) 
  {
    xyplot->cxxobj->Clear();
  }
  void vtkfigSetPlotLineType(vtkfigXYPlot*xyplot,char *t) 
  {
    xyplot->cxxobj->SetPlotLineType(t);
  }
  void vtkfigSetPlotMarkerType(vtkfigXYPlot*xyplot,char *t) 
  {
    xyplot->cxxobj->SetPlotMarkerType(t);
  }
  void vtkfigSetPlotColor(vtkfigXYPlot*xyplot,double r, double g, double b) 
  {
    xyplot->cxxobj->SetPlotColor(r,g,b);
  }
  void vtkfigSetPlotLegend(vtkfigXYPlot*xyplot,char *t) 
  {
    xyplot->cxxobj->SetPlotLegend(t);
  }
  void vtkfigSetGridColor(vtkfigXYPlot*xyplot,double r, double g, double b) 
  {
    xyplot->cxxobj->SetGridColor(r,g,b);
  }
  void vtkfigSetAxesColor(vtkfigXYPlot*xyplot,double r, double g, double b) 
  {
    xyplot->cxxobj->SetAxesColor(r,g,b);
  }
  void vtkfigSetXTitle(vtkfigXYPlot*xyplot,char *t) 
  {
    xyplot->cxxobj->SetXTitle(t);
  }
  void vtkfigSetYTitle(vtkfigXYPlot*xyplot,char *t) 
  {
    xyplot->cxxobj->SetYTitle(t);
  }
  
  void vtkfigSetLineWidth(vtkfigXYPlot*xyplot,double w)
  {
    xyplot->cxxobj->SetLineWidth(w);
  }
  void vtkfigSetMarkerSize(vtkfigXYPlot*xyplot,double s)
  {
    xyplot->cxxobj->SetMarkerSize(s);
  }

  void vtkfigSetXRange(vtkfigXYPlot*xyplot,double x0, double x1) 
  {
    xyplot->cxxobj->SetXRange(x0,x1);
  }
  void vtkfigSetYRange(vtkfigXYPlot*xyplot,double y0, double y1) 
  {
    xyplot->cxxobj->SetYRange(y0,y1);
  }
  void vtkfigSetNumberOfXLabels(vtkfigXYPlot*xyplot,int n)
  {
    xyplot->cxxobj->SetNumberOfXLabels(n);
  }
  void vtkfigSetNumberOfYLabels(vtkfigXYPlot*xyplot,int n)
  {
    xyplot->cxxobj->SetNumberOfYLabels(n);
  }



}