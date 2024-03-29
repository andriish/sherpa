#include "SHERPA/Main/Sherpa.H"
#include "SHERPA/Initialization/Initialization_Handler.H"
#include "SHERPA/Single_Events/Event_Handler.H"
#include "SHERPA/Single_Events/Analysis_Phase.H"
#include "SHERPA/Single_Events/Userhook_Phase.H"
#include "SHERPA/Single_Events/Output_Phase.H"
#include "SHERPA/Single_Events/EvtReadin_Phase.H"
#include "SHERPA/Single_Events/Signal_Processes.H"
#include "SHERPA/Single_Events/Hard_Decays.H"
#include "SHERPA/Single_Events/Minimum_Bias.H"
#include "SHERPA/Single_Events/Multiple_Interactions.H"
#include "SHERPA/Single_Events/Jet_Evolution.H"
#include "SHERPA/Single_Events/Signal_Process_FS_QED_Correction.H"
#include "SHERPA/Single_Events/Beam_Remnants.H"
#include "SHERPA/Single_Events/Hadronization.H"
#include "SHERPA/Single_Events/Hadron_Decays.H"
#include "SHERPA/PerturbativePhysics/Hard_Decay_Handler.H"
#include "SHERPA/Tools/HepMC2_Interface.H"
#include "SHERPA/Tools/HepMC3_Interface.H"
#include "PHASIC++/Decays/Decay_Channel.H"
#include "ATOOLS/Math/Random.H"
#include "ATOOLS/Org/Message.H"
#include "ATOOLS/Org/MyStrStream.H"
#include "ATOOLS/Org/Shell_Tools.H"
#include "ATOOLS/Org/Library_Loader.H"
#include "ATOOLS/Org/My_MPI.H"
#include "ATOOLS/Org/CXXFLAGS.H"
#include "ATOOLS/Org/CXXFLAGS_PACKAGES.H"
#include "ATOOLS/Org/Scoped_Settings.H"
#include "ATOOLS/Org/Run_Parameter.H"
#include "ATOOLS/Phys/KF_Table.H"
#include "PDF/Main/ISR_Handler.H"
#include "PDF/Main/PDF_Base.H"
#include <cstring>

using namespace SHERPA;
using namespace ATOOLS;
using namespace std;

Sherpa::Sherpa(int argc, char* argv[]) :
  p_inithandler(nullptr),
  p_eventhandler(nullptr)
#ifdef USING__HEPMC2
  , p_hepmc2(nullptr)
#endif
#ifdef USING__HEPMC3
  , p_hepmc3(nullptr)
#endif
{
  ATOOLS::mpi = new My_MPI();
  ATOOLS::exh = new Terminator_Object_Handler();
  ATOOLS::msg = new Message();
  // rpa should be constructed before initializing the main settings, since the
  // latter might throw an exception and rpa would be involved in terminating
  // the program then; however, do not call its Init method yet, because this
  // in turn needs the Settings to be initialized
  ATOOLS::rpa = new Run_Parameter();
  Settings::InitializeMainSettings(argc, argv);
  ATOOLS::ran = new Random(1234);
  ATOOLS::s_loader = new Library_Loader();
  PDF::pdfdefs = new PDF::PDF_Defaults();
  m_trials = 0;
  m_debuginterval = 0;
  m_debugstep = -1;
  m_displayinterval = 100;
  m_evt_starttime = -1.0;
  exh->AddTerminatorObject(this);
}

