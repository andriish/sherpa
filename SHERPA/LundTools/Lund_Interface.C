#include "SHERPA/LundTools/Lund_Interface.H"

#include "SHERPA/LundTools/Lund_Wrapper.H"
#include "ATOOLS/Phys/Particle.H"
#include "ATOOLS/Phys/Blob.H"
#include "ATOOLS/Phys/Blob_List.H"
#include "ATOOLS/Org/Run_Parameter.H"
#include "ATOOLS/Math/Random.H"
#include "ATOOLS/Org/Message.H"
#include "ATOOLS/Org/Exception.H"
#include "MODEL/Main/Running_AlphaS.H"
#include "ATOOLS/Org/MyStrStream.H"
#include <list>
#include <cassert>
#include "ATOOLS/Org/Message.H"
#include "ATOOLS/Org/Scoped_Settings.H"
#include "SHERPA/Tools/HepEvt_Interface.H"
#include "ATOOLS/Org/CXXFLAGS.H"

using namespace SHERPA;
using namespace ATOOLS;
using namespace std;

bool Lund_Interface::s_exportas=false;

size_t Lund_Interface::s_errors=0;
size_t Lund_Interface::s_maxerrors=0;

int * Lund_Interface::s_saved_mrpy=new int[6];
double * Lund_Interface::s_saved_rrpy=new double[100];

ATOOLS::Blob_List *Lund_Interface::s_bloblist=NULL; 

