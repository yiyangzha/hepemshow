#include "ad_type.h"

#ifndef SPHERE_HH
#define SPHERE_HH

/**
 * @file    Sphere.hh
 * @class   Sphere
 * @author  Y. Zhao
 * @date    Aug 2025
 *
 * @brief A simplified version of G4Sphere.
 */

#include <string>

class Sphere {

public:
  /**
   * Constructor.
   *
   * @param[in] name    Name of this volume.
   * @param[in] indxMat Index of the material this volume is filled with.
   * @param[in] rMin    Inner radius [mm]. Use 0 for a full sphere.
   * @param[in] rMax    Outer radius (> rMin) [mm].
   */
  Sphere(const std::string& name, int indxMat, G4double rMin, G4double rMax);

  /** Destructor (nothing to do). */
  ~Sphere() {}

  /** Get the name of this volume. */
  const std::string& GetName() const { return fName; }


  /** Set the material this volume is filled with.
    * @param[in]  indx Index of the material.
    */
  void SetMaterialIndx(int indx) { fMaterialIndx = indx; }

  /** Get the material this volume is filled with.
    * @return Index of the material.
    */
  int  GetMaterialIndx() const   { return fMaterialIndx; }

  /**
   * Set radius.
   * @param[in] val Radius value [mm].
   * @param[in] idx 0 --> inner, 1 --> outer.
   */
  void SetRadius(G4double val, int idx);

  /** Get radius. */
  G4double GetRadius(int idx) const;

  /**
   * Distance to the volume boundary from inside along the given direction.
   * @param[in] r Position (local coordinates).
   * @param[in] v Normalised direction.
   * @return Distance to the boundary [mm].
   */
  G4double DistanceToOut(G4double* r, G4double* v) const;

  /**
   * Safety distance to the nearest boundary from a point inside the volume.
   * @param[in] r Position (local coordinates).
   * @return Distance to the nearest boundary [mm].
   */
  G4double DistanceToOut(G4double* r) const;

private:
  /** Name of the volume. */
  const std::string fName;

  /** Material index. */
  int fMaterialIndx;

  /** Inner and outer radii [mm]. */
  G4double fRmin;
  G4double fRmax;

  /** Tolerance parameters. */
  const G4double kCarTolerance = 1.0E-9;
  G4double       fDelta;
};

#endif // SPHERE_HH
