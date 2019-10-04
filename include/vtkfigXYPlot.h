/**
    \file vtkfigXYPlot.h

    Provide user API  class vtkfig::XYPlot derived from vtkfig::Figure for XY plots for 1D data.
*/
#ifndef VTKFIG_XYPLOT_H
#define VTKFIG_XYPLOT_H

#include <vtkDoubleArray.h>
#include <vtkXYPlotActor.h>
#include <vtkRectilinearGrid.h>
#include <vtkPointData.h>
#include <vtkXYPlotWidget.h>
#include <vtkLegendBoxActor.h>


#include "vtkfigFigure.h"

namespace vtkfig
{
  
  /// 
  /// XY Function plot.
  ///
  /// This class is based on vtkXYPlotActor, which has its restrictions.
  /// 
  /// Notably   missing  are:   logscales,  different   marker  sizes,
  /// different line widths, more line types etc. 
  ///
  class XYPlot: public Figure
  {
    XYPlot();
    
  public:


    /// Destructor.
    ~XYPlot(){};

    /// Smartpointer construtor.
    static std::shared_ptr<XYPlot> New();

    
    virtual std::string SubClassName() override final;

    /// Remove all data from plot
    void Clear();
    
    /// Set the title of the plot 
    void SetTitle(const std::string t);
    
    /// Set the title of the x axis
    void SetXTitle(const std::string t);

    /// Set the title of the y axis
    void SetYTitle(const  std::string t );

    /// Set the range of the x values 
    ///
    /// Setting \p x1 < \p x0 enables autodetection
    void SetXRange(double x0, double x1);


    /// Set the range of the y values 
    ///
    /// Setting \p y1 < \p y0 enables autodetection
    void SetYRange(double y0, double y1);

    /// Set the number of x axis labels
    void SetNumberOfXLabels(int n);

    /// Set the number of y axis labels
    void SetNumberOfYLabels(int n);
    
    /// Set the format string for x axis labels
    void SetXAxisLabelFormat(const std::string fmt);

    /// Set the format string for y axis labels
    void SetYAxisLabelFormat(const std::string fmt);

    /// Adjust labels to "nice values" ?
    ///
    /// If true, labels are adjusted to some rounded values.
    /// This is a bit experimental though.
    void AdjustLabels(bool b );

    /// Show grid lines in the plot ?
    void ShowGrid(bool b ); 

    /// Set the color of the grid lines
    void SetGridColor(double r, double g, double b);


    /// Set the color of the axes
    void SetAxesColor(double r, double g, double b);

    
    /// Show a legend  box ?
    void ShowLegend(bool b );


    /// Set the position of the legend box
    ///
    /// Use normalized viewport coordinates ( [0,1]x[0,1] )
    void SetLegendPosition(double x, double y);


    /// Set the size of the legend box
    ///
    /// Use normalized viewport coordinates ( [0,1]x[0,1] )
    void SetLegendSize(double w, double h);

    /// Set the line type of the next plot
    /// 
    /// If it is "none" or "off", no line is plotted.
    /// Otherwise, currently the only value is "-"
    void SetPlotLineType(const std::string type);


    /// Set the information for the legend entry for the next plot
    void SetPlotLegend(const std::string legend);

    /// Set the color  of the next plot
    void SetPlotColor(double r, double g, double b);

    /// Set the width of all lines in the plot
    void SetLineWidth(double w);

    /// Set the size of all markers in the plot
    void SetMarkerSize(double s);

    /// Set marker type.
    ///
    /// For possible types, see  vtkfig::XYPlot::marker_types
    void SetPlotMarkerType(const std::string type);


    //    void SetPlotMarkerSequence(offset,stride);
    //    void SetPlotMarkerColor();

    /// Add a pair of x/y data to the figure
    /// 
    /// \tparam V Vector class counting from zero with member functions
    ///  size() and operator[]. std::vector will work.
    /// \param x vector of x values
    /// \param y vector of y values
    template<typename V>  void AddPlot(const V &x,  const V &y);



  private:


    /// Internal addplot after plot data have
    /// been filled
    void AddPlot();

    /// Plot the label grid
    void PlotGrid(); 

    /// Server-Client send
    void ServerMPSend(vtkSmartPointer<internals::Communicator> communicator) override final;

    /// Server-Client receive
    void ClientMPReceive(vtkSmartPointer<internals::Communicator> communicator) override final; 

    /// Build vtk 
    void RTBuildAllVTKPipelines(vtkSmartPointer<vtkRenderer> renderer) override final;

    /// Pre render actions
    void RTPreRender() override final;
  
    /// Possible types of plot markers
    static std::map<std::string,int> marker_types;
    
    /// Number of "real" plots
    size_t num_plots=0;
    
    /// Number of grid lines
    int num_gridlines=0;
    
