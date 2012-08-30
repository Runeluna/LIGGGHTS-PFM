/* ----------------------------------------------------------------------
   LIGGGHTS - LAMMPS Improved for General Granular and Granular Heat
   Transfer Simulations

   LIGGGHTS is part of the CFDEMproject
   www.liggghts.com | www.cfdem.com

   Christoph Kloss, christoph.kloss@cfdem.com
   Copyright 2009-2012 JKU Linz
   Copyright 2012-     DCS Computing GmbH, Linz

   LIGGGHTS is based on LAMMPS
   LAMMPS - Large-scale Atomic/Molecular Massively Parallel Simulator
   http://lammps.sandia.gov, Sandia National Laboratories
   Steve Plimpton, sjplimp@sandia.gov

   This software is distributed under the GNU General Public License.

   See the README file in the top-level directory.
------------------------------------------------------------------------- */

/* ----------------------------------------------------------------------
   Contributing authors for original version: Leo Silbert (SNL), Gary Grest (SNL)
------------------------------------------------------------------------- */

#ifdef PAIR_CLASS

PairStyle(gran/hooke/history/cohesion,PairGranHookeHistoryCohesion)

#else

#ifndef LMP_PAIR_GRAN_HOOKE_HISTORY_COHESION_H
#define LMP_PAIR_GRAN_HOOKE_HISTORY_COHESION_H

#include "pair_gran_hooke_history.h"

namespace LAMMPS_NS {

class PairGranHookeHistoryCohesion : public PairGranHookeHistory {

 friend class FixWallGranHookeHistory;
 friend class FixCheckTimestepGran;

 public:

  PairGranHookeHistoryCohesion(class LAMMPS *);
  ~PairGranHookeHistoryCohesion();

  virtual void settings(int, char **);
  virtual void init_granular(); //NP makes inits specific to certain gran style     //NP modified C.K.

  virtual void compute(int, int,int);

 protected:

  virtual void history_args(char**);
  void allocate_properties(int);

  //NP modified C.K.

  //NP define the mechanical properties for all models here, since other model classes are derived from
  //NP the PairGranHookeHistory class

  //NP these are properties defined for each atom type (each material) - thus vectors
//  class FixPropertyGlobal* Y1; //Youngs Modulus
//  class FixPropertyGlobal* v1; //Poisson's ratio
//  class FixPropertyGlobal* cohEnergyDens1; //Cohesion energy density

  //NP these are properties defined for each pair of atom types (pair of materials) - thus matrices
//  class FixPropertyGlobal* coeffRest1; //coefficient of restitution
//  class FixPropertyGlobal* coeffFrict1; //coefficient of (static) friction
//  class FixPropertyGlobal* coeffRollFrict1; //characteristic velocity needed for Linear Spring Model

  //NP these are general properties defined only once - thus scalars
//  int charVelflag;
//  class FixPropertyGlobal* charVel1; //characteristic velocity needed for Linear Spring Model

  //NP here, pre-calculated contact parameters for all possible material combinations
//  double **Yeff,**Geff,**betaeff,**veff,**cohEnergyDens,**coeffRestLog,**coeffFrict,charVel,**coeffRollFrict;

  //NP these values are now calculated from the material properties for each contact, they are thus not constant!
  //NP double kn,kt,gamman,gammat,xmu;

  virtual void addCohesionForce(int &, int &,double &,double &);

//  int cohesionflag; //NP indicates if linear cohesion model is used
//  int dampflag,rollingflag; //NP indicates if tang damping or rolling friction is used

  double kn2k2_, kn2kc_;
};

}

#endif
#endif
