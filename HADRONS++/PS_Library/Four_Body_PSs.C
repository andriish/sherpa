#include "HADRONS++/PS_Library/Four_Body_PSs.H"
#include "PHASIC++/Channels/Channel_Elements.H"
#include "ATOOLS/Org/Message.H"
#include "ATOOLS/Org/MyStrStream.H"
#include "ATOOLS/Math/Random.H"
#include "ATOOLS/Math/Poincare.H"


using namespace HADRONS; 
using namespace PHASIC; 
using namespace ATOOLS; 
using namespace std; 

TwoResonances::TwoResonances(const Flavour * fl,
			     SimpleResonanceFlavour prop1,const int _k,
			     SimpleResonanceFlavour prop2,const int _i,
			     const int _j ) : 
  Single_Channel(1,4,fl), 
  m_P(Vec4D(fl[0].HadMass(),0.,0.,0.)), 
  m_i (_i), m_j (_j), m_k (_k),
  m_prop1 (prop1), m_prop2 (prop2)
{
  m_name = string("TwoResonances_")
    + prop1.Name() + string("_")
    + ToString(m_k)
    + string("_") + prop2.Name() + string("_")
    + ToString(m_i)+ToString(m_j);
  // generate channel name
  p_fl = new Flavour[5];
  for (short int i=0;i<m_nin+m_nout;i++) {
    p_fl[i] = fl[i];
    p_ms[i] = sqr(fl[i].HadMass());
  }
  // set masses^2
  for (int i=1;i<5;i++) {
    if (m_i!=i && m_j!=i && m_k!=i) { m_dir=i; break; }
  }				// get the one with no resonance
  msg_Tracking()<<"Init TwoResonances("<<m_name<<") : "<<endl
		<<"     "<<fl[0]<<" -> "
		<<fl[m_dir]<<" "<<fl[m_k]<<" "<<fl[m_i]<<" "<<fl[m_j]
		<<", "<<endl
		<<"     "<<p_ms[0]<<" -> "
		<<p_ms[m_dir]<<" "<<p_ms[m_k]<<" "<<p_ms[m_i]<<" "<<p_ms[m_j]<<endl
		<<"  => "<<p_fl[0]<<" -> "<<p_fl[m_dir]<<" "<<m_prop1.Name()
		<<endl
		<<"     "<<p_fl[0]<<" -> "<<p_fl[m_dir]<<" "<<p_fl[m_k]<<" "
		<<m_prop2.Name()<<endl
		<<"     "<<p_fl[0]<<" -> "<<p_fl[m_dir]<<" "<<p_fl[m_k]<<" "
		<<p_fl[m_i]<<" "<<p_fl[m_j]<<endl;
  msg_Debugging()
    <<"  with axial @ "<<m_prop1.Mass()<<" ("<<m_prop1.Width()<<")"<<endl
    <<"      vector @ "<<m_prop2.Mass()<<" ("<<m_prop2.Width()<<")"<<endl;
  
  m_rannum = 8;
  p_rans = new double[m_rannum];
  p_vegas = new Vegas(m_rannum,100,m_name);
  p_info = new Integration_Info();
  m_kI_123_4.Assign(std::string("I_123_4"),2,0,p_info);
  m_kI_12_3.Assign(std::string("I_12_3"),2,0,p_info);
  m_kI_1_2.Assign(std::string("I_1_2"),2,0,p_info);
}

TwoResonances::~TwoResonances() {
  if (p_fl!=NULL) delete [] p_fl; p_fl = NULL;
  if (p_vegas) delete p_vegas; p_vegas=NULL;
  if (p_info) delete p_info; p_info=NULL;
}

