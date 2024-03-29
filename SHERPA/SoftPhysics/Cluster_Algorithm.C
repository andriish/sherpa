#include "SHERPA/SoftPhysics/Cluster_Algorithm.H"
#include "PDF/Main/Cluster_Definitions_Base.H"
#include "ATOOLS/Math/Random.H"
#include "ATOOLS/Phys/Blob.H"
#include "ATOOLS/Phys/Particle.H"
#include "ATOOLS/Org/Message.H"
#include "ATOOLS/Org/Run_Parameter.H"
#include "ATOOLS/Org/Exception.H"

#include <limits>

using namespace SHERPA;
using namespace ATOOLS;

Cluster_Algorithm::Cluster_Algorithm():
  p_ampl(NULL), p_clus(NULL), p_jf(NULL),m_showerfac(1.) {
    m_histomap[std::string("startvspt")] = new Histogram(0,0.0,100.0,100);
    m_histomap[std::string("vetovspt")] = new Histogram(0,0.0,100.0,100);
    m_histomap[std::string("nstartvspt")] = new Histogram(0,0.0,100.0,100);
    m_histomap[std::string("nvetovspt")] = new Histogram(0,0.0,100.0,100);
}

Cluster_Algorithm::~Cluster_Algorithm()
{
  if (p_jf) delete p_jf;
  //if (p_ampl!=NULL) p_ampl->Delete();
  if (!m_histomap.empty()) {
    Histogram * histo;
    std::string name;
    for (std::map<std::string,Histogram *>::iterator 
	   hit=m_histomap.begin();hit!=m_histomap.end();hit++) {
      histo = hit->second;
      name  = std::string("Ladder_Analysis/")+hit->first+std::string(".dat");
      histo->Finalize();
      histo->Output(name);
      delete histo;
    }
    m_histomap.clear();
  }
}

int Cluster_Algorithm::ColorConnected(const ColorID &i,const ColorID &j) const
{
  return int(i.m_i==j.m_j && i.m_i!=0)+int(i.m_j==j.m_i && i.m_j!=0);
}

