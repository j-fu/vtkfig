#include "visvtk.h"

using namespace std;
using namespace visvtk;


int main(void)
{


  auto fig=visvtk::Figure();
  
  const int NN = 40;
  const double t_low = 0;
  const double t_upp = 5;
  const double dt = (t_upp-t_low)/(NN-1);
  
  
  std::vector<double> x1(NN);
  std::vector<double> y1(NN);
  std::vector<double> x2(NN);
  std::vector<double> y2(NN);
  
  
  
  double color1[3] =
    { 0.0, 0.0, 1.0 };
  double color2[3] =
    { 1.0, 0.0, 0.0 };
  
  
  
  double tt=0;
  int ii=0;
  while (1)
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

    {    
      auto plot=visvtk::XYPlot();
      plot.Add(x1, y1, color1, "-");
      plot.Add(x2, y2, color2, ".-");
      fig.Clear();
      fig.Show(plot);
      if (ii==3) 
        fig.Dump("example1.png");
      plot.Reset();
    }
    tt+=0.1;
    cout << ii++ << endl;
  }
}


