#include "AMISIC++/Perturbative/Sudakov_Argument.H"
#include "AMISIC++/Perturbative/MI_Processes.H"
#include "ATOOLS/Org/Run_Parameter.H"
#include "ATOOLS/Org/Message.H"

using namespace AMISIC;
using namespace ATOOLS;

Sudakov_Argument::
Sudakov_Argument(MI_Processes * procs,const axis & sbins,const axis & pt2bins) :
  p_processes(procs), m_sbins(sbins), m_pt2bins(pt2bins),
  m_integral(TwoDim_Table(m_sbins,m_pt2bins)),
  m_function(TwoDim_Table(m_sbins,m_pt2bins)),
  m_test(false)
{
  FillTables();
  if (m_test) OutputTables();
}

void Sudakov_Argument::FillTables() {
  /////////////////////////////////////////////////////////////////////////////////
  // Iterating over the bins in s (for hadron-hadron collisions typically only one,
  // while more for collisions involving e.g. EPA photons) to fill the table of
  // Sudakov arguments, i.e. int dpt^2 dsigma/dpt^2.
  // We do this with exact MEs, as we use these table to fix the b-dependence, for
  // example for MinBias events - so the approximate hit-or-miss form will not quite
  // be the correct way of doing this.
  // These table will be used in the Impact_Parameter class.
  /////////////////////////////////////////////////////////////////////////////////
  for (size_t sbin=0;sbin<m_sbins.m_nbins;sbin++) {
    double s = m_sbins.x(sbin);
    (*p_processes->GetXSecs())(s);
    p_processes->UpdateS(s);
    FillPT2Values(sbin,p_processes->GetXSecs()->XSndNorm() * p_processes->GetXSecs()->XSnd());
  }
}

void Sudakov_Argument::FillPT2Values(const size_t & sbin,const double & norm) {
  /////////////////////////////////////////////////////////////////////////////////
  // Sudakov form factor for one fixed value of cms energy squared s.
  /////////////////////////////////////////////////////////////////////////////////
  double pt2last = m_pt2bins.x(m_pt2bins.m_nbins-1);
  double sigma, pt2, dpt2, sigmalast = 0., integral = 0.;
  for (int pt2bin=m_pt2bins.m_nbins-2;pt2bin>=0;pt2bin--) {
    pt2       = m_pt2bins.x(pt2bin);
    dpt2      = pt2last-pt2;
    sigma     = p_processes->dSigma(pt2);
    /////////////////////////////////////////////////////////////////////////////////
    // The dSigma is in 1/GeV^4, norm (the non-diffractive cross section is in 1/GeV^2
    // so overall integral does not have any units.
    /////////////////////////////////////////////////////////////////////////////////
    integral += (sigma+sigmalast)/2. * dpt2/norm;
    pt2last   = pt2;
    sigmalast = sigma;
    m_function.Fill(sbin, pt2bin, sigma);
    m_integral.Fill(sbin, pt2bin, integral);
  }
}

const double Sudakov_Argument::XSratio(const double & s) {
  return m_integral.Value(m_sbins.bin(s),0);
}

void Sudakov_Argument::OutputTables() {
  msg_Out()<<"-------------------------------------------------------------------------------\n"
	   <<"Calculated look-up tables and values for the Sudakov_Argument:\n"
	   <<std::setw(15)<<"E_{c.m.} [GeV]"<<" | "
	   <<std::setw(15)<<"xs_hard/xs_ND"<<" | "
	   <<std::setw(10)<<"pt^2"<<" | "
	   <<std::setw(10)<<"f(pt^2)"<<" |  "
	   <<std::setw(10)<<"Int(pt^2)\n"
	   <<std::fixed<<std::setprecision(4);
  for (size_t sbin=0;sbin<m_sbins.m_nbins;sbin++) {
    msg_Out()<<"-------------------------------------------------------------------------------\n";
    double s       = m_sbins.x(sbin);
    double xsratio = XSratio(s);
    for (size_t bin=0;bin<m_pt2bins.m_nbins;bin+=10) {
      msg_Out()<<std::setw(15)<<sqrt(s)<<" | "
	       <<std::setw(15)<<xsratio<<" | "
	       <<std::setw(10)<<m_pt2bins.x(bin)<<" | "
	       <<std::setw(10)<<m_function.Value(sbin,bin)<<" | "
	       <<std::setw(10)<<m_integral.Value(sbin,bin)<<"\n";
    }
  }
  msg_Out()<<"-------------------------------------------------------------------------------\n";
  THROW(normal_exit,"testing complete");
}
