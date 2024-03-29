#include "SHERPA/SoftPhysics/Lund_Decay_Handler.H"
#include "ATOOLS/Org/Message.H"
#include "ATOOLS/Phys/Blob.H"
#include "ATOOLS/Phys/Blob_List.H"
#include "ATOOLS/Phys/Particle.H"
#include "ATOOLS/Phys/KF_Table.H"
#include "ATOOLS/Org/Run_Parameter.H"
#include "ATOOLS/Org/Exception.H"
#include "ATOOLS/Org/My_MPI.H"
#include "ATOOLS/Org/Scoped_Settings.H"
#ifdef USING__PYTHIA
#include "SHERPA/LundTools/Lund_Interface.H"
#endif

using namespace SHERPA;
using namespace ATOOLS;
using namespace PHASIC;
using namespace std;
using namespace METOOLS;

Lund_Decay_Handler::Lund_Decay_Handler(Lund_Interface* lund) :
  Decay_Handler_Base(), p_lund(lund)
{
#ifdef USING__PYTHIA
  auto s = Settings::GetMainSettings()["HADRON_DECAYS"];
  m_qedmode =
    s["QED_Corrections"].SetDefault(1).Get<size_t>();
  const double maxproperlifetime{
    s["Max_Proper_Lifetime"].Get<double>() };
  for(KFCode_ParticleInfo_Map::const_iterator kfit(s_kftable.begin());
      kfit!=s_kftable.end();++kfit) {
    Flavour flav(kfit->first);
    if (flav.IsOn() && flav.IsHadron() && !flav.IsStable()) {
      if (maxproperlifetime > 0.0 &&
          0.197e-12>maxproperlifetime*flav.Width() && flav.Kfcode()!=kf_K) {
        flav.SetStable(true);
        continue;
      }

      if (p_lund->IsAllowedDecay(flav.Kfcode()) ||
          flav.Kfcode()==kf_K_L||flav.Kfcode()==kf_K_S||flav.Kfcode()==kf_K) {
        p_lund->AdjustProperties(flav);
      }
    }
  }

  m_mass_smearing=false;
  p_lund->SwitchOffMassSmearing();
  m_spincorr=false;

  p_decaymap=NULL;
#else
  THROW(fatal_error, "DECAYMODEL=Lund needs configure option --enable-pythia.");
#endif
}

Lund_Decay_Handler::~Lund_Decay_Handler()
{
}

Amplitude2_Tensor*
Lund_Decay_Handler::FillOnshellDecay(Blob *blob, Spin_Density* sigma)
{
#ifdef USING__PYTHIA
  DEBUG_FUNC("");

  if (p_lund->PerformDecay(blob) != Return_Value::Success) {
    PRINT_INFO("Lund decay failed. Retrying event.");
    throw Return_Value::Retry_Event;
  }

  DEBUG_VAR(*blob);
#endif
  return NULL;
}

void Lund_Decay_Handler::CreateDecayBlob(Particle* inpart)
{
  DEBUG_FUNC(inpart->RefFlav());
  if(inpart->DecayBlob()) Abort();
  if(!Decays(inpart->Flav())) return;
  if(inpart->Time()==0.0) inpart->SetTime();
  Blob* blob = p_bloblist->AddBlob(btp::Hadron_Decay);
  blob->AddToInParticles(inpart);
  SetPosition(blob);
  blob->SetTypeSpec("Lund");

  blob->AddData("p_onshell",new Blob_Data<Vec4D>(inpart->Momentum()));
  DEBUG_VAR(inpart->Momentum());
}

void Lund_Decay_Handler::SetPosition(ATOOLS::Blob* blob)
{
  Particle* inpart = blob->InParticle(0);
  if(inpart->Flav().Kfcode()==kf_K) {
    blob->SetPosition(inpart->XProd());
    return;
  }
  
  // boost lifetime into lab
  double gamma = 1./rpa->gen.Accu();
  if (inpart->Flav().HadMass()>rpa->gen.Accu()) {
    gamma = inpart->E()/inpart->Flav().HadMass();
  }
  else {
    double q2    = dabs(inpart->Momentum().Abs2());
    if (q2>rpa->gen.Accu()) gamma = inpart->E()/sqrt(q2);
  }
  double lifetime_boosted = gamma * inpart->Time();
  
  Vec3D      spatial = inpart->Distance( lifetime_boosted );
  Vec4D     position = Vec4D( lifetime_boosted*rpa->c(), spatial );
  blob->SetPosition( inpart->XProd() + position ); // in mm
}

bool Lund_Decay_Handler::CanDecay(const ATOOLS::Flavour& flav)
{
#ifdef USING__PYTHIA
  return p_lund->IsAllowedDecay(flav.Kfcode());
#else
  return false;
#endif
}

