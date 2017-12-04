#include "vtkRectilinearGridWriter.h"
#include "vtkUnstructuredGridWriter.h"

#include "vtkfigDataSet.h"

namespace vtkfig
{

  DataSet::DataSet(){};
    
  
  DataSet::DataType  DataSet::GetDataType()
  {
    auto udata=vtkUnstructuredGrid::SafeDownCast(this->data);
    auto rdata=vtkRectilinearGrid::SafeDownCast(this->data);
    if (udata)  return DataType::UnstructuredGrid;
    if (rdata)  return DataType::RectilinearGrid;
    return DataType::NoType;
  }
  
  
  template<class DATA, class WRITER>
  void DataSet::WriteVTK(vtkSmartPointer<DATA> data, const std::string fname, const std::string filetype )
  {
    auto writer=vtkSmartPointer<WRITER>::New();
    if (filetype=="A")
      writer->SetFileTypeToASCII();
    else if (filetype=="B")
      writer->SetFileTypeToBinary();
    else
      throw std::runtime_error("WriteVTK: wrong file type, choose \"A\" for ascii, \"B\" for binary\n");
    writer->SetFileName(fname.c_str());
    writer->SetInputData(data);
    writer->Write();
  }

  
  
  void  DataSet::WriteVTK(std::string fname, const std::string filetype)
  {
    if (GetDataType()==DataType::UnstructuredGrid) 
      WriteVTK<vtkUnstructuredGrid,vtkUnstructuredGridWriter>(vtkUnstructuredGrid::SafeDownCast(data),fname, filetype);
    if (GetDataType()==DataType::RectilinearGrid) 
      WriteVTK<vtkRectilinearGrid,vtkRectilinearGridWriter>(vtkRectilinearGrid::SafeDownCast(data),fname,filetype);
  }

  
}