Lund_Interface::Lund_Interface(std::string):
  m_maxtrials(2),
  p_hepevt(NULL), 
  m_compress(true),m_writeout(false),
  p_phep(new double[5*HEPEVT_CB_SIZE]),
  p_vhep(new double[4*HEPEVT_CB_SIZE]),
  p_jmohep(new int[2*HEPEVT_CB_SIZE]),
  p_jdahep(new int[2*HEPEVT_CB_SIZE])
{
  RegisterDefaults();
  exh->AddTerminatorObject(this);
  // determine baseline settings from beams
  // NOTE that these are no default settings, because they get not replaced if
  // we set an entry or entries in the run card (or command line), they only
  // get amended.
  string beam[2], frame("CMS");
  Flavour flav[2];
  for (size_t i=0;i<2;++i) flav[i]=rpa->gen.Bunch(i);
  if (flav[0].Kfcode()==kf_e && flav[1].Kfcode()==kf_p_plus) {
    if (flav[0].IsAnti()) beam[0]="e+"; else beam[0]="e-";
    if (flav[1].IsAnti()) beam[1]="p-"; else beam[1]="p+";
    pysubs.msub[9]=1;    
  }
  else if (flav[0].Kfcode()==kf_p_plus && flav[1].Kfcode()==kf_e) {
    if (flav[0].IsAnti()) beam[0]="p-"; else beam[0]="p+";
    if (flav[1].IsAnti()) beam[1]="e+"; else beam[1]="e-";
    pysubs.msub[9]=1;    
  }
  else if (flav[0]==Flavour(kf_e) && flav[1]==Flavour(kf_photon)) {
    if (flav[0].IsAnti()) beam[0]="e+"; else beam[0]="e-";
    beam[1]="gamma";
    pysubs.msub[33]=1;    
  }
  else if (flav[0]==Flavour(kf_photon) && flav[1]==Flavour(kf_e)) {
    beam[0]="gamma";
    if (flav[1].IsAnti()) beam[1]="e+"; else beam[1]="e-";
    pysubs.msub[33]=1;    
  }
  else if (flav[0]==Flavour(kf_photon) && flav[1]==Flavour(kf_photon)) {
    for (size_t i=0;i<2;++i) beam[i]="gamma";
    pysubs.msub[57]=1;    
  }
  else if (flav[0].Kfcode()==kf_e && flav[1].Kfcode()==kf_e) {
    for (size_t i=0;i<2;++i) if (flav[i].IsAnti()) beam[i]="e+"; else beam[i]="e-";
    pysubs.msub[0]=1;    
    pypars.mstp[47]=1;
    pydat1.mstj[100]=5;
  }
  else {
    for (size_t i=0;i<2;++i) if (flav[i].IsAnti()) beam[i]="p-"; else beam[i]="p+";
    pysubs.msub[0]=1;    
    pypars.mstp[47]=1;
    pydat1.mstj[100]=5;
  }
  s_maxerrors=rpa->gen.NumberOfEvents();
  vector<vector<int> > help;
  pysubs.msel=0;

  // default hadronisation parameters
  pydat1.parj[21-1]=0.33;
  pydat1.parj[41-1]=0.3;
  pydat1.parj[42-1]=0.6;
  pydat1.parj[46-1]=0.875;

  Settings& s = Settings::GetMainSettings();
  ReadSettings(s, "MSUB", pysubs.msub);
  ReadSettings(s, "CKIN", pysubs.ckin);
  ReadSettings(s, "MSTJ", pydat1.mstj);
  ReadSettings(s, "MSTP", pypars.mstp);
  ReadSettings(s, "MSTU", pydat1.mstu);
  ReadSettings(s, "PARP", pypars.parp);
  ReadSettings(s, "PARJ", pydat1.parj);
  ReadSettings(s, "PARU", pydat1.paru);

  for (const auto& row : s["KFIN"].GetMatrix<int>()) {
    if (row.size() != 3)
      THROW(inconsistent_option, "KFIN setting entries must have exactly three values.");
    if (row[0] < 1)
      THROW(inconsistent_option, "The first value of a KFIN entry must be greater than 0.");
    if (row[1] < -40)
      THROW(inconsistent_option, "The second value of a KFIN entry must be greater than -40.");
    pysubs.kfin[row[1] + 40][row[0] - 1] = row[2];
  }

  for (const auto& row : s["MDME"].GetMatrix<int>()) {
    if (row.size() != 3)
      THROW(inconsistent_option, "MDME setting entries must have exactly three values.");
    if (row[0] < 1)
      THROW(inconsistent_option, "The first value of an MDME entry must be greater than 0.");
    if (row[1] < 1 || row[1] > 2)
      THROW(inconsistent_option, "The second value of an MDME entry must be either 1 or 2.");
    pydat3.mdme[row[1] - 1][row[0] - 1] = row[2];
  }


  for (const auto& row : s["MDCYKF"].GetMatrix<int>()) {
    if (row.size() != 3)
      THROW(inconsistent_option, "MDCYKF setting entries must have exactly three values.");
    if (row[0] < 1)
      THROW(inconsistent_option, "The first value of an MDCYKF entry must be greater than 0.");
    if (row[1] < 1 || row[1] > 3)
      THROW(inconsistent_option, "The second value for an MDCYKF setting entry must be either 1, 2 or 3.");
    msg_Tracking()<<"Lund_Interface::Lund_Interface(..): "
      <<"Set MDCY("<<pycomp(row[0])<<","<<row[1]
      <<") ( from KF code "<<row[0]<<" ) to "<<row[2]<<endl;
    pydat3.mdcy[row[1] - 1][pycomp(row[0]) - 1] = row[2];
  }

  // the next lines replace the apyinit_ call
  hepevt.nhep=100;
  for (int i=pydat3.mdcy[2-1][23-1];i<pydat3.mdcy[2-1][23-1]+pydat3.mdcy[3-1][23-1];++i) {
    if (abs(pydat3.kfdp[1-1][i-1])>=2) pydat3.mdme[1-1][i-1]=Min(0,pydat3.mdme[1-1][i-1]);
  }
  pyinit(frame.c_str(),beam[0].c_str(),beam[1].c_str(),100.0);
  // replacement ends here
  if (msg_LevelIsDebugging()) ListLundParameters();
  pylist(0);
}

void Lund_Interface::RegisterDefaults() const
{
  Settings& s = Settings::GetMainSettings();
  // empty matrix defaults
  s.DeclareMatrixSettingsWithEmptyDefault({
      "MSUB", "MSTP", "MSTJ", "KFIN", "CKIN", "MSTU",
      "PARP", "PARJ", "PARU", "MDME", "MDCYKF" });
}

template <typename T>
void Lund_Interface::ReadSettings(Settings& settings,
                                  const std::string& key,
                                  T *target) const
{
  for (const auto& row : settings[key].GetMatrix<double>()) {
    if (row.size() != 2)
      THROW(inconsistent_option, key + " setting entries must have exactly two values.");
    if (row[0] < 1)
      THROW(inconsistent_option, "The first value of a " + key + " entry must be greater than 0.");
    target[(size_t)row[0] - 1] = row[1];
  }
}

