#ifndef PHASIC_Selectors_NJet_Finder_h
#define PHASIC_Selectors_NJet_Finder_h

#include "ATOOLS/Phys/Particle_List.H"
#include "PHASIC++/Selectors/Selector.H"
#include "ATOOLS/Math/Poincare.H"

namespace PHASIC {
  class NJet_Finder : public Selector_Base {
    double m_pt2min,m_et2min,m_delta_r,m_r2min,m_etamax,m_ymax,m_massmax;
    int    m_exp, m_type, m_njet;
    double m_ene, m_s, m_sprime; 

    double ** p_ktij;
    int    *  p_imap;
    double *  p_kis;
    std::vector<double> m_jetpt, m_kt2;


    double DEta12(ATOOLS::Vec4D &,ATOOLS::Vec4D &);
    double CosDPhi12(ATOOLS::Vec4D &,ATOOLS::Vec4D &);
    double DPhi12(ATOOLS::Vec4D &,ATOOLS::Vec4D &);
    double R2(ATOOLS::Vec4D &,ATOOLS::Vec4D &);
    double Y12(const ATOOLS::Vec4D &,const ATOOLS::Vec4D &) const;
    double DCos12(const ATOOLS::Vec4D &,const ATOOLS::Vec4D &) const;

    void   AddToJetlist(const ATOOLS::Vec4D &);

    void   ConstructJets(ATOOLS::Vec4D * p, int n);
  public:
    NJet_Finder(Process_Base *const proc, size_t nj,
                double ptmin, double etmin, double dr, int exp,
                double etamax, double ymax, double massmax,
                int type);

    ~NJet_Finder();

    bool   Trigger(ATOOLS::Selector_List &);

    void   BuildCuts(Cut_Data *);
  };
}

#endif

#include "PHASIC++/Process/Process_Base.H"
#include "PHASIC++/Main/Process_Integrator.H"
#include "ATOOLS/Org/Run_Parameter.H"
#include "ATOOLS/Org/Message.H"
#include "ATOOLS/Org/Exception.H"
#include "ATOOLS/Org/MyStrStream.H"


using namespace PHASIC;
using namespace ATOOLS;

/*---------------------------------------------------------------------

  General form - flavours etc are unknown, will operate on a Particle_List.

  --------------------------------------------------------------------- */

NJet_Finder::NJet_Finder(Process_Base *const proc, size_t nj,
                         double ptmin, double etmin, double dr, int exp,
                         double etamax, double ymax, double massmax,
                         int type) :
  Selector_Base("NJetfinder",proc),
  m_pt2min(sqr(ptmin)), m_et2min(sqr(etmin)),
  m_delta_r(dr), m_etamax(etamax), m_ymax(ymax), m_massmax(massmax),
  m_exp(exp), m_type(type)
{
  m_ene        = rpa->gen.Ecms()/2.;
  m_sprime     = m_s = sqr(2.*m_ene); 
  m_smin       = sqr(nj)*Max(m_pt2min,m_et2min);

  m_r2min      = sqr(m_delta_r);

  m_njet       = nj;

  p_kis  = new double[m_nout];
  p_imap = new int[m_nout];
  p_ktij = new double*[m_nout];
  for (int i=0;i<m_nout;++i) p_ktij[i] = new double[m_nout];
  for (int i=0;i<m_nout;++i) p_imap[i] = i;
  
}


NJet_Finder::~NJet_Finder() {
  for (int i=0;i<m_nout;++i) delete [] p_ktij[i];
  delete [] p_ktij;
  delete [] p_imap;
  delete [] p_kis;
}


void NJet_Finder::AddToJetlist(const Vec4D &jet) {
  if (dabs(jet.Eta())<m_etamax && dabs(jet.Y()<m_ymax)) {
    if (jet.EPerp2()>=m_et2min && jet.PPerp2()>=m_pt2min) {
      m_jetpt.push_back(jet.PPerp2());
    }
  }
  m_kt2.push_back(jet.PPerp2());
}

