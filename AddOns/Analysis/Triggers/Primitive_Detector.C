#include "AddOns/Analysis/Triggers/Primitive_Detector.H"

#include "AddOns/Analysis/Triggers/Primitive_Calorimeter.H"
#include "AddOns/Analysis/Main/Primitive_Analysis.H"


#include "ATOOLS/Org/MyStrStream.H"
#include "ATOOLS/Org/Message.H"

using namespace ANALYSIS;
using namespace ATOOLS;

#include <iomanip>

DECLARE_GETTER(Primitive_Detector,"Detector",
	       Analysis_Object,Analysis_Key);

void ATOOLS::Getter<Analysis_Object,Analysis_Key,Primitive_Detector>::
PrintInfo(std::ostream &str,const size_t width) const
{
  str<<"{\n"
     <<std::setw(width+7)<<" "<<"InList:  list,\n"
     <<std::setw(width+7)<<" "<<"OutList: list,\n"
     <<std::setw(width+7)<<" "<<"HadCal:  [etamin, etamax, etacells, phicells]\n"
     <<std::setw(width+4)<<" "<<"}";
}

Analysis_Object *ATOOLS::Getter
<Analysis_Object,Analysis_Key,Primitive_Detector>::
operator()(const Analysis_Key& key) const
{
  ATOOLS::Scoped_Settings s{ key.m_settings };
  const auto inlist = s["InList"].SetDefault("FinalState").Get<std::string>();
  const auto outlist = s["OutList"].SetDefault("Detected").Get<std::string>();
  Primitive_Detector *detector = new Primitive_Detector(inlist,outlist);
  const auto hcparams = s["HadCal"].SetDefault<std::string>({}).GetVector<std::string>();
  if (hcparams.size() < 4)
    THROW(fatal_error, "HadCal must have four or more parameters.");
  detector->Add(new Primitive_Calorimeter(s.Interprete<double>(hcparams[0]),
                                          s.Interprete<double>(hcparams[1]),
                                          s.Interprete<int>(hcparams[2]),
                                          s.Interprete<int>(hcparams[3]),
                                          hcparams.size()>4?hcparams[4]:"NotLepton"));
  if (s["CalCone"].IsSetExplicitly())
    msg_Out()<<"WARNING CalCone   no longer supported by Primitive Detector ! "<<std::endl;
  return detector;
}

#include "AddOns/Analysis/Triggers/Final_Selector.H"

using namespace ATOOLS;

Primitive_Detector_Element::Primitive_Detector_Element() : 
  m_nx(-1), m_ny(-1), m_name(std::string("Unspecified")), p_cells(NULL) { }

Primitive_Detector_Element::Primitive_Detector_Element(std::string name) : 
  m_nx(-1), m_ny(-1), m_name(name), p_cells(NULL) { }

Primitive_Detector_Element::Primitive_Detector_Element(int nx,int ny,std::string name) :
  m_nx(nx), m_ny(ny), m_name(name)
{
  p_cells = new double*[m_nx];
  for (int i=0; i<m_nx;++i) p_cells[i] = new double[m_ny];
}

Primitive_Detector_Element::~Primitive_Detector_Element()
{
  if (p_cells) {
    for (int i=0;i<m_nx;++i) delete [] p_cells[i];
    p_cells=NULL;
  }
}

Primitive_Detector::Primitive_Detector(const std::string &inlist,
				       const std::string &outlist): 
  m_inlistname(inlist), m_outlistname(outlist)
{ 
  m_name = std::string("Full Detector"); 
}

Primitive_Detector::~Primitive_Detector()
{
  for (String_DetectorElement_Iter sdeiter=m_elements.begin();
       sdeiter!=m_elements.end();++sdeiter) {
    if (sdeiter->second!=NULL) {
      m_elements.erase(sdeiter--);
    }
  }
  m_elements.clear();
}

Analysis_Object* Primitive_Detector::GetCopy() const
{
  Primitive_Detector *detector = 
    new Primitive_Detector(m_inlistname,m_outlistname);

  for (String_DetectorElement_Map::const_iterator sdeiter=m_elements.begin();
       sdeiter!=m_elements.end();++sdeiter) {
    if (sdeiter->second!=NULL) {
      detector->Add(sdeiter->second->Copy());
    }
  }

  return detector;
}

void Primitive_Detector::Evaluate(const ATOOLS::Blob_List &bloblist,
				  double value,double ncount)
{
  Particle_List *inparticles=p_ana->GetParticleList(m_inlistname);
  if (inparticles==NULL) {
    msg_Error()<<"Primitive_Detector::Evaluate(..): "
		       <<"Particle list '"<<m_inlistname<<"' not found."<<std::endl;
    return;
  }
  Fill(inparticles);
  Particle_List *outparticles = new Particle_List();
  Extract(outparticles);
  p_ana->AddParticleList(m_outlistname,outparticles);
}

void Primitive_Detector::Add(Primitive_Detector_Element * pde)
{
  m_elements[pde->Name()] = pde;
}

void Primitive_Detector::Print() {
  if (!msg_LevelIsInfo()) return;
  msg_Out()<<"==================================================="<<std::endl
		   <<m_name<<" with "<<m_elements.size()<<" components : "<<std::endl;
  int i=1;
  std::string name;
  for (String_DetectorElement_Iter sdeiter=m_elements.begin();
       sdeiter!=m_elements.end();sdeiter++) {
    msg_Out()<<"Element "<<i<<": "<<sdeiter->second->Name()<<std::endl;
  }
  msg_Out()<<"==================================================="<<std::endl;
}

void Primitive_Detector::Fill(const ATOOLS::Blob_List * bl)
{
  Particle_List * pl = new Particle_List; 
  for (Blob_List::const_iterator blit=bl->begin();blit!=bl->end();++blit) {
    for (int i=0;i<(*blit)->NOutP();++i) {
      Particle * p = (*blit)->OutParticle(i);
      if (p->DecayBlob()==NULL) pl->push_back(new Particle(*p));
    }
  }
  Fill(pl);
  for (Particle_List::iterator pit=pl->begin();pit!=pl->end();++pit) delete (*pit);
  delete pl;
} 

void Primitive_Detector::Fill(const ATOOLS::Particle_List * pl) 
{
  for (String_DetectorElement_Iter sdeiter=m_elements.begin();
       sdeiter!=m_elements.end();++sdeiter) {
    if (sdeiter->second!=NULL) sdeiter->second->Fill(pl);
  }
}

void Primitive_Detector::Extract(ATOOLS::Particle_List * pl) 
{
  for (String_DetectorElement_Iter sdeiter=m_elements.begin();
       sdeiter!=m_elements.end();++sdeiter) {
    if (sdeiter->second!=NULL) sdeiter->second->Extract(pl);
  }
}

Primitive_Detector_Element * Primitive_Detector::GetElement(std::string name)
{
  String_DetectorElement_Iter sdeiter=m_elements.find(name);
  if (sdeiter==m_elements.end()) {
    msg_Error()<<"Potential Error in Primitive_Detector::GetElement("<<name<<") :"<<std::endl
	       <<"   Element not found, return NULL and hope for the best."<<std::endl;
    return NULL;
  }
  return sdeiter->second;
}
