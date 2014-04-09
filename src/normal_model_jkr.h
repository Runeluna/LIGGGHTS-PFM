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
   Contributing authors:
   Christoph Kloss (JKU Linz, DCS Computing GmbH, Linz)
   Richard Berger (JKU Linz)
   Andreas Aigner (JKU Linz)
------------------------------------------------------------------------- */
#ifdef NORMAL_MODEL
NORMAL_MODEL(JKR,jkr,5)
#else
#ifndef NORMAL_MODEL_JKR_H_
#define NORMAL_MODEL_JKR_H_
#include "global_properties.h"
#include "math.h"

namespace ContactModels
{
  template<typename Style>
  class NormalModel<JKR, Style> : protected Pointers
  {
  public:
    static const int MASK = CM_REGISTER_SETTINGS | CM_CONNECT_TO_PROPERTIES | CM_COLLISION;

    NormalModel(LAMMPS * lmp, IContactHistorySetup*) : Pointers(lmp),
      Yeff(NULL),
      Geff(NULL),
      betaeff(NULL),
      coefArea(NULL)
    {
      /*NL*/ printf("JKR loaded\n");
    }

    void registerSettings(Settings & settings)
    {
      settings.registerOnOff("tangential_damping", tangential_damping, true);
    }

    void connectToProperties(PropertyRegistry & registry) {
      registry.registerProperty("Yeff", &MODEL_PARAMS::createYeff);
      registry.registerProperty("Geff", &MODEL_PARAMS::createGeff);
      registry.registerProperty("betaeff", &MODEL_PARAMS::createBetaEff);
      registry.registerProperty("cohEnergyDens", &MODEL_PARAMS::createCohesionEnergyDensity);
      registry.registerProperty("coefArea", &MODEL_PARAMS::createCoefArea);

      registry.connect("Yeff", Yeff);
      registry.connect("Geff", Geff);
      registry.connect("betaeff", betaeff);
      registry.connect("cohEnergyDens", cohEnergyDens);
      registry.connect("coefArea", coefArea);
    }

    inline void collision(CollisionData & cdata, ForceData & i_forces, ForceData & j_forces)
    {
      const int itype = cdata.itype;
      const int jtype = cdata.jtype;
      double ri = cdata.radi;
      double rj = cdata.radj;
      double reff=cdata.is_wall ? cdata.radi : (ri*rj/(ri+rj));
      double meff=cdata.meff;

      double deltan=cdata.deltan;

      double sqrtval = sqrt(reff*cdata.deltan);

      double Sn=2.*Yeff[itype][jtype]*sqrtval;
      double St=8.*Geff[itype][jtype]*sqrtval;

      // original hertz model
      double kn=4./3.*Yeff[itype][jtype]*sqrtval;
      double kt=St;
      const double sqrtFiveOverSix = 0.91287092917527685576161630466800355658790782499663875;
      double gamman=-2.*sqrtFiveOverSix*betaeff[itype][jtype]*sqrt(Sn*meff);
      double gammat=-2.*sqrtFiveOverSix*betaeff[itype][jtype]*sqrt(St*meff);
      if (!tangential_damping) gammat = 0.0;

      // convert Kn and Kt from pressure units to force/distance^2
      kn /= force->nktv2p;
      kt /= force->nktv2p;

      // area calculation (Newton faster than analytic solution) //NP modified A.A.
      double area,areaOld,areaOld2,coef;
      area = areaOld = reff;
      for (int k=0; k<10; k++) {
        areaOld = area;
        areaOld2 = areaOld*areaOld;
        coef = coefArea[itype][jtype]*reff*sqrt(areaOld);
        area = (areaOld*areaOld2 + areaOld*coef + deltan*areaOld*reff)/(2*areaOld2 - coef); //NP modified sign of deltan
        /*NL*/ //fprintf(screen,"deltan = %f, coefArea = %f, area = %f, areaOld = %f\n",deltan,coefArea[itype][jtype],area,areaOld);
        if (fabs(area-areaOld) < __DBL_EPSILON__) break;
      }
      /*NL*/ if (isnan(area)) lmp->error->all(FLERR,"Area is NAN");
      /*NL*/ //fprintf(screen,"coefArea = %f, area = %f\n",coefArea[itype][jtype],area);


      const double Fn_damping = -gamman*cdata.vn;
      const double area3 = area*area*area;
      const double Fn_contact = 4/3*Yeff[itype][jtype]*area3/reff - sqrt(16*M_PI*cohEnergyDens[itype][jtype]*Yeff[itype][jtype]*area3);
      const double Fn = Fn_contact + Fn_damping;

      cdata.Fn = Fn;
      cdata.kn = kn;
      cdata.kt = kt;
      cdata.gamman = gamman;
      cdata.gammat = gammat;

      // apply normal force
      if(cdata.is_wall) {
        const double Fn_ = Fn * cdata.area_ratio;
        i_forces.delta_F[0] = Fn_ * cdata.en[0];
        i_forces.delta_F[1] = Fn_ * cdata.en[1];
        i_forces.delta_F[2] = Fn_ * cdata.en[2];
      } else {
        i_forces.delta_F[0] = cdata.Fn * cdata.en[0];
        i_forces.delta_F[1] = cdata.Fn * cdata.en[1];
        i_forces.delta_F[2] = cdata.Fn * cdata.en[2];

        j_forces.delta_F[0] = -i_forces.delta_F[0];
        j_forces.delta_F[1] = -i_forces.delta_F[1];
        j_forces.delta_F[2] = -i_forces.delta_F[2];
      }
    }

    void noCollision(ContactData&, ForceData&, ForceData&){
      // missing non contact force!!
    }
    void beginPass(CollisionData&, ForceData&, ForceData&){}
    void endPass(CollisionData&, ForceData&, ForceData&){}

  protected:
    double ** Yeff;
    double ** Geff;
    double ** betaeff;

    bool tangential_damping;

    //NP pre-calculated coefficients for the jkr model
    double ** cohEnergyDens;
    double ** coefArea;
  };

}
#endif
#endif
