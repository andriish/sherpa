#include "PDF/Main/NLOMC_Base.H"

#define COMPILE__Getter_Function
#define OBJECT_TYPE PDF::NLOMC_Base
#define PARAMETER_TYPE PDF::NLOMC_Key
#define EXACTMATCH false
#include "ATOOLS/Org/Getter_Function.C"

using namespace PDF;
using namespace ATOOLS;

NLOMC_Base::NLOMC_Base(const std::string& name)
    : m_name(name), p_shower(NULL), m_subtype(subscheme::CS)
{
  m_kt2min[0]=m_kt2min[1]=-1.0;
}

NLOMC_Base::~NLOMC_Base() 
{
}

void NLOMC_Base::ShowSyntax(const int mode)
{
  if (!msg_LevelIsInfo() || mode==0) return;
  msg_Out()<<METHOD<<"(): {\n\n";
  NLOMC_Getter::PrintGetterInfo(msg->Out(),15);
  msg_Out()<<"\n}"<<std::endl;
}

