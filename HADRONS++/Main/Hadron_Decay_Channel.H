#ifndef HADRONS_Main_Hadron_Decay_Channel_H
#define HADRONS_Main_Hadron_Decay_Channel_H

#include <string>
#include <utility>
#include "HADRONS++/Main/Tools.H"
#include "PHASIC++/Decays/Decay_Channel.H"
#include "ATOOLS/Math/Vector.H"
#include "ATOOLS/Org/Scoped_Settings.H"

namespace ATOOLS {
  class Blob;
}

namespace HADRONS {
  class Current_Base;
  class HD_ME_Base;
  class HD_PS_Base;

  /** 
   * Building on the general class Decay_Channel this class implements the
   * features specific for hadron decay channel, including extra properties
   * necessary for mixing phenomena and for reading information from decay
   * channel files.
   */
  class Hadron_Decay_Channel : public PHASIC::Decay_Channel {
    /// Origin of branching ratio/error
    std::string             m_origin;
    /// Whether to ignore stored integration results
    bool                    m_always_integrate;
    /// CP asymmetry coefficient C as used in PDG B0 listings, used to
    /// simulate CP rate asymmetries in this decay channel
    double                  m_cp_asymmetry_C;
    /// CP asymmetry coefficient S as used in PDG B0 listings, used to
    /// simulate CP rate asymmetries in this decay channel
    double                  m_cp_asymmetry_S;
    /// CP asymmetry coefficient lambda, used to
    /// simulate CP rate asymmetries in this decay channel
    Complex                 m_cp_asymmetry_lambda;
    /// global hadron decay parameters
    GeneralModel            m_startmd;
    /// mapping of alias flavours to physical flavours
    ATOOLS::Flavour_Vector  m_physicalflavours;

    
    /** 
     * Add (automatic) partonic channels settings
     * 
     * @param s the channel settings block from HADRON_DECAYS: { <decayer>: {channel: {...}}}
     */
    void   ProcessPartonic(ATOOLS::Scoped_Settings& s);

    /** 
     * Select a matrix element from a string like B_KStar_Semileptonic[0,1,2,3]
     * using the Getter_Function for HD_ME_Base.
     * 
     * @param me_string String containing the (getter-able) name of the matrix
     * element, and in square brackets the mapping of external particle
     * indices in the decay blob to internal indices in the matrix element.
     * @param s the ME settings block from HADRON_DECAYS: { <decayer>: {channel: { ME: {...}}}}
     * 
     * @return The matrix element it has created from the information. If
     * it can't find a valid matrix element, it will abort.
     */
    HD_ME_Base            * SelectME(std::string me_string, ATOOLS::Scoped_Settings s);

  public:
    /** 
     * Nothing initialised here, only created.
     * 
     * @param flin Incoming particle flavour
     * @param ms mass setter to be used for flavours in this channel
     */
    Hadron_Decay_Channel(ATOOLS::Flavour flin, const ATOOLS::Mass_Selector* ms);
    ~Hadron_Decay_Channel();

    /** 
     * Does the actual read-in of the decay channel file, and creates matrix
     * elements, phase space mappings etc. from that.
     * 
     * @param startmd Global hadron decay parameters.
     * @param s the channel settings block from HADRON_DECAYS: { <decayer>: {channel: {...}}}
     */
    void   Initialise(ATOOLS::Scoped_Settings s, GeneralModel startmd);

    /** 
     * Tries to set the color flow for the outparticles of the blob given.
     * This is needed for
     * partonic decay channels, after which the parton shower is started.
     * Either a general color flow assignment is used, or (if possible) the
     * matrix element can set it.
     * 
     * @param blob Blob to set color flow for
     * 
     * @return true if setting the color flow succeeded, false if not.
     */
    bool SetColorFlow(ATOOLS::Blob* blob);

    /** 
     * Outputs the Latex lines for one decay channel (to be embedded in a
     * decay table Latex output)
     * 
     * @param f Stream to write to.
     * @param totalwidth Total width of the decay table (such that branching
     * ratios can be printed).
     */
    void LatexOutput(std::ostream& f, double totalwidth);

    /** 
     * Takes a string e.g. of the form Channel_1_resonance_23 and finds the
     * appropriate phase space mapping for that. This PHASIC::Single_Channel is
     * then added to the decay channel's PHASIC::Multi_Channel.
     * 
     * @param name String describing the phase space channel
     * @param s the PS settings block from HADRON_DECAYS: { <decayer>: {channel: { PhaseSpace: {...}}}}
     * 
     * @return true if an appropriate channel was found, false if not.
     */
    bool AddPSChannel(std::string name, ATOOLS::Scoped_Settings s);

    //@{
    /** Getter/setter method. */
    inline const double& CPAsymmetryC() const { return m_cp_asymmetry_C; }
    inline const double& CPAsymmetryS() const { return m_cp_asymmetry_S; }
    inline const Complex&
    CPAsymmetryLambda() const { return m_cp_asymmetry_lambda; }
    inline const std::string& Origin() const { return m_origin; }
    inline void SetOrigin(const std::string& origin) { m_origin=origin; }    
    //@}
  };  
}


#endif
