#ifndef HADRONS_Main_Hadron_Decay_Table_H
#define HADRONS_Main_Hadron_Decay_Table_H

#include "PHASIC++/Decays/Decay_Table.H"
#include "HADRONS++/Main/Tools.H"
#include "ATOOLS/Org/Scoped_Settings.H"

namespace ATOOLS {
  class Blob;
}

namespace HADRONS {

  class Hadron_Decay_Channel;
  class Mixing_Handler;

  /** 
   * Building on the general class Decay_Table this class implements the
   * features specific for hadron decay tables, i.e. reading decay tables from
   * files.
   */
  class Hadron_Decay_Table : public PHASIC::Decay_Table {
    /// Pointer to the mixing handler. Needed for rate asymmetries.
    Mixing_Handler* p_mixinghandler;
    double m_flavwidth;

  public:
    /** 
     * Sets up the decay table structure, but doesn't fill it yet.
     * 
     * @param decayer The decay particle flavour.
     */
    Hadron_Decay_Table(ATOOLS::Flavour decayer, 
		       const ATOOLS::Mass_Selector* ms, 
                       Mixing_Handler* mh);
    ~Hadron_Decay_Table();

    /** 
     * Wrapper function.
     */
    inline Hadron_Decay_Channel* Select() {
      return (Hadron_Decay_Channel*) Decay_Table::Select();
    }

    /** 
     * Wrapper function.
     */
    inline Hadron_Decay_Channel* at(const size_t i) {
      return (Hadron_Decay_Channel*) Decay_Table::at(i);
    }

    /** 
     * Adds all decay channels and their branching ratios from the decay table
     * file to the decay table and initialises them with their settings
     * 
     * @param s Settings block from HADRON_DECAYS:Channels:<pdgid>
     */
    void Read(ATOOLS::Scoped_Settings s, GeneralModel& startmd);

    /** 
     * Outputs the Latex lines for a hadron decay table (to be embedded in a
     * decay map Latex output)
     * 
     * @param f Stream to write to.
     */
    void LatexOutput(std::ostream& f);

    /** 
     * Redefinition of Decay_Channel::Select to take into account possible
     * CP rate asymmetries due to mixing, and to require partonic decay channels
     * if a partonic decay has been rejected after fragmentation because it
     * produced an exclusive decay channel.
     * 
     * @param blob Decay blob for which to select the channel.
     * 
     * @return Decay_Channel selected according to branching ratios.
     */
    PHASIC::Decay_Channel* Select(ATOOLS::Blob* blob);

    //@{
    /** Getter/setter method */
    inline void SetMixingHandler(Mixing_Handler* m) { p_mixinghandler=m; }
    //@}
  };


}


#endif
