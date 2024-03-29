#include "MCATNLO/Showers/Splitting_Function_Base.H"

namespace MCATNLO {
  
  class LF_VVV1_FF: public SF_Lorentz {
  public:

    inline LF_VVV1_FF(const SF_Key &key): SF_Lorentz(key) { m_col=1; }

    double operator()(const double,const double,const double,
		      const double,const double,ATOOLS::Cluster_Amplitude *const sub);
    double OverIntegrated(const double,const double,
			  const double,const double);
    double OverEstimated(const double,const double);
    double Z();
    double AsymmetryFactor(const double z,const double y,const double Q2);

  };

  class LF_VVV2_FF: public SF_Lorentz {
  public:

    inline LF_VVV2_FF(const SF_Key &key): SF_Lorentz(key) { m_col=-1; }

    double operator()(const double,const double,const double,
		      const double,const double,ATOOLS::Cluster_Amplitude *const sub);
    double OverIntegrated(const double,const double,
			  const double,const double);
    double OverEstimated(const double,const double);
    double Z();
    double AsymmetryFactor(const double z,const double y,const double Q2);

  };

  class LF_VVV1_FI: public SF_Lorentz {
  protected:

    double m_Jmax;

  public:

    inline LF_VVV1_FI(const SF_Key &key): SF_Lorentz(key) { m_col=1; }

    double operator()(const double,const double,const double,
		      const double,const double,ATOOLS::Cluster_Amplitude *const sub);
    double OverIntegrated(const double,const double,
			  const double,const double);
    double OverEstimated(const double,const double);
    double Z();
    double AsymmetryFactor(const double z,const double y,const double Q2);

  };

  class LF_VVV2_FI: public SF_Lorentz {
  protected:

    double m_Jmax;

  public:

    inline LF_VVV2_FI(const SF_Key &key): SF_Lorentz(key) { m_col=-1; }

    double operator()(const double,const double,const double,
		      const double,const double,ATOOLS::Cluster_Amplitude *const sub);
    double OverIntegrated(const double,const double,
			  const double,const double);
    double OverEstimated(const double,const double);
    double Z();
    double AsymmetryFactor(const double z,const double y,const double Q2);

  };

  class LF_VVV1_IF: public SF_Lorentz {
  protected:

    double m_Jmax;

  public:

    inline LF_VVV1_IF(const SF_Key &key): SF_Lorentz(key) { m_col=1; }

    double operator()(const double,const double,const double,
		      const double,const double,ATOOLS::Cluster_Amplitude *const sub);
    double OverIntegrated(const double,const double,
			  const double,const double);
    double OverEstimated(const double,const double);
    double Z();
    double AsymmetryFactor(const double z,const double y,const double Q2);

  };

  class LF_VVV2_IF: public SF_Lorentz {
  protected:

    double m_Jmax;

  public:

    inline LF_VVV2_IF(const SF_Key &key): SF_Lorentz(key) { m_col=-1; }

    double Scale(const double z,const double y,
		 const double _scale,const double Q2) const;
    double operator()(const double,const double,const double,
		      const double,const double,ATOOLS::Cluster_Amplitude *const sub);
    double OverIntegrated(const double,const double,
			  const double,const double);
    double OverEstimated(const double,const double);
    double Z();
    double AsymmetryFactor(const double z,const double y,const double Q2);

  };

  class LF_VVV1_II: public SF_Lorentz {
  protected:

    double m_Jmax;

  public:

    inline LF_VVV1_II(const SF_Key &key): SF_Lorentz(key) { m_col=1; }

    double operator()(const double,const double,const double,
		      const double,const double,ATOOLS::Cluster_Amplitude *const sub);
    double OverIntegrated(const double,const double,
			  const double,const double);
    double OverEstimated(const double,const double);
    double Z();
    double AsymmetryFactor(const double z,const double y,const double Q2);

  };

  class LF_VVV2_II: public SF_Lorentz {
  protected:

    double m_Jmax;

  public:

    inline LF_VVV2_II(const SF_Key &key): SF_Lorentz(key) { m_col=-1; }

    double Scale(const double z,const double y,
		 const double _scale,const double Q2) const;
    double operator()(const double,const double,const double,
		      const double,const double,ATOOLS::Cluster_Amplitude *const sub);
    double OverIntegrated(const double,const double,
			  const double,const double);
    double OverEstimated(const double,const double);
    double Z();
    double AsymmetryFactor(const double z,const double y,const double Q2);

  };

}

