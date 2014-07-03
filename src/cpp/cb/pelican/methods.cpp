#include "methods.h"
#include <cmath>
#include <ctime>
#include <gsl/gsl_sf_bessel.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

#include <iostream>
#include <fstream>

FELAnalysis::FELAnalysis(undulator &unduP, electronBeam &elecP, FELradiation &radiP)
{
    gamma0  = elecP.get_centralEnergy();
    sigmag0 = elecP.get_energySpread ();
    emitn   = elecP.get_emitnx       ();
    avgbeta = elecP.get_avgBetaFunc  ();
    lambdau = unduP.get_period       ();
    bfield  = unduP.get_field        ();
    current = elecP.get_peakCurrent  ();
    lambdas = radiP.get_wavelength   ();

    sigmax = sqrt(avgbeta*emitn/gamma0);
    au = sqrt(lambdas*2*gamma0*gamma0/lambdau-1);
    //au     = 0.934*bfield*lambdau*100/sqrt(2.0);
    b      = au*au/2.0/(1+au*au);
    JJ     = gsl_sf_bessel_J0(b) - gsl_sf_bessel_J1(b);
    rho1D  = pow((0.5/gamma0)*(0.5/gamma0)*(0.5/gamma0)*current/IA*(au*lambdau*JJ/2.0/PI/sigmax)*(au*lambdau*JJ/2.0/PI/sigmax),1.0/3.0);
    Lg1D   = lambdau/4/PI/sqrt(3)/rho1D;
    etad = Lg1D/(4*PI*sigmax*sigmax/lambdas);
    etae = Lg1D/avgbeta*4*PI*emitn/gamma0/lambdas;
    etag = Lg1D/lambdau*4*PI*sigmag0/gamma0;
    CapG = coefs.a1 *pow(etad,coefs.a2 )
         + coefs.a3 *pow(etae,coefs.a4 )
         + coefs.a5 *pow(etag,coefs.a6 )
         + coefs.a7 *pow(etae,coefs.a8 )*pow(etag,coefs.a9 )
         + coefs.a10*pow(etad,coefs.a11)*pow(etag,coefs.a12)
         + coefs.a13*pow(etad,coefs.a14)*pow(etae,coefs.a15)
         + coefs.a16*pow(etad,coefs.a17)*pow(etae,coefs.a18)*pow(etag,coefs.a19);
    Lg3D  = Lg1D*(1+CapG);
    rho3D = lambdau/4/PI/sqrt(3)/Lg3D;
    Psat  = 1.6*rho1D*Lg1D*Lg1D/(Lg3D*Lg3D)*gamma0*0.511*current/1000.0; // GW
}

double FELAnalysis::get_FELparameter1D()
{
    return rho1D;
}

double FELAnalysis::get_FELparameter3D()
{
    return rho3D;
}

double FELAnalysis::get_FELgainlength1D()
{
    return Lg1D;
}

double FELAnalysis::get_FELgainlength3D()
{
    return Lg3D;
}

double FELAnalysis::get_FELsatpower()
{
    return Psat;
}


FELNumerical::FELNumerical(seedfield &seedP, undulator &unduP, electronBeam &elecP, FELradiation &radiP, controlpanel &contP)
{
    seedEx  = seedP.get_Ex           ();
    seedEy  = seedP.get_Ey           ();
    gamma0  = elecP.get_centralEnergy();
    sigmag0 = elecP.get_energySpread ();
    emitn   = elecP.get_emitnx       ();
    avgbeta = elecP.get_avgBetaFunc  ();
    lambdau = unduP.get_period       ();
    bfield  = unduP.get_field        ();
    nstep   = unduP.get_nstep        ();
    num     = unduP.get_num          ();
    current = elecP.get_peakCurrent  ();
    lambdas = radiP.get_wavelength   ();
    npart   = contP.get_npart        ();
}

