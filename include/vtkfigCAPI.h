/**
    \file vtkfigCAPI.h
    C Language API for vtkfig.

    This API is intended for the use of vtkfig from C and Julia.
    It defines C structs as wrappers for the C++ classes of vtkfig
    and  C functions wrapping their methods.
*/

#ifdef __cplusplus
extern "C" {
#endif

  /**
     \copybrief vtkfig::PrintOpenGLInfo
   */
  void vtkfigPrintOpenGLInfo();

  /**
     \copybrief vtkfig::Frame
   */
  typedef struct vtkfigFrame_struct vtkfigFrame;
  /**
     \copybrief vtkfig::Frame::Frame()
   */
  vtkfigFrame* vtkfigCreateFrame(void);
  /**
     \copybrief vtkfig::Frame::~Frame()
   */
  void vtkfigDestroyFrame(vtkfigFrame *frame);

  /**
     \copybrief vtkfig::DataSet
   */
  typedef struct vtkfigDataSet_struct vtkfigDataSet;
  /**
     \copybrief vtkfig::DataSet::DataSet()
   */
  vtkfigDataSet* vtkfigCreateDataSet(void);
  /**
     \copybrief vtkfig::DataSet::~DataSet()
   */
  void vtkfigDestroyDataSet(vtkfigDataSet *dataset);

  /**
     \copybrief vtkfig::ScalarView
   */
  typedef struct vtkfigScalarView_struct vtkfigScalarView;
  /**
     \copybrief vtkfig::ScalarView::ScalarView()
   */
  vtkfigScalarView* vtkfigCreateScalarView(void);
  /**
     \copybrief vtkfig::ScalarView::~ScalarView()
   */
  void vtkfigDestroyScalarView(vtkfigScalarView *scalarview);

  /**
     \copybrief vtkfig::VectorView
   */
  typedef struct vtkfigVectorView_struct vtkfigVectorView;
  /**
     \copybrief vtkfig::VectorView::VectorView
   */
  vtkfigVectorView* vtkfigCreateVectorView(void);
  /**
     \copybrief vtkfig::VectorView::~VectorView
   */
  void vtkfigDestroyVectorView(vtkfigVectorView *vectorview);
  
  /**
     \copybrief vtkfig::GridView
   */
  typedef struct vtkfigGridView_struct vtkfigGridView;
  /**
     \copybrief vtkfig::GridView::GridView()
   */
  vtkfigGridView* vtkfigCreateGridView(void);
  /**
     \copybrief vtkfig::GridView::~GridView()
   */
  void vtkfigDestroyGridView(vtkfigGridView *gridview);

  /**
     \copybrief vtkfig::XYPlot
   */
  typedef struct vtkfigXYPlot_struct vtkfigXYPlot;
  /**
     \copybrief vtkfig::XYPlot::XYPlot()
   */
  vtkfigXYPlot* vtkfigCreateXYPlot(void);
  /**
     \copybrief vtkfig::XYPlot::~XYPlot()
   */
  void vtkfigDestroyXYPlot(vtkfigXYPlot *xyplot);

  
  /**
     \copybrief vtkfig::DataSet::SetRectilinearGrid
   */
  void vtkfigSetRectilinearGrid2D(vtkfigDataSet*dataset,double *x, int nx, double *y, int ny);
  /**
     \copybrief vtkfig::DataSet::SetRectilinearGrid
   */
  void vtkfigSetRectilinearGrid3D(vtkfigDataSet*dataset,double *x, int nx, double *y, int ny, double *z, int nz);
  /**
     \copybrief vtkfig::DataSet::SetPointScalar
   */
  void vtkfigSetPointScalar(vtkfigDataSet*dataset,double *x, int nx, char *name);
  /**
     \copybrief vtkfig::ScalarView::SetData
   */
  void vtkfigSetData(vtkfigScalarView*scalarview, vtkfigDataSet*dataset, char *name);
  /**
     \copybrief vtkfig::Frame::Show
   */
  void vtkfigShow(vtkfigFrame*frame);
  /**
     \copybrief vtkfig::Frame::Interact
   */
  void vtkfigInteract(vtkfigFrame*frame);

  /**
     \copybrief vtkfig::Frame::Clear
   */
  void vtkfigClearFrame(vtkfigFrame *frame);

  /**
     \copybrief vtkfig::Frame::AddFigure
   */
  void vtkfigAddScalarView(vtkfigFrame*frame, vtkfigScalarView *figure);
  /**
     \copybrief vtkfig::Frame::AddFigure
   */
  void vtkfigAddVectorView(vtkfigFrame*frame, vtkfigVectorView  *figure);
  /**
     \copybrief vtkfig::Frame::AddFigure
   */
  void vtkfigAddGridView(vtkfigFrame*frame, vtkfigGridView *figure);
  /**
     \copybrief vtkfig::Frame::AddFigure
   */
  void vtkfigAddXYPlot(vtkfigFrame*frame, vtkfigXYPlot *figure);

  /**
     \copybrief vtkfig::Frame::AddFigure
   */
  void vtkfigAddScalarViewAt(vtkfigFrame*frame, vtkfigScalarView *figure, int ipos);
  /**
     \copybrief vtkfig::Frame::AddFigure
   */
  void vtkfigAddVectorViewAt(vtkfigFrame*frame, vtkfigVectorView  *figure, int ipos);
  /**
     \copybrief vtkfig::Frame::AddFigure
   */
  void vtkfigAddGridViewAt(vtkfigFrame*frame, vtkfigGridView *figure, int ipos);
  /**
     \copybrief vtkfig::Frame::AddFigure
   */
  void vtkfigAddXYPlotAt(vtkfigFrame*frame, vtkfigXYPlot *figure, int ipos);

  /**
     \copybrief vtkfig::Frame::RemoveFigure
   */
  void vtkfigRemoveScalarView(vtkfigFrame*frame, vtkfigScalarView *figure);
  /**
     \copybrief vtkfig::Frame::RemoveFigure
   */
  void vtkfigRemoveVectorView(vtkfigFrame*frame, vtkfigVectorView  *figure);
  /**
     \copybrief vtkfig::Frame::RemoveFigure
   */
  void vtkfigRemoveGridView(vtkfigFrame*frame, vtkfigGridView *figure);
  /**
     \copybrief vtkfig::Frame::RemoveFigure
   */
  void vtkfigRemoveXYPlot(vtkfigFrame*frame, vtkfigXYPlot *figure);

  /**
     \copybrief vtkfig::Frame::WritePNG
   */
  void vtkfigWritePNG(vtkfigFrame*frame, char*fname);
  /**
     \copybrief vtkfig::Frame::StartVideo
   */
  void vtkfigStartVideo(vtkfigFrame*frame, char*fname);
  /**
     \copybrief vtkfig::Frame::StopVideo
   */
  void vtkfigStopVideo(vtkfigFrame*frame);
  /**
     \copybrief vtkfig::Frame::SetSize
   */
  void vtkfigSetSize(vtkfigFrame*frame,int x, int y);
  /**
     \copybrief vtkfig::Frame::SetLayout
   */
  void vtkfigSetLayout(vtkfigFrame*frame,int x, int y);
  /**
     \copybrief vtkfig::Frame::SetWindowTitle
   */
  void vtkfigSetWindowTitle(vtkfigFrame*frame, char*title);
  /**
     \copybrief vtkfig::Frame::SetFrameTitle
   */
  void vtkfigSetFrameTitle(vtkfigFrame*frame, char*title);
  /**
     \copybrief vtkfig::Frame::SetPosition
   */
  void vtkfigSetPosition(vtkfigFrame*frame,int x, int y);

  /**
     \copybrief vtkfig::DataSet::WriteVTK
   */
  void vtkfigWriteVTK(vtkfigDataSet*dataset,char* filename, char* filetype);

  /**
     \copybrief vtkfig::Figure::SetIsolevels
   */
  void vtkfigSetIsolevels(vtkfigScalarView *scalarview,double *v, int nv);

  /**
     \copybrief vtkfig::ScalarView::ShowIsoSurfaces
   */
  void vtkfigShowIsosurfaces(vtkfigScalarView *scalarview, int show);

  /**
     \copybrief vtkfig::XYPlot::AddPlot
   */
  void vtfigAddPlot(vtkfigXYPlot*xyplot, double *x, int nx, double *y, int ny);
  /**
     \copybrief vtkfig::XYPlot::SetTitle
   */
  void vtkfigSetTitle(vtkfigXYPlot*xyplot,char *t); 
  /**
     \copybrief vtkfig::XYPlot::ShowLegend
   */
  void vtkfigShowLegend(vtkfigXYPlot*xyplot,int b);
  /**
     \copybrief vtkfig::XYPlot::ShowGrid
   */
  void vtkfigShowGrid(vtkfigXYPlot*xyplot,int b);

  /**
     \copybrief vtkfig::XYPlot::AdjustLabels
   */
  void vtkfigAdjustLabels(vtkfigXYPlot*xyplot,int b);
  /**
     \copybrief vtkfig::XYPlot::SetLegendPosition
   */
  void vtkfigSetLegendPosition(vtkfigXYPlot*xyplot,double x, double y);
  /**
     \copybrief vtkfig::XYPlot::SetLegendSize
   */
  void vtkfigSetLegendSize(vtkfigXYPlot*xyplot,double w, double h);

  /**
     \copybrief vtkfig::XYPlot::Clear
   */
  void vtkfigClear(vtkfigXYPlot*xyplot); 
  /**
     \copybrief vtkfig::XYPlot::SetPlotLineType
   */
  void vtkfigSetPlotLineType(vtkfigXYPlot*xyplot,char *t); 
  /**
     \copybrief vtkfig::XYPlot::SetPlotMarkerType
   */
  void vtkfigSetPlotMarkerType(vtkfigXYPlot*xyplot,char *t); 
  /**
     \copybrief vtkfig::XYPlot::SetPlotColor
   */
  void vtkfigSetPlotColor(vtkfigXYPlot*xyplot,double r, double g, double b); 
  /**
     \copybrief vtkfig::XYPlot::SetPlotLegend
   */
  void vtkfigSetPlotLegend(vtkfigXYPlot*xyplot,char *t); 
  /**
     \copybrief vtkfig::XYPlot::SetGridColor
   */
  void vtkfigSetGridColor(vtkfigXYPlot*xyplot,double r, double g, double b); 
  /**
     \copybrief vtkfig::XYPlot::SetAxesColor
   */
  void vtkfigSetAxesColor(vtkfigXYPlot*xyplot,double r, double g, double b); 
  /**
     \copybrief vtkfig::XYPlot::SetXTitle
   */
  void vtkfigSetXTitle(vtkfigXYPlot*xyplot,char *t); 
  /**
     \copybrief vtkfig::XYPlot::SetYTitle
   */
  void vtkfigSetYTitle(vtkfigXYPlot*xyplot,char *t); 
  
  /**
     \copybrief vtkfig::XYPlot::SetLineWidth
   */
  void vtkfigSetLineWidth(vtkfigXYPlot*xyplot,double w);
  /**
     \copybrief vtkfig::XYPlot::SetMarkerSize
   */
  void vtkfigSetMarkerSize(vtkfigXYPlot*xyplot,double s);

  /**
     \copybrief vtkfig::XYPlot::SetXRange
   */
  void vtkfigSetXRange(vtkfigXYPlot*xyplot,double x0, double x1); 
  /**
     \copybrief vtkfig::XYPlot::SetYRange
   */
  void vtkfigSetYRange(vtkfigXYPlot*xyplot,double y0, double y1); 
  /**
     \copybrief vtkfig::XYPlot::SetNumberOfXLabels
   */
  void vtkfigSetNumberOfXLabels(vtkfigXYPlot*xyplot,int n);
  /**
     \copybrief vtkfig::XYPlot::SetNumberOfYLabels
   */
  void vtkfigSetNumberOfYLabels(vtkfigXYPlot*xyplot,int n);

  
#ifdef __cplusplus
}
#endif
