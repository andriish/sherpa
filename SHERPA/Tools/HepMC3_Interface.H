#ifndef SHERPA_Tools_HepMC3_Interface_H
#define SHERPA_Tools_HepMC3_Interface_H
#include "ATOOLS/Org/CXXFLAGS_PACKAGES.H"

#ifdef USING__HEPMC3

#include "ATOOLS/Org/CXXFLAGS.H"
#include "ATOOLS/Org/Message.H"
#include "ATOOLS/Phys/Blob.H"
#include "ATOOLS/Phys/NLO_Types.H"
#include "ATOOLS/Phys/NLO_Subevt.H"
#include "ATOOLS/Phys/Weight_Info.H"
#include "PHASIC++/Process/Subprocess_Info.H"
#include "ATOOLS/Phys/Variations.H"

#include <map>
#include <set>
#include <memory>

namespace HepMC3 {
  class GenEvent;
  class GenVertex;
  class GenRunInfo;
  class GenParticle;
  class GenCrossSection;

using GenVertexPtr = std::shared_ptr<GenVertex>;
using GenParticlePtr = std::shared_ptr<GenParticle>;

}

namespace ATOOLS {
  class Blob_List;
  class Particle;
  class ME_Weight_Info;
  class NLO_subevtlist;
  class PDF_Info;
}

namespace SHERPA {

  class EventInfo3 {
  private:
    ATOOLS::Blob * p_sp;
    bool m_usenamedweights, m_extendedweights;
    std::vector<ATOOLS::Variations_Source> m_variationsources;
    std::vector<double> m_orders;
    double m_wgt, m_mewgt, m_wgtnorm, m_ntrials, m_pswgt, m_pwgt;
    double m_mur2, m_muf12, m_muf22;
    double m_alphas, m_alpha;
    bool m_userhook;
    double m_userweight;
    ATOOLS::nlo_type::code m_type;
    ATOOLS::ME_Weight_Info * p_wgtinfo;
    ATOOLS::PDF_Info * p_pdfinfo;
    ATOOLS::NLO_subevtlist * p_subevtlist;
    ATOOLS::Weights_Map m_wgtmap;

    void ReadIn(ATOOLS::Blob_Data_Base * &db, std::string name, bool abort);

  public:
    EventInfo3(ATOOLS::Blob * sp, const double &wgt,
              bool namedweights,
              bool extendedweights,
              bool includemeonlyweights);
    EventInfo3(const EventInfo3 &evtinfo);
    ~EventInfo3() {}
    bool WriteTo(HepMC3::GenEvent &evt, const int& idx=-1);
    inline const ATOOLS::NLO_subevtlist * SubEvtList() { return p_subevtlist; }
    inline void SetWeight(const double &wgt)          { m_wgt=wgt; }
    inline void SetMEWeight(const double &mewgt)      { m_mewgt=mewgt; }
    inline void SetPartonicWeight(const double& pwgt) { m_pwgt=pwgt; }
    inline void SetMuR2(const double &mur2)           { m_mur2=mur2; }
    inline void SetMuF12(const double &muf12)         { m_muf12=muf12; }
    inline void SetMuF22(const double &muf22)         { m_muf22=muf22; }
    void SetAlphaS();
    void SetAlpha();
  };

  class HepMC3_Interface {
  private :
    bool m_usenamedweights, m_extendedweights, m_includemeonlyweights;
    bool m_hepmctree;

    std::map<ATOOLS::Blob*,HepMC3::GenVertexPtr > m_blob2genvertex;
    std::map<ATOOLS::Particle*,HepMC3::GenParticlePtr> m_particle2genparticle;
    std::set<ATOOLS::btp::code> m_ignoreblobs;

    HepMC3::GenEvent *p_event;
    std::shared_ptr<HepMC3::GenRunInfo> m_runinfo;
    std::vector<HepMC3::GenEvent*> m_subeventlist;

    std::shared_ptr<HepMC3::GenParticle> MakeGenParticle(const ATOOLS::Vec4D&,
                                                         const ATOOLS::Flavour&,
                                                         bool incoming);

  public:
    // constructor
    HepMC3_Interface();

    // destructor
    ~HepMC3_Interface();

    // member functions
    bool Sherpa2HepMC(ATOOLS::Blob_List *const blobs, HepMC3::GenEvent& event);
    bool Sherpa2HepMC(ATOOLS::Blob_List *const blobs, std::shared_ptr<HepMC3::GenRunInfo> run= std::shared_ptr<HepMC3::GenRunInfo>());
    bool Sherpa2HepMCBlobtoGenVertex(ATOOLS::Blob *, HepMC3::GenVertexPtr &, HepMC3::GenEvent& event ); 
    bool Sherpa2HepMC(ATOOLS::Particle *, HepMC3::GenParticlePtr &);

    bool Sherpa2ShortHepMC(ATOOLS::Blob_List *const blobs, HepMC3::GenEvent& event);
    bool Sherpa2ShortHepMC(ATOOLS::Blob_List *const blobs, std::shared_ptr<HepMC3::GenRunInfo> run= std::shared_ptr<HepMC3::GenRunInfo>());
    bool SubEvtList2ShortHepMC(EventInfo3 &evtinfo, std::shared_ptr<HepMC3::GenRunInfo> run);

    void AddCrossSection(HepMC3::GenEvent& event,
                         const ATOOLS::Weights_Map& xs,
                         const ATOOLS::Weights_Map& err);

    // inline functions
    inline HepMC3::GenEvent* GenEvent() { return p_event; }
    // For RivetInterface to enable ME/shower only analysis of fully hadronised event
    inline void Ignore(ATOOLS::btp::code type) { m_ignoreblobs.insert(type); }

    inline HepMC3::GenEvent* GenSubEvent(size_t i)
    { if (i<m_subeventlist.size()) return m_subeventlist[i]; return NULL; }
    inline std::vector<HepMC3::GenEvent*> GenSubEventList()
    { return m_subeventlist; }

    inline void SetHepMCNamedWeights(bool x)   { m_usenamedweights=x; }
    inline bool HepMCNamedWeights()            { return m_usenamedweights; }
    inline void SetHepMCExtendedWeights(bool x){ m_extendedweights=x; }
    inline bool HepMCExtendedWeights()         { return m_extendedweights; }
    inline void SetHepMCTreeLike(bool x)       { m_hepmctree=x; }
    inline bool HepMCTreeLike()                { return m_hepmctree; }
    inline void SetHepMCIncludeMEOnlyVariations(bool x)
    { m_includemeonlyweights=x; }

    void DeleteGenSubEventList();

  };// end of class HepMC3_Interface

}// end of namespace SHERPA

#endif
#endif
