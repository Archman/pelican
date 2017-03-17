#ifndef METHODS_H_
#define METHODS_H_

/** \brief define classes/functions to handle FEL physics related problems.
 *
 * class:
 *  1 FELAnalysis : FEL formulae for analytical calculation
 *  2 FELNumerical: Numerical simulations for FEL physics
 * function:
 *  1 *normrand() : generate random distribution with Gaussian distribution
 *  2 meanexph()  : return <exp(-1i*h*psi)>, double complex type
 *
 * created time: Jul. 1st,  15:53, 2014
 * last updated: Jul. 17th, 19:41, 2014
 */

#include "constants.h"
#include "elements.h"
#include <complex>
#include <string>

class FELAnalysis : public physicalConstants
{
    private:
        double gamma0, sigmag0, emitn, avgbeta, lambdau, current, lambdas, bfield;
        double sigmax, au, b, JJ, rho1D, rho3D, Lg1D, Lg3D, etad, etae, etag, CapG, Psat;
    public:
        //!< class initialization to calculate all physics objectives:
        //!< 1D and 3D FEL parameters and gain length, saturation power, as well.
        FELAnalysis(undulator &unduP, electronBeam &elecP, FELradiation &radiP);
        ~FELAnalysis();

        double get_FELparameter1D ();
        double get_FELparameter3D ();
        double get_FELgainlength1D();
        double get_FELgainlength3D();
        double get_FELsatpower    ();
};

class FELNumerical : public physicalConstants
{
    private:
        unsigned int npart, nstep, num;
        double gamma0, sigmag0, emitn, avgbeta, lambdau, current, lambdas, bfield;
        std::complex <double> seedEx, seedEy;
        double *psi, *gam;  //!< longitudinal phase space, phase [rad] and energy [gamma]
        double *zposArr;    //!< longitudinal undulator position, [m]
        double *bfArr;      //!< bunching factor
        std::complex <double> *ExArr, *EyArr; //!< electric field in x and y, [v/m]
        double maxExAmp, maxEyAmp;

        //!< for numerical calculation initialization
        double coef1, coef2, coef3, gammar, omegas, au, ku, sigmax, j0, ndelz;
        unsigned int totalIntSteps;
        double *K0Arr; //!< undulator parameter array, desinged for including errores
        double *JJArr; //!< coupled bessel factor array according to K0Arr

    public:
        //!< class initialization with input parameters
        FELNumerical(seedfield &seedP, undulator &unduP, electronBeam &elecP, FELradiation &radiP, controlpanel &contP);
        ~FELNumerical();

        unsigned int get_npart();
        double* get_psi();
        double* get_gam();
        double get_maxExAmp();
        double get_maxEyAmp();

        //!< generate initial electron longitudinal distribution
        void generateDistribution(double minpsi, double maxpsi);

        //!< parameters initialization for numerical calculation
        void initParams();

        void FELsolverSingleFrequency1D0();
        //!< FEL numerical calculation, single frequency in one-dimensional
        //!< by default Runge-Kutta algorithm is applied,
        //!< or define key word method as "EU1" or "EU2"
        void FELsolverSingleFrequency1D(std::string method = "RK4");

        //!< functions defined for ODE solver
        double odef1(double psi1, std::complex <double> Ex1, std::complex <double> j1, unsigned int idx);
        double odef2(double gam1);
        std::complex <double> odef3(std::complex <double> j1, unsigned int idx);

        //!< dump numerical simulation results
        void dumpResults();
};


//!< generate Gaussian distribution, average value of mu,
//!< standard deviation value of sigma
double* normrand(int N, double mu, double sigma);

//!< return the abs of average of a exp(double) array, i.e. <exp(-1i*h*psi)>
std::complex <double> meanexph(double* &a, unsigned int n, int h);

#endif //!< METHODS_H_
