#ifndef SHRiMPS_Main_Cluster_Algorithm_H
#define SHRiMPS_Main_Cluster_Algorithm_H

#include "SHRiMPS/Main/Soft_Jet_Criterion.H"
#include "PDF/Main/Cluster_Definitions_Base.H"
#include "ATOOLS/Phys/Cluster_Amplitude.H"
#include "ATOOLS/Phys/Flavour.H"
#include "ATOOLS/Org/Run_Parameter.H"
#include "ATOOLS/Math/Histogram.H"
#include <map>
#include <list>

namespace ATOOLS { 
  class Blob;
  class Particle;
}

namespace PDF { 
  class Cluster_Definitions_Base; 
  class Jet_Criterion;
}

namespace SHRIMPS {
  typedef std::list<ATOOLS::Particle *> ParticleList;

  class Cluster_Algorithm: public ATOOLS::Mass_Selector {
  private:
    int           m_mode,m_nlad;
    bool          m_resc;
    ATOOLS::Vec4D m_rescvec;
    double        m_minkt2,m_Ymax;
    double        m_showerfac, m_tmax;

    std::map<std::string, ATOOLS::Histogram * > m_histomap;

    ATOOLS::Cluster_Amplitude     * p_ampl;
    PDF::Cluster_Definitions_Base * p_clus;
    ATOOLS::Mass_Selector         * p_ms;
    JF                            * p_jf;
    Soft_Jet_Criterion            * p_jets;
    
    void   CreateLegs(ATOOLS::Blob * const blob);
    void   InitLeg(ATOOLS::Cluster_Leg * leg,const double & kt2,
		   const size_t & nmaxx);
    double SetShowerScales();
    void   SetAmplitudeProperties(const double & scale);
    double PTij2(const ATOOLS::Vec4D & pi,const ATOOLS::Vec4D & pj) const;
    double PTi2(const ATOOLS::Vec4D & pi,const ATOOLS::Vec4D & pbeam) const;
    int    ColorConnected(const ATOOLS::ColorID &i,
			  const ATOOLS::ColorID &j) const;
    void   WriteOutAndDeleteHistograms();
  public:
    Cluster_Algorithm(const double & Ymax=10.,const double & minkt2=4.);

    ~Cluster_Algorithm();

    bool Cluster(ATOOLS::Blob *const blob);
    
    // inline functions
    inline ATOOLS::Cluster_Amplitude *Amplitude() { return p_ampl; }
    inline void SetShowerParams(const int & mode,const double & minkt2) {
      m_mode   = mode; 
      m_minkt2 = minkt2;
      if (m_mode<2) m_minkt2 = 0.;
    }
    inline void SetClusterDefinitions(PDF::Cluster_Definitions_Base *const cb) { p_clus=cb; }
    inline void SetShowerFac(const double & sfac) { m_showerfac = sfac; }
    inline void SetYmax(const double & Ymax)      { m_Ymax      = Ymax; }
    inline void SetMinKT2(const double & minkt2)  { m_minkt2    = minkt2; }
    inline void SetRescatt(const double & resc)  { m_resc = resc; }
    inline void SetTMax(const double & tmax)     { m_tmax = tmax; }
    inline void SetNLad(const int & nlad)        { m_nlad = nlad; }

    double Mass(const ATOOLS::Flavour &fl) const;
  };// end of class Cluster_Algorithm

}// end of namespace SHRiMPS

#endif