void Cluster_Algorithm::
ProjectOnSinglets(Blob * const blob,std::list<ParticleList *> & singlets) {
  //msg_Out()<<METHOD<<" for \n"<<(*blob)<<"\n";
  ParticleList outs, * sing;
  std::list<Particle * >::iterator piter;
  for (int i(0);i<blob->NOutP();++i) outs.push_back(blob->OutParticle(i));
  int col1,col2;
  bool add;
  while (!outs.empty()) {
    sing  = new std::list<Particle *>;
    if (outs.size()==0) break;
    col1  = col2  = 0;
    add   = false;
    msg_Tracking()<<"++++ Start 3 list at begin of out-particle list: "
	     <<outs.size()<<" particles left.\n";
    if (!add && !outs.empty()) {
      piter = outs.begin();
      do {
	msg_Tracking()<<"   Test "<<(*piter)->Flav()<<" "
		 <<"["<<(*piter)->GetFlow(1)<<", "
		 <<(*piter)->GetFlow(2)<<"], "
		 <<"add = "<<add;
	if (sing->empty() && 
	    (*piter)->GetFlow(1)!=0 && (*piter)->GetFlow(2)==0) {
	  col1 = (*piter)->GetFlow(1);
	  col2 = (*piter)->GetFlow(2);
	  sing->push_back((*piter));
	  msg_Tracking()<<" --> start singlet.\n";
	  outs.erase(piter);
	  if (!outs.empty()) piter = outs.begin();
	  add = true;
	}
	else if (col1!=0 && col2==0 && (*piter)->GetFlow(2)==col1) {
	  col1 = (*piter)->GetFlow(1);
	  sing->push_back((*piter));
	  msg_Tracking()<<" --> add to singlet.\n";
	  outs.erase(piter);
	  if (!outs.empty()) piter = outs.begin();
	  add = true;
	}
	else {
	  msg_Tracking()<<" --> ignore.\n";
	  add = false;
	  piter++;
	}
      } while (add && piter!=outs.end() && !outs.empty());
    }
    msg_Tracking()<<"++++ Start anti-3 list at begin of out-particle list: "
	     <<outs.size()<<" particles left, add = "<<add<<".\n";
    if (!add && !outs.empty()) {
      piter = outs.begin();
      do {
	msg_Tracking()<<"   Test "<<(*piter)->Flav()<<" "
		 <<"["<<(*piter)->GetFlow(1)<<", "
		 <<(*piter)->GetFlow(2)<<"], "
		 <<"add = "<<add;
	if (sing->empty() && 
	    (*piter)->GetFlow(1)==0 && (*piter)->GetFlow(2)!=0) {
	  col1 = (*piter)->GetFlow(1);
	  col2 = (*piter)->GetFlow(2);
	  sing->push_front((*piter));
	  msg_Tracking()<<" --> start singlet.\n";
	  outs.erase(piter);
	  if (!outs.empty()) piter = outs.begin();
	  add = true;
	}
	else if (col1==0 && col2!=0 && (*piter)->GetFlow(1)==col2) {
	  col2 = (*piter)->GetFlow(2);
	  sing->push_front((*piter));
	  msg_Tracking()<<" --> add to singlet.\n";
	  outs.erase(piter);
	  if (!outs.empty()) piter = outs.begin();
	  add = true;
	}
	else {
	  msg_Tracking()<<" --> ignore.\n";
	  add = false;
	  piter++;
	}
      } while (add && piter!=outs.end() && !outs.empty());
    }
    msg_Tracking()<<"++++ Start 8 list at begin of out-particle list: "
	     <<outs.size()<<" particles left, add = "<<add<<".\n";
    if (!add && !outs.empty()) {
      piter = outs.begin();
      do {
	msg_Tracking()<<"   Test "<<(*piter)->Flav()<<" "
		 <<"["<<(*piter)->GetFlow(1)<<", "
		 <<(*piter)->GetFlow(2)<<"], "
		 <<"add = "<<add;
	if (sing->empty() && 
	    (*piter)->GetFlow(1)!=0 && (*piter)->GetFlow(2)!=0) {
	  col1 = (*piter)->GetFlow(1);
	  col2 = (*piter)->GetFlow(2);
	  sing->push_front((*piter));
	  msg_Tracking()<<" --> start singlet.\n";
	  outs.erase(piter);
	  if (!outs.empty()) piter = outs.begin();
	  add = true;
	}
	else if (col1!=0 && col2!=0 && (*piter)->GetFlow(1)==col2) {
	  col2 = (*piter)->GetFlow(2);
	  sing->push_front((*piter));
	  msg_Tracking()<<" --> add to singlet.\n";
	  outs.erase(piter);
	  if (!outs.empty()) piter = outs.begin();
	  add = true;
	}
	else if (col1!=0 && col2!=0 && (*piter)->GetFlow(2)==col1) {
	  col1 = (*piter)->GetFlow(1);
	  sing->push_back((*piter));
	  msg_Tracking()<<" --> add to singlet.\n";
	  outs.erase(piter);
	  if (!outs.empty()) piter = outs.begin();
	  add = true;
	}
	else {
	  msg_Tracking()<<" --> ignore.\n";
	  add = false;
	  piter++;
	}
      } while (add && !(piter==outs.end() || outs.empty()));
    }
    if (sing->empty()) {
      delete sing;
      break;
    }
    else singlets.push_back(sing);
    //msg_Out()<<(*sing)<<"\n";
  }
}

double Cluster_Algorithm::
PTij2(const ATOOLS::Vec4D & pi,const ATOOLS::Vec4D & pj) const
{
  double pti2, ptj2;
  if (m_resc) {
    pti2  = PTi2(pi,m_rescvec);
    ptj2  = PTi2(pj,m_rescvec);
  }
  else {
    pti2  = pi.PPerp2();
    ptj2  = pj.PPerp2();    
  }
  double ptij2 = Min(pti2,ptj2)*(cosh(pi.Eta()-pj.Eta())-
				 cos(pi.Phi()-pj.Phi()));
  return Min(m_showerfac*pti2,ptij2);
}

double Cluster_Algorithm::
PTi2(const ATOOLS::Vec4D & pi,const ATOOLS::Vec4D & pbeam) const
{
  double t((pi+pbeam).Abs2());
  return t*Min(pi[0],pbeam[0])/Max(pi[0],pbeam[0]);
}

