#include "AHADIC++/Tools/Hadronisation_Parameters.H"
#include "AHADIC++/Tools/Multiplet_Constructor.H"
#include "MODEL/Main/Model_Base.H"
#include "ATOOLS/Phys/Flavour.H"
#include "ATOOLS/Phys/Momenta_Stretcher.H"
#include "ATOOLS/Math/MathTools.H"
#include "ATOOLS/Org/Message.H"
#include "ATOOLS/Org/Scoped_Settings.H"

using namespace AHADIC;
using namespace ATOOLS;
using namespace MODEL;
using namespace std;


Hadronisation_Parameters* AHADIC::hadpars = NULL;

Hadronisation_Parameters::Hadronisation_Parameters() : m_shower(0) {}

Hadronisation_Parameters::~Hadronisation_Parameters() {
  if (p_constituents!=NULL) {
    delete p_constituents;
    p_constituents=NULL;
  }
  if (p_stransitions!=NULL) {
    delete p_stransitions;
    p_stransitions=NULL;
  }
  if (p_dtransitions!=NULL) {
    delete p_dtransitions;
    p_dtransitions=NULL;
  }
}

void Hadronisation_Parameters::Init(string shower)
{
  if (shower=="Dire")     m_shower = 0;
  else if (shower=="CSS") m_shower = 1;
  ReadParameters();

  bool test      = false;
  bool diquarks  = true;
  p_constituents = new Constituents(diquarks);
  Multiplet_Constructor multipletconstructor(false);
  Wave_Functions * wavefunctions = multipletconstructor.GetWaveFunctions();
  p_stransitions = new Single_Transitions(wavefunctions);
  p_dtransitions = new Double_Transitions(p_stransitions);

  if (test) {
    msg_Out()<<"Inputs to AHADIC:\n";
    for (map<std::string,double>::iterator pit=m_parametermap.begin();
	 pit!=m_parametermap.end();pit++)
      msg_Out()<<"* "<<pit->first<<" = "<<pit->second<<"\n";
    msg_Out()<<"Resulting wave functions, multiplets, and transitions:\n";
    p_constituents->PrintConstituents();
    multipletconstructor.PrintMultiplets();
    p_stransitions->Print();
    p_dtransitions->Print(true);
    exit(1); // exit after output of parameters
  }
}


void Hadronisation_Parameters::ReadParameters()
{
  ReadGeneralSwitches();
  auto s = Settings::GetMainSettings()["AHADIC"];
  m_parametermap[string("minmass2")] =
    s["MIN_MASS2"].SetDefault(0.10).Get<double>(); 
  ReadPoppingParameters();
  ReadMesonWeights();
  ReadSplittingParameters();
  ReadClusterToMesonPSParameters();
}

const double Hadronisation_Parameters::Get(string keyword) const
{
  map<string,double>::const_iterator piter = m_parametermap.find(keyword);
  if (piter!=m_parametermap.end()) return piter->second;
  msg_Tracking()<<"Error in Hadronisation_Parameters::Get("<<keyword<<") "
		<<"in "<<m_parametermap.size()<<".\n"
		<<"   Keyword not found. Return 0 and hope for the best.\n";
  return 0.;
}

const int Hadronisation_Parameters::Switch(string keyword) const
{
  map<string,int>::const_iterator siter = m_switchmap.find(keyword);
  if (siter!=m_switchmap.end()) return siter->second;
  msg_Tracking()<<"Error in Hadronisation_Parameters::Get("<<keyword<<") "
		<<"in "<<m_switchmap.size()<<".\n"
		<<"   Keyword not found. Return 0 and hope for the best.\n";
  return 0;
}

