#include "PHASIC++/Channels/CS_Dipoles.H"

#include "PHASIC++/Channels/Multi_Channel.H"
#include "PHASIC++/Channels/Vegas.H"
#include "PHASIC++/Channels/Channel_Elements.H"
#include "PHASIC++/Channels/CSS_Kinematics.H"
#include "ATOOLS/Phys/NLO_Subevt.H"
#include "ATOOLS/Math/MathTools.H"
#include "ATOOLS/Math/Poincare.H"
#include "ATOOLS/Math/Tensor.H"
#include "ATOOLS/Org/Run_Parameter.H"
#include "ATOOLS/Org/Exception.H"
#include "ATOOLS/Org/Scoped_Settings.H"

using namespace ATOOLS;
using namespace PHASIC;

FF_Dipole::FF_Dipole(NLO_subevt *const sub,
		     Phase_Space_Handler *const psh,const bool bmcw):
  CS_Dipole(sub,psh,bmcw),
  m_mi(m_fli.Mass()), m_mj(m_flj.Mass()), m_mk(m_flk.Mass()),
  m_mi2(m_mi*m_mi), m_mj2(m_mj*m_mj), m_mij2(sqr(m_flij.Mass())),
  m_mk2(m_mk*m_mk), m_massive(m_mi||m_mj||m_mij2||m_mk)
{
  Scoped_Settings s{ Settings::GetMainSettings()["EEG"] };
  m_yexp = s["FF_Y_EXPONENT"].SetDefault(0.5).Get<double>();
  m_zexp = s["FF_Z_EXPONENT"].SetDefault(0.01).Get<double>();
}

FF_Dipole::~FF_Dipole() {}

bool FF_Dipole::ValidPoint(const ATOOLS::Vec4D_Vector &p)
{
  return m_on=2.0*p[m_ijt]*p[m_kt]>m_q2min;
}

Vec4D_Vector FF_Dipole::GeneratePoint
(const Vec4D_Vector &p,Cut_Data *const cuts,const double *rns)
{
  DEBUG_FUNC("");
  double *rn(p_vegas->GeneratePoint(rns));
  msg_Debugging()<<"vegased :     ";
  msg_Debugging()<<"y = "<<rn[0]<<", z = "<<rn[1]
                 <<", phi = "<<rn[2]<<"\n";
  if (!m_massive) {
    m_rn[0]=PeakedDist(0.0,m_yexp,m_amin,1.0,1,rn[0]);
    m_rn[1]=PeakedDist(0.0,m_zexp,0.0,1.0,1,rn[1]);
  }
  else {
    double Q2((p[m_ijt]+p[m_kt]).Abs2());
    double eps(Q2-m_mi2-m_mj2-m_mk2);
    double ymin(Max(0.0,2.0*m_mi*m_mj/eps));
    double ymax(1.0-2.0*m_mk*(sqrt(Q2)-m_mk)/eps);
    m_rn[0]=PeakedDist(0.0,m_yexp,ymin,ymax,1,rn[0]);
    double viji(sqrt(sqr(eps*m_rn[0])-sqr(2.0*m_mi*m_mj))/
		(eps*m_rn[0]+2.0*m_mi2));
    double vijk(sqrt(sqr(2.0*m_mk2+eps*(1.0-m_rn[0]))-
		     4.0*m_mk2*Q2)/(eps*(1.0-m_rn[0])));
    double zc(0.5*(2.0*m_mi2+eps*m_rn[0])/
	      (m_mi2+m_mj2+eps*m_rn[0]));
    double zmin(zc*(1.0-viji*vijk)), zmax(zc*(1.0+viji*vijk));
    m_rn[1]=PeakedDist(0.0,m_zexp,zmin,zmax,1,rn[1]);
  }
  m_rn[2]=rn[2]*2.0*M_PI;
  msg_Debugging()<<"transformed : ";
  msg_Debugging()<<"y = "<<m_rn[0]<<", z = "<<m_rn[1]
                 <<", phi = "<<m_rn[2]<<"\n";
  Vec4D_Vector pp(p.size()+1);
  for (size_t i(0);i<p.size();++i) pp[m_brmap[i]]=p[i];
  Kin_Args ff(m_rn[0],m_rn[1],m_rn[2]);
  if (ConstructFFDipole(m_mi2,m_mj2,m_mij2,m_mk2,p[m_ijt],p[m_kt],ff)<0)
    msg_Error()<<METHOD<<"(): Invalid kinematics."<<std::endl;
  pp[m_sub.m_i]=ff.m_pi;
  pp[m_sub.m_j]=ff.m_pj;
  pp[m_sub.m_k]=ff.m_pk;
  return pp;
}