void FELNumerical::generateDistribution(double minpsi, double maxpsi)
{
    psi = new double [npart];
    double deltpsi = (maxpsi - minpsi)/(npart - 1);
    for(unsigned int i = 0; i < npart; i++)
    {
        psi[i] = minpsi + i*deltpsi;
    }
    gam = normrand(npart, gamma0, sigmag0);
}

void FELNumerical::initParams()
{
    au     = 0.934*bfield*100*lambdau/sqrt(2);          // normalized undulator parameter, calculated from the input parameters read from namelist file
    ku     = 2*PI/lambdau;                          // undulator wave number
    omegas = 2*PI*C0/lambdas;                       // radiation angular frequency
    gammar = sqrt(lambdau*(1+au*au)/2.0/lambdas);   // resonant gamma
    sigmax = sqrt(avgbeta*emitn/gamma0);
    j0     = current/(PI*sigmax*sigmax);            // transverse current density
    coef1  = E0/M0/C0/C0;
    coef2  = mu0*C0*C0/omegas;
    coef3  = mu0*C0/4.0/gammar;
    ndelz  = lambdau/nstep;                         // integration step size, [m]
    totalIntSteps = (int) nstep*num;                // total integration steps
    K0Arr   = new double [totalIntSteps];
    JJArr   = new double [totalIntSteps];
    zposArr = new double [totalIntSteps];
    bfArr   = new double [totalIntSteps];
    ExArr   = new std::complex <double> [totalIntSteps];
    EyArr   = new std::complex <double> [totalIntSteps];

    double btmp;
    for(int i = 0; i < totalIntSteps; i++)
    {
        K0Arr[i] = au*sqrt(2);
        btmp = K0Arr[i]*K0Arr[i]/(4.0+2.0*K0Arr[i]*K0Arr[i]);
        JJArr[i] = gsl_sf_bessel_J0(btmp) - gsl_sf_bessel_J1(btmp);
    }
}

void FELNumerical::FELsolverSingleFrequency1D()
{
    std::complex <double> icplx(0, 1), tmpcplx, Ex(seedEx), j1;

    for(unsigned int intzstep = 0; intzstep < totalIntSteps; intzstep++)
    {
        j1 = 2*j0*meanexph(psi, npart, 1);
        for(unsigned int i = 0; i < npart; i++)
        {
            tmpcplx = (K0Arr[intzstep]*JJArr[intzstep]*Ex/2.0/gammar-icplx*coef2*j1)*exp(icplx*psi[i]);
            gam[i] -= coef1*tmpcplx.real()*ndelz;
            psi[i] += 2*ku*(gam[i]/gammar-1.0)*ndelz;
        }
        Ex -= coef3*K0Arr[intzstep]*JJArr[intzstep]*j1*ndelz;
        zposArr[intzstep] = intzstep*ndelz;
        ExArr[intzstep]   = Ex;
    }
}

void FELNumerical::dumpResults()
{
    std::ofstream out("tmpout");
    for(int i= 0; i < totalIntSteps; i++)
    {
        out << zposArr[i] << " " << 0.5*pow(abs(ExArr[i]),2)*eps0*C0*PI*sigmax*sigmax << std::endl;
    }
}

double* FELNumerical::get_psi()
{
    return psi;
}

double* FELNumerical::get_gam()
{
    return gam;
}

unsigned int FELNumerical::get_npart()
{
    return npart;
}



// global functions
double* normrand(int N, double mu, double sigma)
{
    gsl_rng *r;
    r = gsl_rng_alloc(gsl_rng_taus);
    gsl_rng_set(r, time(NULL));
    double *distnum = new double [N];
    for(int i = 0; i < N; i++)
        distnum[i] = mu + gsl_ran_gaussian(r, sigma);
    gsl_rng_free(r);
    return distnum;
}

std::complex <double> meanexph(double* &a, unsigned int n, int h)
{
    std::complex <double> s(0, 0), icplx(0, -1);

    for(unsigned int j = 0; j < n; j++)
        s += exp(a[j]*icplx*(std::complex <double>)h);
    return 1.0/(double)n*s;
}
