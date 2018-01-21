#include <vtkProperty2D.h>
#include <vtkTextProperty.h>
#include <vtkAxisActor2D.h>
#include <vtkLegendBoxActor.h>

#include "vtkfigXYPlot.h"
#include "config.h"

namespace vtkfig
{
  XYPlot::XYPlot():Figure()
  {
    this->XYPlotActor=vtkSmartPointer<vtkXYPlotActor>::New();
    this->LegendActor=vtkSmartPointer<vtkLegendBoxActor>::New();
    this->LegendActor->SetNumberOfEntries(0);
  }


  std::shared_ptr<XYPlot> XYPlot::New() 
  { 
    return std::make_shared<XYPlot>(); 
  }
  
  
  std::string XYPlot::SubClassName() 
  {
    return std::string("XYPlot");
  }
  

  void XYPlot::Clear()
  {
    XYPlotActor->RemoveAllActiveCurves();
    num_plots=0;
    NextPlotInfo.line_type[0]='-';
    NextPlotInfo.line_type[1]=0;
    NextPlotInfo.marker_type[0]=0;
    NextPlotInfo.legend[0]=0;
    
    NextPlotInfo.line_rgb[0]=0;
    NextPlotInfo.line_rgb[1]=0;
    NextPlotInfo.line_rgb[2]=0;

    PlotState.dynXMin=1.0e100;
    PlotState.dynXMax=-1.0e100;
    PlotState.dynYMin=1.0e100;
    PlotState.dynYMax=-1.0e100;
  }

  
  void XYPlot::SetTitle(const std::string t) 
  {
    XYPlotActor->SetTitle(t.c_str());    
    title=t;
  }
  
  void XYPlot::SetXTitle(const std::string t) 
  {
    XYPlotActor->SetXTitle(t.c_str()); 
    xtitle=t;
  }

  void XYPlot::SetYTitle(const  std::string t ) 
  {
    XYPlotActor->SetYTitle(t.c_str());    
    ytitle=t;
  }

  void XYPlot::SetXRange(double x0, double x1) 
  {
    PlotState.fixXMin=x0; 
    PlotState.fixXMax=x1;
  }
  
  
  void XYPlot::SetYRange(double y0, double y1) 
  {
    PlotState.fixYMin=y0; 
    PlotState.fixYMax=y1;
  }
  
  void XYPlot::SetNumberOfXLabels(int n) 
  {
    PlotState.nxlabels=n;
  }
  
  void XYPlot::SetNumberOfYLabels(int n) 
  {
    PlotState.nylabels=n;
  }
  
  void XYPlot::SetXAxisLabelFormat(const std::string fmt) 
  { 
    strncpy(PlotState.xlabel_format,fmt.c_str(),desclen);
  }
  

  void XYPlot::SetYAxisLabelFormat(const std::string fmt) 
  {
    strncpy(PlotState.ylabel_format,fmt.c_str(),desclen);
  }
  
  void XYPlot::AdjustLabels(bool b ) 
  {
    PlotState.adjust_labels=b;
  };
  

  void XYPlot::ShowGrid(bool b ) 
  {
    PlotState.grid_show=b;
  };
  
  void XYPlot::SetGridColor(double r, double g, double b) 
  {
    PlotState.grid_rgb[0]=r;
    PlotState.grid_rgb[1]=g;
    PlotState.grid_rgb[2]=b;
  }
  
  void XYPlot::SetAxesColor(double r, double g, double b) 
  {
    PlotState.axes_rgb[0]=r;
    PlotState.axes_rgb[1]=g;
    PlotState.axes_rgb[2]=b;
  }
  
  
  void XYPlot::ShowLegend(bool b ) 
  {
    PlotState.legend_show=b;
  };
  
  void XYPlot::SetLegendPosition(double x, double y) 
  {
    PlotState.legend_posx=x;
    PlotState.legend_posy=y;
  }