void TwoResonances::GeneratePoint(ATOOLS::Vec4D * p,PHASIC::Cut_Data * cuts,
				  double * _ran)
{
  double *ran = p_vegas->GeneratePoint(_ran);
  for(int i=0;i<m_rannum;i++) p_rans[i]=ran[i];
  Vec4D  p1234 = p[0];
  // kinematic variables
  double s1_min = p_ms[m_i];
  double s2_min = p_ms[m_j];
  double s3_min = p_ms[m_k];
  double s12_min = sqr( sqrt(s1_min) + sqrt(s2_min) );
  double s123_min = sqr( sqrt(s12_min) + sqrt(s3_min) );
  double s1234 = dabs(p1234.Abs2());
  double s1 = p_ms[m_i];
  double s2 = p_ms[m_j];
  double s3 = p_ms[m_k];
  double s4 = p_ms[m_dir];
  double s123_max = sqr(sqrt(s1234)-sqrt(s4));
  Vec4D  p123;
  double s123;
  s123 = CE.MassivePropMomenta(m_prop1.Mass(),m_prop1.Width(),
			       s123_min,s123_max,ran[0]);
  CE.Isotropic2Momenta(p1234,s123,s4,p123,p[m_dir],ran[1],ran[2]);
  double s12_max = sqr(sqrt(s123)-sqrt(s3));
  Vec4D  p12;
  double s12;
  s12 = CE.MassivePropMomenta(m_prop2.Mass(),m_prop2.Width(),
			      s12_min,s12_max,ran[3]);
  CE.Isotropic2Momenta(p123,s12,s3,p12,p[m_k],ran[4],ran[5]);
  CE.Isotropic2Momenta(p12,s1,s2,p[m_i],p[m_j],ran[6],ran[7]);
}

void TwoResonances::GenerateWeight(ATOOLS::Vec4D * p,PHASIC::Cut_Data * cuts)
{
  double wt = 1.;
  Vec4D  p1234 = p[0];
  // kinematic variables
  double s1_min = p_ms[m_i];
  double s2_min = p_ms[m_j];
  double s3_min = p_ms[m_k];
  double s12_min = sqr( sqrt(s1_min) + sqrt(s2_min) );
  double s123_min = sqr( sqrt(s12_min) + sqrt(s3_min) );
  double s1234 = dabs(p1234.Abs2());
  double s3 = p_ms[m_k];
  double s4 = p_ms[m_dir];
  double s123_max = sqr(sqrt(s1234)-sqrt(s4));
  Vec4D  p123 = p[m_i]+p[m_j]+p[m_k];
  double s123 = dabs(p123.Abs2());
  wt *= CE.MassivePropWeight(m_prop1.Mass(),m_prop1.Width(),
			     s123_min,s123_max,s123,p_rans[0]);
  m_kI_123_4<<CE.Isotropic2Weight(p[0]-p[m_dir],p[m_dir],m_kI_123_4[0],m_kI_123_4[1]);
  wt *= m_kI_123_4.Weight();

  p_rans[1]= m_kI_123_4[0];
  p_rans[2]= m_kI_123_4[1];
  double s12_max = sqr(sqrt(s123)-sqrt(s3));
  Vec4D  p12 = p[m_i]+p[m_j];
  double s12 = dabs(p12.Abs2());
  wt *= CE.MassivePropWeight(m_prop2.Mass(),m_prop2.Width(),
			     s12_min,s12_max,s12,p_rans[3]);
  m_kI_12_3<<CE.Isotropic2Weight(p12,p[m_k],m_kI_12_3[0],m_kI_12_3[1]);
  wt *= m_kI_12_3.Weight();
 
  p_rans[4]= m_kI_12_3[0];
  p_rans[5]= m_kI_12_3[1];
  m_kI_1_2<<CE.Isotropic2Weight(p[m_i],p[m_j],m_kI_1_2[0],m_kI_1_2[1]);
  wt *= m_kI_1_2.Weight();
 
  p_rans[6]= m_kI_1_2[0];
  p_rans[7]= m_kI_1_2[1];
  double vw = p_vegas->GenerateWeight(p_rans);
  if (wt!=0.) wt = vw/wt/pow(2.*M_PI,4*3.-4.);

  m_weight = wt;
}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


