#ifndef PHASIC_Channels_Vegas_H
#define PHASIC_Channels_Vegas_H

#include "ATOOLS/Org/Message.H"
#include <list>
#include <vector>

namespace PHASIC {

  class Vegas {
  public:
    class Mapping {
    public:
      virtual double operator()(double &x) const = 0;
    };// end of class Mapping
    class Power: public Mapping {
    public:
      double m_exp;
      inline Power(const double &exp): m_exp(exp) {}
      double operator()(double &x) const;
    };// end of class Power
  private:
    std::string m_name;
    int m_nd;
    unsigned long m_nevt,m_nopt,m_snevt,m_cevt;
    unsigned long m_mnevt, m_mcevt, m_const;
    double m_alpha;
    double m_nc;
    double **p_xi;
    double *p_x,*p_xin,*p_r,*p_cx,*p_bm;
    double **p_d,**p_di,*p_dt;
    double **p_md,**p_mdi;
    double *p_chi,*p_bestchi,**p_bestxi;
    int *p_ia,*p_opt,*p_cb;
    int **p_hit, m_sint, m_scnt;
    int **p_mhit;
    int m_dim,m_mode,m_on,m_autooptimize,m_omode,m_cmode;
    static int s_on, s_onext;
    void Rebin(double rc, double * xi);
    void AddPoint(double);
  public:
    Vegas(int dim,int ndx,const std::string &);
    ~Vegas();
    double* GeneratePoint(const double * _ran);
    double GenerateWeight(const double *) const;
    double GenerateBinWeight(int *) const;
    void AddPoint(double,double *);
    void AddBinPoint(double,int *);

    void MPISync();
    void Reset();
    void Optimize();
    void EndOptimize();
    void ConstChannel(int i) { p_opt[i]=0; m_const|=(1<<i); }

    void InitBinInfo();
    void Refine();
    void Preset(const Mapping *,const int);

    void SetOptimizeChannel(int i,const bool opt) { p_opt[i]=opt;    }
    bool OptimizeChannel(int i) const             { return p_opt[i]; }

    void SetCheck(int mode) { m_mode=mode; }

    void WriteOut(const std::string &);
    void ReadIn(const std::string &);
    void WriteHistos(const std::string &);

    void SetAutoOptimize(int ao)    { m_autooptimize=ao*m_nd; }

    void SetAutoRefine() { m_sint=1; }

    static inline void SetOnExternal(const bool on) { s_onext=on;     }
    static inline bool OnExternal()                 { return s_onext; }

    bool Finished();

    std::vector<double> GetMaxPos() const;
    std::vector<double> GetMeanPos() const;

    double *GetBinsMean(int *) const;

    inline std::string Name() const { return m_name; }

    inline int GetNBins() const { return m_nd; }
    inline int GetNDims() const { return m_dim; }

    inline int    *GetPointBins() const     { return p_cb; }
    inline double *GetPointBinsMean() const { return p_cx; }

    inline void SetOutputMode(const int mode) { m_omode=mode; }
    inline void SetCheckMode(const int mode)  { m_cmode=mode; }

  };

}

#endif
