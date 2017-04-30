#include "visvtk.h"


// parameters for quiver plot
const int NNx = 20;
const int NNy = 20;
const double xx_low = 0;
const double xx_upp = 1;
const double yy_low = 0;
const double yy_upp = 1.5;
const double dxx = (xx_upp-xx_low)/(NNx-1);
const double dyy = (yy_upp-yy_low)/(NNy-1);

int main(void)
{
    // Example: Quiver ========================================
    std::vector<double> xx(NNx);
    std::vector<double> yy(NNy);
    std::vector<double> u(NNx*NNy);
    std::vector<double> v(NNx*NNy);

    for (int i=0; i<NNx; i++)
	xx[i] = xx_low+i*dxx;

    for (int i=0; i<NNy; i++)
	yy[i] = yy_low + i*dyy;

    for (int i=0; i<NNx; i++)
	for (int j=0; j<NNy; j++)
	{
	    u[j*NNx+i] = -yy[j];
	    v[j*NNx+i] = xx[i];
	}

    

    auto fig=visvtk::Figure();
    auto plot=visvtk::Quiver2D();
    plot.SetBackground(0,0,0);
    plot.Add(xx, yy, u, v);
    fig.ShowInteractive(plot);

    return 0;
}
