#ifndef AHADIC_Tools_Soft_Cluster_Handler_H
#define AHADIC_Tools_Soft_Cluster_Handler_H

#include "AHADIC++/Tools/KT_Selector.H"
#include "AHADIC++/Tools/Single_Transitions.H"
#include "AHADIC++/Tools/Double_Transitions.H"
#include "AHADIC++/Tools/Cluster.H"

namespace AHADIC {
  class Soft_Cluster_Handler {
  private:
    std::list<Proto_Particle *> * p_hadrons; 
    Constituents                * p_constituents;
    Single_Transitions          * p_singletransitions;
    Double_Transitions          * p_doubletransitions;
    KT_Selector                   m_ktselector;
    double m_dec_threshold,m_trans_threshold, m_chi, m_ktmax, m_ktfac, m_zeta;
    double m_piphoton_threshold, m_dipion_threshold, m_open_threshold;

    Cluster    * p_cluster;
    double       m_mass2, m_mass;
    Flavour_Pair m_flavs;

    bool m_forceddecay, m_direct_transition, m_ktorder;
    
    ATOOLS::Vec4D   m_moms[2];
    ATOOLS::Flavour m_hads[2];

    void   FillFlavours(Cluster * cluster);
    int    CheckOutsideRange();
    int    Decay();
    double PhaseSpace(const double & m2,const double & m3,const bool heavyB);
    double DecayWeight();
    double RadiationWeight(const bool & withPS=true);
    double Annihilation();
    bool   FixKinematics();
    bool   DiQuarkToQuarks(const ATOOLS::Flavour & di,
			   ATOOLS::Flavour & q1,ATOOLS::Flavour & q2);
    bool   AnnihilateFlavour(const ATOOLS::Flavour & one1,
			     const ATOOLS::Flavour & one2,
			     const ATOOLS::Flavour & two1,
			     const ATOOLS::Flavour & two2);
    double DefineHadronsInAnnihilation(const Flavour_Pair & one,
				       const Flavour_Pair & two);
    bool   TreatSingletCluster();
 public:
    Soft_Cluster_Handler(std::list<Proto_Particle *> * hadrons);
    ~Soft_Cluster_Handler();

    void Init();
    void Reset();
    bool PromptTransit(Cluster * cluster,ATOOLS::Flavour & had);
    bool MustPromptDecay(Cluster * cluster);
    bool MustPromptDecay(const ATOOLS::Flavour & fl1,
			 const ATOOLS::Flavour & fl2,
			 const double & mass);
    int  Treat(Cluster * cluster,bool force=false);
    bool TreatTwoGluons(Cluster * cluster);
    bool RadiativeDecay(Cluster * cluster);
    bool Rescue(Cluster * cluster);

    double TransitionThreshold(const ATOOLS::Flavour & fl1,
			       const ATOOLS::Flavour & fl2);
    double DecayThreshold(const ATOOLS::Flavour & fl1,
			  const ATOOLS::Flavour & fl2);
    double MinSingleMass(const ATOOLS::Flavour & fl1,
			 const ATOOLS::Flavour & fl2);
    ATOOLS::Flavour LowestTransition(const ATOOLS::Flavour & fl1,
				     const ATOOLS::Flavour & fl2);
    double MinDoubleMass(const ATOOLS::Flavour & fl1,
			 const ATOOLS::Flavour & fl2);
    ATOOLS::Flavour GetDecHadron(const size_t & i) { return m_hads[i]; }

    std::list<Proto_Particle *> * GetHadrons() const { return p_hadrons; } 
  };
}
#endif