Sherpa::~Sherpa() 
{
  if (msg_LevelIsInfo()) {
    Return_Value::PrintStatistics(msg->Out());
    if (p_inithandler->GetVariations()) {
      p_inithandler->GetVariations()->PrintStatistics(msg->Out());
    }
  }
  Blob_List::PrintMomFailStatistics(msg->Out());
  PHASIC::Decay_Channel::PrintMaxKinFailStatistics(msg->Out());
  rpa->gen.WriteCitationInfo();
  if (p_eventhandler) { delete p_eventhandler; p_eventhandler = nullptr; }
  if (p_inithandler)  { delete p_inithandler;  p_inithandler  = nullptr; }
#ifdef USING__HEPMC2
  if (p_hepmc2)       { delete p_hepmc2;       p_hepmc2       = nullptr; }
#endif
#ifdef USING__HEPMC3
  if (p_hepmc3)       { delete p_hepmc3;       p_hepmc3       = NULL; }
#endif
  Settings& s = Settings::GetMainSettings();
  if (s["CHECK_SETTINGS"].SetDefault(true).Get<bool>())
    Settings::FinalizeMainSettings();
  exh->RemoveTerminatorObject(this);
  delete ATOOLS::s_loader;
  delete PDF::pdfdefs;
  delete ATOOLS::rpa;
  delete ATOOLS::ran;
#ifdef USING__MPI
  mpi->Barrier();
#endif
  delete ATOOLS::msg;
  delete ATOOLS::exh;
  delete ATOOLS::mpi;
  for (KF_Table::const_iterator kfit(s_kftable.begin());kfit!=s_kftable.end();++kfit)
    delete kfit->second;
  ATOOLS::s_kftable.clear();
}

bool Sherpa::InitializeTheRun()
{
  Settings& s = Settings::GetMainSettings();
  p_inithandler = new Initialization_Handler();
  RegisterDefaults();

  mpi->PrintRankInfo();

  DrawLogo(s["PRINT_VERSION_INFO"].Get<bool>());

  if (p_inithandler->InitializeTheFramework()) {
    int initonly=s["INIT_ONLY"].Get<int>();
    if (initonly==1) THROW(normal_exit,"Initialization complete.");
    if (initonly==2) return true;
    if (!p_inithandler->CalculateTheHardProcesses()) return false;
    m_showtrials=s["SHOW_NTRIALS"].Get<bool>();

    // read in from status path
    bool res(true);
    std::string statuspath(s["STATUS_PATH"].Get<std::string>());
    if (statuspath != "") {
      res=exh->ReadInStatus(statuspath);
    }

    m_debuginterval = s["DEBUG_INTERVAL"].Get<long int>();
    m_debugstep     = s["DEBUG_STEP"].Get<long int>();

    m_displayinterval=s["EVENT_DISPLAY_INTERVAL"].Get<int>();
    m_evt_output = s["EVT_OUTPUT"].Get<int>();
    m_evt_output_start = s["EVT_OUTPUT_START"].Get<int>();

    return res;
  }
  msg_Error()<<"Error in Sherpa::InitializeRun()"<<endl
	     <<"   Did not manage to initialize the framework."<<endl
	     <<"   Try to run nevertheless ... ."<<endl;

  return 0;
}

void Sherpa::RegisterDefaults()
{
  Settings& s = Settings::GetMainSettings();
  s["PRINT_VERSION_INFO"].SetDefault(false);
  s["INIT_ONLY"].SetDefault(0);
  s["SHOW_NTRIALS"].SetDefault(false);
  s["DEBUG_INTERVAL"].SetDefault(0);
  s["DEBUG_STEP"].SetDefault(-1);
  s["EVENT_DISPLAY_INTERVAL"].SetDefault(100);
  s["EVT_OUTPUT"].SetDefault(msg->Level());

  const int evtoutput{ s["EVT_OUTPUT"].Get<int>() };
  s["EVT_OUTPUT_START"].SetDefault(evtoutput != msg->Level() ? 1 : 0);
}