  void XYPlot::SetLegendSize(double w, double h)
  {
    PlotState.legend_w=w;
    PlotState.legend_h=h;
  }
  
  
  void XYPlot::SetPlotLineType(const std::string type) 
  {
    strncpy(NextPlotInfo.line_type,type.c_str(),desclen);
  }

  void XYPlot::SetPlotLegend(const std::string legend) 
  {
    strncpy(NextPlotInfo.legend,legend.c_str(),desclen);
  }
  
  void XYPlot::SetPlotColor(double r, double g, double b)     
  { 
    NextPlotInfo.line_rgb[0]=r; 
    NextPlotInfo.line_rgb[1]=g; 
    NextPlotInfo.line_rgb[2]=b;
  }
  
  void XYPlot::SetLineWidth(double w) 
  { 
    PlotState.line_width=w;
  }
  

  void XYPlot::SetMarkerSize(double s) 
  { 
    PlotState.marker_size=s;
  }
  
  void XYPlot::SetPlotMarkerType(const std::string type)
  {
    strncpy(NextPlotInfo.marker_type,type.c_str(),desclen);
  }
  

  
  void XYPlot::RTBuildAllVTKPipelines(vtkSmartPointer<vtkRenderer> renderer)
  {

    XYPlotActor->GetProperty()->SetColor(PlotState.axes_rgb);
    XYPlotActor->SetBorder(20);
    XYPlotActor->GetPositionCoordinate()->SetValue(0.0, 0.0, 0);
    XYPlotActor->GetPosition2Coordinate()->SetValue(1.0, 1.0, 0);

    
    if (PlotState.adjust_labels)
    {
      XYPlotActor->SetAdjustXLabels(1);
      XYPlotActor->SetAdjustYLabels(1);
    }
    else
    {
      XYPlotActor->SetAdjustXLabels(0);
      XYPlotActor->SetAdjustYLabels(0);
    }

    // We build our own legened box which is 
    // not cluttered with grid lines
    // XYPlotActor->LegendOn();
    // XYPlotActor->SetLegendBox(1);
    // XYPlotActor->SetLegendBorder(1);

    
    XYPlotActor->PlotPointsOff();
    XYPlotActor->PlotLinesOn();
    XYPlotActor->PlotCurvePointsOn();
    XYPlotActor->PlotCurveLinesOn();



    XYPlotActor->SetNumberOfXLabels(PlotState.nxlabels);
    XYPlotActor->SetNumberOfYLabels(PlotState.nylabels);
    XYPlotActor->SetNumberOfXMinorTicks(10);
    XYPlotActor->SetNumberOfYMinorTicks(10);



    int fs=30;
    XYPlotActor->SetTitleFontSize(fs);
    XYPlotActor->SetTitleShadow(0);
    XYPlotActor->SetTitleItalic(0);
    XYPlotActor->SetTitleBold(1);
    XYPlotActor->SetTitleColor(0,0,0);

    XYPlotActor->SetAdjustTitlePosition(0);
    XYPlotActor->SetTitlePosition(0.5,0.95);


    XYPlotActor->SetAxisTitleFontSize(fs);
    XYPlotActor->SetAxisTitleItalic(0);
    XYPlotActor->SetAxisTitleShadow(0);
    XYPlotActor->SetAxisTitleBold(1);
    XYPlotActor->SetAxisTitleColor(0,0,0);

    XYPlotActor->SetAxisLabelFontSize(fs);
    XYPlotActor->SetAxisLabelShadow(0);
    XYPlotActor->SetAxisLabelItalic(0);
    XYPlotActor->SetAxisLabelColor(0,0,0);
    XYPlotActor->SetYTitlePositionToVCenter();



    XYPlotActor->SetXValuesToValue();
    Figure::RTAddActor2D(XYPlotActor);


    if (PlotState.legend_show)
    {
      this->LegendActor->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
      this->LegendActor->GetPosition2Coordinate()->SetCoordinateSystemToNormalizedViewport();
      this->LegendActor->GetPosition2Coordinate()->SetReferenceCoordinate( NULL );
      
      
      this->LegendActor->SetPosition(PlotState.legend_posx,PlotState.legend_posy);
      this->LegendActor->SetPosition2(
        PlotState.legend_posx+PlotState.legend_w,
        PlotState.legend_posy+PlotState.legend_h);
      this->LegendActor->SetBorder(true);
      this->LegendActor->SetBox(true);
      LegendActor->GetProperty()->DeepCopy(XYPlotActor->GetProperty());
      LegendActor->ScalarVisibilityOff();
      auto text_prop = LegendActor->GetEntryTextProperty();
      text_prop->ItalicOff();
      text_prop->SetFontFamilyToArial();
      text_prop->SetFontSize(fs);
      
      
      Figure::RTAddActor2D(LegendActor);
    }

  }

  

  
  void XYPlot::GridLine::SetYLine(double xrange[2], double y)
  {
    X->InsertTuple1(0,xrange[0]);
    Y->InsertTuple1(0,y);
    
    X->InsertTuple1(1,xrange[1]);
    Y->InsertTuple1(1,y);
  }
  void XYPlot::GridLine::SetXLine(double yrange[2], double x)
  {
    X->InsertTuple1(0,x);
    Y->InsertTuple1(0,yrange[0]);
    
    X->InsertTuple1(1,x);
    Y->InsertTuple1(1,yrange[1]);
  }
  