void Hadronisation_Parameters::ReadSplittingParameters()
{
  auto s = Settings::GetMainSettings()["AHADIC"];
  // modes/forms for decays of gluons and clusters
  m_switchmap["KT_Ordering"] =
    s["KT_ORDER"].SetDefault(0).Get<int>();
  m_switchmap["GluonDecayForm"] =
    s["GLUON_DECAY_MODE"].SetDefault(0).Get<int>();
  m_switchmap["ClusterSplittingForm"] =
    s["CLUSTER_SPLITTING_MODE"].SetDefault(2).Get<int>();
  m_switchmap["RemnantSplittingForm"] =
    s["REMNANT_CLUSTER_MODE"].SetDefault(2).Get<int>();
  // generic parameter for non-perturbative transverse momentum
  m_parametermap[string("kT_0")]   =
    s["KT_0"].SetDefault(1.21).Get<double>();
  // gluon fragmentation
  m_parametermap[string("alphaG")] =
    s["ALPHA_G"].SetDefault(0.97).Get<double>();
  // light quark fragmentation
  m_parametermap[string("alphaL")] =
    s["ALPHA_L"].SetDefault(3.9).Get<double>();
  m_parametermap[string("betaL")]  =
    s["BETA_L"].SetDefault(0.18).Get<double>();
  m_parametermap[string("gammaL")] =
    s["GAMMA_L"].SetDefault(0.48).Get<double>();
  // di-quark fragmentation
  m_parametermap[string("alphaD")] =
    s["ALPHA_D"].SetDefault(3.4).Get<double>();
  m_parametermap[string("betaD")]  =
    s["BETA_D"].SetDefault(0.72).Get<double>();
  m_parametermap[string("gammaD")] =
    s["GAMMA_D"].SetDefault(0.77).Get<double>();
  // beam particle fragmentation
  m_parametermap[string("alphaB")] =
    s["ALPHA_B"].SetDefault(14.2).Get<double>();
  m_parametermap[string("betaB")]  =
    s["BETA_B"].SetDefault(1.6).Get<double>();
  m_parametermap[string("gammaB")] =
    s["GAMMA_B"].SetDefault(8.1).Get<double>();
  // heavy quark fragmentation function
  m_parametermap[string("alphaH")] =
    s["ALPHA_H"].SetDefault(-0.6).Get<double>();
  m_parametermap[string("betaH")]  =
    s["BETA_H"].SetDefault(1.8).Get<double>();
  m_parametermap[string("gammaH")] =
    s["GAMMA_H"].SetDefault(0.024).Get<double>();
  // These guys make a lot of difference - especially the transition ones, once we switch them on. 
  m_switchmap["direct_transition"] =
    s["DIRECT_TRANSITIONS"].SetDefault(1).Get<int>();
  m_parametermap[string("decay_threshold")] =
    s["DECAY_THRESHOLD"].SetDefault(0.02).Get<double>();
  m_parametermap[string("transition_threshold")] =
    s["TRANSITION_THRESHOLD"].SetDefault(0.51).Get<double>();
  // Probably irrelevant as long as they are small.
  // We will probably not have to tune them.
  m_parametermap[string("piphoton_threshold")] =
    s["PI_PHOTON_THRESHOLD"].SetDefault(0.150).Get<double>();
  m_parametermap[string("dipion_threshold")] =
    s["DI_PION_THRESHOLD"].SetDefault(0.300).Get<double>();
  m_parametermap[string("open_threshold")] =
    s["OPEN_THRESHOLD"].SetDefault(0.100).Get<double>();
  Settings & sets = Settings::GetMainSettings();
  m_parametermap[string("kT_max")] =
    s["PT_MAX"].SetDefault(0.68).Get<double>();
}

void Hadronisation_Parameters::ReadClusterToMesonPSParameters()
{
  auto s = Settings::GetMainSettings()["AHADIC"];
  m_parametermap[string("mass_exponent")]          =
    s["MASS_EXPONENT"].SetDefault(0.0).Get<double>();
  m_parametermap[string("prompt_decay_exponent")]          =
    s["PROMPT_DECAY_EXPONENT"].SetDefault(-1.0).Get<double>();
}