bool Sherpa::InitializeTheEventHandler() 
{
  eventtype::code mode = p_inithandler->Mode();
  p_eventhandler  = new Event_Handler();
  Analysis_Vector *anas(p_inithandler->GetAnalyses());
  for (Analysis_Vector::iterator it=anas->begin(); it!=anas->end(); ++it) {
    (*it)->SetEventHandler(p_eventhandler);
  }

  if (mode==eventtype::EventReader) {
    p_eventhandler->AddEventPhase(new EvtReadin_Phase(p_inithandler->GetEventReader()));
    p_eventhandler->AddEventPhase(new Hard_Decays(p_inithandler->GetHardDecayHandler()));
    p_eventhandler->AddEventPhase(new Beam_Remnants(p_inithandler->GetBeamRemnantHandler()));
  }
  else {
    p_eventhandler->AddEventPhase(new Signal_Processes(p_inithandler->GetMatrixElementHandler()));
    p_eventhandler->AddEventPhase(new Minimum_Bias(p_inithandler->GetSoftCollisionHandlers()));
    p_eventhandler->AddEventPhase(new Hard_Decays(p_inithandler->GetHardDecayHandler()));
    p_eventhandler->AddEventPhase(new Jet_Evolution(p_inithandler->GetMatrixElementHandler(),
                                                    p_inithandler->GetHardDecayHandler(),
						    p_inithandler->GetHDHandler(),
						    p_inithandler->GetMIHandlers(),
						    p_inithandler->GetSoftCollisionHandlers(),
						    p_inithandler->GetShowerHandlers(),
						    p_inithandler->GetRemnantHandlers()));
    p_eventhandler->AddEventPhase(new Signal_Process_FS_QED_Correction(
						    p_inithandler->GetMatrixElementHandler(),
						    p_inithandler->GetSoftPhotonHandler()));
    p_eventhandler->AddEventPhase(new Multiple_Interactions(p_inithandler->GetMIHandlers()));
    p_eventhandler->AddEventPhase(new Beam_Remnants(p_inithandler->GetBeamRemnantHandler()));
    p_eventhandler->AddEventPhase(new Hadronization(p_inithandler->GetColourReconnectionHandler(),
						    p_inithandler->GetFragmentation()));
    p_eventhandler->AddEventPhase(new Hadron_Decays(p_inithandler->GetHDHandler()));
  }
  p_eventhandler->AddEventPhase(new Userhook_Phase(this));
  if (!anas->empty()) p_eventhandler->AddEventPhase(new Analysis_Phase(anas));
  if (!p_inithandler->GetOutputs()->empty())
    p_eventhandler->AddEventPhase(new Output_Phase(p_inithandler->GetOutputs(), p_eventhandler));
  p_eventhandler->SetFilter(p_inithandler->GetFilter());
  p_eventhandler->PrintGenericEventStructure();

  ran->EraseLastIncrementedSeed();

  return 1;
}


