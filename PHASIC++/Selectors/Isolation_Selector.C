#include "ATOOLS/Math/Vector.H"
#include "ATOOLS/Math/Poincare.H"
#include "ATOOLS/Org/Run_Parameter.H"
#include "ATOOLS/Org/Message.H"
#include "ATOOLS/Org/Exception.H"
#include "ATOOLS/Org/MyStrStream.H"
#include "ATOOLS/Phys/Flavour.H"
#include "ATOOLS/Phys/Particle_List.H"
#include "PHASIC++/Main/Process_Integrator.H"
#include "PHASIC++/Process/Process_Base.H"
#include "PHASIC++/Selectors/Selector.H"
#include <algorithm>
#include <vector>


namespace PHASIC {
  class Isolation_Selector : public Selector_Base {
  private:
    double m_dR,m_exp,m_emax;
    double m_ptmin,m_etmin,m_etamin,m_etamax,m_ymin,m_ymax;
    size_t m_nmin, m_nmax;
    ATOOLS::Flavour m_iflav;
    ATOOLS::Flavour_Vector m_rejflav;
    kf_code m_outisokf;
    bool   m_removeiso,m_removenoniso;

    double Chi(double eg,double dr);
    double DR(const ATOOLS::Vec4D & p1,const ATOOLS::Vec4D & p2);
    double DEta12(const ATOOLS::Vec4D &,const ATOOLS::Vec4D &);
    double DPhi12(const ATOOLS::Vec4D &,const ATOOLS::Vec4D &);

  public:
    Isolation_Selector(const Selector_Key &key);
    ~Isolation_Selector();

    bool   Trigger(ATOOLS::Selector_List &sl);

    void   BuildCuts(Cut_Data *);
  };
}



using namespace PHASIC;
using namespace ATOOLS;

class edr {
public:
  double E;
  double dr;
  edr(double _e,double _dr) : E(_e), dr(_dr) {}
};
class Order_edr {
public:
  int operator()(const edr a, const edr b);
};
int Order_edr::operator()(const edr a, const edr b) {
  if (a.dr<b.dr) return 1;
  return 0;
}

/*--------------------------------------------------------------------

  photon isolation cut: hep-ph/9801442

  --------------------------------------------------------------------*/

Isolation_Selector::Isolation_Selector(const Selector_Key &key) :
  Selector_Base("Isolation_Selector",key.p_proc),
  m_dR(0.), m_exp(0.), m_emax(0.), m_ptmin(0.), m_etmin(0.),
  m_etamin(-std::numeric_limits<double>::max()),
  m_etamax(std::numeric_limits<double>::max()),
  m_ymin(-std::numeric_limits<double>::max()),
  m_ymax(std::numeric_limits<double>::max()),
  m_nmin(0), m_nmax(std::numeric_limits<size_t>::max()),
  m_iflav(Flavour(kf_none)), m_outisokf(kf_none),
  m_removeiso(false), m_removenoniso(false)
{
  DEBUG_FUNC("");
  auto s = key.m_settings["Isolation_Selector"];

  auto kfc = s["Isolation_Particle"].SetDefault(kf_none).Get<long int>();
  m_iflav = Flavour(abs(kfc), kfc<0);

  auto kfcs = s["Rejection_Particles"]
    .SetDefault(std::vector<long int>())
    .GetVector<long int>();
  for (const auto rejkfc : kfcs)
    if (rejkfc != kf_none)
      m_rejflav.push_back(Flavour(abs(rejkfc), rejkfc<0));

  m_outisokf = s["Output_ID"].SetDefault(kf_none).Get<kf_code>();

  m_removeiso = s["Remove_Isolated"].SetDefault(false).Get<bool>();
  m_removenoniso = s["Remove_Nonisolated"].SetDefault(false).Get<bool>();

  auto isolation = s["Isolation_Parameters"];
  m_dR = isolation["R"].SetDefault(0.0).Get<double>();
  m_emax = isolation["EMAX"].SetDefault(0.0).Get<double>();
  m_exp = isolation["EXP"].SetDefault(0.0).Get<double>();
  m_ptmin = isolation["PT"].SetDefault(0.0).Get<double>();
  m_etmin = isolation["ET"].SetDefault(0.0).Get<double>();
  const auto eta = isolation["ETA"]
    .SetDefault(std::numeric_limits<double>::max())
    .Get<double>();
  m_etamax = isolation["ETAMAX"].SetDefault(eta).Get<double>();
  m_etamin = isolation["ETAMIN"].SetDefault(-eta).Get<double>();
  const auto y = isolation["Y"]
    .SetDefault(std::numeric_limits<double>::max())
    .Get<double>();
  m_ymax = isolation["YMAX"].SetDefault(y).Get<double>();
  m_ymin = isolation["YMIN"].SetDefault(-y).Get<double>();

  // jet numbers
  m_nmin = s["NMin"].SetDefault(0).Get<size_t>();
  m_nmax = s["NMax"]
    .SetDefault(std::numeric_limits<size_t>::max())
    .Get<size_t>();

  ReadInSubSelectors(key);

  for (int i=m_nin;i<m_nin+m_nout;i++)
    if (m_iflav.Includes(p_fl[i])) m_on=true;

  if (m_outisokf==kf_none) m_outisokf=m_iflav.Kfcode();

//  m_smin       = sqr(m_ptmin);

  if (m_nmin>m_nmax) THROW(fatal_error,"Inconsistent setup.");
  if (msg_LevelIsDebugging()) {
    msg_Out()<<"Isolation particles:"<<m_iflav<<std::endl;
    msg_Out()<<"Rejection particles:"<<m_rejflav<<std::endl;
    msg_Out()<<"Isolation parameter: dR="<<m_dR
             <<", Emax="<<m_emax<<", exp="<<m_exp
             <<", pT>"<<m_ptmin<<", ET>"<<m_etmin
             <<", "<<m_etamin<<"<eta<"<<m_etamax
             <<", "<<m_ymin<<"<y<"<<m_ymax<<std::endl;
    msg_Out()<<"NMin: "<<m_nmin<<", NMax: "<<m_nmax<<std::endl;
    msg_Out()<<"Isolated particles assigned ID="<<m_outisokf<<std::endl;
    msg_Out()<<"Isolated particles are "<<(m_removeiso?"":"not ")
             <<"removed from the particle list."<<std::endl;
    msg_Out()<<"Non-isolated particles are "<<(m_removenoniso?"":"not ")
             <<"removed from the particle list."<<std::endl;
    msg_Out()<<"Additional Selectors: "<<m_sels.size()<<"\n";
    for (size_t i(0);i<m_sels.size();++i)
      msg_Out()<<"  "<<m_sels[i]->Name()<<std::endl;
  }
}