void Hadronisation_Parameters::ReadMesonWeights()
{
  auto s = Settings::GetMainSettings()["AHADIC"];
  m_parametermap[string("Singlet_Suppression")]   = 
    s["SINGLET_MODIFIER"].SetDefault(2.0).Get<double>();
  m_parametermap[string("Mixing_Angle_0+")]    = 
    s["Mixing_0+"].SetDefault<double>(-14.1/180.0*M_PI).Get<double>();;
  m_parametermap[string("Mixing_Angle_1-")]    = 
    s["Mixing_1-"].SetDefault<double>(36.4/180.0*M_PI).Get<double>();;
  m_parametermap[string("Mixing_Angle_2+")]    = 
    s["Mixing_2+"].SetDefault<double>(27.0/180.0*M_PI).Get<double>();;
  m_parametermap[string("Mixing_Angle_3-")]    = 
    s["Mixing_3-"].SetDefault<double>(0.5411).Get<double>();;
  m_parametermap[string("Mixing_Angle_4+")]    = 
    s["Mixing_4+"].SetDefault<double>(0.6283).Get<double>();;

  // Mesons currently included
  m_parametermap[string("Multiplet_Meson_R0L0S0")]   =
    s["MULTI_WEIGHT_R0L0_PSEUDOSCALARS"].SetDefault(1.00).Get<double>();
  m_parametermap[string("Multiplet_Meson_R0L0S1")]   =
    s["MULTI_WEIGHT_R0L0_VECTORS"].SetDefault(m_shower ? 2.5 : 2.2).Get<double>();
  m_parametermap[string("Multiplet_Meson_R0L0S2")]   =
    s["MULTI_WEIGHT_R0L0_TENSORS2"].SetDefault(1.5).Get<double>();
  m_parametermap[string("Multiplet_Meson_R0L1S0")]   =
    s["MULTI_WEIGHT_R0L1_SCALARS"].SetDefault(0.0).Get<double>();
  m_parametermap[string("Multiplet_Meson_R0L1S1")]   =
    s["MULTI_WEIGHT_R0L1_AXIALVECTORS"].SetDefault(0.0).Get<double>();
  m_parametermap[string("Multiplet_Meson_R0L2S2")]   =
    s["MULTI_WEIGHT_R0L2_VECTORS"].SetDefault(0.5).Get<double>();
  // Baryons currently included
  m_parametermap[string("Multiplet_Baryon_R0L0S1/2")]   =
    s["MULTI_WEIGHT_R0L0_N_1/2"].SetDefault(1.00).Get<double>();
  m_parametermap[string("Multiplet_Baryon_R1L0S1/2")]   =
    s["MULTI_WEIGHT_R1L0_N_1/2"].SetDefault(0.1).Get<double>();
  m_parametermap[string("Multiplet_Baryon_R2L0S1/2")]   =
    s["MULTI_WEIGHT_R2L0_N_1/2"].SetDefault(0.0).Get<double>();
  m_parametermap[string("Multiplet_Baryon_R1_1L0S1/2")] =
    s["MULTI_WEIGHT_R1_1L0_N_1/2"].SetDefault(0.0).Get<double>();
  m_parametermap[string("Multiplet_Baryon_R0L0S3/2")]   =
    s["MULTI_WEIGHT_R0L0_DELTA_3/2"].SetDefault(0.15).Get<double>();
  // Individual hadrons or groups of hadrons
  m_parametermap[string("eta_modifier")]   =
    s["ETA_MODIFIER"].SetDefault(m_shower ? 2.2 : 2.82).Get<double>();
  m_parametermap[string("eta_prime_modifier")]   =
    s["ETA_PRIME_MODIFIER"].SetDefault(m_shower ? 4.5 : 2.03).Get<double>();
  m_parametermap[string("Singlet_Baryon_modifier")]    =
    s["SINGLETBARYON_MODIFIER"].SetDefault(1.80).Get<double>();
  m_parametermap[string("CharmBaryon_Enhancement")]    =
    s["CHARMBARYON_ENHANCEMENT"].SetDefault(8.00).Get<double>();
  m_parametermap[string("BeautyBaryon_Enhancement")]    =
    s["BEAUTYBARYON_ENHANCEMENT"].SetDefault(0.80).Get<double>();
  m_parametermap[string("CharmStrange_Enhancement")]    =
    s["CHARMSTRANGE_ENHANCEMENT"].SetDefault(2.00).Get<double>();
  m_parametermap[string("BeautyStrange_Enhancement")]    =
    s["BEAUTYSTRANGE_ENHANCEMENT"].SetDefault(1.40).Get<double>();
  m_parametermap[string("BeautyCharm_Enhancement")]    =
    s["BEAUTYCHARM_ENHANCEMENT"].SetDefault(1.00).Get<double>();
}

