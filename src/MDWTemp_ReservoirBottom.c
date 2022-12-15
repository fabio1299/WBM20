/******************************************************************************
GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2022, UNH - ASRC/CUNY

MDWTemp_ReservoirBottom.c

bfekete@gc.cuny.edu

*******************************************************************************/

#include <string.h>
#include <math.h>
#include <MF.h>
#include <MD.h>
#include <res_strat.h>

// Input
static int _MDInAux_StepCounterID       = MFUnset;
static int _MDInCommon_SolarRadID       = MFUnset;
static int _MDInCommon_RadAbsortionID   = MFUnset;
static int _MDInCommon_AirTemperatureID = MFUnset;
static int _MDInCommon_HumidityRelID    = MFUnset;
static int _MDInCommon_WindSpeedID      = MFUnset;
static int _MDInReservoir_InflowID      = MFUnset;
static int _MDInReservoir_ReleaseID     = MFUnset;
static int _MDInReservoir_StorageID     = MFUnset;
static int _MDInWTemp_RiverTopID        = MFUnset;
static int _MDInStrat_GMjID             = MFUnset;
static int _MDInStrat_DepthID           = MFUnset;
static int _MDInStrat_HeightID          = MFUnset;
static int _MDInStrat_LengthID          = MFUnset;
static int _MDInStrat_WidthID           = MFUnset;
static int _MDInStrat_VolumeErrID       = MFUnset;
static int _MDInStrat_AreaErrID         = MFUnset;
static int _MDInStrat_VolumeCoeffID     = MFUnset;
static int _MDInStrat_AreaCoeffID       = MFUnset;
static int _MDInStrat_VolumeDiffID      = MFUnset;
static int _MDInStrat_AreaDiffID        = MFUnset;
// State
static int _MDStateStrat_dZ  [NLAYER_MAX] = { MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset }; 
static int _MDStateStrat_tZ  [NLAYER_MAX] = { MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset };
static int _MDStateStrat_aD  [NLAYER_MAX] = { MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset };
static int _MDStateStrat_mZn [NLAYER_MAX] = { MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset };
static int _MDStateStrat_dV  [NLAYER_MAX] = { MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset };
static int _MDStateStrat_vZt [NLAYER_MAX] = { MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset, MFUnset };
// Output
static int _MDOutWTemp_ReservoirBottomID = MFUnset;

#define MinTemp 1.0