double FF_Dipole::GenerateWeight(const Vec4D_Vector &p,Cut_Data *const cuts)
{
  Vec4D_Vector pp(p.size()-1);
  for (size_t i(0);i<p.size();++i) pp[m_rbmap[i]]=p[i];
  Kin_Args ff(ClusterFFDipole(m_mi2,m_mj2,m_mij2,m_mk2,
			      p[m_sub.m_i],p[m_sub.m_j],p[m_sub.m_k],1));
  if (ff.m_stat!=1) msg_Error()<<METHOD<<"(): Invalid kinematics"<<std::endl;
  m_rn[0]=ff.m_y;
  m_rn[1]=ff.m_z;
  m_rn[2]=ff.m_phi;
  pp[m_ijt]=ff.m_pi;
  pp[m_kt]=ff.m_pk;
  if (!ValidPoint(pp)) return m_weight=m_rbweight=0.0;
  if (m_bmcw) {
    p_fsmc->GenerateWeight(&pp.front(),cuts);
    if (p_ismc) {
      m_isrspkey[3]=(pp[0]+pp[1]).Abs2();
      m_isrykey[2]=(pp[0]+pp[1]).Y();
      p_ismc->GenerateWeight();
    }
  }
  if (m_rn[2]<0.0) m_rn[2]+=2.0*M_PI;
  msg_Debugging()<<"again :       ";
  msg_Debugging()<<"y = "<<m_rn[0]<<", z = "<<m_rn[1]
                 <<", phi = "<<m_rn[2]<<"\n";
  if (m_rn[0]<m_amin) {
    m_rbweight=m_weight=0.0;
    return 0.0;
  }
  m_weight=(pp[m_ijt]+pp[m_kt]).Abs2()/(16.0*sqr(M_PI))*(1.0-m_rn[0]);
  m_weight*=pow(m_rn[0],m_yexp)*pow(m_rn[1],m_zexp);
  if (!m_massive) {
    m_weight*=PeakedWeight
      (0.0,m_yexp,m_amin,1.0,m_rn[0],1,m_rn[0]);
    m_weight*=PeakedWeight
      (0.0,m_zexp,0.0,1.0,m_rn[1],1,m_rn[1]);
  }
  else {
    double Q2((pp[m_ijt]+pp[m_kt]).Abs2());
    double eps(Q2-m_mi2-m_mj2-m_mk2);
    double ymin(Max(0.0,2.0*m_mi*m_mj/eps));
    double ymax(1.0-2.0*m_mk*(sqrt(Q2)-m_mk)/eps);
    double viji(sqrt(sqr(eps*m_rn[0])-sqr(2.0*m_mi*m_mj))/
		(eps*m_rn[0]+2.0*m_mi2));
    double vijk(sqrt(sqr(2.0*m_mk2+eps*(1.0-m_rn[0]))-
		     4.0*m_mk2*Q2)/(eps*(1.0-m_rn[0])));
    double zc(0.5*(2.0*m_mi2+eps*m_rn[0])/
	      (m_mi2+m_mj2+eps*m_rn[0]));
    double zmin(zc*(1.0-viji*vijk)), zmax(zc*(1.0+viji*vijk));
    m_weight*=eps*eps/Q2/sqrt(sqr(Q2-m_mij2-m_mk2)-4.0*m_mij2*m_mk2);
    m_weight*=PeakedWeight
      (0.0,m_yexp,ymin,ymax,m_rn[0],1,m_rn[0]);
    m_weight*=PeakedWeight
      (0.0,m_zexp,zmin,zmax,m_rn[1],1,m_rn[1]);
  }
  m_rn[2]/=2.0*M_PI;
  msg_Debugging()<<"recovered :   ";
  msg_Debugging()<<"y = "<<m_rn[0]<<", z = "<<m_rn[1]
                 <<", phi = "<<m_rn[2]<<"\n";
  m_rbweight=m_weight*=p_vegas->GenerateWeight(m_rn);
  if (!m_bmcw) return m_weight;
  if (p_ismc) m_weight*=p_ismc->Weight();
  return m_weight*=p_fsmc->Weight();
}

