#include "visvtk.h"
#include <cstdio>

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
  
  string title;
  
  double tt=0;
  int ii=0;

  double t0=(double)clock()/(double)CLOCKS_PER_SEC;
  double i0=ii;
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
    
    char titlebuf[20];
  
    {    
      auto plot=visvtk::XYPlot();
      snprintf(titlebuf,20,"frame %d",ii++);
      plot.Title(titlebuf);
      plot.Add(x1, y1, color1, "-");
      plot.Add(x2, y2, color2, ".-");
      fig.Clear();
      fig.Add(plot);
      fig.Show();

      if (ii==3) 
        fig.Dump("example1.png");
    }

    double t1=(double)clock()/(double)CLOCKS_PER_SEC;
    double i1=ii;
    if (t1-t0>4.0)
    {
      printf("Frame rate: %.2f fps\n",(double)(i1-i0)/4.0);
      t0=(double)clock()/(double)CLOCKS_PER_SEC;
      i0=ii;
    }
    tt+=0.1;
  }
}


