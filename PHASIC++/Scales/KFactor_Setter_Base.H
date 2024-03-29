#ifndef PHASIC_Scales_KFactor_Setter_Base_H
#define PHASIC_Scales_KFactor_Setter_Base_H

#include "ATOOLS/Org/Exception.H"
#include "PHASIC++/Scales/Scale_Setter_Base.H"

namespace PHASIC {

  struct KFactor_Setter_Arguments {
    Process_Base *p_proc;
    std::string m_kfac;
    KFactor_Setter_Arguments(const std::string& kfac,
                             Process_Base* proc=nullptr):
      m_kfac{kfac}, p_proc{proc} {}
  };// end of class KFactor_Setter_Arguments

  class KFactor_Setter_Base {
  public:

    typedef ATOOLS::Getter_Function
    <KFactor_Setter_Base,KFactor_Setter_Arguments> KFactor_Getter_Function;

  protected:

    Process_Base *p_proc;
    
    double m_weight;

    bool m_on;

  public:

    KFactor_Setter_Base(const KFactor_Setter_Arguments &args);

    virtual ~KFactor_Setter_Base();

    // compute KFactor according to kinematics and flavours of p_proc
    virtual double KFactor(const int mode=0) = 0;
    // compute KFactor according to kinematics and flavours of NLO subevent
    virtual double KFactor(const ATOOLS::NLO_subevt& evt);

    virtual bool UpdateKFactor(const ATOOLS::QCD_Variation_Params &var);

    inline Process_Base *Process() const { return p_proc; }

    inline void SetOn(const bool on) { m_on=on; }

    inline bool On() const { return m_on; }

    inline double LastKFactor() const { return m_weight; }

    static void ShowSyntax(const size_t i);

  };// end of class KFactor_Setter_Base

}// end of namespace PHASIC

#endif