#include "ATOOLS/Math/Random.H"

using namespace MCATNLO;
using namespace PDF;
using namespace ATOOLS;

double LF_VVV1_FF::operator()
  (const double z,const double y,const double eta,
   const double scale,const double Q2,Cluster_Amplitude *const sub)
{
  double muk2  = sqr(p_ms->Mass(m_flspec))/Q2;
  //the massless case
  double massless = 2. * ( 1./(1.-z+z*y) -1. + z*(1.-z)/2.0 );
  if (muk2==0.) {
    double value = 2.0 * p_cf->Coupling(scale,0,sub) * massless;
    return value * JFF(y,0.0,0.0,0.0,0.0);
  }
  else {
    //the massive case
    double vijk  = (sqr(2.*muk2+(1.-muk2)*(1.-y))-4.*muk2);
    if (vijk<0.0) return 0.0;
    vijk=sqrt(vijk)/((1.-muk2)*(1.-y));
    double zm = 0.5*(1.- vijk);  
    double zp = 0.5*(1.+ vijk);
    double massive = 2. * ( 1./(1.-z+z*y) + (z*(1.-z)/2. - (1.0-s_kappa)*zp*zm/2. - 1.)/vijk );
    massive *= 1./(1.-muk2);
    double value = 2.0 * p_cf->Coupling(scale,0,sub) * massive;
    return value * JFF(y,0.0,0.0,muk2,0.0);
  }
}

double LF_VVV1_FF::AsymmetryFactor(const double z,const double y,const double Q2)
{
  double muk2  = p_ms->Mass2(m_flspec)/Q2;
  if (muk2==0.) {
    return ( 1./(1.-z+z*y) -1. + z*(1.-z)/2.0 ) /
      ( 1./(1.-z+z*y) + 1./(z+y-z*y) -2. + z*(1.-z) );
  }
  double vijk  = (sqr(2.*muk2+(1.-muk2)*(1.-y))-4.*muk2);
  if (vijk<0.0) return 0.0;
  vijk=sqrt(vijk)/((1.-muk2)*(1.-y));
  double zm = 0.5*(1.- vijk), zp = 0.5*(1.+ vijk);
  return ( 1./(1.-z+z*y) + (z*(1.-z)/2. - (1.0-s_kappa)*zp*zm/2. - 1.)/vijk ) /
    ( 1./(1.-z+z*y) + 1./(z+y-z*y) + (z*(1.-z) - (1.0-s_kappa)*zp*zm - 2.)/vijk );
}

double LF_VVV1_FF::OverIntegrated
(const double zmin,const double zmax,const double scale,const double xbj)
{
  m_zmin = zmin; m_zmax = zmax;
  return 4.*p_cf->MaxCoupling(0) * log((1.-m_zmin)/(1.-m_zmax));
}

double LF_VVV1_FF::OverEstimated(const double z,const double y)
{
  return 4.*p_cf->MaxCoupling(0) * ( 1./(1.-z) );
}

double LF_VVV1_FF::Z()
{
  return 1.-(1.-m_zmin)*pow((1.-m_zmax)/(1.-m_zmin),ATOOLS::ran->Get());
}

double LF_VVV2_FF::operator()
  (const double z,const double y,const double eta,
   const double scale,const double Q2,Cluster_Amplitude *const sub)
{
  double muk2  = sqr(p_ms->Mass(m_flspec))/Q2;
  //the massless case
  double massless = 2. * ( 1./(z+y-z*y) -1. + z*(1.-z)/2.0 );
  if (muk2==0.) {
    double value = 2.0 * p_cf->Coupling(scale,0,sub) * massless;
    return value * JFF(y,0.0,0.0,0.0,0.0);
  }
  else {
    //the massive case
    double vijk  = (sqr(2.*muk2+(1.-muk2)*(1.-y))-4.*muk2);
    if (vijk<0.0) return 0.0;
    vijk=sqrt(vijk)/((1.-muk2)*(1.-y));
    double zm = 0.5*(1.- vijk);  
    double zp = 0.5*(1.+ vijk);
    double massive = 2. * ( 1./(z+y-z*y) + (z*(1.-z)/2. - (1.0-s_kappa)*zp*zm/2. - 1.)/vijk );
    massive *= 1./(1.-muk2);
    double value = 2.0 * p_cf->Coupling(scale,0,sub) * massive;
    return value * JFF(y,0.0,0.0,muk2,0.0);
  }
}

