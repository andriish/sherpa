#ifndef PHASIC_Channels_Channel_Elements_H
#define PHASIC_Channels_Channel_Elements_H

#include "ATOOLS/Math/Vector.H"
#include "ATOOLS/Org/Info_Key.H"

namespace PHASIC {

  double SqLam(double,double,double);
  double PeakedDist(double,double,double,double,int,double);
  double PeakedWeight(double,double,double,double,double,int,double&);
  double ExponentialDist(double ca,double cxm,double cxp,double ran);
  double ExponentialWeight(double ca,double cxm,double cxp);
  double BoundaryPeakedDist(double cxm,double cxp,double ran);
  double BoundaryPeakedWeight(double cxm,double cxp,double res,double &ran);
  double Tj1(double,double,double,double);
  double Hj1(double,double,double);

  class Channel_Elements {
  private:

    void QcdAntenna(ATOOLS::Vec4D* &,int,double, double);
    void BasicAntenna(ATOOLS::Vec4D,ATOOLS::Vec4D,ATOOLS::Vec4D &, double*, double);
    void PermP(int,int* &);
    void Polytope(int,double* &);
    void CheckMasses(const double & s1,ATOOLS::Vec4D & p1,
		     const double & s2,ATOOLS::Vec4D & p2) const;
  public:

    double Isotropic2Weight(const ATOOLS::Vec4D&,const ATOOLS::Vec4D&,double&,double&,double=-1.0,double=1.0,
			    const ATOOLS::Vec4D &x=ATOOLS::Vec4D(1.,1.,0.,0.));
    void Isotropic2Momenta(ATOOLS::Vec4D,double,double,
			   ATOOLS::Vec4D&,ATOOLS::Vec4D&,double,double,double=-1.0,double=1.0,
			   const ATOOLS::Vec4D &x=ATOOLS::Vec4D(1.,1.,0.,0.));

    double Anisotropic2Weight(const ATOOLS::Vec4D&,const ATOOLS::Vec4D&,
			      double&,double&,double,double=-1.0,double=1.0,
			      const ATOOLS::Vec4D &x=ATOOLS::Vec4D(1.,1.,0.,0.));
    void Anisotropic2Momenta(ATOOLS::Vec4D,double,double,
			     ATOOLS::Vec4D&,ATOOLS::Vec4D&,double,double,
			     double,double=-1.0,double=1.0,
			     const ATOOLS::Vec4D &x=ATOOLS::Vec4D(1.,1.,0.,0.));

    double TChannelWeight(const ATOOLS::Vec4D&,const ATOOLS::Vec4D&,
			  const ATOOLS::Vec4D&,const ATOOLS::Vec4D&,
			  double,double,double,double,double&,double&);
    void TChannelMomenta(ATOOLS::Vec4D,ATOOLS::Vec4D,ATOOLS::Vec4D&,ATOOLS::Vec4D&,
			double,double,double,double,double,double,double,double);

    double BremsstrahlungWeight(double,double,double,
				const ATOOLS::Vec4D&,const ATOOLS::Vec4D&);
    void BremsstrahlungMomenta(ATOOLS::Vec4D&,const double,const double,const double,
			       const double,const double,const double,
			       ATOOLS::Vec4D &, ATOOLS::Vec4D &,const double,const double);

    double MasslessPropMomenta(double,double,double,double);
    double MasslessPropWeight(double,double,double,double,double&);
    double MasslessPropMomenta(double,double,double,double,double);
    double MasslessPropWeight(double,double,double,double,double,double&);

    double ExponentialMomenta(double,double,double,double[],double);
    double ExponentialWeight(double,double,double,double[],double,double&);

    double AntennaWeight(double amin,double amax,const double a,double &ran);
    double AntennaMomenta(double amin,double amax,double ran);

    double MassivePropWeight(double,double,double,double,double,double&);
    double MassivePropMomenta(double,double,double,double,double);

    double QCDAPWeight(ATOOLS::Vec4D*,int,double);
    void QCDAPMomenta(ATOOLS::Vec4D*,ATOOLS::Vec4D,int,double);

    double LLPropWeight(double,double,double,double,double,double&);
    double LLPropMomenta(double,double,double,double,double);

    double ThresholdMomenta(double,double,double,double,double);
    double ThresholdWeight(double,double,double,double,double,double&);

    double GenerateDMRapidityUniform(const double masses[], const ATOOLS::Double_Container &spinfo,
      ATOOLS::Double_Container &xinfo, const double cosxi, const double ran, const int mode);
    double GenerateDMAngleUniform(const double ran, const int mode) const;
    double GenerateYUniform(const double tau,const ATOOLS::Double_Container &xinfo,
			const ATOOLS::Double_Container &yinfo,const double ran,const int mode) const;
    double WeightYUniform(const double tau,const ATOOLS::Double_Container &xinfo,
			  const ATOOLS::Double_Container &yinfo,double&,const int mode) const;
    double GenerateYCentral(const double tau,const ATOOLS::Double_Container &xinfo,
			const ATOOLS::Double_Container &yinfo,const double ran,const int mode) const;
    double WeightYCentral(const double tau,const ATOOLS::Double_Container &xinfo,
			  const ATOOLS::Double_Container &yinfo,double&,const int mode) const;
    double GenerateYForward(const double yexponent,const double tau,
			const ATOOLS::Double_Container &xinfo,
			const ATOOLS::Double_Container &yinfo,const double ran,const int mode) const;
    double WeightYForward(const double yexponent,const double tau,
			  const ATOOLS::Double_Container &xinfo,
			  const ATOOLS::Double_Container &yinfo,double&,const int mode) const;
    double GenerateYBackward(const double yexponent,const double tau,
			 const ATOOLS::Double_Container &xinfo,
			 const ATOOLS::Double_Container &yinfo,const double ran,const int mode) const;
    double WeightYBackward(const double yexponent,const double tau,
			   const ATOOLS::Double_Container &xinfo,
			   const ATOOLS::Double_Container &yinfo,double&,const int mode) const;

  };// end of class Channel_Elements

  extern Channel_Elements CE;

  /*!
    Weights and momenta for isotropic two-body decays.
    The idea for the gbeneration of the momenta is to boost the
    decaying vector into its rest frame, select the orientation of
    the two decay products with given masses 4-pi isotropically and
    to boost back. The weight is just a constant.
  */

  /*!
    Weights and momenta for anisotropic two-body decays as they occur
    in splittings involving one outgoing massless vector boson.
    The idea here is to use a peaked distribution for the polar angle
    of one decay product (not the "vector boson") along the original direction
    of the decaying particle.
  */

  /*!
    Weights and momenta for a Tchannel process. Again, there is a strong peak in
    the forward direction in order to minimize the mass of the propagator.
  */

  /*!
     Decay p -> q + p1, q is space-like with energy Eq given from outside
     cos(pq) is constrained by ctmin and ctmax.
  */

  /*!
    Scale and weight for a massless propagator. It is distributed
    according to a simple pole structure peaking at 0 (or the minimal
    scale that is kinematically allowed).
  */

  /*!
    Scale and weight for a massive propagator. It is distributed
    according to a physical Breit-Wigner distribution with given
    mass and width.
  */

  /*!
    Scale and weight for a typical electron initial state radiation
    event. It is distributed according to a simple pole structure peaking
    at the maximal scale that is kinematically allowed.
  */

}// end of namespace PHASIC

#endif