IsotropicSpectator::IsotropicSpectator(const ATOOLS::Flavour * fl, 
				       int nOut, int spectator,
                                       const ATOOLS::Mass_Selector* ms) :
  Single_Channel(1,nOut,fl), m_spectator(spectator),
  m_spectator_mass(ms->Mass(fl[spectator])), m_residual_mass(0.)
{
  Flavour *isotropicflavs = new Flavour[m_nout];
  isotropicflavs[0] = Flavour(kf_none);
  int j=1;
  for (short int i=1;i<1+m_nout;i++) {
    if(i!=m_spectator) {
      msg_Debugging()<<"   PS: add decay product: "<<fl[i]
		     <<" from i = "<<i<<" ("<<m_nout<<").\n";
      isotropicflavs[j] = fl[i];
      m_residual_mass  += ms->Mass(fl[i]);
      j++;
    }
    else 
      msg_Debugging()<<"   PS: add spectator: "<<fl[i]
		     <<" from i = "<<i<<" ("<<m_nout<<").\n";
  }
  double mass_saved = ms->Mass(isotropicflavs[0]);
  m_decayer_mass = ms->Mass(fl[0])-ms->Mass(fl[spectator]);
  isotropicflavs[0].SetMass(m_decayer_mass);
  m_rambo = new Rambo(1,m_nout-1,isotropicflavs,ms);
  isotropicflavs[0].SetMass(mass_saved);
  msg_Debugging()<<"   PS: m_decayermass = "<<m_decayer_mass
		 <<" from "<<mass_saved
		 <<" and "<<ms->Mass(fl[spectator])<<", m_nout = "<<m_nout<<"\n";
  delete[] isotropicflavs;
}

void IsotropicSpectator::GeneratePoint(ATOOLS::Vec4D * p,
				       PHASIC::Cut_Data * cuts,double * _ran)
{
  // generate the momentum of the decayer and spectator quark
  // according to PSpat() = gauss(mean=lambda_qcd, sigma=lambda_qcd/mQ)
  // and costheta, phi uniform
  double lambda_qcd=0.2, pspat, critE2(sqr(m_residual_mass));
  do {
    do {
      pspat = lambda_qcd+lambda_qcd/m_decayer_mass*ran->GetGaussian();
      if (pspat < 1e-6)
        pspat = lambda_qcd+lambda_qcd/m_decayer_mass*ran->GetGaussian();
    } while(pspat<1e-6);
  } while (sqr(p[0][0]-sqrt(sqr(m_spectator_mass)+sqr(pspat))) - sqr(pspat) < critE2);

  double costheta = ran->Get()*2.0-1.0;
  double sintheta = sin(acos(costheta));
  double phi      = ran->Get()*2.0*M_PI;
  double px       = pspat*sintheta*cos(phi);
  double py       = pspat*sintheta*sin(phi);
  double pz       = pspat*costheta;
  Vec4D spectator_mom = Vec4D(sqrt(sqr(m_spectator_mass)+sqr(pspat)), 
			      px, py, pz);
  Vec4D decayer_mom = Vec4D(p[0][0]-spectator_mom[0], -px, -py, -pz);
  
  Vec4D *isotropicmoms = new Vec4D[m_nout+1];
  Poincare boost(decayer_mom);
  isotropicmoms[0] = boost*decayer_mom;
  m_rambo->GeneratePoint(isotropicmoms);
  
  boost.Invert();
  int j=1;
  for (short int i=1;i<1+m_nout;i++) {
    if(i==m_spectator) {//at the moment spectator always at 1.
      p[i] = spectator_mom;
    }
    else {
      p[i] = boost*isotropicmoms[j];
      j++;
    }
  }
  delete[] isotropicmoms;
}


void IsotropicSpectator::GenerateWeight(ATOOLS::Vec4D * p,
					PHASIC::Cut_Data * cuts)
{
  Vec4D *isotropicmoms = new Vec4D[m_nout+1];
  int j=1;
  for (short int i=1;i<1+m_nout;i++) {
    if(i!=m_spectator) {
      isotropicmoms[j] = p[i];
      j++;
    }
  }
  isotropicmoms[0] = isotropicmoms[1];
  for (short int i=2;i<m_nout;i++) isotropicmoms[0] += isotropicmoms[i];
  Poincare boost(isotropicmoms[0]);
  for(int i=0;i<m_nout;i++) {
    boost.Boost(isotropicmoms[i]);
  }
  m_rambo->GenerateWeight(isotropicmoms);
  SetWeight(m_rambo->Weight());

  if (IsNan(m_rambo->Weight())) {
    msg_Error()<<"Rambo weight gives a nan!\n"
	       <<"   boost vector: "<<isotropicmoms[0]<<",\n";
    for(int i=0;i<m_nout;i++) {
      msg_Error()<<"   "<<isotropicmoms[i]<<" "
		 <<"("<<sqrt(Max(0.,isotropicmoms[i].Abs2()))<<") vs. "
		 <<p[i]<<" ("<<sqrt(Max(0.,p[i].Abs2()))<<").\n";
    }
  }
  delete[] isotropicmoms;
}
