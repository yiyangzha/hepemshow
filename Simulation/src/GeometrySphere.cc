#include "ad_type.h"

#include "GeometrySphere.hh"
#include "Sphere.hh"

#include <cmath>
#include <iostream>
#include <sstream>

GeometrySphere::GeometrySphere()
{
  // ---------------- default parameters ----------------
  fNumLayers = 50;
  fAbsThick = 2.3; // [mm]
  fGapThick = 5.7; // [mm]

  // ---------------- one-time allocation ----------------
  // World (vacuum, idx 0) – placeholder radii, will be set in UpdateParameters
  fSphereWorld = new Sphere("World", 0, 0.0, 1.0);

  // Pre-allocate absorber & (optional) gap spheres for each layer
  for (int i = 0; i < fNumLayers; ++i)
  {
    std::ostringstream na;
    na << "Abs_" << i;
    fSphereAbs.push_back(new Sphere(na.str(), 1, 0.0, 1.0));

    if (fGapThick > 0.0)
    {
      std::ostringstream ng;
      ng << "Gap_" << i;
      fSphereGap.push_back(new Sphere(ng.str(), 2, 0.0, 1.0));
    }
  }

  UpdateParameters();
}

GeometrySphere::~GeometrySphere()
{
  delete fSphereWorld;
  for (auto *s : fSphereAbs)
    delete s;
  for (auto *s : fSphereGap)
    delete s;
}

//-----------------------------------------------------------------------------
// helper to grow / shrink vectors after parameter change (no leaks)
//-----------------------------------------------------------------------------
template <typename Vec>
static void ResizeShellVector(Vec &vec, int newSize,
                              const std::string &prefix, int matIdx)
{
  // shrink
  while (static_cast<int>(vec.size()) > newSize)
  {
    delete vec.back();
    vec.pop_back();
  }
  // grow
  while (static_cast<int>(vec.size()) < newSize)
  {
    std::ostringstream n;
    n << prefix << "_" << vec.size();
    vec.push_back(new Sphere(n.str(), matIdx, 0.0, 1.0));
  }
}

//-----------------------------------------------------------------------------
// UpdateParameters : recompute radii & resize vectors
//-----------------------------------------------------------------------------
void GeometrySphere::UpdateParameters()
{
  // Ensure vectors match requested sizes
  ResizeShellVector(fSphereAbs, fNumLayers, "Abs", 1);
  ResizeShellVector(fSphereGap, (fGapThick > 0) ? fNumLayers : 0, "Gap", 2);

  // Derived radial dimensions
  const G4double layerThick = fAbsThick + fGapThick;
  const G4double outerR = (fGapThick > 0.0)
                              ? fNumLayers * layerThick
                              : fNumLayers * fAbsThick;
  const G4double worldR = 1.1 * outerR;

  // --- World sphere ---
  fSphereWorld->SetRadius(0.0, 0);
  fSphereWorld->SetRadius(worldR, 1);

  // --- Set radii of each Abs / Gap shell ---
  G4double rMin = 0.0;
  for (int i = 0; i < fNumLayers; ++i)
  {
    // absorber
    G4double rMaxAbs = rMin + fAbsThick;
    fSphereAbs[i]->SetRadius(rMin, 0);
    fSphereAbs[i]->SetRadius(rMaxAbs, 1);
    rMin = rMaxAbs;

    // gap (if any)
    if (fGapThick > 0.0)
    {
      G4double rMaxGap = rMin + fGapThick;
      fSphereGap[i]->SetRadius(rMin, 0);
      fSphereGap[i]->SetRadius(rMaxGap, 1);
      rMin = rMaxGap;
    }
  }
}

//-----------------------------------------------------------------------------
// Navigation – locate volume & distance to next boundary
//-----------------------------------------------------------------------------
G4double GeometrySphere::CalculateDistanceToOut(G4double *r, G4double *v,
                                          Sphere **currentVol,
                                          int *indxLayer, int *indxAbs)
{
  // Defaults: in world
  *currentVol = fSphereWorld;
  *indxLayer = -1;
  *indxAbs = -1;

  // Radial quantities
  const G4double rMag = std::sqrt(r[0] * r[0] + r[1] * r[1] + r[2] * r[2]);
  const G4double pDotV = r[0] * v[0] + r[1] * v[1] + r[2] * v[2];

  const G4double outerR = fSphereAbs.empty()
                              ? 0.0
                              : (fGapThick > 0.0
                                     ? fSphereGap.back()->GetRadius(1)
                                     : fSphereAbs.back()->GetRadius(1));

  // --- outside calorimeter (world) ---
  if (rMag >= outerR)
  {
    return (pDotV > 0.0) ? 1.0e+20 : fSphereWorld->DistanceToOut(r, v);
  }

  // --- inside calorimeter: iterate shells ---
  for (int i = 0; i < fNumLayers; ++i)
  {
    Sphere *absS = fSphereAbs[i];
    if (rMag <= absS->GetRadius(1))
    {
      *currentVol = absS;
      *indxLayer = i;
      *indxAbs = 0;
      return absS->DistanceToOut(r, v);
    }
    if (fGapThick > 0.0)
    {
      Sphere *gapS = fSphereGap[i];
      if (rMag <= gapS->GetRadius(1))
      {
        *currentVol = gapS;
        *indxLayer = i;
        *indxAbs = 1;
        return gapS->DistanceToOut(r, v);
      }
    }
  }

  // Should not reach – fallback to world
  return fSphereWorld->DistanceToOut(r, v);
}