bool Lund_Interface::IsAllowedDecay(kf_code can)
{
  int kc=pycomp(SherpaToIdhep(Flavour(can)));
  if (kc>0 && kc<501 && pydat3.mdcy[1-1][kc-1]==1) return true;
  return false;
}

void Lund_Interface::SwitchOffDecays(kf_code kfc)
{
  int kc = pycomp(SherpaToIdhep(Flavour(kfc)));
  if(kc>500) return;
  pydat3.mdcy[1-1][kc-1]=0;
}

void Lund_Interface::AdjustProperties(Flavour flav)
{
  int kc = pycomp(SherpaToIdhep(flav));
  if(kc>500) return;
  // adjust mass
  double pythiamass = pydat2.pmas[1-1][kc-1];
  double sherpamass = flav.HadMass();
  flav.SetMass(pythiamass);
  if( !(abs(sherpamass-pythiamass)/sherpamass < 1.e-2) ) {
    msg_Info()<<METHOD<<" Adjusted mass of "<<flav<<" ("<<flav.Kfcode()
        <<") from "<<sherpamass<<" to "<<pythiamass<<" to allow Pythia decays."<<endl;
  }
}

void Lund_Interface::SwitchOffMassSmearing()
{
  pydat1.mstj[24-1]=0;
}

double Lund_Interface::GenerateMass(Flavour flav, double min, double max)
{
  int kc = pycomp(SherpaToIdhep(flav))-1;
  double peak = pydat2.pmas[1-1][kc];
  double w_cut = pydat2.pmas[3-1][kc];
  if(w_cut == 0.0) {
    if(peak<min-Accu() || peak>max+Accu()) return -1.0;
    else return peak;
  }
  else {
    double finalmin = min>(peak-w_cut)? min : peak-w_cut;
    double finalmax = max<(peak+w_cut)? max : peak+w_cut;
    if(finalmin>finalmax) return -1.0;
    else return flav.RelBWMass(finalmin, finalmax, this->Mass(flav));
  }
}

Lund_Interface::~Lund_Interface()
{
  exh->RemoveTerminatorObject(this);
  NextFile(false);
  if (p_hepevt) { 
    p_hepevt->SetNhep(0);
    p_hepevt->SetIsthep(NULL);
    p_hepevt->SetIdhep(NULL);
    p_hepevt->SetJmohep(NULL);
    p_hepevt->SetJdahep(NULL);
    p_hepevt->SetPhep(NULL);
    p_hepevt->SetVhep(NULL);
    delete p_hepevt; p_hepevt = NULL; 
  }
  if (p_jmohep) { delete [] p_jmohep; p_jmohep = NULL; }
  if (p_jdahep) { delete [] p_jdahep; p_jdahep = NULL; }
  if (p_phep)   { delete [] p_phep;   p_phep   = NULL; }
  if (p_vhep)   { delete [] p_vhep;   p_vhep   = NULL; }
}

Return_Value::code Lund_Interface::Hadronize(Blob_List *bloblist) 
{
  s_bloblist=bloblist;
  int nhep(0);
  for (Blob_List::iterator blit=bloblist->begin();blit!=bloblist->end();++blit) {
    if ((*blit)->Has(blob_status::needs_hadronization)) {
      if ((*blit)->Type()!=btp::Fragmentation) {
	msg_Error()<<"Error in "<<METHOD<<":"<<std::endl
		   <<"   Blob with status=needs_hadronizazion and "<<std::endl
		   <<"   type different from 'Fragmentation' illegally found."<<std::endl
		   <<"   Will retry the event and hope for the best."<<std::endl;
	return Return_Value::Retry_Event;
      }
      (*blit)->SetTypeSpec("Pythia_v6.214");

      Return_Value::code result=Return_Value::Nothing;
      for(size_t trials=0; trials<m_maxtrials; ++trials) {
	nhep = PrepareFragmentationBlob(*blit);
	int errs=pydat1.mstu[23-1];
	result=StringFragmentation(*blit,bloblist,nhep);
	if(result==Return_Value::Success) break;
	assert(result==Return_Value::Retry_Phase);
	if(trials+1<m_maxtrials) {
	  msg_Tracking()<<"Error in "<<METHOD<<"."<<endl
		     <<"   Hadronization failed. Retry it."<<endl;
	  pydat1.mstu[23-1]=errs;   //New try, set back error sum.
	  continue;
	}
	msg_Tracking()<<"Error in "<<METHOD<<"."<<endl
		   <<"   Hadronization failed. Retry the event."<<endl;
	if (!rpa->gen.Beam1().IsHadron() ||
	    !rpa->gen.Beam2().IsHadron()) {
	  msg_Tracking()<<METHOD<<"(): Non-hh collision. Request new event instead.\n";
	  return Return_Value::New_Event;
	}
	return Return_Value::Retry_Event;
      }
      //Up to this point fragmentation blob remained unchanged.
      //Therefore, could be used as the safe initial state.
      FillFragmentationBlob(*blit);
    }
  }
  return Return_Value::Success;
}

