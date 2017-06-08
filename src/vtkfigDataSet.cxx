#include "vtkRectilinearGridWriter.h"
#include "vtkUnstructuredGridWriter.h"

#include "vtkfigDataSet.h"

namespace vtkfig
{

  void DataSet::GetRange(const std::string name, double &min, double & max)
  {
    auto  range=ranges.find(name);
    if (range==ranges.end())
      throw std::runtime_error("vtkfig::DataSet::GetRange: key "+name+" not found");
    min=range->second.min;
    max=range->second.max;
  }

  bool DataSet::DataAvailable(const std::string name)
  {
    auto  range=ranges.find(name);
    if (range==ranges.end())
      return false;
    else 
      return true;
  }

  
  DataSet::DataType  DataSet::GetDataType()
  {
    auto udata=vtkUnstructuredGrid::SafeDownCast(this->data);
    auto rdata=vtkRectilinearGrid::SafeDownCast(this->data);
    if (udata)  return DataType::UnstructuredGrid;
    if (rdata)  return DataType::RectilinearGrid;
    return DataType::NoType;
  }
  
  
  template<class DATA, class WRITER>
  void DataSet::WriteVTK(vtkSmartPointer<DATA> data, std::string fname)
  {
    auto writer=vtkSmartPointer<WRITER>::New();
    writer->SetFileTypeToBinary();
    writer->SetFileName(fname.c_str());
    writer->SetInputData(data);
    writer->Write();
  }

  
  
  void  DataSet::WriteVTK(std::string fname)
  {
    if (GetDataType()==DataType::UnstructuredGrid) 
      WriteVTK<vtkUnstructuredGrid,vtkUnstructuredGridWriter>(vtkUnstructuredGrid::SafeDownCast(data),fname);
    if (GetDataType()==DataType::RectilinearGrid) 
      WriteVTK<vtkRectilinearGrid,vtkRectilinearGridWriter>(vtkRectilinearGrid::SafeDownCast(data),fname);
  }

  
}