FI_Dipole::FI_Dipole(ATOOLS::NLO_subevt *const sub,
		     Phase_Space_Handler *const psh,const bool bmcw):
  CS_Dipole(sub,psh,bmcw),
  m_mi(m_fli.Mass()), m_mj(m_flj.Mass()),
  m_mi2(m_mi*m_mi), m_mj2(m_mj*m_mj), m_mij2(sqr(m_flij.Mass())),
  m_massive(m_mi||m_mj||m_mij2)
{
  Scoped_Settings s{ Settings::GetMainSettings()["EEG"] };
  m_xexp = s["FI_X_EXPONENT"].SetDefault(0.5).Get<double>();
  m_zexp = s["FI_Z_EXPONENT"].SetDefault(0.01).Get<double>();
}

FI_Dipole::~FI_Dipole() {}

bool FI_Dipole::ValidPoint(const ATOOLS::Vec4D_Vector &p)
{
  if (p[m_ijt].PPerp2()<m_amin*m_q2min) return m_on=false;
  if (2.0*p[m_ijt]*p[m_kt]<=m_q2min) return m_on=false;
  double xmin(0.0);
  if (p[m_kt][3]>0.0) xmin=p[m_kt].PPlus()/rpa->gen.PBunch(0).PPlus();
  else xmin=p[m_kt].PMinus()/rpa->gen.PBunch(1).PMinus();
  return m_on=xmin<1.0-m_amin;
}

ATOOLS::Vec4D_Vector FI_Dipole::GeneratePoint
(const ATOOLS::Vec4D_Vector &p,Cut_Data *const cuts,const double *rns)
{
  DEBUG_FUNC("");
  double *rn(p_vegas->GeneratePoint(rns));
  msg_Debugging()<<"vegased :     ";
  if (p[m_kt][3]>0.0) m_xmin=p[m_kt].PPlus()/rpa->gen.PBunch(0).PPlus();
  else m_xmin=p[m_kt].PMinus()/rpa->gen.PBunch(1).PMinus();
  msg_Debugging()<<"x = "<<rn[0]<<", z = "<<rn[1]
                 <<", phi = "<<rn[2]<<", xmin = "<<m_xmin<<"\n";
  if (!m_massive) {
    m_rn[0]=PeakedDist(0.0,m_xexp,m_xmin,1.0-m_amin,1,rn[0]);
    m_rn[1]=PeakedDist(0.0,m_zexp,0.0,1.0,1,rn[1]);
  }
  else {
    double Q2(2.0*p[m_ijt]*p[m_kt]);
    m_rn[0]=PeakedDist(0.0,m_xexp,m_xmin,1-m_amin,1,rn[0]);
    double eps((1-m_rn[0])*Q2+m_rn[0]*(m_mij2+m_mi2-m_mj2));
    double kap(sqrt(sqr(eps-2.0*m_rn[0]*m_mi2)-4.0*m_mi2*m_mj2));
    double zmin(0.5*(eps-kap)/((1-m_rn[0])*Q2+m_rn[0]*m_mij2));
    double zmax(0.5*(eps+kap)/((1-m_rn[0])*Q2+m_rn[0]*m_mij2));
    if (zmax>1.0 && IsEqual(zmax,1.0)) zmax=1.0;
    m_rn[1]=PeakedDist(0.0,m_zexp,zmin,zmax,1,rn[1]);
  }
  m_rn[2]=rn[2]*2.0*M_PI;
  msg_Debugging()<<"transformed : ";
  msg_Debugging()<<"x = "<<m_rn[0]<<", z = "<<m_rn[1]
                 <<", phi = "<<m_rn[2]<<"\n";
  Vec4D_Vector pp(p.size()+1);
  for (size_t i(0);i<p.size();++i) pp[m_brmap[i]]=p[i];
  Kin_Args fi(1.0-m_rn[0],m_rn[1],m_rn[2]);
  if (ConstructFIDipole(m_mi2,m_mj2,m_mij2,0.0,p[m_ijt],p[m_kt],fi)<0)
    msg_Error()<<METHOD<<"(): Invalid kinematics"<<std::endl;
  pp[m_sub.m_i]=fi.m_pi;
  pp[m_sub.m_j]=fi.m_pj;
  pp[m_sub.m_k]=fi.m_pk;
  return pp;
}

