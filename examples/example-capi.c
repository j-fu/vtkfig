#include <vtkfigcapi.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

double G(double x,double y, double t) 
{
  
  return exp(-(x*x+y*y))*sin(t+x)*cos(y-t);
}

int main(int argc, char *argv[])
{
  int nspin=100;
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

  for (int i=0; i<Nx; i++)
    x[i] = x_low+i*dx;
  
  for (int i=0; i<Ny; i++)
    y[i] = y_low + i*dy;

  for (int i=0; i<Nx; i++)
    for (int j=0; j<Ny; j++)
    {
      z[j*Nx+i] = G(x[i],y[j],0);
    }
    

  
  vtkfigFrame* frame=vtkfigCreateFrame();
  vtkfigDataSet* griddata=vtkfigCreateDataSet();
  vtkfigSetRectilinearGrid(griddata,x,Nx,y,Ny);
  vtkfigSetPointScalar(griddata,z,Nx*Ny,"V");
  vtkfigScalarView*contour=vtkfigCreateScalarView();
  vtkfigSetData(contour,griddata,"V");
  vtkfigAddScalarView(frame,contour);

  int ii=0;
  double t=0;
  double dt=0.1;
  while (ii<nspin)
  {
    for (int i=0; i<Nx; i++)
      for (int j=0; j<Ny; j++)
      {
        z[j*Nx+i] = G(x[i],y[j],t);
      }
    
    vtkfigSetPointScalar(griddata,z,Nx*Ny,"V");
    vtkfigShow(frame);
    t+=dt;
    ii++;
  }
  vtkfigInteract(frame);
  
  
  printf("Hi\n");

  vtkfigDestroyScalarView(contour);
  vtkfigDestroyDataSet(griddata);
  vtkfigDestroyFrame(frame);
  free(x);
  free(y);
  free(z);
}

