#include "ad_type.h"

#ifndef GEOMETRYSPHERE_HH
#define GEOMETRYSPHERE_HH

/**
 * @file    GeometrySphere.hh
 * @class   GeometrySphere
 * @author  Y. Zhao
 * @date    Aug 2025
 *
 * @brief Radially-layered spherical calorimeter geometry.
 *
 * The detector consists of `fNumLayers` concentric sampling layers.
 * Each layer contains
 *   – an **absorber** spherical shell of thickness `fAbsThick`, then
 *   – a **gap**     spherical shell of thickness `fGapThick` (may be 0).
 *
 * Layer 0 starts at the origin (its absorber is a full sphere with
 * inner-radius 0). All volumes share the same coordinate frame, so no
 * local transforms are required during navigation.
 */

#include <vector>

class Sphere;   // forward declaration

class GeometrySphere
{
public:
  GeometrySphere();                ///< construct default GeometrySphere, allocate spheres
  ~GeometrySphere();               ///< delete all allocated spheres

  // ---------------- configuration setters / getters ----------------
  void      SetNumLayers(int n)           { if (n>0) { fNumLayers = n; UpdateParameters(); } }
  int       GetNumLayers()        const   { return fNumLayers; }

  void      SetAbsThick(G4double t)       { fAbsThick = t; UpdateParameters(); }
  G4double  GetAbsThick()         const   { return fAbsThick; }

  void      SetGapThick(G4double t)       { fGapThick = t; UpdateParameters(); }
  G4double  GetGapThick()         const   { return fGapThick; }

  G4double  GetCaloThick()        const   { return fCaloThick; }

  // Radial “x-axis” helpers kept for primary generator compatibility
  G4double  GetPrimaryXposition() const   { return fPrimaryXPosition; }
  G4double  GetCaloStartXposition() const { return fCaloStartX; }

  /**
   * Locate a point and return distance to the next spherical boundary.
   * @param[in,out] r            global position (will stay unchanged: no transforms)
   * @param[in]     v            normalised direction
   * @param[out]    currentVol   deepest volume (Sphere*) containing the point
   * @param[out]    indxLayer    index of radial layer (0 … fNumLayers-1) or −1
   * @param[out]    indxAbs      0 for absorber, 1 for gap, −1 otherwise
   * @return distance [mm] to the first boundary along v
   */
  G4double CalculateDistanceToOut(G4double* r, G4double* v,
                                  Sphere** currentVol,
                                  int* indxLayer, int* indxAbs);

private:
  /// Recompute all derived sizes and (re)allocate shells after a parameter change
  void UpdateParameters();

  // ---------------- user parameters ----------------
  int       fNumLayers;     ///< number of (Abs+Gap) layers
  G4double  fAbsThick;      ///< absorber shell thickness [mm]
  G4double  fGapThick;      ///< gap      shell thickness [mm]

  // ---------------- derived parameters -------------
  G4double  fCaloThick;         ///< total calorimeter diameter [mm] (2 × outerR)
  G4double  fCaloStartX;        ///< –outerR  (kept for legacy interfaces)
  G4double  fPrimaryXPosition;  ///< suggested primary start x (world/ calo mid-pt)

  // ---------------- volume containers --------------
  Sphere*              fSphereWorld;  ///< single vacuum world sphere
  std::vector<Sphere*> fSphereAbs;    ///< size = fNumLayers
  std::vector<Sphere*> fSphereGap;    ///< size = fNumLayers (empty if fGapThick==0)
};

#endif // GEOMETRYSPHERE_HH