Return_Value::code Lund_Interface::PerformDecay(Blob * blob)
{
  if (blob->NInP()!=1 ||
      blob->InParticle(0)->Status()!=part_status::active)
  {
    msg_Error()<<METHOD<<" returns Error."<<endl;
    msg_Error()<<" blob->Status()="<<blob->Status()<<endl;
    msg_Error()<<" blob->NInP()="<<blob->NInP()<<endl;
    msg_Error()<<" part->Status()="<<blob->InParticle(0)->Status()<<endl;
    return Return_Value::Error;
  }

  Particle * part = blob->InParticle(0);
  Flavour fl = part->Flav();
  int kc = pycomp(SherpaToIdhep(fl))-1;
  double peak = pydat2.pmas[1-1][kc];
  double w_cut = pydat2.pmas[3-1][kc];
  if( part->FinalMass()+Accu() < peak-w_cut || part->FinalMass()-Accu() > peak+w_cut) {
    return Return_Value::Retry_Method;
  }

  int nhep(0);
  int idhep = SherpaToIdhep(fl);
  hepevt.idhep[nhep] = idhep;
  for (short int j=1;j<4;++j) hepevt.phep[nhep][j-1]=part->Momentum()[j];
  hepevt.phep[nhep][3] = part->Momentum()[0];
  hepevt.phep[nhep][4] = part->FinalMass();
  for (short int j=0;j<4;++j) hepevt.vhep[nhep][j]=0.0;
  hepevt.isthep[nhep]=1;
  hepevt.jmohep[nhep][0]=0;
  hepevt.jmohep[nhep][1]=0;
  hepevt.jdahep[nhep][0]=0;
  hepevt.jdahep[nhep][1]=0;
  nhep++;
  
  hepevt.nevhep=0;
  hepevt.nhep=nhep;
  pyhepc(2);
  pydat1.mstu[70-1]=1;
  pydat1.mstu[71-1]=hepevt.nhep;
  int ip(1);
  pydecy(ip);
  if (pydat1.mstu[24-1]!=0) {
    msg_Tracking()<<"ERROR in "<<METHOD<<" : "<<std::endl
	       <<"   PYDECY call results in error code : "<<pydat1.mstu[24-1]<<std::endl
                  <<"   for decay of "<<fl<<" ("<<(long int) fl<<" -> "<<idhep<<")"<<std::endl;
    if (pydat1.mstu[23-1]<int(rpa->gen.NumberOfGeneratedEvents()/100) ||
	rpa->gen.NumberOfGeneratedEvents()<200) {
      msg_Tracking()<<"   Up to now: "<<pydat1.mstu[23-1]<<" errors, try new event."<<std::endl;
      return Return_Value::Retry_Method;
    }
    msg_Tracking()<<"   Up to now: "<<pydat1.mstu[23-1]<<" errors, abort the run."<<std::endl;
  }
  part->SetStatus(part_status::decayed);
  pyhepc(1);
  FillOutgoingParticlesInBlob(blob);
  return Return_Value::Success;
} 

int Lund_Interface::IsConnectedToRemnant(Particle *p,int anti)
{
  DEBUG_FUNC("("<<p->GetFlow(1)<<","<<p->GetFlow(2)<<") "<<anti);
  msg_Debugging()<<*p<<"\n";
  Blob_List remnants(s_bloblist->Find(btp::Beam));
  for (int i(0);i<remnants.size();++i) {
    if (!remnants[i]->InParticle(0)->Flav().IsHadron()) continue;
    if (remnants[i]==p->ProductionBlob()) continue;
    msg_Debugging()<<*remnants[i];
    for (int j(1);j<remnants[i]->NOutP();++j) {
      if (remnants[i]->OutParticle(j)->GetFlow(3-anti)==
	  p->GetFlow(anti)) {
	msg_Debugging()<<"connected to "<<j<<"\n";
	return 1;
      }
    }
  }
  msg_Debugging()<<"not connected\n";
  return 0;
}