bool Sherpa::GenerateOneEvent(bool reset) 
{
  if (m_evt_output_start>0 &&
      m_evt_output_start==rpa->gen.NumberOfGeneratedEvents()+1) {
    msg->SetLevel(m_evt_output);
  }
  
  if(m_debuginterval>0 &&
     rpa->gen.NumberOfGeneratedEvents()%m_debuginterval==0 &&
     (p_inithandler->GetMatrixElementHandler()->SeedMode()!=3 ||
      rpa->gen.NumberOfGeneratedEvents()==0)) {
      std::string fname=ToString(rpa->gen.NumberOfGeneratedEvents())+".dat";
      ran->WriteOutStatus(("random."+fname).c_str());
  }
  if (m_debugstep>=0) {
    if (p_inithandler->GetMatrixElementHandler()->SeedMode()!=3)
      ran->ReadInStatus(("random."+ToString(m_debugstep)+".dat").c_str());
    else {
      ran->ReadInStatus("random.0.dat");
      ran->FastForward(m_debugstep);
    }
  }
  
  if (m_evt_starttime<0.0) m_evt_starttime=rpa->gen.Timer().RealTime();
  
  if (reset) p_eventhandler->Reset();
  if (p_eventhandler->GenerateEvent(p_inithandler->Mode())) {
    if(m_debuginterval>0 && rpa->gen.NumberOfGeneratedEvents()%m_debuginterval==0){
      std::string fname=ToString(rpa->gen.NumberOfGeneratedEvents())+".dat";
      std::ofstream eventout(("refevent."+fname).c_str());
      eventout<<"# trial "<<rpa->gen.NumberOfTrials()-1<<std::endl;
      eventout<<*p_eventhandler->GetBlobs()<<std::endl;
      eventout.close();
    }
    if (m_debugstep>=0) {
      std::ofstream event(("event."+ToString(m_debugstep)+".dat").c_str());
      event<<*p_eventhandler->GetBlobs()<<std::endl;
      event.close();
      THROW(normal_exit,"Debug event written.");
    }
    rpa->gen.SetNumberOfGeneratedEvents(rpa->gen.NumberOfGeneratedEvents()+1);
    Blob_List *blobs(p_eventhandler->GetBlobs());
    
    /// Increase m_trials --- based on signal blob["Trials"] if existent
    if (blobs->FindFirst(btp::Signal_Process) == nullptr) {
      m_trials+=1;
      msg_Debugging()<<"  No Signal_Process Blob found, increasing m_trials by 1\n";
    }
    else {
      m_trials+=(*blobs->FindFirst(btp::Signal_Process))["Trials"]->Get<double>();
    }
    
    if (msg_LevelIsEvents()) {
      if (!blobs->empty()) {
	msg_Out()<<"  -------------------------------------------------\n";
	for (Blob_List::iterator blit=blobs->begin();
	     blit!=blobs->end();++blit) 
	  msg_Out()<<*(*blit)<<std::endl;
	msg_Out()<<"  -------------------------------------------------\n";
      }
      else msg_Out()<<"  ******** Empty event ********  "<<std::endl;
    }
    
    for (Blob_List::const_iterator bit=blobs->begin(); bit!=blobs->end();++bit) {
      double currQ = (*bit)->CheckChargeConservation();
      if (fabs(currQ)>1e-12) {
	msg_Error()<<"Charge conservation failed for "<<(*bit)->Type()<<": "<<currQ<<".\n"
		   <<"Will ignore event.\n";
	return 0;
      }
    }
    
    int i=rpa->gen.NumberOfGeneratedEvents();
    int nevt=rpa->gen.NumberOfEvents();
    msg_Events()<<"Sherpa : Passed "<<i<<" events."<<std::endl;
    int exp;
    for (exp=5; i/int(pow(10,exp))==0; --exp) {}
    if (((rpa->gen.BatchMode()&4 && i%m_displayinterval==0) ||
	 (!(rpa->gen.BatchMode()&4) && i%int(pow(10,exp))==0)) &&
	i<rpa->gen.NumberOfEvents()) {
      double diff=rpa->gen.Timer().RealTime()-m_evt_starttime;
      msg_Info()<<"  Event "<<i;
      if (m_showtrials)
        msg_Info()<<"("+ToString(m_trials)+")";
      msg_Info()<<" ( ";
      if (rpa->gen.BatchMode()&16) {
        msg_Info()<<diff<<"s elapsed / "
                  <<((nevt-i)/(double)i*diff)<<"s";
      } else {
        msg_Info()<<FormatTime(size_t(diff))<<" elapsed / "
                  <<FormatTime(size_t((nevt-i)/(double)i*diff));
      }
      msg_Info()<<" left ) -> ETA: "<<rpa->gen.Timer().
        StrFTime("%a %b %d %H:%M",time_t((nevt-i)/(double)i*diff))<<"  ";
      p_eventhandler->PerformMemoryMonitoring();
      const Uncertain<double> xs = p_eventhandler->TotalNominalXSMPI();
      if (!(rpa->gen.BatchMode()&2)) msg_Info()<<"\n  ";
      msg_Info() << "XS = " << xs.value << " pb +- ( " << xs.error
                 << " pb = " << xs.PercentError() << " % )  ";
      if (rpa->gen.BatchMode()&8)
        msg_Info()<<"  Process was "<<p_eventhandler->CurrentProcess()<<"  ";
      if (!(rpa->gen.BatchMode()&2))
	msg_Info()<<mm(1,mm::up);
      if (rpa->gen.BatchMode()&2) { msg_Info()<<std::endl; }
      else { msg_Info()<<bm::cr<<std::flush; }
    }
    return 1;
  }
  return 0;
}