double FI_Dipole::GenerateWeight
(const ATOOLS::Vec4D_Vector &p,Cut_Data *const cuts)
{
  Vec4D_Vector pp(p.size()-1);
  for (size_t i(0);i<p.size();++i) pp[m_rbmap[i]]=p[i];
  Kin_Args fi(ClusterFIDipole(m_mi2,m_mj2,m_mij2,0.0,
			      p[m_sub.m_i],p[m_sub.m_j],p[m_sub.m_k],1));
  if (fi.m_stat!=1) msg_Error()<<METHOD<<"(): Invalid kinematics"<<std::endl;
  m_rn[0]=1.0-fi.m_y;
  m_rn[1]=fi.m_z;
  m_rn[2]=fi.m_phi;
  pp[m_ijt]=fi.m_pi;
  pp[m_kt]=fi.m_pk;
  double Q2(2.0*pp[m_ijt]*pp[m_kt]);
  if (!ValidPoint(pp)) return m_weight=m_rbweight=0.0;
  if (m_rn[2]<0.0) m_rn[2]+=2.0*M_PI;
  if (pp[m_kt][3]>0.0) m_xmin=pp[m_kt].PPlus()/rpa->gen.PBunch(0).PPlus();
  else m_xmin=pp[m_kt].PMinus()/rpa->gen.PBunch(1).PMinus();
  msg_Debugging()<<"again :       ";
  msg_Debugging()<<"x = "<<m_rn[0]<<", z = "<<m_rn[1]
                 <<", phi = "<<m_rn[2]<<"\n";
  if (m_rn[0]<m_xmin || m_rn[0]>1.0-m_amin) {
    m_rbweight=m_weight=0.0;
    return 0.0;
  }
  if (m_bmcw) {
    p_fsmc->GenerateWeight(&pp.front(),cuts);
    m_isrspkey[3]=(pp[0]+pp[1]).Abs2();
    m_isrykey[2]=(pp[0]+pp[1]).Y();
    p_ismc->GenerateWeight();
  }
  m_weight=Q2/(16.0*sqr(M_PI))/sqr(m_rn[0]);
  m_weight*=pow(m_rn[0],m_xexp)*pow(m_rn[1],m_zexp);
  if (!m_massive) {
    m_weight*=PeakedWeight
      (0.0,m_xexp,m_xmin,1.0-m_amin,m_rn[0],1,m_rn[0]);
    m_weight*=PeakedWeight
      (0.0,m_zexp,0.0,1.0,m_rn[1],1,m_rn[1]);
  }
  else {
    double eps((1-m_rn[0])*Q2+m_rn[0]*(m_mij2+m_mi2-m_mj2));
    double kap(sqrt(sqr(eps-2.0*m_rn[0]*m_mi2)-4.0*m_mi2*m_mj2));
    double zmin(0.5*(eps-kap)/((1-m_rn[0])*Q2+m_rn[0]*m_mij2));
    double zmax(0.5*(eps+kap)/((1-m_rn[0])*Q2+m_rn[0]*m_mij2));
    if (zmax>1.0 && IsEqual(zmax,1.0)) zmax=1.0;
    m_weight*=PeakedWeight
      (0.0,m_xexp,m_xmin,1.0-m_amin,m_rn[0],1,m_rn[0]);
    m_weight*=PeakedWeight
      (0.0,m_zexp,zmin,zmax,m_rn[1],1,m_rn[1]);
  }
  m_rn[2]/=2.0*M_PI;
  msg_Debugging()<<"recovered :   ";
  msg_Debugging()<<"x = "<<m_rn[0]<<", z = "<<m_rn[1]
                 <<", phi = "<<m_rn[2]<<", xmin = "<<m_xmin<<"\n";
  m_rbweight=m_weight*=p_vegas->GenerateWeight(m_rn);
  if (!m_bmcw) return m_weight;
  return m_weight*=p_fsmc->Weight()*p_ismc->Weight();
}