double LF_VVV2_FF::AsymmetryFactor(const double z,const double y,const double Q2)
{
  double muk2  = p_ms->Mass2(m_flspec)/Q2;
  if (muk2==0.) {
    return ( 1./(z+y-z*y) -1. + z*(1.-z)/2.0 ) /
      ( 1./(1.-z+z*y) + 1./(z+y-z*y) -2. + z*(1.-z) );
  }
  double vijk  = (sqr(2.*muk2+(1.-muk2)*(1.-y))-4.*muk2);
  if (vijk<0.0) return 0.0;
  vijk=sqrt(vijk)/((1.-muk2)*(1.-y));
  double zm = 0.5*(1.- vijk), zp = 0.5*(1.+ vijk);
  return ( 1./(z+y-z*y) + (z*(1.-z)/2. - (1.0-s_kappa)*zp*zm/2. - 1.)/vijk ) /
    ( 1./(1.-z+z*y) + 1./(z+y-z*y) + (z*(1.-z) - (1.0-s_kappa)*zp*zm - 2.)/vijk );
}

double LF_VVV2_FF::OverIntegrated
(const double zmin,const double zmax,const double scale,const double xbj)
{
  m_zmin = zmin; m_zmax = zmax;
  return 4.*p_cf->MaxCoupling(0) * log(m_zmax/m_zmin);
}

double LF_VVV2_FF::OverEstimated(const double z,const double y)
{
  return 4.*p_cf->MaxCoupling(0) * ( 1./z );
}

double LF_VVV2_FF::Z()
{
  return m_zmin*pow(m_zmax/m_zmin,ATOOLS::ran->Get());
}

double LF_VVV1_FI::operator()
  (const double z,const double y,const double eta,
   const double scale,const double Q2,Cluster_Amplitude *const sub)
{
  double value = 4.0*p_cf->Coupling(scale,0,sub) * ( z/(1.-z+y) + z*(1.-z)/2.0 );
  return value * JFI(y,eta,scale,sub);
}

double LF_VVV1_FI::AsymmetryFactor(const double z,const double y,const double Q2)
{
  return ( z/(1.-z+y) + z*(1.-z)/2.0 ) /
    ( z/(1.-z+y) + (1.-z)/(z+y) + z*(1.-z) );
}

double LF_VVV1_FI::OverIntegrated
(const double zmin,const double zmax,const double scale,const double xbj)
{
  m_zmin = zmin; m_zmax = zmax;
  m_Jmax=5.;
  return 4.*p_cf->MaxCoupling(0) * log((1.-m_zmin)/(1.-m_zmax)) * m_Jmax;
}

double LF_VVV1_FI::OverEstimated(const double z,const double y)
{
  return 4.*p_cf->MaxCoupling(0) * ( 1./(1.-z) ) * m_Jmax;
}

double LF_VVV1_FI::Z()
{
  return 1.-(1.-m_zmin)*pow((1.-m_zmax)/(1.-m_zmin),ATOOLS::ran->Get());
}

double LF_VVV2_FI::operator()
  (const double z,const double y,const double eta,
   const double scale,const double Q2,Cluster_Amplitude *const sub)
{
  double value = 4.0*p_cf->Coupling(scale,0,sub) * ( (1.-z)/(z+y) + z*(1.-z)/2.0 );
  return value * JFI(y,eta,scale,sub);
}

double LF_VVV2_FI::AsymmetryFactor(const double z,const double y,const double Q2)
{
  return ( (1.-z)/(z+y) + z*(1.-z)/2.0 ) /
    ( z/(1.-z+y) + (1.-z)/(z+y) + z*(1.-z) );
}

double LF_VVV2_FI::OverIntegrated
(const double zmin,const double zmax,const double scale,const double xbj)
{
  m_zmin = zmin; m_zmax = zmax;
  m_Jmax=5.;
  return 4.*p_cf->MaxCoupling(0) * log(m_zmax/m_zmin) * m_Jmax;
}

double LF_VVV2_FI::OverEstimated(const double z,const double y)
{
  return 4.*p_cf->MaxCoupling(0) * ( 1./z ) * m_Jmax;
}

