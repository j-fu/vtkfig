#ifdef __cplusplus
extern "C" {
#endif
  
  typedef struct vtkfigFrame_struct vtkfigFrame;
  vtkfigFrame* vtkfigCreateFrame(void);
  void vtkfigDestroyFrame(vtkfigFrame *frame);

  typedef struct vtkfigDataSet_struct vtkfigDataSet;
  vtkfigDataSet* vtkfigCreateDataSet(void);
  void vtkfigDestroyDataSet(vtkfigDataSet *dataset);

  typedef struct vtkfigScalarView_struct vtkfigScalarView;
  vtkfigScalarView* vtkfigCreateScalarView(void);
  void vtkfigDestroyScalarView(vtkfigScalarView *scalarview);

  void vtkfigSetRectilinearGrid(vtkfigDataSet*dataset,double *x, int nx, double *y, int ny);
  void vtkfigSetPointScalar(vtkfigDataSet*dataset,double *x, int nx, char *name);
  void vtkfigSetData(vtkfigScalarView*scalarview, vtkfigDataSet*dataset, char *name);
  void vtkfigAddScalarView(vtkfigFrame*frame, vtkfigScalarView *scalarview);
  void vtkfigShow(vtkfigFrame*frame);
  void vtkfigInteract(vtkfigFrame*frame);


#ifdef __cplusplus
}
#endif