  XYPlot::GridLine::GridLine(vtkSmartPointer<vtkXYPlotActor> plot, int & ds_num, double rgb[3])
    : ds_num(ds_num++)
  {
    
    X=vtkSmartPointer<vtkDoubleArray>::New();
    Y=vtkSmartPointer<vtkDoubleArray>::New();
    X->SetNumberOfTuples(2);
    Y->SetNumberOfTuples(2);
    curve=vtkSmartPointer<vtkRectilinearGrid>::New();
    curve->SetXCoordinates(X);
    curve->GetPointData()->SetScalars(Y);
    curve->SetDimensions(2, 1, 1);
    plot->AddDataSetInput(curve);
    plot->SetPlotColor(ds_num, rgb);
    plot->SetPlotLines(ds_num, 1);
    plot->SetPlotPoints(ds_num,0);
  }


  void XYPlot::PlotGrid()
  {
    if (PlotState.grid_show)
    {
      double xrange[2];
      double yrange[2];
      int mynxlabels;
      int mynylabels;
      

      // nice to have but not with opengl2
      // https://gitlab.kitware.com/vtk/vtk/issues/15799
      XYPlotActor->GetProperty()->SetLineStipplePattern(0xf0f0);
      XYPlotActor->GetProperty()->SetLineStippleRepeatFactor(5);


      if (PlotState.adjust_labels)
      {
        XYPlotActor->GetXAxisActor2D()->GetAdjustedRange(xrange);
        XYPlotActor->GetYAxisActor2D()->GetAdjustedRange(yrange);
        mynxlabels=XYPlotActor->GetXAxisActor2D()->GetAdjustedNumberOfLabels();
        mynylabels=XYPlotActor->GetYAxisActor2D()->GetAdjustedNumberOfLabels();
      }
      else
      {
        XYPlotActor->GetXRange(xrange);
        XYPlotActor->GetYRange(yrange);
        mynxlabels=XYPlotActor->GetNumberOfXLabels();
        mynylabels=XYPlotActor->GetNumberOfYLabels();
      }

      // The number of grid lines can change, so there may be more or less
      // of them in a new plot. We however keep the once allocated vectors
      // and have to extend them if they are too short.
      // If they are too long , we switch off the superfluous ones
      // (via the second argument in SetPlotLines())

      for (int i=XGridLines.size();i<mynxlabels; i++)
        XGridLines.emplace_back(XYPlotActor, num_curves,PlotState.grid_rgb);
      
      for (int i=YGridLines.size();i<mynylabels; i++)
        YGridLines.emplace_back(XYPlotActor, num_curves,PlotState.grid_rgb);
      

      double dx=0.999999*(xrange[1]-xrange[0])/(mynxlabels-1);
      double x=xrange[0]+dx;
      for (int i=1;i<mynxlabels;i++,x+=dx)
      {
        XGridLines[i].SetXLine(yrange,x);
        XYPlotActor->SetPlotLines(XGridLines[i].GetDataSetNumber(), 1);
      }
      for (int i=mynxlabels;i<XGridLines.size();i++)
      {
        XYPlotActor->SetPlotLines(XGridLines[i].GetDataSetNumber(), 0);
      }

      
      double dy=0.9999999*(yrange[1]-yrange[0])/(mynylabels-1);
      double y=yrange[0]+dy;
      for (int i=1;i<mynylabels;i++,y+=dy)
      {
        YGridLines[i].SetYLine(xrange,y);
        XYPlotActor->SetPlotLines(YGridLines[i].GetDataSetNumber(), 1);
      }
      for (int i=mynylabels;i<XGridLines.size();i++)
      {
        XYPlotActor->SetPlotLines(YGridLines[i].GetDataSetNumber(), 0);
      }
      
    }
  }