IF_Dipole::IF_Dipole(ATOOLS::NLO_subevt *const sub,
		     Phase_Space_Handler *const psh,const bool bmcw):
  CS_Dipole(sub,psh,bmcw),
  m_mk2(sqr(m_flk.Mass()))
{
  Scoped_Settings s{ Settings::GetMainSettings()["EEG"] };
  m_xexp = s["IF_X_EXPONENT"].SetDefault(0.5).Get<double>();
  m_uexp = s["IF_U_EXPONENT"].SetDefault(0.5).Get<double>();
}

IF_Dipole::~IF_Dipole() {}

bool IF_Dipole::ValidPoint(const ATOOLS::Vec4D_Vector &p)
{
  if (p[m_kt].PPerp2()<m_amin*m_q2min) return m_on=false;
  double xmin(0.0);
  if (p[m_ijt][3]>0.0) xmin=p[m_ijt].PPlus()/rpa->gen.PBunch(0).PPlus();
  else xmin=p[m_ijt].PMinus()/rpa->gen.PBunch(1).PMinus();
  if (1.0-xmin<m_amin) return m_on=false;
  return m_on=2.0*p[m_ijt]*p[m_kt]>m_q2min;
}

ATOOLS::Vec4D_Vector IF_Dipole::GeneratePoint
(const ATOOLS::Vec4D_Vector &p,Cut_Data *const cuts,const double *rns)
{
  DEBUG_FUNC("");
  double *rn(p_vegas->GeneratePoint(rns));
  if (p[m_ijt][3]>0.0) m_xmin=p[m_ijt].PPlus()/rpa->gen.PBunch(0).PPlus();
  else m_xmin=p[m_ijt].PMinus()/rpa->gen.PBunch(1).PMinus();
  msg_Debugging()<<"vegased :     ";
  msg_Debugging()<<"x = "<<rn[0]<<", u = "<<rn[1]
                 <<", phi = "<<rn[2]<<", xmin = "<<m_xmin<<"\n";
  m_rn[0]=PeakedDist(0.0,m_xexp,m_xmin,1.0,1,rn[0]);
  double umax((1.0-m_rn[0])/(1.0-m_rn[0]+m_rn[0]*m_mk2/(2.0*p[m_ijt]*p[m_kt])));
  m_rn[1]=PeakedDist(0.0,m_uexp,m_amin,umax,1,rn[1]);
  m_rn[2]=rn[2]*2.0*M_PI;
  msg_Debugging()<<"transformed : ";
  msg_Debugging()<<"x = "<<m_rn[0]<<", u = "<<m_rn[1]
                 <<", phi = "<<m_rn[2]<<"\n";
  Vec4D_Vector pp(p.size()+1);
  for (size_t i(0);i<p.size();++i) pp[m_brmap[i]]=p[i];
  Kin_Args ifp(m_rn[1],m_rn[0],m_rn[2],1);
  if (ConstructIFDipole(0.0,0.0,0.0,m_mk2,0.0,
			p[m_ijt],p[m_kt],Vec4D(),ifp)<0)
    msg_Error()<<METHOD<<"(): Invalid kinematics"<<std::endl;
  pp[m_sub.m_i]=ifp.m_pi;
  pp[m_sub.m_j]=ifp.m_pj;
  pp[m_sub.m_k]=ifp.m_pk;
  return pp;
}

