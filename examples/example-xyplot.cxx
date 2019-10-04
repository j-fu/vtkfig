///
///   \example   example-xyplot.cxx
///
///  Plot of 1D functions
///

#include <cstdio>
#include "vtkfigFrame.h"
#include "vtkfigXYPlot.h"
#include "vtkfigMainThread.h"
#include "vtkfigTools.h"


int main()
{
  size_t nspin=vtkfig::NSpin();

  const int NN = 40;
  const double t_low = 0;
  const double t_upp = 5;

  const double dt = (t_upp-t_low)/(NN-1);
  
  
  std::vector<double> x1(NN);
  std::vector<double> y1(NN);
  std::vector<double> x2(NN);
  std::vector<double> y2(NN);
  
  
  
  
  std::string title;
  
  double tt=0;
  size_t ii=0;

  auto t0=std::chrono::system_clock::now();
  double i0=ii;

  auto frame=vtkfig::Frame::New();
  auto dataset=vtkfig::DataSet::New();
  auto xyplot=vtkfig::XYPlot::New();

  frame->AddFigure(xyplot);
  while (ii<nspin)
  {
    double t = tt;

    for (int i=0; i<NN; i++)
    {
      x1[i] = cos(t)*t;
      y1[i] = sin(t);
      x2[i] = sin(t)*t+1;
      y2[i] = cos(t)*(0.1-t);
      t+=dt;
    }
    
    char titlebuf[20];
  
    snprintf(titlebuf,20,"frame %lu",ii++);
    xyplot->Clear();
    xyplot->SetTitle(titlebuf);
    xyplot->SetPlotColor(0,0,1);
    xyplot->SetXAxisLabelFormat("%3.1f");
    xyplot->SetYAxisLabelFormat("%3.1f");

    xyplot->SetPlotLineType("-");
    xyplot->SetPlotMarkerType("d");
    xyplot->SetMarkerSize(1);
    xyplot->SetPlotLegend("A");

    xyplot->AddPlot(x1, y1);
    xyplot->SetPlotColor(1,0,0);
    xyplot->SetPlotMarkerType("o");
    xyplot->SetPlotLegend("B");
    xyplot->AddPlot(x2, y2);


    frame->Show();
    
    if (ii==3) 
        frame->WritePNG("example-xyplot.png");

    tt+=0.1;
    auto t1=std::chrono::system_clock::now();
    double xdt=std::chrono::duration_cast<std::chrono::duration<double>>(t1-t0).count();
    double i1=ii;

    if (xdt>4.0)
    {
      printf("Frame rate: %.2f fps\n",(double)(i1-i0)/4.0);
      t0=std::chrono::system_clock::now();
      i0=ii;
      fflush(stdout);
    }
    
    ii++;
  }

}