static void _MDWTempReservoirBottom (int itemID) {
// Input
    double inflow        = MFVarGetFloat (_MDInReservoir_InflowID,      itemID, 0.0); // Innflow in m3/s 
    double release       = MFVarGetFloat (_MDInReservoir_ReleaseID,     itemID, 0.0); // Release in m3/s 
    double storage       = MFVarGetFloat (_MDInReservoir_StorageID,     itemID, 0.0); // Reservoir storage km3
   	double riverTempTop  = MFVarGetFloat (_MDInWTemp_RiverTopID,        itemID, 0.0); // Runoff temperature degC
    double airTemp       = MFVarGetFloat (_MDInCommon_AirTemperatureID, itemID, 0.0); // Air temperature degC
    double humidityRel   = MFVarGetFloat (_MDInCommon_HumidityRelID,    itemID, 0.0); // Relative humdity %
    double windSpeed     = MFVarGetFloat (_MDInCommon_WindSpeedID,      itemID, 0.0); // Wind speed m/s
    double solarRad      = MFVarGetFloat (_MDInCommon_SolarRadID,       itemID, 0.0); // Solar radiation W/m2 
    double radAbsorption = MFVarGetFloat (_MDInCommon_RadAbsortionID,   itemID, 0.0); // Radiation absoption W/m2 
    struct reservoir_geometry resGeom;
// Output
    float riverTempBottom; // River bottom temprature in degC
// Model
    float dt = MFModelGet_dt (); // Model time step in seconds
// Local
    int tStep, layer;
    double dZ[NLAYER_MAX], tZ[NLAYER_MAX], aD[NLAYER_MAX], mZn[NLAYER_MAX], dV[NLAYER_MAX], vZt[NLAYER_MAX];
    double s_tin;
    double m_cal;
    int lme_error;
    int   day    = MFDateGetDayOfYear ();
	float lambda = MFModelGetLatitude (itemID);
    float sigma  = -23.4 * cos (2.0 * M_PI * (day + 11.0) / 365.25);
    double cosZen = cos ((lambda + sigma) * M_PI / 360.0);

	tStep           = MFVarGetInt   (_MDInAux_StepCounterID,   itemID, 0);
    resGeom.gm_j    = MFVarGetInt   (_MDInStrat_GMjID,         itemID, 0);
    resGeom.depth   = MFVarGetFloat (_MDInStrat_DepthID,       itemID, 0.0);
    resGeom.d_ht    = MFVarGetFloat (_MDInStrat_HeightID,      itemID, 0.0);
    resGeom.M_L     = MFVarGetFloat (_MDInStrat_LengthID,      itemID, 0.0);
    resGeom.M_W     = MFVarGetFloat (_MDInStrat_WidthID,       itemID, 0.0);
    resGeom.V_err   = MFVarGetFloat (_MDInStrat_VolumeErrID,   itemID, 0.0);
    resGeom.Ar_err  = MFVarGetFloat (_MDInStrat_AreaErrID,     itemID, 0.0);
    resGeom.C_v     = MFVarGetFloat (_MDInStrat_VolumeCoeffID, itemID, 0.0);
    resGeom.C_a     = MFVarGetFloat (_MDInStrat_AreaCoeffID,   itemID, 0.0);
    resGeom.V_df    = MFVarGetFloat (_MDInStrat_VolumeDiffID,  itemID, 0.0);
    resGeom.A_df    = MFVarGetFloat (_MDInStrat_VolumeDiffID,  itemID, 0.0);
    for (layer = 0; layer < NLAYER_MAX; ++layer) {
        dZ [layer] = MFVarGetFloat (_MDStateStrat_dZ [layer], itemID, 0.0);
        tZ [layer] = MFVarGetFloat (_MDStateStrat_tZ [layer], itemID, 0.0);
        aD [layer] = MFVarGetFloat (_MDStateStrat_aD [layer], itemID, 0.0);
        mZn[layer] = MFVarGetFloat (_MDStateStrat_mZn[layer], itemID, 0.0);
        dV [layer] = MFVarGetFloat (_MDStateStrat_dV [layer], itemID, 0.0);
        vZt[layer] = MFVarGetFloat (_MDStateStrat_vZt[layer], itemID, 0.0);
    }
    stratify(tStep + 1, &lme_error, &riverTempTop, &inflow, &release,
             &cosZen, &radAbsorption, &solarRad, &humidityRel, &airTemp, &windSpeed,
             &resGeom, (double **) &dZ, (double **) &tZ, (double **) &mZn, (double **) &aD, (double **) &dV, (double **) &vZt, &s_tin, &m_cal);
    MFVarSetFloat(_MDOutWTemp_ReservoirBottomID, itemID, riverTempBottom);
    for (layer = 0; layer < NLAYER_MAX; ++layer) {
        MFVarSetFloat (_MDStateStrat_dZ [layer], itemID, dZ [layer]);
        MFVarSetFloat (_MDStateStrat_tZ [layer], itemID, tZ [layer]);
        MFVarGetFloat (_MDStateStrat_aD [layer], itemID, aD [layer]);
        MFVarGetFloat (_MDStateStrat_mZn[layer], itemID, mZn[layer]);
        MFVarGetFloat (_MDStateStrat_dV [layer], itemID, dV [layer]);
        MFVarGetFloat (_MDStateStrat_vZt[layer], itemID, vZt[layer]);
    }
}