    /// Number of all curves including grid lines
    int num_curves=0;

    /// The main graphics actor
    vtkSmartPointer<vtkXYPlotActor> XYPlotActor;

    /// We create our own legend box.
    ///  Otherwise, all grid lines
    /// would occur in the default legend box of vtkXYPlotActor
    vtkSmartPointer<vtkLegendBoxActor> LegendActor;
  
 
    /// Fixed length string size
    static const int desclen=32;
    std::string title="";
    std::string ytitle="y";
    std::string xtitle="x";

    /// Subclass holding a grid line.
    /// The idea is to plot a grid line just as another curv in the plot.
    class GridLine
    {
      vtkSmartPointer<vtkDoubleArray> X;
      vtkSmartPointer<vtkDoubleArray> Y;
      vtkSmartPointer<vtkRectilinearGrid> curve;
      const int ds_num;
    public:
      /// Return dataset number in XYPLotActor
      int GetDataSetNumber(){return ds_num;}
      void SetYLine(double xrange[2], double y);
      void SetXLine(double yrange[2], double x);
      GridLine(vtkSmartPointer<vtkXYPlotActor> plot, int & ds_num, double rgb[3]);
    };
    
    /// vector holding grid lines
    std::vector<GridLine> XGridLines;

    /// vector holding grid lines
    std::vector<GridLine> YGridLines;

    /// Subclass holding plot data
    class  PlotData
    {
      vtkSmartPointer<vtkRectilinearGrid> curve;
      const int ds_num;
    public:
      vtkSmartPointer<vtkDoubleArray> X;
      vtkSmartPointer<vtkDoubleArray> Y;
      /// Fix curve size to X size
      void FixSize();
      int GetDataSetNumber();
      PlotData(vtkSmartPointer<vtkXYPlotActor> plot, int & ds_num);
    };
    
    
    /// vector holding data of all plots
    std::vector<PlotData> AllPlotData;

    /// General state of the XYPlot
    struct 
    {
      // has to have fixed size for easy c-s transfer

      char xlabel_format[desclen]="%8.2e";
      char ylabel_format[desclen]="%8.2e";
      bool grid_show=true;
      double grid_rgb[3]={0.8,0.8,0.8};
      double line_width=1;
      double marker_size=1;

      double axes_rgb[3]={0,0,0};
      bool legend_show=true;
      double legend_posx=0.8;
      double legend_posy=0.7;
      double legend_w=0.1;
      double legend_h=0.2;
      bool adjust_labels=false;

      double dynXMin=1.0e100;
      double dynXMax=-1.0e100;
      double dynYMin=1.0e100;
      double dynYMax=-1.0e100;
      double fixXMin=1.0e100;
      double fixXMax=-1.0e100;
      double fixYMin=1.0e100;
      double fixYMax=-1.0e100;
      int nxlabels=5;
      int nylabels=5;
    } PlotState;

    /// Info for particular plot lne
    struct PlotInfo
    {
      // has to have fixed size for easy c-s transfer
      char line_type[desclen]={'-',0};
      char marker_type[desclen]={0};
      char legend[desclen]={0};
      double line_rgb[3]={0,0,0};
      PlotInfo(){};
      ~PlotInfo(){};
    };
    
    /// Vector holding all plot info
    std::vector<PlotInfo> AllPlotInfo;
    
    /// "finite state machine" struture
    /// accumulating repective info for next plot
    PlotInfo NextPlotInfo;
        
  };
  
  
  template<typename V>
    inline
  void XYPlot::AddPlot(const V &x, const V &y)
  {
 
    while (num_plots>=AllPlotData.size())
    {
      AllPlotData.emplace_back(XYPlotActor,num_curves);
    }
    auto plot=AllPlotData[num_plots];

    int N=x.size();
    assert(x.size()==y.size());
    
    plot.X->SetNumberOfTuples(N);
    plot.Y->SetNumberOfTuples(N);

    double xmin=1.0e100;
    double xmax=-1.0e100;
    double ymin=1.0e100;
    double ymax=-1.0e100;


    for (int i=0; i<N; i++)
    {
      plot.X->InsertTuple1(i,x[i]);
      plot.Y->InsertTuple1(i,y[i]);
      xmin=std::min(xmin,x[i]);
      ymin=std::min(ymin,y[i]);
      xmax=std::max(xmax,x[i]);
      ymax=std::max(ymax,y[i]);
    }

    PlotState.dynXMin=std::min(xmin,PlotState.dynXMin);
    PlotState.dynXMax=std::max(xmax,PlotState.dynXMax);
    PlotState.dynYMin=std::min(ymin,PlotState.dynYMin);
    PlotState.dynYMax=std::max(ymax,PlotState.dynYMax);
    
    AddPlot();
  }
  

}

#endif
