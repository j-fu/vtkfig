#include <cstdio>
#include "vtkfigFrame.h"
#include "vtkfigChartXY.h"
#include "vtkfigMainThread.h"
#include "vtkfigTools.h"


int main()
{
  size_t nspin=vtkfig::NSpin();
  auto frame=vtkfig::Frame::New();


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

  double t0=(double)clock()/(double)CLOCKS_PER_SEC;
  double i0=ii;
  auto chartxy=vtkfig::ChartXY::New();

  frame->AddFigure(chartxy);

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
    chartxy->Clear();
    chartxy->Title(titlebuf);
    chartxy->LineColorRGB(0,0,1);
    chartxy->LineType("-");
    chartxy->AddPlot(x1, y1);
    chartxy->LineColorRGB(1,0,0);
    chartxy->LineType(".-");
    chartxy->AddPlot(x2, y2);


    frame->Show();
    
    if (ii==3) 
        frame->Dump("example-chartxy.png");

    double t1=(double)clock()/(double)CLOCKS_PER_SEC;
    double i1=ii;
    if (t1-t0>4.0)
    {
      printf("Frame rate: %.2f fps\n",(double)(i1-i0)/4.0);
      fflush(stdout);
      t0=(double)clock()/(double)CLOCKS_PER_SEC;
      i0=ii;
    }
    tt+=0.1;
  }

}


