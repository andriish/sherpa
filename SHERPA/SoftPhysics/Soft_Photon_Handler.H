#ifndef SHERPA_SoftPhysics_Soft_Photon_Handler_H
#define SHERPA_SoftPhysics_Soft_Photon_Handler_H

#include "ATOOLS/Phys/Momenta_Stretcher.H"
#include <string>
#include <vector>

namespace ATOOLS {
  class Blob;
  class Particle;
  typedef std::vector<Blob *> Blob_Vector;
  typedef std::vector<Particle *> Particle_Vector;
}

namespace PHOTONS {
  class Photons;
}

namespace SHERPA {

  class Resonance_Finder;
  class Matrix_Element_Handler;

  class Soft_Photon_Handler {
  private:
    bool                      m_photonsadded;
    std::string               m_name;
    ATOOLS::Momenta_Stretcher m_stretcher;
    PHOTONS::Photons        * p_yfs;
    Resonance_Finder        * p_clusterer;
    Matrix_Element_Handler  * p_mehandler;

    void BoostDecayBlob(ATOOLS::Blob * blob);
    bool CheckOnshellness(ATOOLS::Blob * blob);
  public :
    // constructor
    Soft_Photon_Handler(Matrix_Element_Handler*);
    // destructor
    ~Soft_Photon_Handler();
    bool AddRadiation(ATOOLS::Blob *);
    bool AddRadiation(ATOOLS::Particle_Vector&, ATOOLS::Blob_Vector& blobs);

    inline std::string SoftQEDGenerator() { return m_name; }
    inline size_t      AddedAnything()    { return m_photonsadded; }
    inline Resonance_Finder * Clusterer() { return p_clusterer; }
  };// end of class Soft_Photon_Handler

}// end of namespace SHERPA

#endif