void Hadronisation_Parameters::ReadPoppingParameters()
{
  auto s = Settings::GetMainSettings()["AHADIC"];
  double strange;
  m_parametermap[string("Strange_fraction")] = strange =
    s["STRANGE_FRACTION"].SetDefault(0.46).Get<double>();
  m_parametermap[string("Baryon_fraction")]        =
    s["BARYON_FRACTION"].SetDefault(0.17).Get<double>();
  m_parametermap[string("P_qs_by_P_qq")]           =
    (s["P_QS_by_P_QQ_norm"].SetDefault(0.56).Get<double>())*strange;
  m_parametermap[string("P_ss_by_P_qq")]           =
    (s["P_SS_by_P_QQ_norm"].SetDefault(0.056).Get<double>())*sqr(strange);
  m_parametermap[string("P_di_1_by_P_di_0")]       =
    s["P_QQ1_by_P_QQ0"].SetDefault(0.60).Get<double>();
}


void Hadronisation_Parameters::ReadGeneralSwitches()
{
  // General switches for operational modes
  //auto s = Settings::GetMainSettings()["AHADIC"];
  //m_switchmap["Analysis"] = 0;
}


bool Hadronisation_Parameters::AdjustMomenta(const int n,
                                             ATOOLS::Vec4D* moms,
                                             const double* masses)
{
  Momenta_Stretcher stretcher;
  if (n==1) return false;
  bool success(true);
  if (n!=2) {
    bool  prepare=false,boost=false;
    Poincare rest;
    Vec4D cms = Vec4D(0.,0.,0.,0.);
    double mass(0.);
    for (int i=0;i<n;i++) {
      cms  += moms[i];
      mass += masses[i];
      if (dabs(moms[i].Abs2())>1.e-6) prepare = true;
    } 
    if (Vec3D(cms).Abs()>1.e-6) { 
      boost = true;
      rest  = Poincare(cms);
      for (int i=0;i<n;i++) rest.Boost(moms[i]);
    }
    if (mass>sqrt(cms.Abs2())) {
      msg_Error()<<"Error in "<<METHOD<<" : "<<"\n"
		 <<"   Total mass = "<<mass<<", "
		 <<"total E = "<<sqrt(cms.Abs2())<<":\n";
      for (int i=0;i<n;i++) {
	msg_Error()<<"   "<<i<<"th mass = "<<masses[i]<<"\n";
      }
      msg_Error()<<"   Will possibly lead to retrying the event.\n";
      return false;
    }
    if (prepare) success = success && stretcher.ZeroThem(0,n,moms,1.e-10);
    if (!success) std::cout<<METHOD<<" failed for ZeroThem(0,"<<n<<").\n";
    success = success && stretcher.MassThem(0,n,moms,masses);
    if (!success) std::cout<<METHOD<<" failed for MassThem(0,"<<n<<").\n";
    if (boost) {
      for (int i=0;i<n;i++) rest.BoostBack(moms[i]);
    }
  }
  else {
    success = stretcher.MassThem(0,n,moms,masses,1.e-10);
    if (!success) std::cout<<METHOD<<" failed for MassThem(0,"<<n<<"), 2nd.\n";
  }
  if (!success && msg->LevelIsDebugging()) {
    msg_Debugging()<<"Error in "<<METHOD<<" : "<<"\n"
		  <<"   Could not shift particles on new shells.\n";
    for (int i=0;i<n;i++) {
      msg_Debugging()<<"   "<<i<<"th mass = "<<masses[i]<<"\n";
    }
    msg_Debugging()<<"   Will lead to retrying the event.\n";
  }
  return success;
}
