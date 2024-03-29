#include "PHASIC++/Selectors/Cut_Data.H"

#include "ATOOLS/Org/Run_Parameter.H"
#include "ATOOLS/Org/MyStrStream.H"
#include "ATOOLS/Math/Poincare.H"
#include "ATOOLS/Org/Message.H"
#include "ATOOLS/Org/Scoped_Settings.H"

using namespace PHASIC;
using namespace ATOOLS;
using namespace std;


std::ostream & PHASIC::operator<<(std::ostream & s , Cut_Data & cd)
{
  s<<" Cut Data : "<<cd.ncut<<" \n"<<std::endl;
  for (int i=0;i<cd.ncut;++i) {
    for (int j=0;j<cd.ncut;++j) s<<cd.scut[i][j]<<"  ";
    s<<std::endl;
  }
  return s;
}


Cut_Data::Cut_Data() {
  energymin = 0;
  etmin = 0;
  scut      = 0;
  ncut      = 0;
}

Cut_Data::~Cut_Data() {
  if (!scut) return;
  for (short int i=0;i<ncut;i++) {
    delete[] scut[i];
    delete[] scut_save[i];
  }
  delete[] scut;
  delete[] scut_save;
  delete[] energymin;
  delete[] energymin_save;
  delete[] etmin;
}

void Cut_Data::Init(int _nin,const Flavour_Vector &_fl) {
  if (energymin != 0) return;
  smin = 0.;
  nin            = _nin;
  ncut           = _fl.size();
  fl             = &_fl.front();
  energymin      = new double[ncut];
  energymin_save = new double[ncut];
  etmin          = new double[ncut];
  scut           = new double*[ncut];
  scut_save      = new double*[ncut];

  for (int i=0;i<ncut;i++) {
    scut[i]        = new double[ncut];
    scut_save[i]   = new double[ncut];
    energymin[i]   = Max(0.,fl[i].SelMass());
    if (fl[i].IsKK()) energymin[i] = 0.;
    smin += energymin_save[i] = energymin[i];
    etmin[i]       = 0.;
  }
  smin = sqr(smin);

  Settings& s = Settings::GetMainSettings();
  double sijminfac{ s["INT_MINSIJ_FACTOR"].SetDefault(0.).Get<double>() };
  for (int i=0;i<ncut;i++) {
    for (int j=i;j<ncut;j++) {
      scut[i][j] = scut[j][i] = scut_save[i][j] =
              (i<nin)^(j<nin)?0.0:sijminfac*sqr(rpa->gen.Ecms());
    }
  }  
}

void Cut_Data::Complete()
{
  for (int i=0;i<ncut;i++) {
    for (int j=i+1;j<ncut;j++) {
      if ((i<nin)^(j<nin)) continue;
      scut[i][j] = scut[j][i] =
	Max(scut[i][j],sqr(fl[i].SelMass()+fl[j].SelMass()));
    }
  } 

  size_t str(0);
  for (int i=0;i<ncut;i++) {
    energymin_save[i] = energymin[i];
    for (int j=i+1;j<ncut;j++) {
      scut_save[i][j]   = scut[i][j];
    }
    if (i>=2) str|=(1<<i);
  }
  double local_smin = 0.;
  double etmm = 0.; 
  double e1=0.,e2=0.;
  for (int i=2;i<ncut;i++) {
    if (etmin[i]>etmm) etmm = etmin[i];
    local_smin += etmin[i];
    e1 += energymin[i];
  }
  smin = Max(smin,sqr(local_smin));
  smin = Max(smin,sqr(e1)-sqr(e2));
  smin = Max(smin,sqr(2.*etmm));
  smin = Max(smin,Getscut(str));

  msg_Tracking()<<"Cut_Data::Complete(): s_{min} = "<<smin<<endl;
  m_smin_map.clear();
}

char Cut_Data::GetIndexID(int id)
{
  char c = id;
  c<10 ? c+=48 : c+=55;
  return c;
}

double Cut_Data::Getscut
(std::vector<int> pl,std::vector<int> pr,int n,int k,int li)
{
  if (n==k) {
    size_t idl=0, idr=0;
    for (size_t i(0);i<pl.size();++i) if (pl[i]) idl|=(1<<pl[i]);
    for (size_t i(0);i<pr.size();++i) if (pr[i]) idr|=(1<<pr[i]);
    double ml(sqrt(Getscut(idl))), mr(sqrt(Getscut(idr)));
#ifdef DEBUG__Cut_Data
    msg_Debugging()<<"m_"<<ID(idl)<<" + m_"<<ID(idr)<<" = "
		   <<ml<<" + "<<mr<<" = "<<ml+mr<<"\n";
#endif
    return sqr(ml+mr);
  }
  msg_Indent();
  double sc(0.0);
  for (size_t i(li+1);i<pl.size();++i) {
    std::swap<int>(pl[i],pr[i]);
    sc=Max(sc,Getscut(pl,pr,n,k+1,i));
    std::swap<int>(pl[i],pr[i]);
  }
  return sc;
}

double Cut_Data::GetscutAmegic(std::string str)
{
  size_t id(0);
  int length = str.length();
  for (int i=0;i<length;i++) {
    char cur = str[i];
    if (cur<58) id|=(1<<(cur-48));
    else id|=(1<<(cur-55));
  }
  return Getscut(id);
}

double Cut_Data::Getscut(size_t str)
{
  map<size_t,double>::iterator it = m_smin_map.find(str);
  if (it!=m_smin_map.end())
    if (it->second>=0.) return it->second;

  std::vector<int> pr(ID(str));
  double sc = 0.;

  if (pr.size()==1) {
    m_smin_map[str] = sc = sqr(fl[pr.front()].SelMass());
    return sc;
  }

  if (pr.size()==2) {
    m_smin_map[str] = sc = scut[pr[0]][pr[1]];
    return sc;
  }

  for (int i=0;i<pr.size();i++) sc += Getscut(1<<pr[i]);
  sc *= 2.-(double)pr.size();
  for (int i=0;i<pr.size();i++) {
    for (int j=i+1;j<pr.size();j++) {
      sc += Getscut((1<<pr[i])|(1<<pr[j]));
    }
  }

  std::vector<int> pl(pr.size(),0);
  for (int i(1);i<=pr.size()/2;++i) sc=Max(sc,Getscut(pl,pr,i,0,-1));
  
  m_smin_map[str] = sc;
  return sc;
}

void Cut_Data::Setscut(size_t str,double d)
{
  m_smin_map[str]=d;
}
