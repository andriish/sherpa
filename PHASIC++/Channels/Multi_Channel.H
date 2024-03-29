#ifndef PHASIC_Channels_Multi_Channel_H
#define PHASIC_Channels_Multi_Channel_H

#include "ATOOLS/Org/Info_Key.H"

namespace PHASIC {

  class Cut_Data;
  class Single_Channel;

  struct channel_type {
    enum code {
      simple     = 0,
      resonance  = 1,
      threshold  = 2,
      leadinglog = 3,
      laserback  = 4,
      exponential= 5,
      unknown    = 99
    };
  };

  struct Channel_Info {
    channel_type::code   type;
    std::vector <double> parameters;

    Channel_Info(const channel_type::code & ctype=channel_type::unknown) :
      type(ctype) {}
    Channel_Info(const channel_type::code & ctype,const double & p0) :
      type(ctype) { parameters.push_back(p0); }
    Channel_Info(const channel_type::code & ctype,const double & p0,
		 const double & p1) : type(ctype)
    { parameters.push_back(p0); parameters.push_back(p1); }
    Channel_Info(const channel_type::code & ctype,const double & p0,
		 const double & p1,const double & p2) : type(ctype)
    { parameters.push_back(p0); parameters.push_back(p1); parameters.push_back(p2); }

    bool operator==(const Channel_Info & ci) {
      if (type != ci.type)                           return 0;
      if (parameters.size() != ci.parameters.size()) return 0;
      for (unsigned int i=0;i<parameters.size();i++) {
	if (parameters[i] != ci.parameters[i])       return 0;
      }
      return 1;
    }
    bool operator!=(const Channel_Info & ci) { return (!(operator==(ci))); }
  };

  typedef std::vector<PHASIC::Channel_Info> channelinfos;

  class Multi_Channel {
  protected :
    int                           nin, nout;
    std::string                   name;
    long unsigned int             n_points,n_contrib;
    long unsigned int             mn_points,mn_contrib;
    double                        m_weight;
    double                        rans[2];
    double                        s1xmin, * s1;
    std::vector<Single_Channel *> channels;
    std::map<std::string,double>  m_erans;
    bool m_readin;
    int m_lastdice;
    int m_otype;
  public:
    Multi_Channel(std::string name);
    virtual ~Multi_Channel();

    virtual bool Initialize();

    void             Add(Single_Channel *);
    virtual void     DropAllChannels(const bool del=true);
    void             DropChannel(int);
    Single_Channel * Channel(int);
    virtual void     Reset();
    virtual void     Optimize(double);
    virtual void     MPISync();

    inline std::vector<Single_Channel*> &Channels() { return channels; }

    /*!
      Endoptimize replaces, after an ordinary optimization step, the set of a-priori weights
      with the best set so far which has been stored during the optimization procedure.
      Channels, whose a-priori weight drops under a critical value are discarded
      in the follwoing by setting their a-priori weights to 0.
    */
    std::string SelectChannel();

    virtual void EndOptimize(double);
    virtual bool OptimizationFinished();

    virtual void AddPoint(double);
    virtual void GeneratePoint(ATOOLS::Vec4D *,Cut_Data *);
    virtual void GenerateWeight(ATOOLS::Vec4D *,Cut_Data *);

    void GeneratePoint();
    void GenerateWeight();

    virtual void ISRInfo(int,int &,double &,double &);
    virtual void ISRInfo(std::vector<int> &ts,
                         std::vector<double> &ms,std::vector<double> &ws) const;

    inline  Single_Channel *operator[](size_t i) { return channels[i]; }

    virtual void    Print();
    virtual void    WriteOut(std::string);
    virtual bool    ReadIn(std::string);

    size_t NChannels() const;

    virtual size_t              Number()    { return channels.size(); }
    inline long unsigned int    N()         { return n_points; }
    inline long unsigned int    ValidN()    { return n_contrib; }
    inline long unsigned int    ValidMN()   { return mn_contrib; }
    inline int                  Nin()       { return nin; }
    inline int                  Nout()      { return nout; }
    inline double               Weight()    { return m_weight; }
    inline std::string          Name()      { return name; }
    std::string                 ChID(int);
    Single_Channel*             LastChannel() { if (m_lastdice>=0) return Channel(m_lastdice); return 0;}
    void                        NoGenerate()    { m_lastdice=-1; }

    inline void SetNin(int _nin)   { nin=_nin; }
    inline void SetNout(int _nout) { nout=_nout; }

    int                         OType() const  { return m_otype; }

    virtual int                 HandicapFactor() { return 1; }
    inline void AddERan(const std::string &id) { m_erans[id]=0.0; }
    inline double ERan(const std::string &id) const { return m_erans.find(id)->second; }
  };


  /*!
    This is the Multi_Channel, AMEGICs preferred integration
    tool. The idea is to have a vector of Single_Channels
    that are repeatedly called according to their a priori
    weights (alpha in the channels). These weights are optimized
    during integration such that the overall variance is minimized.
  */
  /*!
    Some basic ingredients for all channels :
    numbers of legs and their flavours (not needed for isr)
    and the name of the channel. This name might be given explicitly
    (for instance for ISR_Channels) or it may be constructed from amplitudes
    in the Phase_Space/Channel_Generator.
  */
  /*!
    Stuff for the immediate integration
  */
  /*!
    Managing the multichannel, adding and dropping channels as well as
    access to individual channels.
  */
  /*!
    Reset does a complete reset of the multi-channel. All a-priori weights are reset
    to uniform probabilities, the individual channels are reset, and the actual weights
    of all channels are set to 0. Furthermore, the result and the result2 within
    the multi-channel are set to 0, the minimal spread so far is set to some
    arbitrary high value. If not existing so far, spread-vectors are initialized.
  */
  /*!
    Here the counters within an iterations are reset to 0.
    I doubt whether we need this.
  */
  /*!
    Optimzing the a-priori weights of the multi-channel.
    The idea is to compare the individual variance with their average and
    reshuffle the a-priori weights such that all variances would equal the
    average. What I find strange in this method at the moment is the occurence
    of s2 ... . I'll have to check this ... .
  */
  /*!
    Endoptimize replaces, after an ordinary optimization step, the set of a-priori weights
    with the best set so far which has been stored during the optimization procedure.
    Channels, whose a-priori weight drops under a critical value are discarded
    in the follwoing by setting their a-priori weights to 0.
  */
  /*!
    A value is added to the multi-channel, and, in due proportion, to the
    individual channels. This amounts to incrementing the results and the results squared
    of the channels, for the multi-channel, the global result(2) is incremented,
    for the individual channels within, the res(1,2,3) are incremented that are valid
    only during one iteration.
  */
  /*!
    The variance of a statistical distribution (values accumulated in result and their
    squares in result2) over a number of trials. This is equivalent to one Standard deviation
    and thus a measure for the quality of an integral-estimate.
  */
  /*!
    Methods for the generation of phase space points
  */
  /*!
    This is used for ISR Channels to set the y and s' range.
  */
  /*!
    Simple access methods
  */
}


#endif
