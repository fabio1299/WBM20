/******************************************************************************
 GHAAS Water Balance/Transport Model
 Global Hydrological Archive and Analysis System
 Copyright 1994-2022, UNH - ASRC/CUNY

 MDCommon_HumiditySaturatedVP.c

 amiara@ccny.cuny.edu

 *******************************************************************************/

/********************************************************************************
 * Calculates Specific Humidity from pressure, air temperature and rh
 * ******************************************************************************/

#include <math.h>
#include <MF.h>
#include <MD.h>

// Input
static int _MDInCommon_AirTemperatureID    = MFUnset;
// Output
static int _MDOutCommon_HumiditySaturatedVaporPressID = MFUnset;

static void _MDCommon_HumiditySaturatedVaporPressure (int itemID) {
    float airTemp;     // Air temperature in degC
    float saturatedVP; // Saturated vapor pressure in Pa

    saturatedVP = airTemp > 0.0 ? 611 * exp (17.27 * airTemp / (airTemp + 237.3))  // Over water
                                : 611 * exp (21.87 * airTemp / (airTemp + 265.5)); // Over ice
    
    MFVarSetFloat(_MDOutCommon_HumiditySaturatedVaporPressID, itemID, saturatedVP);
}

int MDCommon_HumiditySaturatedVaporPressureDef () {
    int optID = MFinput;
    const char *optStr;
     
    if (_MDOutCommon_HumiditySaturatedVaporPressID != MFUnset) return (_MDOutCommon_HumiditySaturatedVaporPressID);    

    MFDefEntering ("RelativeHumidity");
    if ((optStr = MFOptionGet(MDVarCommon_HumiditySaturatedVaporPress)) != (char *) NULL) optID = CMoptLookup(MFsourceOptions, optStr, true);
    switch (optID) {
        default:
        case MFhelp: MFOptionMessage (MDVarCommon_HumiditySaturatedVaporPress, optStr, MFsourceOptions); return (CMfailed);
        case MFinput: _MDOutCommon_HumiditySaturatedVaporPressID = MFVarGetID (MDVarCommon_HumiditySaturatedVaporPress, "Pa", MFInput, MFState, MFBoundary); break;
        case MFcalculate:
            if (((_MDInCommon_AirTemperatureID               = MDCommon_AirTemperatureDef ()) == CMfailed) ||
                ((_MDOutCommon_HumiditySaturatedVaporPressID = MFVarGetID (MDVarCommon_HumiditySaturatedVaporPress, "Pa", MFOutput, MFState, MFBoundary)) == CMfailed) ||
                ((MFModelAddFunction (_MDCommon_HumiditySaturatedVaporPressure) == CMfailed))) return (CMfailed);
            break;
    }
    MFDefLeaving ("RelativeHumidity");
    return (_MDOutCommon_HumiditySaturatedVaporPressID);
}