int Lund_Interface::PrepareFragmentationBlob(Blob * blob) 
{
  int nhep = 0;
  hepevt.idhep[nhep]=SherpaToIdhep(Flavour(kf_photon));
  for (short int j=1;j<4;++j) hepevt.phep[nhep][j-1]=blob->CMS()[j];
  hepevt.phep[nhep][3]=blob->CMS()[0];
  double pabs=(blob->CMS()).Abs2();
  if (pabs<0) hepevt.phep[nhep][4]=0.0;
  else hepevt.phep[nhep][4]=sqrt(pabs);
  for (short int j=0;j<4;++j) hepevt.vhep[nhep][j]=0.0;
  hepevt.isthep[nhep]=1;
  hepevt.jmohep[nhep][0]=0;
  hepevt.jmohep[nhep][1]=0;
  hepevt.jdahep[nhep][0]=0;
  hepevt.jdahep[nhep][1]=0;
  
  // gluon rings
  int gluonring(1);
  for (int i(0);i<blob->NInP();++i)
    if (blob->InParticle(i)->Flav().StrongCharge()==3) {
      gluonring=false;
      break;
    }
  if (gluonring) {
    Particle * part = blob->InParticle(0);
    Flavour flav = Flavour(kf_d);
    if (ran->Get()<0.5) flav = Flavour(kf_u);
    Particle *help1(new Particle(-1,flav,0.5*part->Momentum()));
    Particle *help2(new Particle(-1,flav.Bar(),0.5*part->Momentum()));
    help1->SetStatus(part_status::active);
    help2->SetStatus(part_status::active);
    AddPartonToString(help1,nhep);
    for (int i(1);i<blob->NInP();++i)
      AddPartonToString(blob->InParticle(i),nhep);
    AddPartonToString(help2,nhep);
    delete help2;
    delete help1;
    return nhep;
  }
  // gluons connected to remnant
  for (int i(0);i<blob->NInP();++i) {
    Particle * part = blob->InParticle(i);
    int c1(IsConnectedToRemnant(part,1));
    int c2(IsConnectedToRemnant(part,2));
    if (part->GetFlow(1) && part->GetFlow(2) && (c1^c2)) {
      Flavour flav = Flavour(kf_d);
      if (ran->Get()<0.5) flav = Flavour(kf_u);
      double x(ran->Get()), m(flav.HadMass());
      if (part->Momentum()[0]<m) AddPartonToString(part,nhep);
      else {
	if (c1 && !c2) x=pow(m/part->Momentum()[0],ran->Get());
	if (!c1 && c2) x=1.0-pow(m/part->Momentum()[0],ran->Get());
	Particle *help1(new Particle(-1,flav,x*part->Momentum()));
	Particle *help2(new Particle(-1,flav.Bar(),(1.0-x)*part->Momentum()));
	help1->SetStatus(part_status::active);
	help2->SetStatus(part_status::active);
	AddPartonToString(help2,nhep);
	AddPartonToString(help1,nhep);
	delete help2;
	delete help1;
      }
    }
    else {
      AddPartonToString(part,nhep);
    }
  }
  return nhep;
}


