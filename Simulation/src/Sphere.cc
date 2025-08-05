#include "ad_type.h"

#include "Sphere.hh"

#include <iostream>
#include <sstream>
#include <cmath>

Sphere::Sphere(const std::string& name, int indxMat, G4double rMin, G4double rMax)
  : fName(name),
    fMaterialIndx(indxMat),
    fRmin(rMin),
    fRmax(rMax)
{
  fDelta = 0.5 * kCarTolerance;
}

void Sphere::SetRadius(G4double val, int idx)
{
  if (val > 2*kCarTolerance) {
    if (idx == 0) {
      fRmin = val;
    } else {
      fRmax = val;
    }
  }
}

G4double Sphere::GetRadius(int idx) const { 
  return (idx == 0) ? fRmin : fRmax; 
}

G4double Sphere::DistanceToOut(G4double* p, G4double* v) const
{
  // Point coordinates and normalised direction components
  const G4double px = p[0], py = p[1], pz = p[2];
  const G4double vx = v[0], vy = v[1], vz = v[2];

  const G4double pDotV = px*vx + py*vy + pz*vz;   // P·V
  const G4double r2    = px*px + py*py + pz*pz;   // |P|^2
  const G4double rLen  = std::sqrt(r2);           // |P|

  // -----------------------------------------------------------
  // Early exits: point on/near a surface and moving outward
  // -----------------------------------------------------------
  // On or outside outer surface and moving further out
  if ( (rLen - fRmax) >= -fDelta && pDotV > 0 ) {
    return 0.0;
  }
  // On or inside inner surface and moving toward centre (leaving volume)
  if ( fRmin > 0.0 && (fRmin - rLen) >= -fDelta && pDotV < 0 ) {
    return 0.0;
  }

  // -----------------------------------------------------------
  // Solve quadratic for outer sphere intersection |P + tV| = Rmax
  // t = -P·V ± sqrt( (P·V)^2 - (|P|^2 - R^2) )
  // We want the smallest positive t > fDelta
  // -----------------------------------------------------------
  G4double tOuter = 1.0E+20;
  {
    const G4double disc = pDotV*pDotV - (r2 - fRmax*fRmax);
    if (disc > 0.0) {
      const G4double sqrtD = std::sqrt(disc);
      const G4double tCandidate = -pDotV + sqrtD; // exiting outer sphere
      if (tCandidate > fDelta) tOuter = tCandidate;
    }
  }

  // -----------------------------------------------------------
  // Intersection with inner sphere (if hollow)
  // -----------------------------------------------------------
  G4double tInner = 1.0E+20;
  if (fRmin > 0.0) {
    const G4double disc = pDotV*pDotV - (r2 - fRmin*fRmin);
    if (disc > 0.0) {
      const G4double sqrtD = std::sqrt(disc);
      // Two intersections: entering (t1) and leaving (t2) inner cavity
      const G4double t1 = -pDotV - sqrtD; // entering inner cavity
      const G4double t2 = -pDotV + sqrtD; // exiting inner cavity

      // We are in the *shell*: rLen > fRmin. So the first surface we meet when
      // moving along +V depends on sign of pDotV.
      if (t1 > fDelta) {
        tInner = t1; // moving inward toward centre
      } else if (t2 > fDelta) {
        tInner = t2; // already inside inner cavity – unlikely but safe
      }
    }
  }

  // -----------------------------------------------------------
  // Return the smallest positive intersection distance (> fDelta)
  // -----------------------------------------------------------
  G4double dist = (tOuter < tInner) ? tOuter : tInner;
  if (dist > 1.0E+20) return 0.0; // No valid intersection (numerical safety)

  //FIX start
  {
    const G4double ix = px + dist*vx;
    const G4double iy = py + dist*vy;
    const G4double iz = pz + dist*vz;
    const G4double invR   = 1.0 / std::sqrt(ix*ix + iy*iy + iz*iz);
    const G4double cosInc = (vx*ix + vy*iy + vz*iz) * invR;   // V·n
    if (std::abs(cosInc) < 0.5) {
      dist.setGradient(-999);
    }
  }
  //FIX end
  return dist;
}

// -----------------------------------------------------------------------------
//  Safety distance (minimum distance to either boundary, no direction)
// -----------------------------------------------------------------------------
G4double Sphere::DistanceToOut(G4double* p) const
{
  const G4double rLen = std::sqrt(p[0]*p[0] + p[1]*p[1] + p[2]*p[2]);

  G4double safetyOuter = fRmax - rLen;
  G4double safetyInner = (fRmin > 0.0) ? (rLen - fRmin) : safetyOuter;

  G4double dist = (safetyOuter < safetyInner) ? safetyOuter : safetyInner;
  return (dist > 0.0) ? dist : 0.0;
}
