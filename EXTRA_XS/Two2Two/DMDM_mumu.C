#include "PHASIC++/Process/External_ME_Args.H"
#include "MODEL/Main/Running_AlphaS.H"
#include "MODEL/Main/Running_AlphaQED.H"
#include "ATOOLS/Phys/Flow.H"
#include "MODEL/Main/Model_Base.H"
#include "MODEL/UFO/UFO_Model.H"
#include "EXTRA_XS/Main/ME2_Base.H"

#define PropID(i,j) ((1<<i)|(1<<j))

using namespace EXTRAXS;
using namespace MODEL;
using namespace ATOOLS;
using namespace PHASIC;
using namespace std;

namespace EXTRAXS {
  class DMDM_mumu : public ME2_Base {
    /* Describing the annihilation of fermionic dark matter with its antiparticle
       through a Z to produce a muon-antimuon pair.
    */
  private:
    double V,A;
    double Vtil,Atil;
    double sintW,costW;

  public:
    DMDM_mumu(const External_ME_Args& args);
    double operator()(const Vec4D_Vector& mom);
  };
}

DMDM_mumu::DMDM_mumu(const External_ME_Args& args) :
  ME2_Base(args)
{
  Settings& settings = Settings::GetMainSettings();

  V = settings["DM_Z_v"].Get<double>();
  A = settings["DM_Z_a"].Get<double>();
  sintW = std::abs(sqrt(MODEL::s_model->ComplexConstant("csin2_thetaW")));
  costW = sqrt(1-sqr(sintW));

  m_sintt=1; //what should this be? initialised to 0, but most processes use 1
  m_oew = 2; m_oqcd = 0;
  m_cfls[3] = Flavour_Vector{};
  m_cfls[3].push_back(kf_photon);
  m_cfls[3].push_back(kf_Z);
  m_cfls[12] = Flavour_Vector{};
  m_cfls[12].push_back(kf_photon);
  m_cfls[12].push_back(kf_Z);
}

double DMDM_mumu::operator()(const Vec4D_Vector& mom)
{
  double s=(mom[0]+mom[1]).Abs2();

  double alpha = MODEL::s_model->ScalarConstant("alpha_QED");
  Vtil = sqrt(4*M_PI*alpha)/(2*costW) * (-0.5 + 2*sqr(sintW)); //lepton vector coupling constant. alpha=g^2/4pi
  Atil = -sqrt(4*M_PI*alpha)/(4*costW); //lepton axial c.c.

  double M = ATOOLS::Flavour(kf_Z).Mass();
  double gamma = ATOOLS::Flavour(kf_Z).Width();
  double m_DM = m_flavs[0].Mass();

  double factor1 = 4/(sqr(s-M*M) + M*M*sqr(gamma));
  double part1 = (V*V+A*A)*(Vtil*Vtil+Atil*Atil) * ((mom[0]*mom[2])*(mom[1]*mom[3])
                  + (mom[0]*mom[3])*(mom[1]*mom[2]));
  double part2 = -4*V*A*Vtil*Atil * ((mom[0]*mom[2])*(mom[1]*mom[3])
                  - (mom[0]*mom[3])*(mom[1]*mom[2]));
  double part3 = 2*(V*V-A*A)*(Vtil*Vtil+Atil*Atil)*sqr(m_DM) * (mom[2]*mom[3]);

  return factor1*(part1+part2+part3)/m_symfac;
}

DECLARE_TREEME2_GETTER(DMDM_mumu,"DMDM_mumu")
Tree_ME2_Base *ATOOLS::Getter<Tree_ME2_Base,External_ME_Args,DMDM_mumu>::
operator()(const External_ME_Args &args) const
{
  if (dynamic_cast<UFO::UFO_Model*>(MODEL::s_model)) return NULL;
  if (MODEL::s_model->Name()!="SMDM") return NULL;

  const Flavour_Vector fl=args.Flavours();
  if (fl.size()!= 4) return NULL;
  if (fl[0].Kfcode()==52 && fl[1].Kfcode()==52 &&
      fl[2].IsFermion() && fl[2].Charge() &&
      fl[3]==fl[2].Bar()) {
        if (args.m_orders[0]==0 && args.m_orders[1]==2) {
	  std::cout<<"   initialising ME.\n";
          return new DMDM_mumu(args);
        }
  }
  return NULL;
}