double LF_VVV2_FI::Z()
{
  return m_zmin*pow(m_zmax/m_zmin,ATOOLS::ran->Get());
}

double LF_VVV1_IF::operator() 
  (const double z,const double y,const double eta,
   const double scale,const double Q2,Cluster_Amplitude *const sub)
{
  double mk2 = p_ms->Mass2(m_flspec), muk2 = mk2/(Q2+mk2);
  double massless = 2. * ( z/(1.-z+y) + (1.-z)/z/2.0);
  if (muk2==0.) {
    //the massless case
    double value = 2.0 * p_cf->Coupling(scale,0,sub) * massless;
    return value * JIF(z,y,eta,scale,sub);
  }
  else {
    //the massive case
    double massive = massless - muk2*y/(1.-y);
    double value = 2.0 * p_cf->Coupling(scale,0,sub) * massive;
    return value * JIF(z,y,eta,scale,sub);
  }
}

double LF_VVV1_IF::AsymmetryFactor(const double z,const double y,const double Q2)
{
  double mk2 = p_ms->Mass2(m_flspec), mt = mk2/(Q2+mk2)*y/(1.-y);
  return ( z/(1.-z+y) + (1.-z)/z/2.0 - mt/2.0 ) /
    ( z/(1.-z+y) + (1.-z)/z + z*(1.-z) - mt );
}

double LF_VVV1_IF::OverIntegrated
(const double zmin,const double zmax,const double scale,const double xbj)
{
  m_zmin = zmin; m_zmax = zmax;
  m_Jmax = 1.; 
  return 4.*p_cf->MaxCoupling(0) * log((1.-m_zmin)*m_zmax/(m_zmin*(1.-m_zmax))) * m_Jmax;
}

double LF_VVV1_IF::OverEstimated(const double z,const double y)
{
  return 4.*p_cf->MaxCoupling(0) * ( 1./(z*(1.-z)) ) * m_Jmax;
}

double LF_VVV1_IF::Z()
{
  return 1./(1. + ((1.-m_zmin)/m_zmin) *
             pow( m_zmin*(1.-m_zmax)/((1.-m_zmin)*m_zmax), ATOOLS::ran->Get()));
}

double LF_VVV2_IF::Scale
(const double z,const double y,
 const double _scale,const double Q2) const
{
  if (p_sf->ScaleScheme()&8) return _scale;
  double scale = (Q2+p_ms->Mass2(m_flspec))*y/z;
  return scale;
}

double LF_VVV2_IF::operator() 
  (const double z,const double y,const double eta,
   const double scale,const double Q2,Cluster_Amplitude *const sub)
{
  double mk2 = p_ms->Mass2(m_flspec), muk2 = mk2/(Q2+mk2);
  double massless = 2. * ( z*(1.-z) + (1.-z)/z/2.0);
  if (muk2==0.) {
    //the massless case
    double value = 2.0 * p_cf->Coupling(scale,0,sub) * massless;
    return value * JIF(z,y,eta,scale,sub);
  }
  else {
    //the massive case
    double massive = massless - muk2*y/(1.-y);
    double value = 2.0 * p_cf->Coupling(scale,0,sub) * massive;
    return value * JIF(z,y,eta,scale,sub);
  }
}

double LF_VVV2_IF::AsymmetryFactor(const double z,const double y,const double Q2)
{
  double mk2 = p_ms->Mass2(m_flspec), mt = mk2/(Q2+mk2)*y/(1.-y);
  return ( z*(1.-z) + (1.-z)/z/2.0 - mt/2.0 ) /
    ( z/(1.-z+y) + (1.-z)/z + z*(1.-z) - mt );
}

double LF_VVV2_IF::OverIntegrated
(const double zmin,const double zmax,const double scale,const double xbj)
{
  m_zmin = zmin; m_zmax = zmax;
  m_Jmax = 1.; 
  return 4.*p_cf->MaxCoupling(0) * log(m_zmax/m_zmin) * m_Jmax;
}

double LF_VVV2_IF::OverEstimated(const double z,const double y)
{
  return 4.*p_cf->MaxCoupling(0) * ( 1./z ) * m_Jmax;
}

double LF_VVV2_IF::Z()
{
  return m_zmin*pow(m_zmax/m_zmin,ATOOLS::ran->Get());
}