bool NJet_Finder::Trigger(Selector_List &sl)
{
  if (m_njet==0) return true;

  // create copy
  m_jetpt.clear();
  m_kt2.clear();
  size_t n(0);
  Vec4D * moms = new Vec4D[m_nout];
  for (size_t i=m_nin;i<sl.size();i++) {
    if (sl[i].Flavour().Strong() && sl[i].Flavour().Mass()<=m_massmax) {
      moms[n]=sl[i].Momentum();
      n++;
    }
  }

  // cluster
  for (size_t i=0;i<n;++i) p_imap[i] = i;

  ConstructJets(moms,n);

  delete [] moms;

  if (m_njet<0) {
    size_t np(0);
    for (size_t i(0);i<m_kt2.size();++i) {
      if (i>0 && m_kt2[i]<m_kt2[i-1])
	return 1-m_sel_log->Hit(1);
      if (m_kt2[i]>m_pt2min) ++np;
    }
    return 1-m_sel_log->Hit(np<-m_njet);
  }

  if (n<m_njet) return 0;

  bool trigger(true);
  if (m_jetpt.size()<size_t(m_njet)) trigger=false;

  return (1-m_sel_log->Hit(1-trigger));
}

void NJet_Finder::ConstructJets(Vec4D * p, int n)
{
  if (n==0) return;
  if (n==1) {
    AddToJetlist(p[0]);
    return;
  }

  //cal first matrix
  int ii=0, jj=0;
  double dmin=(m_type>1)?p[0].PPerp2():sqr(p[0][0]);
  dmin=pow(dmin,m_exp);
  for (int i=0;i<n;++i) {
    double di = (m_type>1)?p[i].PPerp2():sqr(p[i][0]);
    p_ktij[i][i] = di = pow(di,m_exp);
    if (di<dmin) { dmin=di; ii=i; jj=i;}
    for (int j=0;j<i;++j) {
      double dj  = p_ktij[j][j];
      double dij = p_ktij[i][j] = Min(di,dj)*R2(p[i],p[j]) /m_r2min;
      if (dij<dmin) {dmin=dij; ii=i; jj=j;}
    }
  }
  // recalc matrix
  while (n>0) {
    if (ii!=jj) {
      // combine precluster
      p[p_imap[jj]]+=p[p_imap[ii]];
      m_kt2.push_back(p_ktij[ii][jj]);
    }
    else {
      // add to jet list
      AddToJetlist(p[p_imap[ii]]);
    }

    --n;

    for (int i=ii;i<n;++i) p_imap[i]=p_imap[i+1];

    if (n==1) {
      int jjx=p_imap[jj];
      p_ktij[jjx][jjx] = pow((m_type>1)?p[jjx].PPerp2():sqr(p[jjx][0]),m_exp);
      break;
    }
    // update map (remove precluster)
    // update matrix (only what is necessary)
    int jjx=p_imap[jj];
    p_ktij[jjx][jjx] = pow((m_type>1)?p[jjx].PPerp2():sqr(p[jjx][0]),m_exp);
    for (int j=0;j<jj;++j)   p_ktij[jjx][p_imap[j]] = 
                               Min(p_ktij[jjx][jjx],p_ktij[p_imap[j]][p_imap[j]])
			       *R2(p[jjx],p[p_imap[j]])/m_r2min;
    for (int i=jj+1;i<n;++i) p_ktij[p_imap[i]][jjx] = 
                               Min(p_ktij[jjx][jjx],p_ktij[p_imap[i]][p_imap[i]])
			       *R2(p[p_imap[i]],p[jjx])/m_r2min;
    // redetermine rmin and dmin
    ii=jj=0;
    dmin=p_ktij[p_imap[0]][p_imap[0]];
    for (int i=0;i<n;++i) {
      int ix=p_imap[i];
      double di = p_ktij[ix][ix];
      if (di<dmin) { dmin=di; ii=jj=i;}
      for (int j=0;j<i;++j) {
	int jx=p_imap[j];
	double dij = p_ktij[ix][jx];
	if (dij<dmin) { dmin=dij; ii=i; jj=j;}
      }
    }
  }

  // add remaining preclusters to jetlist
  for (int i=0;i<n;++i) {
    AddToJetlist(p[p_imap[i]]);
  }
}


void NJet_Finder::BuildCuts(Cut_Data * cuts) 
{
  cuts->smin=Max(cuts->smin,m_smin);
}


/*----------------------------------------------------------------------------

  Utilities

  ----------------------------------------------------------------------------*/
double NJet_Finder::DEta12(Vec4D & p1,Vec4D & p2)
{
  // eta1,2 = -log(tan(theta_1,2)/2)   
  double c1=p1[3]/Vec3D(p1).Abs();
  double c2=p2[3]/Vec3D(p2).Abs();
  return  0.5 *log( (1 + c1)*(1 - c2)/((1-c1)*(1+c2)));
}