double IF_Dipole::GenerateWeight
(const ATOOLS::Vec4D_Vector &p,Cut_Data *const cuts)
{
  Vec4D_Vector pp(p.size()-1);
  for (size_t i(0);i<p.size();++i) pp[m_rbmap[i]]=p[i];
  Kin_Args ifp(ClusterIFDipole
	       (0.0,0.0,0.0,m_mk2,0.0,
		p[m_sub.m_i],p[m_sub.m_j],p[m_sub.m_k],Vec4D(),1|4));
  if (ifp.m_stat!=1) msg_Error()<<METHOD<<"(): Invalid kinematics"<<std::endl;
  m_rn[0]=ifp.m_z;
  m_rn[1]=ifp.m_y;
  m_rn[2]=ifp.m_phi;
  pp[m_ijt]=ifp.m_pi;
  pp[m_kt]=ifp.m_pk;
  if (!ValidPoint(pp)) return m_weight=m_rbweight=0.0;
  if (m_rn[2]<0.0) m_rn[2]+=2.0*M_PI;
  if (pp[m_ijt][3]>0.0) m_xmin=pp[m_ijt].PPlus()/rpa->gen.PBunch(0).PPlus();
  else m_xmin=pp[m_ijt].PMinus()/rpa->gen.PBunch(1).PMinus();
  msg_Debugging()<<"again :       ";
  msg_Debugging()<<"x = "<<m_rn[0]<<", u = "<<m_rn[1]
                 <<", phi = "<<m_rn[2]<<"\n";
  if (m_rn[0]<m_xmin ||
      m_rn[1]<m_amin) {
    m_rbweight=m_weight=0.0;
    return 0.0;
  }
  if (m_bmcw) {
    p_fsmc->GenerateWeight(&pp.front(),cuts);
    m_isrspkey[3]=(pp[0]+pp[1]).Abs2();
    m_isrykey[2]=(pp[0]+pp[1]).Y();
    p_ismc->GenerateWeight();
  }
  double Q2(2.0*pp[m_ijt]*pp[m_kt]);
  m_weight=Q2/(16.0*sqr(M_PI))/sqr(m_rn[0]);
  m_weight*=pow(m_rn[1],m_uexp)*pow(m_rn[0],m_xexp);
  double umax((1.0-m_rn[0])/(1.0-m_rn[0]+m_rn[0]*m_mk2/Q2));
  m_weight*=PeakedWeight
    (0.0,m_uexp,m_amin,umax,m_rn[1],1,m_rn[1]);
  m_weight*=PeakedWeight
    (0.0,m_xexp,m_xmin,1.0,m_rn[0],1,m_rn[0]);
  m_rn[2]/=2.0*M_PI;
  msg_Debugging()<<"recovered :   ";
  msg_Debugging()<<"x = "<<m_rn[0]<<", u = "<<m_rn[1]
                 <<", phi = "<<m_rn[2]<<", xmin = "<<m_xmin<<"\n";
  m_rbweight=m_weight*=p_vegas->GenerateWeight(m_rn);
  if (!m_bmcw) return m_weight;
  return m_weight*=p_fsmc->Weight()*p_ismc->Weight();
}

II_Dipole::II_Dipole(ATOOLS::NLO_subevt *const sub,
		     Phase_Space_Handler *const psh,const bool bmcw):
  CS_Dipole(sub,psh,bmcw)
{
  Scoped_Settings s{ Settings::GetMainSettings()["EEG"] };
  m_xexp = s["II_X_EXPONENT"].SetDefault(0.5).Get<double>();
  m_vexp = s["II_V_EXPONENT"].SetDefault(0.5).Get<double>();
}

II_Dipole::~II_Dipole() {}

bool II_Dipole::ValidPoint(const ATOOLS::Vec4D_Vector &p)
{
  if (2.0*p[m_ijt]*p[m_kt]<=m_q2min) return m_on=false;
  double xmin(0.0);
  if (p[m_ijt][3]>0.0) xmin=p[m_ijt].PPlus()/rpa->gen.PBunch(0).PPlus();
  else xmin=p[m_ijt].PMinus()/rpa->gen.PBunch(1).PMinus();
  return m_on=xmin<1.0-m_amin;
}