Isolation_Selector::~Isolation_Selector() {
}


bool Isolation_Selector::Trigger(Selector_List &sl)
{
  DEBUG_FUNC(m_on);
  if (!m_on) return true;
  size_t n(m_n);
  Vec4D_Vector pc;
  std::vector<size_t> vfsub;
  for (size_t i=m_nin;i<n;i++) {
    if (m_iflav.Includes(sl[i].Flavour())) {
      vfsub.push_back(i);
    }
  }
  const std::vector<size_t> *const vf(&vfsub);
  std::vector<bool> vfiso(vf->size(),false);
  size_t cnt(0);
  msg_Debugging()<<"trying to find "<<m_nmin<<"-"<<m_nmax
                 <<" out of "<<vf->size()<<std::endl;
  for (size_t k=0;k<vf->size();k++) {
    size_t idx((*vf)[k]);
    if ((m_ptmin==0. || sl[idx].Momentum().PPerp()>m_ptmin) &&
        (m_etmin==0. || sl[idx].Momentum().MPerp()>m_etmin) &&
        (m_etamin==-std::numeric_limits<double>::max() || sl[idx].Momentum().Eta()>m_etamin) &&
        (m_etamax==std::numeric_limits<double>::max() || sl[idx].Momentum().Eta()<m_etamax) &&
        (m_ymin==-std::numeric_limits<double>::max() || sl[idx].Momentum().Y()>m_ymin) &&
        (m_ymax==std::numeric_limits<double>::max() || sl[idx].Momentum().Y()<m_ymax)) {
      bool iso(true);
      double egamma(sl[idx].Momentum().MPerp());
      std::vector<edr> edrlist;
      for (size_t i=m_nin;i<n;i++) {
        if (i!=idx) {
          for (size_t j(0);j<m_rejflav.size();j++) {
            if (m_rejflav[j].Includes(sl[i].Flavour())) {
              double dr=DR(sl[(*vf)[k]].Momentum(),sl[i].Momentum());
              if (dr<m_dR) edrlist.push_back(edr(sl[i].Momentum().MPerp(),dr));
            }
          }
        }
      }
      msg_Debugging()<<"  "<<edrlist.size()<<" particles in cone"<<std::endl;
      if (edrlist.size()>0) {
        std::stable_sort(edrlist.begin(),edrlist.end(),Order_edr());
        double etot(0.);
        for (size_t i=0;i<edrlist.size();i++) {
          etot+=edrlist[i].E;
          msg_Debugging()<<"  ET="<<edrlist[i].E<<"("<<edrlist[i].dr<<"), "
                         <<"chi="<<Chi(egamma,edrlist[i].dr)<<std::endl;
          if (etot>Chi(egamma,edrlist[i].dr)) {
            msg_Debugging()<<"  not isolated."<<std::endl;
            iso=false;
            break;
          }
        }
        edrlist.clear();
      }
      if (iso) {
        vfiso[k]=true;
        cnt++;
        msg_Debugging()<<"  isolated, n="<<cnt<<std::endl;
      }
    }
  }

  // modify momenta list
  // TODO: implement m_outisokf
  if (m_removeiso) {
    for (size_t i(0);i<vfiso.size();++i)
      if (vfiso[i])
        sl[(*vf)[i]].Momentum()=Vec4D(0.,0.,0.,0.);
  }
  if (m_removenoniso) {
    for (size_t i(0);i<vfiso.size();++i)
      if (!vfiso[i])
        sl[(*vf)[i]].Momentum()=Vec4D(0.,0.,0.,0.);
  }

  bool trigger(cnt>=m_nmin && cnt<=m_nmax);
  if (!trigger) {
    msg_Debugging()<<"Point discarded by isolation selector"<<std::endl;
    m_sel_log->Hit(true);
    return false;
  }
  for (size_t k=0;k<m_sels.size();++k) {
    if (!m_sels[k]->Trigger(sl)) {
      msg_Debugging()<<"Point discarded by subselector"<<std::endl;
      m_sel_log->Hit(true);
      return false;
    }
  }
  msg_Debugging()<<"Point passed"<<std::endl;
  m_sel_log->Hit(false);
  return true;
}

