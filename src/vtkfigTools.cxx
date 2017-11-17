#include "vtkfigTools.h"



#include "vtkColorTransferFunction.h"
#include "vtkTextProperty.h"
namespace vtkfig
{



  size_t NSpin()
  {
    char *spinstr=getenv("VTKFIG_NSPIN");
    if (spinstr==nullptr)
      return  std::numeric_limits<size_t>::max();
    else
      return atoi(spinstr);
  }


  // E.g. get such an array like this:
  // probeFilter->Update();
  // auto out =probeFilter->GetPolyDataOutput()->GetPointData()->GetVectors();
  // PrintArray(out, cout);
  

  void PrintArray(vtkSmartPointer<vtkDataArray> data, std::ostream & os)
  {
    auto nt=data->GetNumberOfTuples();
    auto nc=data->GetNumberOfComponents();
    assert(nc<32);
    for (int it=0;it<nt;it++)
    {
      double tuple[32];
      data->GetTuple(it,tuple);
      for (int ic=0;ic<nc ; ic++)
      {
        cout << tuple[ic] << " ";
      }
      cout << endl;
    }

  }


  void PrintPoints(vtkSmartPointer<vtkPoints> pts, std::ostream & os)
  {
    PrintArray(pts->GetData(),os);
  }




  vtkSmartPointer<vtkLookupTable>  BuildLookupTable(std::vector<RGBPoint> & xrgb, size_t size)
  {
    vtkSmartPointer<vtkColorTransferFunction> ctf = vtkSmartPointer<vtkColorTransferFunction>::New();
    vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();

    for (size_t i=0;i<xrgb.size(); i++)
    {
      if (xrgb[i].x<-1.0e-10)
      {
        lut->SetBelowRangeColor(xrgb[i].r,xrgb[i].g, xrgb[i].b,1);
        lut->UseBelowRangeColorOn();
      }
      else if (xrgb[i].x>1.0+1.0e-10)
      {
        lut->SetAboveRangeColor(xrgb[i].r,xrgb[i].g, xrgb[i].b,1);
        lut->UseAboveRangeColorOn();
      }
      else
        ctf->AddRGBPoint(xrgb[i].x,xrgb[i].r,xrgb[i].g, xrgb[i].b);
    }
    
    lut->SetNumberOfTableValues(size);
    lut->Build();
    for(size_t i = 0; i < size; ++i)
     {
       double rgb[3];
       ctf->GetColor(static_cast<double>(i)/(double)size,rgb);
       lut->SetTableValue(i,rgb[0],rgb[1],rgb[2]);
     }
    return lut;
  }

  vtkSmartPointer<vtkScalarBarActor> BuildColorBar(vtkSmartPointer<vtkPolyDataMapper> mapper, int irank)
  {

// http://public.kitware.com/pipermail/vtkusers/2016-March/094540.html :
// vtkTextProperty::SetFontSize
// vtkScalarBarActor::GetTitleTextProperty
// vtkScalarBarActor::GetLabelTextProperty
// vtkScalarBarActor::GetAnnotationTextProperty
// vtkScalarBarActor::SetUnconstrainedFontSize

    
    vtkSmartPointer<vtkScalarBarActor>     colorbar = vtkSmartPointer<vtkScalarBarActor>::New();
    
    //colorbar->UnconstrainedFontSizeOn();
    //colorbar->AnnotationTextScalingOff();
    colorbar->SetLookupTable(mapper->GetLookupTable());
    colorbar->SetWidth(0.175);
    colorbar->SetHeight(0.8);
    colorbar->SetLabelFormat(" %+9.2e");
    colorbar->SetMaximumWidthInPixels(150);
    colorbar->SetBarRatio(0.1);
    colorbar->SetVerticalTitleSeparation(0);
    colorbar->SetNumberOfLabels(11);
    colorbar->SetPosition(0.8, 0.1);
    if (irank>0)
      colorbar->SetPosition(0.9, 0.1);
      
    colorbar->GetTitleTextProperty()->ItalicOff();
    colorbar->GetLabelTextProperty()->ItalicOff();
    colorbar->GetTitleTextProperty()->BoldOn();
    colorbar->GetLabelTextProperty()->BoldOn();
    colorbar->GetTitleTextProperty()->SetFontSize(10);
    colorbar->GetLabelTextProperty()->SetFontSize(80);
    
//        colorbar->GetAnnotationTextProperty()->SetFontSize(80);
    
    colorbar->GetTitleTextProperty()->SetColor(0,0,0);     
    colorbar->GetTitleTextProperty()->SetFontFamilyToCourier();     
    colorbar->GetLabelTextProperty()->SetColor(0,0,0);     
    colorbar->GetLabelTextProperty()->SetFontFamilyToCourier();     
//        colorbar->GetAnnotationTextProperty()->SetColor(1,0,0);
    
    return colorbar;
  }

}
