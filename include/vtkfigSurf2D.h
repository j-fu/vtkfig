#ifndef VTKFIG_SURF2D_H
#define VTKFIG_SURF2D_H

#include "vtkRectilinearGrid.h"
#include "vtkStructuredGrid.h"
#include "vtkPointData.h"
#include "vtkFloatArray.h"
#include "vtkWarpScalar.h"


#include "vtkfigFigure.h"
#include "vtkfigTools.h"
#include "vtkfigCommunicator.h"

namespace vtkfig
{

  ///////////////////////////////////////////
  class Surf2D: public Figure
    {
    public:
      
      Surf2D();
      static std::shared_ptr<Surf2D> New() { return std::make_shared<Surf2D>(); }
      virtual std::string SubClassName() {return std::string("Surf2D");}
      

      template<typename V>
      void SetGrid(const V &xcoord, 
                   const V &ycoord);

      template<typename V>
        void UpdateValues(const V &values);
      
      void SetRGBTable(RGBTable & tab, int tabsize)
      {
        state.rgbtab_size=tabsize;
        state.rgbtab_modified=true;
        state.rgbtab_npoints=tab.size();;
        rgbtab=tab;
        lut=BuildLookupTable(tab,tabsize);
      }
      void ShowColorbar(bool b) {show_colorbar=b;}

      void ServerRTSend(vtkSmartPointer<Communicator> communicator) 
      {
        communicator->SendCharBuffer((char*)&state,sizeof(state));
        if (state.rgbtab_modified)
          communicator->SendFloatBuffer((float*)rgbtab.data(),state.rgbtab_npoints*sizeof(RGBPoint)/sizeof(float));
        communicator->Send(gridfunc,1,1);
        state.rgbtab_modified=false;
      };

      void ClientMTReceive(vtkSmartPointer<Communicator> communicator) 
      {
        communicator->ReceiveCharBuffer((char*)&state,sizeof(state));
        if (state.rgbtab_modified)
        {
          RGBTable new_rgbtab(state.rgbtab_npoints);
          communicator->ReceiveFloatBuffer((float*)new_rgbtab.data(),state.rgbtab_npoints*sizeof(RGBPoint)/sizeof(float));
          SetRGBTable(new_rgbtab,state.rgbtab_size);
        }
        communicator->Receive(gridfunc,1,1);
        update_warp_and_lut();
        gridfunc->Modified();
      };


    private:
      void RTBuild(void);
      vtkSmartPointer<vtkStructuredGrid> 	    gridfunc;
      vtkSmartPointer<vtkPoints> points;
      vtkSmartPointer<vtkFloatArray> colors;
      vtkSmartPointer<vtkWarpScalar> warp;
      

      int Nx;
      int Ny;

      RGBTable rgbtab{{0,0,1},{1,0,0}};
      // all float in order to be endianness-aware
      struct 
      {
        float Lxy;
        float Lz;
        float vmax=0;
        float vmin=0;
        float rgbtab_size=255;
        float rgbtab_npoints=2;
        float rgbtab_modified=0.0;
      } state;

      vtkSmartPointer<vtkLookupTable> lut;
      bool show_colorbar=true;

      void update_warp_and_lut()
      {
        state.Lz = state.vmax-state.vmin;
        lut->SetTableRange(state.vmin,state.vmax);
        lut->Modified();
        
        double scale = state.Lxy/state.Lz;
        warp->XYPlaneOn();
        warp->SetScaleFactor(scale);
        warp->Modified();
      }
  };


  template<typename V>
  inline
  void Surf2D::SetGrid(const V &x, const V &y)
  {
    Nx = x.size();
    Ny = y.size();
    int i,j,k;
    
    if (x[Nx-1]-x[0] > y[Ny-1]-y[0])
      state.Lxy = x[Nx-1]-x[0];
    else
      state.Lxy = y[Ny-1]-y[0];

    double z_low = 10000, z_upp = -10000;
    
    
    gridfunc->SetDimensions(Nx, Ny, 1);
    
    points = vtkSmartPointer<vtkPoints>::New();
    for (j = 0; j < Ny; j++)
    {
      for (i = 0; i < Nx; i++)
      {
        points->InsertNextPoint(x[i], y[j], 0);
      }
    }
    gridfunc->SetPoints(points);
    
    colors = vtkSmartPointer<vtkFloatArray>::New();
    colors->SetNumberOfComponents(1);
    colors->SetNumberOfTuples(Nx*Ny);
    k = 0;
    for (j = 0; j < Ny; j++)
      for (i = 0; i < Nx; i++)
      {
        colors->InsertComponent(k, 0, 0);
        k++;
      }
    
    gridfunc->GetPointData()->SetScalars(colors);
    
  }

  template<typename V>
  inline
  void Surf2D::UpdateValues(const V &z)
  {
    for (int j = 0; j < Ny; j++)
    {
      for (int i = 0; i < Nx; i++)
      {
        int k=j*Nx+i;
        float v=z[k];
        state.vmin=std::min(v,state.vmin);
        state.vmax=std::max(v,state.vmax);
        double  p[3];
        points->GetPoint(k,p);
        p[2]=v;
        points->SetPoint(k,p);
        colors->InsertComponent(k, 0,v);
      }
    }

    points->Modified();
    colors->Modified();
    gridfunc->Modified();

    update_warp_and_lut();

  }
}
#endif