double LF_VVV1_II::operator()
  (const double z,const double y,const double eta,
   const double scale,const double Q2,Cluster_Amplitude *const sub)
{
  double value = 4.0 * p_cf->Coupling(scale,0,sub) * ( (z+y)/(1.-z) + (1.-z-y)/(z+y)/2.0);
  return value * JII(z,y,eta,scale,sub);
}

double LF_VVV1_II::AsymmetryFactor(const double z,const double y,const double Q2)
{
  return ( (z+y)/(1.-z) + (1.-z-y)/(z+y)/2.0 ) /
    ( (z+y)/(1.-z) + (1.-z-y)/(z+y) + (z+y)*(1.-z-y) );
}

double LF_VVV1_II::OverIntegrated
(const double zmin,const double zmax,const double scale,const double xbj)
{
  m_zmin = zmin; m_zmax = zmax;
  m_Jmax = 1.;
  return 4.*p_cf->MaxCoupling(0) * log((1.-m_zmin)*m_zmax/(m_zmin*(1.-m_zmax))) * m_Jmax;
}

double LF_VVV1_II::OverEstimated(const double z,const double y)
{
  return 4.*p_cf->MaxCoupling(0) * ( 1./(z*(1.-z)) ) * m_Jmax;
}

double LF_VVV1_II::Z()
{
  return 1./(1. + ((1.-m_zmin)/m_zmin) *
             pow( m_zmin*(1.-m_zmax)/((1.-m_zmin)*m_zmax), ATOOLS::ran->Get()));
}

double LF_VVV2_II::Scale
(const double z,const double y,
 const double _scale,const double Q2) const
{
  if (p_sf->ScaleScheme()&8) return _scale;
  double scale = (Q2-p_ms->Mass2(m_flspec))*y/z;
  return scale;
}

double LF_VVV2_II::operator()
  (const double z,const double y,const double eta,
   const double scale,const double Q2,Cluster_Amplitude *const sub)
{
  double value = 4.0 * p_cf->Coupling(scale,0,sub) * ( (z+y)*(1.-z-y) + (1.-z-y)/(z+y)/2.0);
  return value * JII(z,y,eta,scale,sub);
}

double LF_VVV2_II::AsymmetryFactor(const double z,const double y,const double Q2)
{
  return ( (z+y)*(1.-z-y) + (1.-z-y)/(z+y)/2.0 ) /
    ( (z+y)/(1.-z) + (1.-z-y)/(z+y) + (z+y)*(1.-z-y) );
}

double LF_VVV2_II::OverIntegrated
(const double zmin,const double zmax,const double scale,const double xbj)
{
  m_zmin = zmin; m_zmax = zmax;
  m_Jmax = 1.;
  return 4.*p_cf->MaxCoupling(0) * log(m_zmax/m_zmin) * m_Jmax;
}

double LF_VVV2_II::OverEstimated(const double z,const double y)
{
  return 4.*p_cf->MaxCoupling(0) * ( 1./z ) * m_Jmax;
}

double LF_VVV2_II::Z()
{
  return m_zmin*pow(m_zmax/m_zmin,ATOOLS::ran->Get());
}

DECLARE_GETTER(LF_VVV1_FF,"VVV",SF_Lorentz,SF_Key);

SF_Lorentz *ATOOLS::Getter<SF_Lorentz,SF_Key,LF_VVV1_FF>::
operator()(const Parameter_Type &args) const
{
  if (args.m_col==1) {
  switch (args.m_type) {
  case cstp::FF: return new LF_VVV1_FF(args);
  case cstp::FI: return new LF_VVV1_FI(args);
  case cstp::IF: return new LF_VVV1_IF(args);
  case cstp::II: return new LF_VVV1_II(args);
  case cstp::none: break;
  }
  }
  else {
  switch (args.m_type) {
  case cstp::FF: return new LF_VVV2_FF(args);
  case cstp::FI: return new LF_VVV2_FI(args);
  case cstp::IF: return new LF_VVV2_IF(args);
  case cstp::II: return new LF_VVV2_II(args);
  case cstp::none: break;
  }
  }
  return NULL;
}

void ATOOLS::Getter<SF_Lorentz,SF_Key,LF_VVV1_FF>::
PrintInfo(std::ostream &str,const size_t width) const
{
  str<<"vvv lorentz functions";
}
