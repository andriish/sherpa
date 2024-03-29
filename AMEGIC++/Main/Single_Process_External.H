#ifndef AMEGIC_Main_Single_Process_External_H
#define AMEGIC_Main_Single_Process_External_H

#include "AMEGIC++/Main/Process_Base.H"

#include "AMEGIC++/Main/Polarisation.H"
#include "PHASIC++/Process/Tree_ME2_Base.H"

namespace AMEGIC {
  class Helicity;
  class Phase_Space_Generator;
  class Single_Process_External: public Process_Base {

  typedef std::set<std::pair<size_t,size_t> >     Combination_Set;
  typedef std::map<size_t,ATOOLS::Flavour_Vector> CFlavVector_Map;

  private:
    PHASIC::Tree_ME2_Base *p_me2;
    size_t m_pn;
    bool                    m_newlib, m_keep_zero_procs;
    Polarisation            m_pol;
    double                  m_iresult;

    Combination_Set m_ccombs;
    CFlavVector_Map m_cflavs;

    Single_Process_External        * p_partner;
    /*------------------------------------------------------------------------------

      Constructors

      ------------------------------------------------------------------------------*/
  public:

    Single_Process_External();
    ~Single_Process_External();
    
    bool Combinable(const size_t &idi,const size_t &idj);

    const ATOOLS::Flavour_Vector &CombinedFlavour(const size_t &idij);

    bool FillIntegrator(PHASIC::Phase_Space_Handler *const psh);
   private:

    /*------------------------------------------------------------------------------

      Initializing libraries, amplitudes, etc.

      ------------------------------------------------------------------------------*/
  public:
    void                AddChannels(std::list<std::string>*);
    bool                NewLibs() {return 0;}
    bool                FoundMappingFile(std::string &,std::string &);
    int                 InitAmplitude(Amegic_Model *,Topology *,
				      std::vector<Process_Base *> &,
				      std::vector<Process_Base *> &);
    bool                SetUpIntegrator();
    Amplitude_Handler * GetAmplitudeHandler()          { return 0;}
    Helicity *          GetHelicity()                  { return 0; }    
    double              Result()                       { return m_iresult; } 

    int                             NumberOfDiagrams() { return 0; }
    AMEGIC::Point                 * Diagram(int i)     { return 0; }
    /*------------------------------------------------------------------------------

      Process management

      ------------------------------------------------------------------------------*/
  public:
    std::string      LibName()                          { return std::string("");     }
    std::string      PSLibName()                        { return std::string("");   }
    AMEGIC::Process_Base   * Partner() const            { return p_partner;     }
    void             Minimize();

    /*------------------------------------------------------------------------------

      Calculating total cross sections

      ------------------------------------------------------------------------------*/
  public:
    double Partonic(const ATOOLS::Vec4D_Vector&,
                    ATOOLS::Variations_Mode varmode,
                    int mode);
    double         DSigma(const ATOOLS::Vec4D_Vector &,bool);
    double         operator()(const ATOOLS::Vec4D *);

  };
}



#endif