Return_Value::code Lund_Interface::StringFragmentation(Blob *blob,Blob_List *bloblist,int nhep) 
{
  hepevt.nevhep=0;
  hepevt.nhep=nhep;
  pyhepc(2);
  pydat1.mstu[70-1]=1;
  pydat1.mstu[71-1]=hepevt.nhep;
  int ip(1);
  pyprep(ip);
  pystrf(ip);
  pyhepc(1);
  bool goon(true), flag(false);
  if(hepevt.nhep<=nhep) { goon=false; flag=true;}
  while(goon) {
    goon=false;
    for(int i=0; i<hepevt.nhep; ++i) {
      ATOOLS::Flavour flav(IdhepToSherpa(hepevt.idhep[i]));
      if(hepevt.isthep[i]==1 && (flav.Strong()||flav.IsDiQuark())) {
	goon=true;
	int ipp(i+1);
	int save(hepevt.nhep);
	pystrf(ipp);
	pyhepc(1);
	if(hepevt.nhep<=save) { goon=false; flag=true;}
	break;
      }
    }
  }
  if(pydat1.mstu[24-1]!=0) {
    Vec4D cms(0.,0.,0.,0.);
    for (int i=0;i<blob->NInP();i++) cms+=blob->InParticle(i)->Momentum();
    msg_Tracking()<<"ERROR in "<<METHOD<<" : "<<std::endl
	       <<"   PYSTRF call results in error code : "<<pydat1.mstu[24-1]
	       <<" for "<<std::endl<<(*blob)<<"  "<<cms<<", "<<cms.Abs2()
	       <<std::endl;
    pydat1.mstu[24-1]=0;
    if(pydat1.mstu[23-1]<int(rpa->gen.NumberOfGeneratedEvents()/100) ||
       rpa->gen.NumberOfGeneratedEvents()<200) {
      msg_Tracking()<<"   Up to now: "<<pydat1.mstu[23-1]<<" errors, retrying..."
		 <<std::endl;
      return Return_Value::Retry_Phase;
    }
    msg_Tracking()<<"   Up to now: "<<pydat1.mstu[23-1]<<" errors, abort the run."
	       <<std::endl;
  }
  if(flag) {
    msg_Tracking()<<"ERROR in "<<METHOD<<" : "<<std::endl
	       <<"   Incomplete fragmentation."<<std::endl;
    return Return_Value::Retry_Phase;
  }
  pydat1.mstu[70-1]=2;
  pydat1.mstu[72-1]=hepevt.nhep;
  return Return_Value::Success;
}

void Lund_Interface::AddPartonToString(Particle *parton,int &nhep)
{
  hepevt.idhep[nhep]=SherpaToIdhep(parton->Flav());
  for (short int j=1; j<4; ++j) hepevt.phep[nhep][j-1]=parton->Momentum()[j];
  hepevt.phep[nhep][3]=parton->Momentum()[0];
  double pabs=(parton->Momentum()).Abs2();
  if (pabs<0) hepevt.phep[nhep][4]=0.0;
  else hepevt.phep[nhep][4]=sqrt(pabs);
  for (short int j=1;j<4;++j) hepevt.vhep[nhep][j-1]=parton->XProd()[j];
  hepevt.vhep[nhep][3]=parton->XProd()[0];
  hepevt.isthep[nhep]=1;
  hepevt.jmohep[nhep][0]=0;
  hepevt.jmohep[nhep][1]=0;
  hepevt.jdahep[nhep][0]=0;
  hepevt.jdahep[nhep][1]=0;
  nhep++;
}

void Lund_Interface::FillFragmentationBlob(Blob *blob)
{
  Particle *particle;
  Flavour flav;
  Vec4D momentum, position;
  for (int i=0;i<hepevt.nhep;++i) {
    if ((hepevt.isthep[i]!=2)&&(hepevt.isthep[i]!=1)&&(hepevt.isthep[i]!=149)) continue;
    if (hepevt.idhep[i]==93) flav=Flavour(kf_cluster);
    else flav=IdhepToSherpa(hepevt.idhep[i]);
    if (flav==Flavour(kf_string) || 
	flav==Flavour(kf_cluster)) {
      for (int j=hepevt.jdahep[i][0]-1;j<hepevt.jdahep[i][1];j++) {
        flav=IdhepToSherpa(hepevt.idhep[j]);
	momentum=Vec4D(hepevt.phep[j][3],hepevt.phep[j][0],
		       hepevt.phep[j][1],hepevt.phep[j][2]);
	position=Vec4D(hepevt.vhep[j][3],hepevt.vhep[j][0],
		       hepevt.vhep[j][1],hepevt.vhep[j][2]);
	particle = new Particle(-1,flav,momentum);
	particle->SetNumber(0);
	particle->SetStatus(part_status::active);
	particle->SetInfo('P');
	particle->SetFinalMass(hepevt.phep[j][4]);
	blob->SetPosition(position);
	blob->AddToOutParticles(particle);
      }
    }
  }
  blob->SetStatus(blob_status::needs_hadrondecays);
}