  std::map<std::string,int>  XYPlot::marker_types
  {
      {"+",3},
      {"plus",3},
      {"^^",5},
      {"filled triangle",5},
      {"^",18},
      {"triangle",18},
      {"#",19},
      {"square",19},
      {"##",6},
      {"filled square",6},
      {"o",20},
      {"circle",20},
      {"oo",7},
      {"filled circle",7},
      {"d",21},
      {"diamond",21},
      {"dd",8},
      {"filled diamond",8}
    };


  void XYPlot::PlotData::FixSize()
  {
    curve->SetDimensions(X->GetNumberOfTuples(), 1, 1);
  }
  
  int XYPlot::PlotData::GetDataSetNumber()
  {
    return ds_num;
  }
  
  XYPlot::PlotData::PlotData(vtkSmartPointer<vtkXYPlotActor> plot, int & ds_num)
    : ds_num(ds_num++)
  {
    X=vtkSmartPointer<vtkDoubleArray>::New();
    Y=vtkSmartPointer<vtkDoubleArray>::New();
    curve=vtkSmartPointer<vtkRectilinearGrid>::New();
    curve->SetXCoordinates(X);
    curve->GetPointData()->SetScalars(Y);
    plot->AddDataSetInput(curve);
  }


  void XYPlot::AddPlot()
  {



    while (num_plots>=AllPlotData.size())
    {
      AllPlotData.emplace_back(XYPlotActor,num_curves);
    }

    auto plot=AllPlotData[num_plots];
    AllPlotInfo.insert(AllPlotInfo.begin()+num_plots,NextPlotInfo);
    plot.FixSize();


    XYPlotActor->SetPlotColor(plot.GetDataSetNumber(), NextPlotInfo.line_rgb[0], NextPlotInfo.line_rgb[1], NextPlotInfo.line_rgb[2]);

    int plot_lines = -1;
    std::string line_type(NextPlotInfo.line_type);
    if (
      line_type == "" 
      || line_type == "none"
      || line_type == "off"
      )
      plot_lines = 0;
    else if (
      line_type == "-"
      || line_type == "on"
      )
      plot_lines = 1;

    if (plot_lines==-1)
      throw std::runtime_error("Unknown line type: "+line_type);
    XYPlotActor->SetPlotLines(plot.GetDataSetNumber(), plot_lines);

    int plot_points = -1;
    std::string marker_type(NextPlotInfo.marker_type);
    if (
      marker_type == "" 
      || marker_type == "none"
      || marker_type == "off"
      )
    {
      plot_points = 0;
      XYPlotActor->SetPlotGlyphType(plot.GetDataSetNumber(), 1);
    };

    if (marker_types.find(marker_type)!=marker_types.end())
    {
      plot_points = 1;
      XYPlotActor->SetPlotGlyphType(plot.GetDataSetNumber(), marker_types[marker_type]);
    }
    if (plot_points==-1)
      throw std::runtime_error("Unknown marker type: "+marker_type);

    XYPlotActor->SetPlotPoints(plot.GetDataSetNumber(), plot_points);
  



    XYPlotActor->SetPlotLabel(plot.GetDataSetNumber(), NextPlotInfo.legend);


    if (PlotState.fixXMin>PlotState.fixXMax)     
      XYPlotActor->SetXRange(PlotState.dynXMin,PlotState.dynXMax);
    else
      XYPlotActor->SetXRange(PlotState.fixXMin,PlotState.fixXMax);

    if (PlotState.fixYMin>PlotState.fixYMax)     
      XYPlotActor->SetYRange(PlotState.dynYMin,PlotState.dynYMax);
    else
      XYPlotActor->SetYRange(PlotState.fixYMin,PlotState.fixYMax);

    XYPlotActor->SetLegendPosition2(0.15,0.05*(num_plots+1));



    num_plots++;

    XYPlotActor->Modified();
  }