#ifdef USING__HEPMC2
void Sherpa::FillHepMCEvent(HepMC::GenEvent& event)
{
  if (!p_hepmc2) p_hepmc2 = new SHERPA::HepMC2_Interface();
  ATOOLS::Blob_List* blobs = GetEventHandler()->GetBlobs();
  p_hepmc2->Sherpa2HepMC(blobs, event);
  p_hepmc2->AddCrossSection(event, p_eventhandler->TotalNominalXS());
}
#endif

#ifdef USING__HEPMC3
void Sherpa::FillHepMCEvent(HepMC3::GenEvent& event)
{
  if (p_hepmc3==NULL) p_hepmc3 = new SHERPA::HepMC3_Interface();
  ATOOLS::Blob_List* blobs=GetEventHandler()->GetBlobs();
  p_hepmc3->Sherpa2HepMC(blobs, event);
  p_hepmc3->AddCrossSection(event, p_eventhandler->TotalXS(),
                            p_eventhandler->TotalErr());
}
#endif

Uncertain<double> Sherpa::TotalNominalXS() const
{
  return p_eventhandler->TotalNominalXS();
}

std::string Sherpa::PDFInfo()
{
  std::string pdf="Unknown";
  PDF::ISR_Handler* isr=GetInitHandler()->GetISRHandler(PDF::isr::hard_process);
  if (isr) {
    if (isr->PDF(0)) {
      pdf=isr->PDF(0)->Type();
      if (isr->PDF(1) && isr->PDF(1)->Type()!=pdf) {
        pdf="Unknown";
      }
    }
  }
  return pdf;
}

void Sherpa::PrepareTerminate()
{
  SummarizeRun();
  exh->RemoveTerminatorObject(this);
}

bool Sherpa::SummarizeRun() 
{
  if (p_eventhandler) {
    msg_Info()<<"  Event "<<rpa->gen.NumberOfGeneratedEvents()<<" ( "
              <<size_t(rpa->gen.Timer().RealTime()-m_evt_starttime)
              <<" s total ) = "
              << rpa->gen.NumberOfGeneratedEvents()*3600*24/
                 ((size_t) rpa->gen.Timer().RealTime()-m_evt_starttime)
              <<" evts/day                    "<<std::endl;
    p_eventhandler->Finish();
  }
  return true; 
}

long int Sherpa::NumberOfEvents() const
{
  return rpa->gen.NumberOfEvents();
}

const Blob_List &Sherpa::GetBlobList() const
{
  return *p_eventhandler->GetBlobs();
}

double Sherpa::GetMEWeight(const Cluster_Amplitude &ampl,const int mode) const
{
  return p_inithandler->GetMatrixElementHandler()->
    GetWeight(ampl,ATOOLS::nlo_type::lo,mode);
}