void Isolation_Selector::BuildCuts(Cut_Data * cuts)
{
  if (m_isnlo) return;
  double sumM2(0.);
  for (int i=m_nin;i<m_n;i++) {
    sumM2+=sqr(p_fl[i].SelMass());
  }
  for (int i=m_nin;i<m_n;i++) {
    if (m_iflav.Includes(p_fl[i])) {
      if (m_ptmin>0.) {
        cuts->energymin[i] = Max(sqrt(sqr(m_ptmin)+sqr(p_fl[i].SelMass())),
                                 cuts->energymin[i]);
      }
      if (m_etmin>0.) {
        cuts->energymin[i] = Max(sqrt(sqr(m_etmin)+sqr(p_fl[i].SelMass())),
                                 cuts->energymin[i]);
      }
    }
  }
  for (size_t i(0);i<m_sels.size();++i) m_sels[i]->BuildCuts(cuts);
}

double Isolation_Selector::Chi(double eg,double dr)
{
  if      (m_exp<0.)  return 0.;//rpa->gen.Ecms();
  else if (m_exp==0.) return eg*m_emax;
  else if (m_exp==1.) return eg*m_emax*(1.-cos(dr))/(1.-cos(m_dR));
  else if (m_exp==2.) return eg*m_emax*sqr((1.-cos(dr))/(1.-cos(m_dR)));
  else                return eg*m_emax*pow((1.-cos(dr))/(1.-cos(m_dR)),m_exp);
}

double Isolation_Selector::DR(const Vec4D & p1,const Vec4D & p2)
{
  return  sqrt(sqr(DEta12(p1,p2)) + sqr(DPhi12(p1,p2)));
}
double Isolation_Selector::DEta12(const Vec4D & p1,const Vec4D & p2)
{
  // eta1,2 = -log(tan(theta_1,2)/2)
  double c1=p1[3]/Vec3D(p1).Abs();
  double c2=p2[3]/Vec3D(p2).Abs();
  return  0.5 *log( (1 + c1)*(1 - c2)/((1-c1)*(1+c2)));
}

double Isolation_Selector::DPhi12(const Vec4D & p1,const Vec4D & p2)
{
  double pt1=sqrt(p1[1]*p1[1]+p1[2]*p1[2]);
  double pt2=sqrt(p2[1]*p2[1]+p2[2]*p2[2]);
  return acos((p1[1]*p2[1]+p1[2]*p2[2])/(pt1*pt2));
}

DECLARE_GETTER(Isolation_Selector,"Isolation_Selector",
               Selector_Base,Selector_Key);

Selector_Base *ATOOLS::Getter<Selector_Base,Selector_Key,
                              Isolation_Selector>::operator()
(const Selector_Key &key) const
{
  return new Isolation_Selector(key);
}

void ATOOLS::Getter<Selector_Base,Selector_Key,Isolation_Selector>::
PrintInfo(std::ostream &str,const size_t width) const
{
  std::string w(width+4,' ');
  str<<"Isolation_Selector:\n"
     <<w<<"  Isolation_Particle: <kf1>\n"
     <<w<<"  Rejection_Particles [<kf1>, <kf2>, ...]\n"
     <<w<<"  Isolation_Parameters: {\n"
     <<w<<"    R: <dR>,\n"
     <<w<<"    EMAX: <Emax>,\n"
     <<w<<"    EXP: <exp>,\n"
     <<w<<"    PT: <ptmin>,\n"
     <<w<<"    # optional isolation parameters:\n"
     <<w<<"    ETA: <etamax>,\n"
     <<w<<"    Y: <ymax>\n"
     <<w<<"    }\n"
     <<w<<"  NMin: <nmin>\n"
     <<w<<"  # optional parameters:\n"
     <<w<<"  NMax: <nmax>\n"
     <<w<<"  Output_ID: <kf>\n"
     <<w<<"  Remove_Isolated: true|false\n"
     <<w<<"  Remove_Nonisolated: true|false\n"
     <<w<<"  Subselectors: [...]";
}
