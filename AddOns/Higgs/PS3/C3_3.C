#include "PHASIC++/Channels/Single_Channel.H"
#include "ATOOLS/Org/Run_Parameter.H"
#include "ATOOLS/Org/MyStrStream.H"
#include "ATOOLS/Org/Scoped_Settings.H"
#include "PHASIC++/Channels/Channel_Elements.H"
#include "PHASIC++/Channels/Vegas.H"

using namespace PHASIC;
using namespace ATOOLS;

namespace PHASIC {
  class C3_3 : public Single_Channel {
    double m_alpha,m_ctmax,m_ctmin;
    Info_Key m_kTC_0__1_4_3_2,m_kTC_0__1__23_4,m_kZS_0;
    Vegas* p_vegas;
  public:
    C3_3(int,int,Flavour*,Integration_Info * const);
    ~C3_3();
    void   GenerateWeight(Vec4D *,Cut_Data *);
    void   GeneratePoint(Vec4D *,Cut_Data *,double *);
    void   AddPoint(double);
    void   MPISync()                 { p_vegas->MPISync(); }
    void   Optimize()                { p_vegas->Optimize(); } 
    void   EndOptimize()             { p_vegas->EndOptimize(); } 
    void   WriteOut(std::string pId) { p_vegas->WriteOut(pId); } 
    void   ReadIn(std::string pId)   { p_vegas->ReadIn(pId); } 
    void   ISRInfo(int &,double &,double &);
    std::string ChID();
  };
}

extern "C" Single_Channel * Getter_C3_3(int nin,int nout,Flavour* fl,Integration_Info * const info) {
  return new C3_3(nin,nout,fl,info);
}

void C3_3::GeneratePoint(Vec4D * p,Cut_Data * cuts,double * _ran)
{
  double *ran = p_vegas->GeneratePoint(_ran);
  for(int i=0;i<m_rannum;i++) p_rans[i]=ran[i];
  Vec4D p234=p[0]+p[1];
  double s234_max = p234.Abs2();
  double s4 = p_ms[4];
  double s23_max = sqr(sqrt(s234_max)-sqrt(p_ms[4]));
  double s2 = p_ms[2];
  double s3 = p_ms[3];
  double s23_min = cuts->Getscut((1<<2)|(1<<3));
  Vec4D  p23;
  double s23 = CE.MasslessPropMomenta(.5,s23_min,s23_max,ran[0]);
  CE.TChannelMomenta(p[0],p[1],p23,p[4],s23,s4,0.,m_alpha,m_ctmax,m_ctmin,ran[1],ran[2]);
  Vec4D  p1_4 = p[1]-p[4];
  CE.TChannelMomenta(p[0],p1_4,p[3],p[2],s3,s2,0.,m_alpha,1.,-1.,ran[3],ran[4]);
}

void C3_3::GenerateWeight(Vec4D* p,Cut_Data * cuts)
{
  double wt = 1.;
  Vec4D p234=p[0]+p[1];
  double s234_max = p234.Abs2();
  double s23_max = sqr(sqrt(s234_max)-sqrt(p_ms[4]));
  double s23_min = cuts->Getscut((1<<2)|(1<<3));
  Vec4D  p23 = p[2]+p[3];
  double s23 = dabs(p23.Abs2());
  wt *= CE.MasslessPropWeight(.5,s23_min,s23_max,s23,p_rans[0]);
  if (m_kTC_0__1__23_4.Weight()==ATOOLS::UNDEFINED_WEIGHT)
    m_kTC_0__1__23_4<<CE.TChannelWeight(p[0],p[1],p23,p[4],0.,m_alpha,m_ctmax,m_ctmin,m_kTC_0__1__23_4[0],m_kTC_0__1__23_4[1]);
  wt *= m_kTC_0__1__23_4.Weight();

  p_rans[1]= m_kTC_0__1__23_4[0];
  p_rans[2]= m_kTC_0__1__23_4[1];
  Vec4D  p1_4 = p[1]-p[4];
  if (m_kTC_0__1_4_3_2.Weight()==ATOOLS::UNDEFINED_WEIGHT)
    m_kTC_0__1_4_3_2<<CE.TChannelWeight(p[0],p1_4,p[3],p[2],0.,m_alpha,1.,-1.,m_kTC_0__1_4_3_2[0],m_kTC_0__1_4_3_2[1]);
  wt *= m_kTC_0__1_4_3_2.Weight();

  p_rans[3]= m_kTC_0__1_4_3_2[0];
  p_rans[4]= m_kTC_0__1_4_3_2[1];
  double vw = p_vegas->GenerateWeight(p_rans);
  if (wt!=0.) wt = vw/wt/pow(2.*M_PI,3*3.-4.);

  m_weight = wt;
}

C3_3::C3_3(int nin,int nout,Flavour* fl,Integration_Info * const info)
       : Single_Channel(nin,nout,fl)
{
  m_name = std::string("C3_3");
  m_rannum = 5;
  p_rans  = new double[m_rannum];
  Settings& s = Settings::GetMainSettings();
  m_alpha = s["SCHANNEL_ALPHA"].Get<double>();
  m_ctmax = 1.;
  m_ctmin = -1.;
  m_kTC_0__1_4_3_2.Assign(std::string("TC_0__1_4_3_2"),2,0,info);
  m_kTC_0__1__23_4.Assign(std::string("TC_0__1__23_4"),2,0,info);
  m_kZS_0.Assign(std::string("ZS_0"),2,0,info);
  p_vegas = new Vegas(m_rannum,100,m_name);
}

C3_3::~C3_3()
{
  delete p_vegas;
}

void C3_3::ISRInfo(int & type,double & mass,double & width)
{
  type  = 2;
  mass  = 0;
  width = 0.;
}

void C3_3::AddPoint(double Value)
{
  Single_Channel::AddPoint(Value);
  p_vegas->AddPoint(Value,p_rans);
}
std::string C3_3::ChID()
{
  return std::string("CGND$MTH_23$TC_0__1_4_3_2$TC_0__1__23_4$ZS_0$");
}