void Sherpa::DrawLogo(const bool& shouldprintversioninfo)
{ 
  msg_Info()<<"-----------------------------------------------------------------------------"<<std::endl;
  if (msg->Level()>0) msg_Out()<<"-----------    Event generation run with SHERPA started .......   -----------"<<std::endl;
  msg_Info()<<"-----------------------------------------------------------------------------"<<std::endl
	    <<"................................................ |       +                   "<<std::endl
	    <<"................................................ ||  |       +  +            "<<std::endl
	    <<"...................................        ....  | |         /   +           "<<std::endl
	    <<"................. ................   _,_ |  ....  ||         +|  +  +        "<<std::endl
	    <<"...............................  __.'  ,\\|  ...  ||    /    +|          +    "<<std::endl
	    <<".............................. (  \\    \\   ...  | |  |   + + \\         +   "<<std::endl
	    <<".............................  (    \\   -/  .... ||       +    |          +  "<<std::endl
	    <<"........ ...................  <S   /()))))~~~~~~~~##     +     /\\    +       "<<std::endl
	    <<"............................ (!H   (~~)))))~~~~~~#/     +  +    |  +         "<<std::endl
	    <<"................ ........... (!E   (~~~)))))     /|/    +         +          "<<std::endl
	    <<"............................ (!R   (~~~)))))   |||   + +            +        "<<std::endl
	    <<"..... ...................... (!P    (~~~~)))   /|  + +          +            "<<std::endl
	    <<"............................ (!A>    (~~~~~~~~~##        + +        +        "<<std::endl
	    <<"............................. ~~(!    '~~~~~~~ \\       +     + +      +      "<<std::endl
	    <<"...............................  `~~~QQQQQDb //   |         + + +        +   "<<std::endl
	    <<"........................ ..........   IDDDDP||     \\  + + + + +             +"<<std::endl
	    <<"....................................  IDDDI||       \\                      + "<<std::endl
	    <<".................................... IHD HD||         \\ + +  + + + + +      +"<<std::endl
	    <<"...................................  IHD ##|            :-) + +\\          +  "<<std::endl
	    <<"......... ............... ......... IHI ## /      /   +  + + + +\\       +    "<<std::endl
	    <<"................................... IHI/ /       /      + + + +        +     "<<std::endl
	    <<"................................... ## | | /    / + +      + + /      +      "<<std::endl
	    <<"....................... /TT\\ .....  ##/ ///  / + + + + + + +/       +        "<<std::endl
	    <<"......................./TTT/T\\ ... /TT\\/\\\\\\ / + + + + + + +/   \\         +   "<<std::endl
	    <<"....................../TTT/TTTT\\...|TT/T\\\\\\/   +    ++  + /              "<<std::endl
	    <<"-----------------------------------------------------------------------------"<<std::endl
	    <<std::endl
	    <<"     SHERPA version "<<SHERPA_VERSION<<"."<<SHERPA_SUBVERSION<<" ("<<SHERPA_NAME<<")"<<std::endl
	    <<"                                                                             "<<std::endl
	    <<"     Authors:        Enrico Bothmann, Stefan Hoeche, Frank Krauss,           "<<std::endl
	    <<"                     Silvan Kuttimalai, Marek Schoenherr, Holger Schulz,     "<<std::endl
	    <<"                     Steffen Schumann, Frank Siegert, Korinna Zapp           "<<std::endl
	    <<"     Former Authors: Timo Fischer, Tanju Gleisberg, Hendrik Hoeth,           "<<std::endl
	    <<"                     Ralf Kuhn, Thomas Laubrich, Andreas Schaelicke,         "<<std::endl
	    <<"                     Jan Winter                                              "<<std::endl
	    <<"                                                                             "<<std::endl
	    <<"     This program uses a lot of genuine and original research work           "<<std::endl
	    <<"     by other people. Users are encouraged to refer to                       "<<std::endl
	    <<"     the various original publications.                                      "<<std::endl
	    <<"                                                                             "<<std::endl
	    <<"     Users are kindly asked to refer to the documentation                    "<<std::endl
	    <<"     published under JHEP 02(2009)007                                        "<<std::endl
	    <<"                                                                             "<<std::endl
	    <<"     Please visit also our homepage                                          "<<std::endl
	    <<"                                                                             "<<std::endl
	    <<"       http://sherpa.hepforge.org                                            "<<std::endl
	    <<"                                                                             "<<std::endl
	    <<"     for news, bugreports, updates and new releases.                         "<<std::endl
	    <<"                                                                             "<<std::endl
	    <<"-----------------------------------------------------------------------------"<<std::endl;
  rpa->gen.PrintGitVersion(msg->Info(), shouldprintversioninfo);
  rpa->gen.AddCitation
    (0,"The complete Sherpa package is published under \\cite{Gleisberg:2008ta}.");
}
