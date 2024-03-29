#ifndef MEProcess_H
#define MEProcess_H

#include <vector>
#include <string>
#include <memory>

#include "ATOOLS/Math/Vector.H"
#include "ATOOLS/Phys/Flavour.H"
#include "ATOOLS/Phys/NLO_Types.H"
#include "PHASIC++/Main/Color_Integrator.H"

namespace SHERPA{
  class Sherpa;
}
namespace ATOOLS{
  class Cluster_Amplitude;
  class ColorID;
  class Mass_Selector;
}
namespace PHASIC{
  class Process_Base;
  class Rambo;
}

class MEProcess{

private:

  ATOOLS::nlo_type::code     m_nlotype;
  ATOOLS::Cluster_Amplitude* p_amp;
  SHERPA::Sherpa*            p_gen;
  PHASIC::Process_Base *     p_proc;
  PHASIC::Rambo*             p_rambo;
  std::shared_ptr<PHASIC::Color_Integrator> p_colint;

  size_t                         m_ncolinds;
  std::vector<std::vector<int> > m_colcombinations;
  std::vector<int>               m_gluinds, m_quainds, m_quabarinds;
  std::vector<int>               m_inpdgs, m_outpdgs;
  std::vector<size_t>            m_mom_inds;

  // Store flavs here separately in the same order
  // in which they were added (for performance reasons)
  ATOOLS::Flavour_Vector         m_flavs; 

  size_t m_npsp,m_nin,m_nout;
  double m_kpz[2];

  void SetMomentumIndices(const std::vector<int> &pdgs);
  PHASIC::Process_Base* FindProcess(const ATOOLS::Cluster_Amplitude* amp);
  PHASIC::Process_Base* FindProcess();
  void SetColors();
  void SetUpColorStructure();

public:
  MEProcess(SHERPA::Sherpa* Generator);
  ~MEProcess();
  void AddInFlav(const int &id);
  void AddOutFlav(const int &id);
  void AddInFlav(const int &id, const int &col1, const int &col2);
  void AddOutFlav(const int &id, const int &col1, const int &col2);
  double GenerateColorPoint();
  bool HasColorIntegrator();
  void Initialize();

  std::vector<double> NLOSubContributions();

  size_t NumberOfPoints();

  void PrintProcesses() const;
  void ReadProcess(size_t n);

  void SetMomenta(const std::vector<double*> &p);
  void SetMomenta(const ATOOLS::Vec4D_Vector &p);
  void SetMomentum(const size_t &id, const double &E , const double &px,
                                     const double &py, const double &pz);
  void SetMomentum(const size_t &id, const ATOOLS::Vec4D &p);
  void SetColor   (const size_t &id, const ATOOLS::ColorID &col);

  // Get the momenta that were previously set
  ATOOLS::Vec4D_Vector GetMomenta();

  // Generate a PS point via Rambo and return the phase space weight
  double TestPoint(const double& sqrts);
  
  double MatrixElement();
  double CSMatrixElement();

  // Calculate and retuen 1/F with F=4*sqrt((pa*pb)^2 - ma^2*mb^2)
  double GetFlux();
  
  std::string GeneratorName();

  ATOOLS::Flavour GetFlav(size_t i);
  inline ATOOLS::Flavour GetInFlav(size_t i)  { return GetFlav(i); }
  inline ATOOLS::Flavour GetOutFlav(size_t i) { return GetFlav(i+m_nin); }

  inline ATOOLS::Cluster_Amplitude* GetAmp() { return p_amp; }
  inline PHASIC::Process_Base* GetProc() {return p_proc;}
  std::string Name() const;

  inline void SetNLOType(ATOOLS::nlo_type::code nlotype) { m_nlotype=nlotype; }
};

#endif
