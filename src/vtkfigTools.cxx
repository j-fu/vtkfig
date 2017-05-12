#include "vtkfigTools.h"



#include "vtkColorTransferFunction.h"
#include "vtkTextProperty.h"
namespace vtkfig
{

  vtkSmartPointer<vtkLookupTable>  BuildLookupTable(std::vector<RGBPoint> & xrgb, int size)
  {
    vtkSmartPointer<vtkColorTransferFunction> ctf = vtkSmartPointer<vtkColorTransferFunction>::New();
    for (int i=0;i<xrgb.size(); i++)
      ctf->AddRGBPoint(xrgb[i].x,xrgb[i].r,xrgb[i].g, xrgb[i].b);
    
    vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
    
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

  vtkSmartPointer<vtkScalarBarActor> BuildColorBar(vtkSmartPointer<vtkPolyDataMapper> mapper)
  {

// http://public.kitware.com/pipermail/vtkusers/2016-March/094540.html :
// vtkTextProperty::SetFontSize
// vtkScalarBarActor::GetTitleTextProperty
// vtkScalarBarActor::GetLabelTextProperty
// vtkScalarBarActor::GetAnnotationTextProperty
// vtkScalarBarActor::SetUnconstrainedFontSize


        vtkSmartPointer<vtkScalarBarActor>     colorbar = vtkSmartPointer<vtkScalarBarActor>::New();

        // 7.1 ?colorbar->SetUnconstrainedFontSize(true);
        colorbar->SetLookupTable(mapper->GetLookupTable());
        colorbar->SetWidth(0.175);
        colorbar->SetHeight(0.7);
        colorbar->SetMaximumWidthInPixels(150);
        colorbar->SetBarRatio(0.1);
        colorbar->SetNumberOfLabels(10);
        colorbar->SetPosition(0.8, 0.2);
        colorbar->GetTitleTextProperty()->SetFontSize(80);
        colorbar->GetLabelTextProperty()->SetFontSize(80);
        colorbar->SetLabelFormat("%.2e");

//        colorbar->GetAnnotationTextProperty()->SetFontSize(80);

        colorbar->GetTitleTextProperty()->SetColor(0,0,0);     
        colorbar->GetLabelTextProperty()->SetColor(0,0,0);     
//        colorbar->GetAnnotationTextProperty()->SetColor(1,0,0);

        return colorbar;
  }

}
