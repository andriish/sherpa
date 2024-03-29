#include "ATOOLS/Org/Message.H"
#include "ATOOLS/Org/Run_Parameter.H"
#include "MODEL/Main/Model_Base.H"
#include "PDF/GRS/GRSph_Fortran_Interface.H"
#include <unistd.h>

using namespace PDF;
using namespace ATOOLS;

extern "C" {
// GRSG99(ISET,X,Q2,UL,DL,SL,GL)
void grsg99_(int &, double &, double &, double &, double &, double &, double &);
}

GRSph_Fortran_Interface::GRSph_Fortran_Interface(const ATOOLS::Flavour _bunch,
                                                 const std::string _set)
    : Photon_PDF_Base(_bunch, _set, 3) {
  /*
   * ISET = 1  LEADING ORDER SET (DATA FILE 'grsg99lo.grid')        *
   *           ISET = 2  NEXT-TO-LEADING ORDER MSbar SET            *
   *                     (DATA FILE 'grsg99m.grid')                 *
   *           ISET = 3  NEXT-TO-LEADING ORDER DISgamma SET         *
   *                    (DATA FILE 'grsg99d.grid')
   */
  m_path = rpa->gen.Variable("SHERPA_SHARE_PATH") + "/GRSGrid";

  m_xmin = 1.e-5;
  m_xmax = 1.;
  m_q2min = .4;
  m_q2max = 1.e6;

  if (m_set == std::string("GRSLO"))
    m_iset = 1;
  else if (m_set == std::string("GRSMSbar"))
    m_iset = 2;
  else if (m_set == std::string("GRSMSbar"))
    m_iset = 3;
  else {
    msg_Out() << METHOD
              << ": Cannot recognize the chosen PDF parametrization. Will "
                 "use the Leading Order parametrization. \n";
    m_iset = 1;
  }

  rpa->gen.AddCitation(
      1, "The GRS photon PDF is published under \\cite{Gluck:1999ub}.");
}

PDF_Base *GRSph_Fortran_Interface::GetCopy() {
  return new GRSph_Fortran_Interface(m_bunch, m_set);
}

void GRSph_Fortran_Interface::CalculateSpec(const double &_x,
                                            const double &_Q2) {
  if (m_include_photon_in_photon)
    m_ph = GetPhotonCoefficient(_x, _Q2);

  double x = _x / m_rescale, Q2 = _Q2;

  char buffer[1024];
  char *err = getcwd(buffer, 1024);
  if (chdir(m_path.c_str()) != 0 || err == nullptr)
    msg_Error() << "Error in GRSph_Fortran_Interface.C " << std::endl
                << "   path " << m_path << " not found " << std::endl;

  grsg99_(m_iset, x, Q2, m_u, m_d, m_s, m_g);

  if (chdir(buffer) != 0)
    msg_Error() << "Error in GRSph_Fortran_Interface.C " << std::endl
                << "   path " << m_path << " not found." << std::endl;

  double alphaem = MODEL::s_model->ScalarFunction(std::string("alpha_QED"), 0);
  m_u *= alphaem;
  m_d *= alphaem;
  m_s *= alphaem;
  m_g *= alphaem;
}

DECLARE_PDF_GETTER(GRSph_Getter);

PDF_Base *GRSph_Getter::operator()(const Parameter_Type &args) const {
  if (!args.m_bunch.IsPhoton())
    return NULL;
  return new GRSph_Fortran_Interface(args.m_bunch, args.m_set);
}

void GRSph_Getter::PrintInfo(std::ostream &str, const size_t width) const {
  str << "GRS photon PDF, see Phys. Rev. D60(1999)054019\n"
         "The following sets can be chosen\n"
         " - GRSLO for leading order\n"
         " - GRSMSbar for next-to-leading order in MSbar\n"
         " - GRSDISg for next-to-leading order in DISgamma\n";
}

GRSph_Getter *p_get_GRS[3];

extern "C" void InitPDFLib() {
  p_get_GRS[0] = new GRSph_Getter("GRSLO");
  p_get_GRS[1] = new GRSph_Getter("GRSMSbar");
  p_get_GRS[2] = new GRSph_Getter("GRSDISg");
}

extern "C" void ExitPDFLib() {
  for (int i = 0; i < 3; i++)
    delete p_get_GRS[i];
}
