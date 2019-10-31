/**
   \example example-capi.c

   Example for usage of vtkfig C API.
*/
#include <vtkfigCAPI.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

double G(double x,double y, double t) 
{
  
  return exp(-(x*x+y*y))*sin(t+x)*cos(y-t);
}

int main(int argc, char *argv[])
{
  vtkfigFrame* frame=vtkfigCreateFrame();
  vtkfigDataSet* griddata=vtkfigCreateDataSet();
  vtkfigScalarView*contour=vtkfigCreateScalarView();

  int nspin=10;
  int Nx = 200;
  int Ny = 250;
  
  double *x=malloc(Nx*sizeof(double));
  double *y=malloc(Ny*sizeof(double));
  double *z=malloc(Nx*Ny*sizeof(double));
  
  
  double x_low = -2.5;
  double x_upp = 1.5;
  double y_low = -2.5;
  double y_upp = 4;
  double dx = (x_upp-x_low)/(Nx-1);
  double dy = (y_upp-y_low)/(Ny-1);
  int i,j;

  int ii=0;
  double t=0;
  double dt=0.1;

  
  for (i=0; i<Nx; i++)
    x[i] = x_low+i*dx;
  
  for (i=0; i<Ny; i++)
    y[i] = y_low + i*dy;

  for (i=0; i<Nx; i++)
    for (j=0; j<Ny; j++)
    {
      z[j*Nx+i] = G(x[i],y[j],0);
    }
    

  

  vtkfigSetRectilinearGrid2D(griddata,x,Nx,y,Ny);
  vtkfigSetPointScalar(griddata,z,Nx*Ny,"V");
  vtkfigSetData(contour,griddata,"V");
  vtkfigAddScalarView(frame,contour);

  while (ii<nspin)
  {
    for (i=0; i<Nx; i++)
      for (j=0; j<Ny; j++)
      {
        z[j*Nx+i] = G(x[i],y[j],t);
      }
    
    vtkfigSetPointScalar(griddata,z,Nx*Ny,"V");
    vtkfigShow(frame);
    t+=dt;
    ii++;
  }
/*  vtkfigInteract(frame);*/
  
  
  /* printf("Hi\n"); */

  /* printf("-scalarview:\n"); */
  /* vtkfigDestroyScalarView(contour); */
  /* printf("-dataset:\n"); */
  /* vtkfigDestroyDataSet(griddata); */
  /* printf("-frame:\n"); */
  /* vtkfigDestroyFrame(frame); */
  /* free(x); */
  /* free(y); */
  /* free(z); */

  printf("Ist alles weg ?\n");
  return 0;
}