int MDWTemp_ReservoirBottomDef () {
    int i;
    char stateName [64];
	if (_MDOutWTemp_ReservoirBottomID != MFUnset) return (_MDOutWTemp_ReservoirBottomID);

	MFDefEntering ("Reservoir bottom temperature");
	if (((_MDInAux_StepCounterID        = MDAux_StepCounterDef ())         == CMfailed) ||
        ((_MDInReservoir_InflowID       = MDReservoir_InflowDef ())        == CMfailed) ||
        ((_MDInReservoir_ReleaseID      = MDReservoir_OperationDef ())     == CMfailed) ||
        ((_MDInReservoir_StorageID      = MDReservoir_StorageDef ())       == CMfailed) ||
        ((_MDInWTemp_RiverTopID         = MDWTemp_RiverTopDef ())          == CMfailed) ||
        ((_MDInCommon_AirTemperatureID  = MDCommon_AirTemperatureDef ())   == CMfailed) ||
        ((_MDInCommon_HumidityRelID     = MDCommon_HumidityRelativeDef ()) == CMfailed) ||
        ((_MDInCommon_SolarRadID        = MDCommon_SolarRadDef ())         == CMfailed) ||
        ((_MDInCommon_RadAbsortionID    = MFVarGetID ("RadiationAbsoption",   "W/m2",   MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDInCommon_WindSpeedID       = MFVarGetID (MDVarCommon_WindSpeed,  "m/s",    MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDInStrat_GMjID              = MFVarGetID ("ReservoirGeometry",    MFNoUnit, MFInt,    MFState, MFBoundary)) == CMfailed) ||
        ((_MDInStrat_DepthID            = MFVarGetID ("ReservoirMeanDepth",   "m",      MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDInStrat_HeightID           = MFVarGetID ("ReservoirDamHeight",   "m",      MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDInStrat_LengthID           = MFVarGetID ("ReservoirLength",      "km",     MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDInStrat_WidthID            = MFVarGetID ("ReservoirWidth",       "km",     MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDInStrat_VolumeErrID        = MFVarGetID ("ReservoirVolumeError", "%",      MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDInStrat_AreaErrID          = MFVarGetID ("ReservoirAreaError",   "%",      MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDInStrat_VolumeCoeffID      = MFVarGetID ("ReservoirVolumeCoeff", MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDInStrat_AreaCoeffID        = MFVarGetID ("ReservoirAreaCoeff",   MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDInStrat_VolumeDiffID       = MFVarGetID ("ReservoirVolumeDiff",  "mcm",    MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDInStrat_AreaDiffID         = MFVarGetID ("ReservoirAreaDiff",    "km2",    MFInput,  MFState, MFBoundary)) == CMfailed) ||
        ((_MDOutWTemp_ReservoirBottomID = MFVarGetID (MDVarWTemp_RiverTop,    "degC",   MFOutput, MFState, MFBoundary)) == CMfailed) ||
        (MFModelAddFunction (_MDWTempReservoirBottom) == CMfailed)) return (CMfailed);
    for (i = 0; i < NLAYER_MAX; ++i) {
        if ((snprintf (stateName,strlen(stateName), "dZ%02d", i) == 0) || ((_MDStateStrat_dZ  [i] = MFVarGetID (stateName, MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
            (snprintf (stateName,strlen(stateName), "tZ%02d", i) == 0) || ((_MDStateStrat_tZ  [i] = MFVarGetID (stateName, MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
            (snprintf (stateName,strlen(stateName), "aD%02d", i) == 0) || ((_MDStateStrat_aD  [i] = MFVarGetID (stateName, MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
            (snprintf (stateName,strlen(stateName),"mZn%02d", i) == 0) || ((_MDStateStrat_mZn [i] = MFVarGetID (stateName, MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
            (snprintf (stateName,strlen(stateName), "dV%02d", i) == 0) || ((_MDStateStrat_dV  [i] = MFVarGetID (stateName, MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed) ||
            (snprintf (stateName,strlen(stateName),"vZt%02d", i) == 0) || ((_MDStateStrat_vZt [i] = MFVarGetID (stateName, MFNoUnit, MFInput,  MFState, MFBoundary)) == CMfailed))
            return (CMfailed);
    }
	MFDefLeaving ("Reservoir bottom temperature");
	return (_MDOutWTemp_ReservoirBottomID);
}