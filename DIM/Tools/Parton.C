#include "DIM/Tools/Parton.H"

#include "DIM/Tools/Splitting.H"
#include "ATOOLS/Org/Run_Parameter.H"
#include "ATOOLS/Org/MyStrStream.H"
#include "ATOOLS/Org/STL_Tools.H"
#include "ATOOLS/Org/Message.H"

using namespace DIM;
using namespace ATOOLS;

size_t Parton::s_cnt(0);

Parton::Parton
(Amplitude *const ampl,
 const ATOOLS::Flavour &f,const ATOOLS::Vec4D &p,
 const Color &c,const int h):
  p_ampl(ampl), m_f(f), m_p(p), m_c(c), m_h(h), m_b(0),
  m_id(0)
{
  ++s_cnt;
}

Parton::~Parton()
{
  --s_cnt;
}

double Parton::GetXB() const
{
  if (m_b==1) return -m_p.PPlus()/rpa->gen.PBunch(0).PPlus();
  if (m_b==2) return -m_p.PMinus()/rpa->gen.PBunch(1).PMinus();
  return 0.0;
}

void Parton::AddWeight(const Splitting &s,const int acc)
{
  double w(acc?s.m_w.Accept():s.m_w.Reject());
  if (w==1.0 && s.m_vars.empty()) return;
  Weight_Map::iterator wit=m_ws.insert(make_pair(s.p_s,Weight_Vector())).first;
  Weight c(wit->second.empty()?Weight(s.m_vars.size()):wit->second.back());
  c.m_t=s.m_t;
  c.m_w*=w;
  for (size_t i(0);i<s.m_vars.size();++i) c.m_v[i]*=w*s.m_vars[i];
  wit->second.push_back(c);
}

double Parton::GetWeight(const double &t,std::vector<double> &v) const
{
  if (m_ws.empty()) return 1.0;
  double wgt(1.0);
  for (Weight_Map::const_iterator
	 wit(m_ws.begin());wit!=m_ws.end();++wit) {
    const Weight_Vector &ws(wit->second);
    size_t l(0), r(ws.size()-1), c((l+r)/2);
    double a(ws[c].m_t);
    while (r-l>1) {
      if (t>a) r=c;
      else l=c;
      c=(l+r)/2;
      a=ws[c].m_t;
    }
    if (t<=ws[r].m_t) {
      wgt*=ws[r].m_w;
      v.resize(ws[r].m_v.size(),1.0);
      for (size_t i(0);i<v.size();++i) v[i]*=ws[r].m_v[i];
    }
    else if (t<=ws[l].m_t) {
      wgt*=ws[l].m_w;
      v.resize(ws[r].m_v.size(),1.0);
      for (size_t i(0);i<v.size();++i) v[i]*=ws[l].m_v[i];
    }
  }
  return wgt;
}

void Parton::SetColor(const Color &c)
{
  m_c=c;
}

namespace DIM {

  std::ostream &operator<<(std::ostream &s,const Parton &p)
  {
    std::string hist;
    if (p.Beam()) hist+=ToString(p.Beam())+" ";
    for (Parton::Weight_Map::const_iterator 
	   wit(p.Weights().begin());wit!=p.Weights().end();++wit) {
      const Parton::Weight_Vector &ws(wit->second);
      hist+=" ["+ToString(wit->first->Id())+"]{"
	+ToString(ws[0].m_t)+":"+ToString(ws[0].m_w);
      for (size_t i(1);i<ws.size();++i)
	hist+=","+ToString(ws[i].m_t)+":"+ToString(ws[i].m_w);
      hist+="}";
    }
    double m(p.Mom().Abs2());
    m=m>=0.0?sqrt(dabs(m)):-sqrt(dabs(m));
    return s<<std::setw(6)<<ToString(p.Id())
	    <<std::right<<std::setw(4)<<p.Flav()
	    <<std::left<<" ["<<p.Hel()<<"]"
	    <<std::setw(10)<<ToString(p.Col())
	    <<p.Mom()<<" "<<m<<" "<<hist;
  }

}