  void XYPlot::RTPreRender()
  {
    XYPlotActor->SetXLabelFormat(PlotState.ylabel_format);
    XYPlotActor->SetYLabelFormat(PlotState.ylabel_format);
    XYPlotActor->SetTitle(title.c_str());
    XYPlotActor->SetXTitle(xtitle.c_str());
    XYPlotActor->SetYTitle(ytitle.c_str());


    XYPlotActor->SetLineWidth(2.0*PlotState.line_width);
    XYPlotActor->SetGlyphSize(0.01*PlotState.marker_size);
    PlotGrid();
    
    if (PlotState.legend_show)
    {
      this->LegendActor->SetNumberOfEntries(num_plots);
      this->LegendActor->SetPadding(2);
      for (int iplot=0;iplot<num_plots;iplot++)
      {
        int ds_num=AllPlotData[iplot].GetDataSetNumber();
        this->LegendActor->SetEntryString(iplot,XYPlotActor->GetPlotLabel(ds_num));
        this->LegendActor->SetEntryColor(iplot, XYPlotActor->GetPlotColor(ds_num));
        auto symbol=XYPlotActor->GetPlotSymbol(ds_num);
        if (symbol)
          this->LegendActor->SetEntrySymbol(iplot,symbol);    
        
      }
      this->LegendActor->Modified();
    }
  }



  void  XYPlot::ServerRTSend(vtkSmartPointer<internals::Communicator> communicator) 
  {
    communicator->SendString(title);
    communicator->SendString(xtitle);
    communicator->SendString(ytitle);
    
    communicator->SendCharBuffer((char*)&PlotState,sizeof(PlotState));
    

    communicator->SendInt(num_plots);
    communicator->SendInt(AllPlotInfo.size());
    int ndata=AllPlotInfo.size()*sizeof(PlotInfo);
    communicator->SendCharBuffer((char*)AllPlotInfo.data(),ndata);


    for (int i=0;i<num_plots;i++)
    {
      auto &plot=AllPlotData[i];
      communicator->Send(plot.X,1,1);
      communicator->Send(plot.Y,1,1);
    }
  };
  
  void  XYPlot::ClientMTReceive(vtkSmartPointer<internals::Communicator> communicator) 
  {
    Clear();
    communicator->ReceiveString(title);
    communicator->ReceiveString(xtitle);
    communicator->ReceiveString(ytitle);
    communicator->ReceiveCharBuffer((char*)&PlotState,sizeof(PlotState));

    int np;
    int npi;
    communicator->ReceiveInt(np);
    communicator->ReceiveInt(npi);
    std::vector<PlotInfo> NewPlotInfo(npi);
    char *data=(char*)NewPlotInfo.data();
    int ndata=npi*sizeof(PlotInfo);
    communicator->ReceiveCharBuffer(data,ndata);
    

    for (int i=0;i<np;i++)
    {
      while (num_plots>=AllPlotData.size())
      {
        AllPlotData.emplace_back(XYPlotActor,num_curves);
      }
      
      auto plot=AllPlotData[num_plots];
      NextPlotInfo=NewPlotInfo[i];
      communicator->Receive(plot.X,1,1);
      communicator->Receive(plot.Y,1,1);
      AddPlot();
    }
  };


}
