#ifndef PHASIC_Channels_Rambo_H
#define PHASIC_Channels_Rambo_H

#include "PHASIC++/Channels/Single_Channel.H"

namespace PHASIC {
  class Rambo : public Single_Channel {
    bool     massflag;
    double   Z_N, * xm2, * p2, * E;
    short int itmax;
    double accu;
    const ATOOLS::Mass_Selector* p_masssel;
  public:
    Rambo(size_t,size_t,const ATOOLS::Flavour *,const ATOOLS::Mass_Selector* ms=NULL);
    Rambo(size_t nin, std::vector<double> masses);
    ~Rambo();
    void Init(const size_t& nin, const size_t& nout, const std::vector<double>& masses);
    ATOOLS::Vec4D_Vector GeneratePoint(const double& E);
    void GenerateWeight(ATOOLS::Vec4D *,Cut_Data *);  
    void GeneratePoint(ATOOLS::Vec4D *,Cut_Data *);
    void GenerateWeight(ATOOLS::Vec4D * p) { GenerateWeight(p,NULL); }  
    void GeneratePoint(ATOOLS::Vec4D * p)  { GeneratePoint(p,NULL); }
    void GeneratePoint(ATOOLS::Vec4D *,Cut_Data *,double *);
    void MassiveWeight(ATOOLS::Vec4D *,double);
    void MassivePoint(ATOOLS::Vec4D *,double);
    std::string Name() { return std::string("Rambo"); }
    int CountResonances(ATOOLS::Flavour *&) { return 0; }
    void MPISync() {}
  };
  //! Flag to see wether massive or massless points are to be generated.
  //! Constants needed for the weight and the phase space distribution.
  //! Maximal number of iterations to generate the massive weight
  //! Accuracy when generating the massive weight.
  /*!
    Generates the weight. In case of massless Rambo this is given by
    a constant, since Rambo uses uniform distribution. For massive
    points, this constant is corrected via MassiveWeight.
    /sa MassiveWeight
  */
  /*!
    To generate a phase space point according to a uniform distribution.
    In case massive vectors are needed this point gets modified by 
    MassivePoint.
    /sa MassivePoint
  */
  /*!
    This method is only a dummy method meant to comply with the multi-channel.
    In fact it calls the GeneratePoint method without random numbers.
  */
  /*!
    To generate a massive weight. The idea is to invert the generation of
    massive vectors, i.e. we start with massive vectors and force them
    on a zero-mass shell by rescaling and boosting all momenta by common 
    factors, such that both four momentum conservation and mass shell 
    conditions are fulfilled.
  */
  /*!
    To generate a massive point. The idea is to start from a massless point
    given by ordinary RAMBO::GeneratePoint and then to force them
    on their mass shell by rescaling and boosting all momenta by common 
    factors, such that both four momentum conservation and mass shell 
    conditions are fulfilled.
  */
}

#endif
