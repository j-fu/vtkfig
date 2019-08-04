#include <fstream>
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <chrono>

#include "vtkfigFrame.h"
#include "vtkfigDataSet.h"
#include "vtkfigScalarView.h"
#include "vtkfigTools.h"
#include "config.h"




std::shared_ptr<std::vector<double>>  read_file(std::string name)
{
  auto pX= std::make_shared<std::vector<double>>();
  std::string line;
  int row,col;


  std::ifstream pFile(name);
  if (pFile.is_open())
  {
    std::cout << "Reading file " << name << " ..."<<std::endl;
    while(!pFile.eof())
    {
      getline(pFile, line);
      std::stringstream ss(line);
      double xx;
      while(ss >> xx)
        pX->push_back(xx);
    } 
    std::cout << "   " << pX->size() << " entries read." << std::endl;
    pFile.close();
  }
  else 
  {
    cout << "Unable to open file " << name << std::endl; 
    exit(1);
  }
  return pX;
}

int main (const int argc, const char * argv[])
{
  if (argc!=5)
  {
    cout << R"(
vtkfig-view-xyzrect X Y Z F

Visualize 3D tensor product grid data using VTK and vtkfig.

Input: X,Y,Z: coordinate values
           F: scalar function values defined on grid X x Y x Z

All data arrays are given as sequences of values in ASCII format.

(c) J. Fuhrmann 2017
)";
    return 1;
  }


  auto pX=read_file(argv[1]);
  auto pY=read_file(argv[2]);
  auto pZ=read_file(argv[3]);
  auto pF=read_file(argv[4]);
  
  assert(pF->size()==pX->size()*pY->size()*pZ->size());
  
  auto colors=vtkfig::RGBTable
    { 
      {0.0, 0.0, 0.0, 1.0},
      {0.5, 0.0, 1.0, 0.0},
      {1.0, 1.0, 0.0, 0.0}
    };


  auto frame=vtkfig::Frame::New();

  
  auto griddata=vtkfig::DataSet::New();
  griddata->SetRectilinearGrid(*pX, *pY, *pZ);
  griddata->SetPointScalar(*pF ,argv[4]);

  auto contour=vtkfig::ScalarView::New();
  contour->SetData(griddata,argv[4]);
  contour->SetSurfaceRGBTable(colors,255);
  frame->AddFigure(contour);
  frame->Interact();
}