ATOOLS::Vec4D_Vector II_Dipole::GeneratePoint
(const ATOOLS::Vec4D_Vector &p,Cut_Data *const cuts,const double *rns)
{
  DEBUG_FUNC("");
  // massless x- and v-bounds so far
  double *rn(p_vegas->GeneratePoint(rns));
  if (p[m_ijt][3]>0.0) m_xmin=p[m_ijt].PPlus()/rpa->gen.PBunch(0).PPlus();
  else m_xmin=p[m_ijt].PMinus()/rpa->gen.PBunch(1).PMinus();
  msg_Debugging()<<"vegased :     ";
  msg_Debugging()<<"x = "<<rn[0]<<", v = "<<rn[1]
                 <<", phi = "<<rn[2]<<", xmin = "<<m_xmin<<"\n";
  m_rn[0]=PeakedDist(0.0,m_xexp,m_xmin,1.0-m_amin,1,rn[0]);
  m_rn[1]=PeakedDist(0.0,m_vexp,m_amin,1.0-m_rn[0],1,rn[1]);
  m_rn[2]=rn[2]*2.0*M_PI;
  msg_Debugging()<<"transformed : ";
  msg_Debugging()<<"x = "<<m_rn[0]<<", v = "<<m_rn[1]
                 <<", phi = "<<m_rn[2]<<"\n";
  Vec4D_Vector pp(p.size()+1);
  for (size_t i(0);i<p.size();++i) pp[m_brmap[i]]=p[i];
  if (m_rn[1]>1.-m_rn[0]) {
    msg_Error()<<METHOD<<"(): v > 1-x, "<<m_rn[1]
	       <<" vs. "<<1.0-m_rn[0]<<"\n";
    m_rn[1]=(1.0-1.0e-6)*(1.0-m_rn[0]);
  }
  Construct(pp[m_sub.m_i],pp[m_sub.m_j],pp[m_sub.m_k],pp,
            m_rn[0],m_rn[1],m_rn[2],p[m_ijt],p[m_kt],p);
  return pp;
}

double II_Dipole::GenerateWeight
(const ATOOLS::Vec4D_Vector &p,Cut_Data *const cuts)
{
  // massless x-/v-bounds and weight so far
  Vec4D_Vector pp(p.size()-1);
  Calculate(p[m_sub.m_i],p[m_sub.m_j],p[m_sub.m_k],p,
            m_rn[0],m_rn[1],m_rn[2],pp[m_ijt],pp[m_kt],pp);
  if (!ValidPoint(pp)) return m_weight=m_rbweight=0.0;
  if (m_rn[2]<0.0) m_rn[2]+=2.0*M_PI;
  if (pp[m_ijt][3]>0.0) m_xmin=pp[m_ijt].PPlus()/rpa->gen.PBunch(0).PPlus();
  else m_xmin=pp[m_ijt].PMinus()/rpa->gen.PBunch(1).PMinus();
  msg_Debugging()<<"again :       ";
  msg_Debugging()<<"x = "<<m_rn[0]<<", v = "<<m_rn[1]
                 <<", phi = "<<m_rn[2]<<"\n";
  if (m_rn[0]<m_xmin || m_rn[0]>1.0-m_amin ||
      m_rn[1]<m_amin || m_rn[1]>1.0-m_rn[0]) {
    m_rbweight=m_weight=0.0;
    return 0.0;
  }
  if (m_bmcw) {
    p_fsmc->GenerateWeight(&pp.front(),cuts);
    m_isrspkey[3]=(pp[0]+pp[1]).Abs2();
    m_isrykey[2]=(pp[0]+pp[1]).Y();
    p_ismc->GenerateWeight();
  }
  // 2(pa*pb)/16pi^2
  m_weight=(p[m_sub.m_i]+p[m_sub.m_k]).Abs2()/
    (16.0*sqr(M_PI))/m_rn[0];
  m_weight*=pow(m_rn[1],m_vexp)*pow(m_rn[0],m_xexp);
  m_weight*=PeakedWeight
    (0.0,m_vexp,m_amin,1.0-m_rn[0],m_rn[1],1,m_rn[1]);
  m_weight*=PeakedWeight
    (0.0,m_xexp,m_xmin,1.0-m_amin,m_rn[0],1,m_rn[0]);
  m_rn[2]/=2.0*M_PI;
  msg_Debugging()<<"recovered :   ";
  msg_Debugging()<<"x = "<<m_rn[0]<<", v = "<<m_rn[1]
                 <<", phi = "<<m_rn[2]<<", xmin = "<<m_xmin<<"\n";
  m_rbweight=m_weight*=p_vegas->GenerateWeight(m_rn);
  if (!m_bmcw) return m_weight;
  return m_weight*=p_fsmc->Weight()*p_ismc->Weight();
}