void Lund_Interface::FillOutgoingParticlesInBlob(Blob *blob)
{
  Flavour    flav;
  Vec4D      momentum;
  Particle * particle;

  int n_q(0), n_g(0); Particle_Vector partons;
  for (int j=hepevt.jdahep[0][0]-1;j<hepevt.jdahep[0][1];j++) {
    int idhep = hepevt.idhep[j];
    flav=IdhepToSherpa(idhep);
    momentum=Vec4D(hepevt.phep[j][3],hepevt.phep[j][0],
		   hepevt.phep[j][1],hepevt.phep[j][2]);
    // don't fill blob position vector here, because decay is in CMS until boosting back
    particle = new Particle(-1,flav,momentum);
    particle->SetNumber(0);
    particle->SetStatus(part_status::active);
    particle->SetInfo('D');
    particle->SetFinalMass(hepevt.phep[j][4]);
    blob->AddToOutParticles(particle);
    
    if(abs(idhep)>0 && abs(idhep)<7) {
      n_q++;
      partons.push_back(particle);
    }
    else if(abs(idhep)==21) {
      n_g++;
      partons.push_back(particle);
    }
  }
  
  size_t n=partons.size();
  if(n>0) blob->SetStatus(blob_status::needs_showers);
  if(n_q==2 && n_g==0 && n==2) {
    if(partons[0]->Flav().IsAnti()) {
      partons[0]->SetFlow(2,-1);
      partons[1]->SetFlow(1,partons[0]->GetFlow(2));
    }
    else {
      partons[0]->SetFlow(1,-1);
      partons[1]->SetFlow(2,partons[0]->GetFlow(1));
    }
  }
  else if(n_q==0 && n_g==2 && n==2) {
    partons[0]->SetFlow(2,-1);
    partons[0]->SetFlow(1,-1);
    partons[1]->SetFlow(2,partons[0]->GetFlow(1));
    partons[1]->SetFlow(1,partons[0]->GetFlow(2));
  }
  else if(n_q==0 && n_g==3 && n==3) {
    partons[0]->SetFlow(2,-1);
    partons[0]->SetFlow(1,-1);
    partons[1]->SetFlow(2,partons[0]->GetFlow(1));
    partons[1]->SetFlow(1,-1);
    partons[2]->SetFlow(2,partons[1]->GetFlow(1));
    partons[2]->SetFlow(1,partons[0]->GetFlow(2));
  }
  else if(n>0) {
    msg_Error()<<METHOD<<" wasn't able to set the color flow for"<<endl<<*blob<<endl;
  }
}

void Lund_Interface::RestoreStatus() {
  for(int i=0;i<5;i++) {
    pydatr.mrpy[i] = s_saved_mrpy[i];
  }
  for(int i=0;i<100;i++) {
    pydatr.rrpy[i] = s_saved_rrpy[i];
  }
}

void Lund_Interface::SaveStatus() {
  for(int i=0;i<5;i++) {
    s_saved_mrpy[i] = pydatr.mrpy[i];
  }
  for(int i=0;i<100;i++) {
    s_saved_rrpy[i] = pydatr.rrpy[i];
  }
}

bool Lund_Interface::ReadInStatus(const std::string &path)
{
  ReadInStatus((path+"Lund_random.dat").c_str(),0);
  return true;
}

void Lund_Interface::ReadInStatus(const std::string &filename, int mode) {
  ifstream myinstream(filename.c_str());
  if (myinstream.good()) {
    for(int i=0;i<5;i++) {
      myinstream>>pydatr.mrpy[i];
    }
    for(int i=0;i<100;i++) {
      myinstream>>pydatr.rrpy[i];
    }
    myinstream.close();
  }
  else msg_Error()<<"ERROR in "<<METHOD<<": "<<filename<<" not found!!"<<endl;
}

void Lund_Interface::WriteOutStatus(const std::string &filename)
{
  ofstream myoutstream(filename.c_str());
  if (myoutstream.good()) {
    myoutstream.precision(32);
    for(int i=0;i<5;i++) {
      myoutstream<<pydatr.mrpy[i]<<"\t";
    }
    for(int i=0;i<100;i++) {
      myoutstream<<pydatr.rrpy[i]<<"\t";
    }
    myoutstream<<endl;
    myoutstream.close();
  }
  else msg_Error()<<"ERROR in "<<METHOD<<": "<<filename<<" not found!!"<<endl;
}

void Lund_Interface::PrepareTerminate()
{
  std::string path(rpa->gen.Variable("SHERPA_STATUS_PATH"));
  if (path=="") return;
  RestoreStatus();
  WriteOutStatus((path+"/Lund_random.dat").c_str());
}

