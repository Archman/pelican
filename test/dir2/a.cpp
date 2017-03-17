#include "a.h"
#include <iostream>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <ctime>

FELnumerical::FELnumerical(int N, double g0, double dg0)
{
    npart  = N;
    gamma0 = g0;
    sigmag = dg0;
}

void FELnumerical::gendis(double minrad, double maxrad)
{
    psi = new double [npart];
    double delrad = (maxrad - minrad)/(npart-1);
    for(int i = 0; i < npart; i++)
    {
        psi[i] = minrad + i*delrad;
    }
    gam = normrand(npart,gamma0,sigmag);
}

double* FELnumerical::get_psi()
{
    return psi;
}

double* FELnumerical::get_gam()
{
    return gam;
}

int FELnumerical::get_npart()
{
    return npart;
}

double* normrand(int N, double mu, double sigma)
{
//    const gsl_rng_type *T;
    gsl_rng *r;
    r = gsl_rng_alloc(gsl_rng_taus);
    gsl_rng_set(r,time(NULL));

    double *distnum = new double [N];
    for(int i = 0; i < N; i++)    
        distnum[i] = mu + gsl_ran_gaussian(r, sigma);
    gsl_rng_free(r);

    return distnum;
}

double FELnumerical::fmean()
{
    cplxArr = new std::complex <double> [2];
    std::complex <double> s(0,0);
    std::complex <double> icplx(0,1);
    for(int ii = 0; ii < 2; ii++)
    {
        //cplxArr[ii] = {(double)rand()/RAND_MAX,(double)rand()/RAND_MAX};
        cplxArr[ii] = {(double)rand()/RAND_MAX,0};
        s += exp(cplxArr[ii]*icplx);
        std::cout << cplxArr[ii] << std::endl;
        
    }
    //std::cout << s/10.0 << std::endl;
    return abs(s/2.0);
}