void II_Dipole::Calculate
(const ATOOLS::Vec4D &pi, const ATOOLS::Vec4D &pj, const ATOOLS::Vec4D &pk,
 const ATOOLS::Vec4D_Vector& kj,
 double &x, double &v, double &phi,
 ATOOLS::Vec4D &pijt, ATOOLS::Vec4D &pkt,
 ATOOLS::Vec4D_Vector& kjt)
{
  double pipj(pi*pj), pipk(pi*pk), pjpk(pj*pk);
  x=(pipk-pipj-pjpk)/pipk;
  v=pipj/pipk;

  pijt=x*pi;
  pkt=pk;

  double kp(sqrt(2.*pipk*v*(1.-x-v)));
  Vec4D kperp(pj-(1.-x-v)/x*pijt-v*pkt);
  if      ((kperp[1]>=0. && kperp[2]>=0.) || (kperp[1]>=0. && kperp[2]<0.))
    phi=acos(kperp[2]/kp);
  else if ((kperp[1]<0. && kperp[2]<0.) || (kperp[1]<0. && kperp[2]>=0.))
    phi=-acos(kperp[2]/kp)+2.*M_PI;
  else THROW(fatal_error,"Could not determine phi.");

  Vec4D K(pi-pj+pk), Kt(pijt+pkt);
  ATOOLS::Lorentz_Ten2D Lambda = MetricTensor()
                                 - 2./(K+Kt).Abs2()*BuildTensor(Kt+K,Kt+K)
                                 + 2./Kt.Abs2()*BuildTensor(Kt,K);
  for (size_t j(2), i(j);j<kjt.size();++i,++j) {
    if (i==m_sub.m_j) ++i;
    kjt[m_rbmap[i]] = Contraction(Lambda,2,kj[i]);
    msg_Debugging()<<"("<<i<<"):"<<kj[i]
		   <<" -> ("<<m_rbmap[i]<<"):"<<kjt[m_rbmap[i]]<<std::endl;
  }
}

void II_Dipole::Construct
(ATOOLS::Vec4D &pi, ATOOLS::Vec4D &pj, ATOOLS::Vec4D &pk,
 ATOOLS::Vec4D_Vector& kj,
 const double &x, const double &v, const double &phi,
 const ATOOLS::Vec4D &ipijt, const ATOOLS::Vec4D &ipkt,
 const ATOOLS::Vec4D_Vector& kjt)
{
  DEBUG_FUNC("");
  Vec4D pijt=ipijt;
  Vec4D pkt=ipkt;
  pi=1./x*pijt;
  pk=pkt;

  double kp=sqrt(2.*pi*pk*v*(1.-x-v));
  Vec4D kperp(0.,sin(phi)*kp,cos(phi)*kp,0.);

  pj=(1.-x-v)/x*pijt + v*pkt + kperp;
  msg_Debugging()<<"("<<m_ijt<<"):"<<pijt
                 <<" -> ("<<m_sub.m_i<<"):"<<pi
                 <<" ("<<m_sub.m_j<<"):"<<pj<<std::endl;
  msg_Debugging()<<"("<<m_kt<<"):"<<pkt
                 <<" -> ("<<m_sub.m_k<<"):"<<pk<<std::endl;

  Vec4D K(pi-pj+pk), Kt(pijt+pkt);
  ATOOLS::Lorentz_Ten2D Lambda = MetricTensor()
                                 - 2./(K+Kt).Abs2()*BuildTensor(Kt+K,Kt+K)
                                 + 2./Kt.Abs2()*BuildTensor(K,Kt);

  for (size_t i(0);i<kjt.size();++i) {
    if (i!=m_ijt && i!=m_kt) {
      kj[m_brmap[i]] = Contraction(Lambda,2,kjt[i]);
      msg_Debugging()<<"("<<i<<"):"<<kjt[i]
                     <<" -> ("<<m_brmap[i]<<"):"<<kj[m_brmap[i]]<<std::endl;
    }
  }
}