bool Cluster_Algorithm::Cluster(Blob *const blob)
{
  std::list<ParticleList * > singlets;
  ProjectOnSinglets(blob,singlets);

  double ymin(10000.),ymax(-10000.);
  int iymin(-1),iymax(-1),n(1);

  p_ampl=Cluster_Amplitude::New(NULL);
  Vec4D axis1(blob->GetParticle(0)->Momentum());
  Vec4D axis2(blob->GetParticle(1)->Momentum());
  for (int i(0);i<blob->NInP();++i) {
    Particle *const copy(blob->GetParticle(i));
    size_t id(1<<p_ampl->Legs().size());
    ColorID col(copy->GetFlow(1),copy->GetFlow(2));
    col=col.Conj();
    Flavour flav(copy->Flav().Bar());
    Vec4D mom(-copy->Momentum());
    p_ampl->CreateLeg(mom,flav,col,id);
    Cluster_Leg * leg(p_ampl->Legs().back());
    leg->SetNMax(blob->NOutP());
  }
  while (!singlets.empty()) {
    ParticleList * sing=singlets.front();
    while (!sing->empty()) {
      n++;
      Particle *const copy(sing->front());
      size_t id(1<<p_ampl->Legs().size());
      ColorID col(copy->GetFlow(1),copy->GetFlow(2));
      Flavour flav(copy->Flav());
      Vec4D mom(copy->Momentum());
      if(mom.Y()<ymin){
	ymin = mom.Y();
	iymin = n;
      }
      if(mom.Y()>ymax){
	ymax = mom.Y();
	iymax = n;
      }
      p_ampl->CreateLeg(mom,flav,col,id);
      Cluster_Leg * leg(p_ampl->Legs().back());
      leg->SetStat(0);
      leg->SetKT2(0,m_minkt2);
      leg->SetKT2(1,m_minkt2);
      leg->SetNMax(blob->NOutP()+3);
      sing->pop_front();
    }
    delete sing;
    singlets.pop_front();
  }

  ClusterLeg_Vector legs(p_ampl->Legs());
  Cluster_Leg * split, * spect;

  double kt2max, kt2min, kt2FS, sFS, ysplit, totmax(m_minkt2);
  double magicfac(0.3),ybar,deltay;
  m_rescvec = legs[0]->Mom()+legs[1]->Mom();
  double shat(m_rescvec.Abs2());
  size_t nlegs(legs.size());


  Vec4D pbeam0(-legs[0]->Mom()),  pbeam1(-legs[1]->Mom());
  ColorID colbeam0(legs[0]->Col()), colbeam1(legs[1]->Col());
  for (size_t i=2;i<nlegs;i++) {
    split   = legs[i];
    ysplit  = dabs(split->Mom().Y());
    kt2max  = Max(m_tmax,m_minkt2);
    for (size_t j=nlegs;j>2;j--) {
      if (i==j-1) continue;
      spect = legs[j-1];
      int nconn(ColorConnected(split->Col(),spect->Col()));
      if (nconn==0) continue;
      kt2FS = PTij2(split->Mom(),spect->Mom());
      if (kt2FS<kt2min) kt2min = kt2FS;
      if (kt2FS>kt2max) kt2max = kt2FS;
    }
    if (kt2max>totmax) totmax = kt2max;
    split->SetKT2(0,kt2max);
    split->SetKT2(1,kt2max);
    
    m_histomap[std::string("startvspt")]->Insert(split->Mom().PPerp(),kt2max);  
    m_histomap[std::string("vetovspt")]->Insert(split->Mom().PPerp(),kt2min);  
    m_histomap[std::string("nstartvspt")]->Insert(split->Mom().PPerp());  
  }
  p_ampl->SetNIn(blob->NInP());
  p_ampl->SetOrderEW(0);
  p_ampl->SetOrderQCD(blob->NOutP());

  p_ampl->SetMS(this);
  p_ampl->SetKT2(totmax);
  p_ampl->SetMuQ2(totmax);
  p_ampl->SetMuR2(totmax);
  p_ampl->SetMuF2(totmax);
  p_ampl->SetMu2(totmax);

  for (size_t i(0);i<p_ampl->NIn();++i) {
    Cluster_Leg *li(p_ampl->Leg(i));
    li->SetKT2(0,0.0);
    li->SetKT2(1,0.0);
    for (size_t j(p_ampl->NIn());j<p_ampl->Legs().size();++j) {
      Cluster_Leg *lj(p_ampl->Leg(j));
      if (li->Col().m_j==lj->Col().m_i) lj->SetKT2(0,0.0);
      if (li->Col().m_i==lj->Col().m_j) lj->SetKT2(1,0.0);
    }
  }

  return true;
}

double Cluster_Algorithm::Mass(const Flavour &fl) const
{
  return fl.Mass(true);
}

