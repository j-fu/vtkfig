#ifdef __cplusplus
extern "C" {
#endif
  
  typedef struct vtkfig_frame_struct vtkfig_frame;
  vtkfig_frame* vtkfig_create_frame(void);
  void vtkfig_destroy_frame(vtkfig_frame *frame);

  typedef struct vtkfig_dataset_struct vtkfig_dataset;
  vtkfig_dataset* vtkfig_create_dataset(void);
  void vtkfig_destroy_dataset(vtkfig_dataset *dataset);

  void set_rectilinear_grid(vtkfig_dataset*dataset,double *x, int nx, double *y, int ny);
  void set_point_scalar(vtkfig_dataset*dataset,double *x, int nx, char *name);

  
#ifdef __cplusplus
}
#endif
