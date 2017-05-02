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
        vtkSmartPointer<vtkScalarBarActor>     colorbar = vtkSmartPointer<vtkScalarBarActor>::New();
        colorbar->SetLookupTable(mapper->GetLookupTable());
        colorbar->SetWidth(0.085);
        colorbar->SetHeight(0.9);
        colorbar->SetNumberOfLabels(10);
        colorbar->SetPosition(0.9, 0.1);

        vtkSmartPointer<vtkTextProperty> text_prop_cb = colorbar->GetLabelTextProperty();
        text_prop_cb->SetColor(0,0,0);
        text_prop_cb->SetFontSize(40);
        colorbar->SetLabelTextProperty(text_prop_cb);
        return colorbar;
  }

}