double NJet_Finder::CosDPhi12(Vec4D & p1,Vec4D & p2)
{
  double pt1=sqrt(p1[1]*p1[1]+p1[2]*p1[2]);
  double pt2=sqrt(p2[1]*p2[1]+p2[2]*p2[2]);
  return (p1[1]*p2[1]+p1[2]*p2[2])/(pt1*pt2);
}

double NJet_Finder::DPhi12(Vec4D & p1,Vec4D & p2)
{
  double pt1=sqrt(p1[1]*p1[1]+p1[2]*p1[2]);
  double pt2=sqrt(p2[1]*p2[1]+p2[2]*p2[2]);
  return acos(Min(1.0,Max(-1.0,(p1[1]*p2[1]+p1[2]*p2[2])/(pt1*pt2))));
}


double NJet_Finder::R2(Vec4D &p1, Vec4D &p2)
{
  return sqr(p1.Y()-p2.Y()) + sqr(DPhi12(p1,p2));
}

double NJet_Finder::Y12(const Vec4D & p1, const Vec4D & p2) const
{
  return 2.*sqr(Min(p1[0],p2[0]))*(1.-DCos12(p1,p2))/m_sprime;
}

double NJet_Finder::DCos12(const Vec4D & p1,const Vec4D & p2) const
{
  double s  = p1[1]*p2[1]+p1[2]*p2[2]+p1[3]*p2[3];
  double b1 = p1[1]*p1[1]+p1[2]*p1[2]+p1[3]*p1[3];
  double b2 = p2[1]*p2[1]+p2[2]*p2[2]+p2[3]*p2[3];
  return s/sqrt(b1*b2);
  //  return Vec3D(p1)*Vec3D(p2)/(Vec3D(p1).Abs()*Vec3D(p2).Abs());
}

DECLARE_GETTER(NJet_Finder,"NJetFinder",Selector_Base,Selector_Key);

Selector_Base *ATOOLS::Getter<Selector_Base,Selector_Key,NJet_Finder>::
operator()(const Selector_Key &key) const
{
  auto s = key.m_settings["NJetFinder"];

  // min/max settings
  const auto etamax
    = s["EtaMax"] .SetDefault("None").UseMaxDoubleReplacements().Get<double>();
  const auto ymax
    = s["YMax"]   .SetDefault("None").UseMaxDoubleReplacements().Get<double>();
  const auto massmax
    = s["MassMax"].SetDefault(0.0)   .UseMaxDoubleReplacements().Get<double>();
  const auto ptmin
    = s["PTMin"]  .SetDefault("None").UseZeroReplacements()     .Get<double>();
  const auto etmin
    = s["ETMin"]  .SetDefault("None").UseZeroReplacements()     .Get<double>();
  const auto n
    = s["N"]      .SetDefault("None").UseZeroReplacements()     .Get<int>();
  if (n < 0)
    THROW(not_implemented,"Negative multiplicities are not supported.");

  // parameter/mode settings
  auto exp  = s["Exp"] .SetDefault(1)  .Get<int>();
  auto type = s["Mode"].SetDefault(2)  .Get<int>();
  auto R    = s["R"]   .SetDefault(0.4).Get<double>();

  NJet_Finder *jf(new NJet_Finder(key.p_proc,
                                  n, ptmin, etmin, R,
                                  exp,etamax,ymax,massmax,type));
  return jf;
}

void ATOOLS::Getter<Selector_Base,Selector_Key,NJet_Finder>::
PrintInfo(std::ostream &str,const size_t width) const
{
  str<<"NJetFinder:\n"
     <<width<<"  N: number of jets\n"
     <<width<<"  PTMin: minimum jet pT\n"
     <<width<<"  ETMin: minimum jet eta\n"
     <<width<<"  R: jet distance parameter\n"
     <<width<<"  # optional settings:\n"
     <<width<<"  Exp: exponent for jet distances (default=1)\n"
     <<width<<"  YMax: maximum jet rapidity (default=None)\n"
     <<width<<"  EtaMax: maximum jet eta (default=None)\n"
     <<width<<"  MassMax: maximum jet constituent mass (default=0)\n"
     <<width<<"  Mode: type (default=2)";
}
