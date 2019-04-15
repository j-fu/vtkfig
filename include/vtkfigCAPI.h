#ifdef __cplusplus
extern "C" {
#endif


  void vtkfigPrintOpenGLInfo();

  typedef struct vtkfigFrame_struct vtkfigFrame;
  vtkfigFrame* vtkfigCreateFrame(void);
  void vtkfigDestroyFrame(vtkfigFrame *frame);

  typedef struct vtkfigDataSet_struct vtkfigDataSet;
  vtkfigDataSet* vtkfigCreateDataSet(void);
  void vtkfigDestroyDataSet(vtkfigDataSet *dataset);

  typedef struct vtkfigScalarView_struct vtkfigScalarView;
  vtkfigScalarView* vtkfigCreateScalarView(void);
  void vtkfigDestroyScalarView(vtkfigScalarView *scalarview);

  typedef struct vtkfigVectorView_struct vtkfigVectorView;
  vtkfigVectorView* vtkfigCreateVectorView(void);
  void vtkfigDestroyVectorView(vtkfigVectorView *vectorview);
  
  typedef struct vtkfigGridView_struct vtkfigGridView;
  vtkfigGridView* vtkfigCreateGridView(void);
  void vtkfigDestroyGridView(vtkfigGridView *gridview);

  typedef struct vtkfigXYPlot_struct vtkfigXYPlot;
  vtkfigXYPlot* vtkfigCreateXYPlot(void);
  void vtkfigDestroyXYPlot(vtkfigXYPlot *xyplot);

  
  void vtkfigSetRectilinearGrid(vtkfigDataSet*dataset,double *x, int nx, double *y, int ny);
  void vtkfigSetPointScalar(vtkfigDataSet*dataset,double *x, int nx, char *name);
  void vtkfigSetData(vtkfigScalarView*scalarview, vtkfigDataSet*dataset, char *name);
  void vtkfigShow(vtkfigFrame*frame);
  void vtkfigInteract(vtkfigFrame*frame);

  void vtkfigAddScalarView(vtkfigFrame*frame, vtkfigScalarView *figure);
  void vtkfigAddVectorView(vtkfigFrame*frame, vtkfigVectorView  *figure);
  void vtkfigAddGridView(vtkfigFrame*frame, vtkfigGridView *figure);
  void vtkfigAddXYPlot(vtkfigFrame*frame, vtkfigXYPlot *figure);

  void vtkfigAddScalarViewAt(vtkfigFrame*frame, vtkfigScalarView *figure, int ipos);
  void vtkfigAddVectorViewAt(vtkfigFrame*frame, vtkfigVectorView  *figure, int ipos);
  void vtkfigAddGridViewAt(vtkfigFrame*frame, vtkfigGridView *figure, int ipos);
  void vtkfigAddXYPlotAt(vtkfigFrame*frame, vtkfigXYPlot *figure, int ipos);

  void vtkfigRemoveScalarView(vtkfigFrame*frame, vtkfigScalarView *figure);
  void vtkfigRemoveVectorView(vtkfigFrame*frame, vtkfigVectorView  *figure);
  void vtkfigRemoveGridView(vtkfigFrame*frame, vtkfigGridView *figure);
  void vtkfigRemoveXYPlot(vtkfigFrame*frame, vtkfigXYPlot *figure);

  void vtkfigWritePNG(vtkfigFrame*frame, char*fname);
  void vtkfigStartVideo(vtkfigFrame*frame, char*fname);
  void vtkfigStopVideo(vtkfigFrame*frame);
  void vtkfigSetSize(vtkfigFrame*frame,int x, int y);
  void vtkfigSetLayout(vtkfigFrame*frame,int x, int y);
  void vtkfigSetWindowTitle(vtkfigFrame*frame, char*title);
  void vtkfigSetFrameTitle(vtkfigFrame*frame, char*title);
  void vtkfigSetPosition(vtkfigFrame*frame,int x, int y);

  void vtkfigWriteVTK(vtkfigDataSet*dataset,char* filename, char* filetype);
  
  void vtfigAddPlot(vtkfigXYPlot*xyplot, double *x, int nx, double *y, int ny);
  void vtkfigSetTitle(vtkfigXYPlot*xyplot,char *t); 
  void vtkfigShowLegend(vtkfigXYPlot*xyplot,int b);
  void vtkfigShowGrid(vtkfigXYPlot*xyplot,int b);

  void vtkfigAdjustLabels(vtkfigXYPlot*xyplot,int b);
  void vtkfigSetLegendPosition(vtkfigXYPlot*xyplot,double x, double y);
  void vtkfigSetLegendSize(vtkfigXYPlot*xyplot,double w, double h);

  void vtkfigClear(vtkfigXYPlot*xyplot); 
  void vtkfigSetPlotLineType(vtkfigXYPlot*xyplot,char *t); 
  void vtkfigSetPlotMarkerType(vtkfigXYPlot*xyplot,char *t); 
  void vtkfigSetPlotColor(vtkfigXYPlot*xyplot,double r, double g, double b); 
  void vtkfigSetPlotLegend(vtkfigXYPlot*xyplot,char *t); 
  void vtkfigSetGridColor(vtkfigXYPlot*xyplot,double r, double g, double b); 
  void vtkfigSetAxesColor(vtkfigXYPlot*xyplot,double r, double g, double b); 
  void vtkfigSetXTitle(vtkfigXYPlot*xyplot,char *t); 
  void vtkfigSetYTitle(vtkfigXYPlot*xyplot,char *t); 
  
  void vtkfigSetLineWidth(vtkfigXYPlot*xyplot,double w);
  void vtkfigSetMarkerSize(vtkfigXYPlot*xyplot,double s);

  void vtkfigSetXRange(vtkfigXYPlot*xyplot,double x0, double x1); 
  void vtkfigSetYRange(vtkfigXYPlot*xyplot,double y0, double y1); 
  void vtkfigSetNumberOfXLabels(vtkfigXYPlot*xyplot,int n);
  void vtkfigSetNumberOfYLabels(vtkfigXYPlot*xyplot,int n);

  
#ifdef __cplusplus
}
#endif