void Lund_Interface::Error(const int error)
{
  ++s_errors;
  if (s_errors>s_maxerrors) {
    THROW(critical_error,"Pythia calls PYERRM("+
	  ToString(error)+")");
  }
  else {
    msg_Tracking()<<"Lund_Interface::Error("<<error<<") "<<om::red
	       <<"Pythia calls PYERRM("<<error<<") in event "
	       <<rpa->gen.NumberOfGeneratedEvents()<<"."
	       <<om::reset<<endl;
//     if (msg_LevelIsDebugging()) {
//       msg_Tracking()<<*s_bloblist<<endl;
    if (msg_LevelIsTracking()) pylist(2);
//     }
  }
}

void Lund_Interface::NextFile(const bool newfile) 
{
  if (!m_writeout) return; 
  string oldfile;
  ofstream *outfile=p_hepevt->GetOutStream();
  if (outfile!=NULL) {
    oldfile=m_outfile+ToString(m_curfile)+string(".evts");
    if (newfile) 
      (*outfile)<<(m_outfile+ToString(++m_curfile)+string(".evts"))<<endl;
    if (m_compress) {
      if (!system((string("gzip ")+oldfile+string(".gz ")+oldfile).c_str()))
        msg_Error()<<METHOD<<"(): Error gzipping "<<oldfile<<std::endl;
      if (!system((string("rm ")+oldfile).c_str()))
        msg_Error()<<METHOD<<"(): Could not remove "<<oldfile<<std::endl;
    }
  }
  if (!newfile) {
    if (p_hepevt) { 
      p_hepevt->SetNhep(0);
      p_hepevt->SetIsthep(NULL);
      p_hepevt->SetIdhep(NULL);
      p_hepevt->SetJmohep(NULL);
      p_hepevt->SetJdahep(NULL);
      p_hepevt->SetPhep(NULL);
      p_hepevt->SetVhep(NULL);
      delete p_hepevt; 
      p_hepevt = NULL; 
    }
    return;
  }
  string file = string(m_outfile+ToString(m_curfile)+string(".evts"));
  p_hepevt->ChangeOutStream(file,m_evtsperfile);
}

Return_Value::code Lund_Interface::OneEvent(Blob_List * const blobs,double &weight)
{
  bool okay = false;
  for (int i=0;i<200;i++) {
    pyevnt();
    pyhepc(1);
    //pylist(2);
    weight=1.;  //*=pypars.pari[10];
    for (int i=0;i<hepevt.nhep;i++) {
      for (int j=0;j<2;j++) {
	p_jmohep[2*i+j] = hepevt.jmohep[i][j]; 
	p_jdahep[2*i+j] = hepevt.jdahep[i][j];
      } 
      for (int j=0;j<5;j++) p_phep[5*i+j] = hepevt.phep[i][j];
      for (int j=0;j<4;j++) p_vhep[4*i+j] = hepevt.vhep[i][j];
    }
    p_hepevt->SetNhep(hepevt.nhep);
    p_hepevt->SetIsthep(hepevt.isthep);
    p_hepevt->SetIdhep(hepevt.idhep);
    p_hepevt->SetJmohep(p_jmohep);
    p_hepevt->SetJdahep(p_jdahep);
    p_hepevt->SetPhep(p_phep);
    p_hepevt->SetVhep(p_vhep);
    if (msg_LevelIsDebugging()) pylist(3);
    if (p_hepevt->HepEvt2Sherpa(blobs)) { 
      okay = true; 
      break; 
    }
  }
  if (okay) return Return_Value::Success;
  return Return_Value::Nothing;
} 

Flavour Lund_Interface::IdhepToSherpa(long int idhep) {
  // This is not a simple conversion of PDG or HepEvt <-> Sherpa
  // since Pythia is using the same swapped codes for a_0 and f_0 as Sherpa
  kf_code kfc = (kf_code) abs(idhep);
  if (abs(idhep)==91) kfc=kf_cluster;
  else if (abs(idhep)==92) kfc=kf_string;

  return Flavour(kfc, idhep<0);
}

long int Lund_Interface::SherpaToIdhep(const Flavour& flav) {
  // This is not a simple conversion of PDG or HepEvt <-> Sherpa
  // since Pythia is using the same swapped codes for a_0 and f_0 as Sherpa
  long int idhep=flav.Kfcode();
  if (idhep==kf_cluster) idhep=91;
  else if (idhep==kf_string) idhep=92;

  return (flav.IsAnti() ? -idhep : idhep);
}

DEFINE_FRAGMENTATION_GETTER(Lund_Interface, "Lund")

