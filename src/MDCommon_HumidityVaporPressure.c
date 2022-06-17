/******************************************************************************
 GHAAS Water Balance/Transport Model
 Global Hydrological Archive and Analysis System
 Copyright 1994-2022, UNH - ASRC/CUNY

 MDCommon_HumidityVaporPressure.c

 amiara@ccny.cuny.edu

 *******************************************************************************/

/********************************************************************************
 * Calculates Specific Humidity from pressure, air temperature and rh
 * ******************************************************************************/

#include <math.h>
#include <MF.h>
#include <MD.h>

// Input
static int _MDInCommon_HumiditySpecificID       = MFUnset;
static int _MDInCommon_AirPressureID            = MFUnset;
// Output
static int _MDOutCommon_HumidityVaporPressureID = MFUnset;

static void _MDCommon_HumidityVaporPressure (int itemID) {
    float specificHumidity; // Specific humidity in percent
    float airpressure;      // Air pressure in Pa
    float vaporPressure;    // Vapor pressure in Pa

    specificHumidity = MFVarGetFloat (_MDInCommon_HumiditySpecificID, itemID, 0.0) / 100.0;
    airpressure      = MFVarGetFloat (_MDInCommon_AirPressureID,      itemID, 0.0); // air pressure (Pa)

    vaporPressure = specificHumidity * airpressure / (0.622 + specificHumidity);
    MFVarSetFloat(_MDOutCommon_HumidityVaporPressureID, itemID, vaporPressure);
}

int MDCommon_HumidityVaporPressureDef () {
    int optID = MFinput;
    const char *optStr;
     
    if (_MDOutCommon_HumidityVaporPressureID != MFUnset) return (_MDOutCommon_HumidityVaporPressureID);    

    MFDefEntering ("VaporPressure");
    if ((optStr = MFOptionGet(MDVarCommon_HumidityVaporPressure)) != (char *) NULL) optID = CMoptLookup(MFsourceOptions, optStr, true);
    switch (optID) {
        default:      MFOptionMessage (MDVarCommon_HumidityVaporPressure, optStr, MFsourceOptions); return (CMfailed);
        case MFhelp:  MFOptionMessage (MDVarCommon_HumidityVaporPressure, optStr, MFsourceOptions);
        case MFinput: _MDOutCommon_HumidityVaporPressureID = MFVarGetID (MDVarCommon_HumidityVaporPressure, "Pa", MFInput, MFState, MFBoundary); break;
        case MFcalculate:
            if (((_MDInCommon_HumiditySpecificID       = MDCommon_HumiditySpecificDef ()) == CMfailed) ||
                ((_MDInCommon_AirPressureID            = MFVarGetID (MDVarCommon_AirPressure,           "Pa", MFInput,  MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutCommon_HumidityVaporPressureID = MFVarGetID (MDVarCommon_HumidityVaporPressure, "Pa", MFOutput, MFState, MFBoundary)) == CMfailed) ||
                ((MFModelAddFunction (_MDCommon_HumidityVaporPressure) == CMfailed))) return (CMfailed);
            break;
    }
    MFDefLeaving ("VaporPressure");
    return (_MDOutCommon_HumidityVaporPressureID);
}
