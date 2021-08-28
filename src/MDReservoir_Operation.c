/******************************************************************************

GHAAS Water Balance/Transport Model
Global Hydrological Archive and Analysis System
Copyright 1994-2021, UNH - ASRC/CUNY

MDReservoir_Operation.c

bfekete@gc.cuny.edu

******************************************************************************/

#include <math.h>
#include <MF.h>
#include <MD.h>

// Input
static int _MDInRouting_DischargeID      = MFUnset;
static int _MDInAux_MeanDischargeID      = MFUnset;
static int _MDInResTargetHighFlowID      = MFUnset;
static int _MDInResTargetLowFlowID       = MFUnset;
static int _MDInResCapacityID            = MFUnset;
static int _MDInResUptakeID              = MFUnset;

static int _MDInResNatInflowID           = MFUnset;
static int _MDInResNatMeanInflowID       = MFUnset;
static int _MDInNonIrrDemand             = MFUnset;
static int _MDInNonIrrMeanDemand         = MFUnset;
static int _MDInIrrDemand                = MFUnset;
static int _MDInIrrMeanDemand            = MFUnset;
static int _MDInResInitStorage           = MFUnset;

// Output
static int _MDOutResStorageID            = MFUnset;
static int _MDOutResStorageChgID         = MFUnset;
static int _MDOutResInflowID             = MFUnset;
static int _MDOutResReleaseID            = MFUnset;
static int _MDOutResExtractableReleaseID = MFUnset;

static void _MDReservoirWisser (int itemID) {

// Input
	float discharge;           // Current discharge [m3/s]
	float meanDischarge;       // Long-term mean annual discharge [m3/s]
	float resCapacity;         // Reservoir capacity [km3]
	float resUptake;           // Water uptake from reservoir [m3/s]
// Output
	float resStorage    = 0.0; // Reservoir storage [km3]
	float resStorageChg = 0.0; // Reservoir storage change [km3/dt]
	float resInflow;           // Reservoir release [m3/s] 
	float resRelease;          // Reservoir release [m3/s] 
	float resExtRelease;       // Reservoir extractable release [m3/s]
// local
	float prevResStorage;      // Reservoir storage from the previous time step [km3]
	float dt;                  // Time step length [s]
// Parameters
	float drySeasonPct = 0.60; // RJS 071511
	float wetSeasonPct = 0.16; // RJS 071511

	resRelease    =	
	resInflow     =
	discharge     = MFVarGetFloat (_MDInRouting_DischargeID,      itemID, 0.0);
	resExtRelease = MFVarGetFloat (_MDOutResExtractableReleaseID, itemID, 0.0);

	if ((resCapacity = MFVarGetFloat (_MDInResCapacityID, itemID, 0.0)) > 0.0) { 
		            dt = MFModelGet_dt ();
		 meanDischarge = MFVarGetFloat (_MDInAux_MeanDischargeID,      itemID, discharge);
		     resUptake = _MDInResUptakeID != MFUnset ? MFVarGetFloat (_MDInResUptakeID,itemID, 0.0) : 0.0; 
		prevResStorage = MFVarGetFloat(_MDOutResStorageID, itemID, 0.0);

		resRelease = discharge > meanDischarge ? wetSeasonPct * discharge : drySeasonPct * discharge  + (meanDischarge - discharge);
 		resStorage = prevResStorage + (discharge - resRelease - resUptake) * dt / 1e9;

		if (resStorage > resCapacity) {
			resRelease = (discharge - resUptake) * dt / 1e9 + prevResStorage - resCapacity;
			resRelease = resRelease * 1e9 / dt;
			resStorage = resCapacity;
		}
		else if (resStorage < 0.0) {
			resRelease = prevResStorage + (discharge + resUptake) * dt / 1e9;
			resRelease = resRelease * 1e9 / dt;
			resStorage = 0.0;
		}
		resStorageChg  = resStorage - prevResStorage;	
		resExtRelease = resRelease > discharge ? resRelease - discharge + (resExtRelease < discharge ? resExtRelease : discharge) : 0.0;
	}
	MFVarSetFloat (_MDOutResStorageID,            itemID, resStorage);
	MFVarSetFloat (_MDOutResStorageChgID,         itemID, resStorageChg);
	MFVarSetFloat (_MDOutResInflowID,             itemID, resInflow);
	MFVarSetFloat (_MDOutResReleaseID,            itemID, resRelease);
	MFVarSetFloat (_MDOutResExtractableReleaseID, itemID, resExtRelease);
}

static void _MDReservoirOptimized (int itemID) {
// Input
	float discharge;           // Current discharge [m3/s]
	float targetLowFlow;       // Target low flow [m3/s]
	float targetHighFlow;      // Target high flow [m3/s]
	float resUptake;           // Water uptake from reservoir [m3/s]
	float resCapacity;         // Reservoir capacity [km3]
// Output
	float resStorage    = 0.0; // Reservoir storage [km3]
	float resStorageChg = 0.0; // Reservoir storage change [km3/dt]
	float resInflow;           // Reservoir release [m3/s] 
	float resRelease;          // Reservoir release [m3/s] 
	float resExtRelease;       // Reservoir extractable release [m3/s]
// Local
	float prevResStorage;      // Previous reservoir storage [km3]
	float dt;                  // Time step length [s]

	resRelease     =
	resInflow      =
	discharge      = MFVarGetFloat (_MDInRouting_DischargeID,      itemID, 0.0);
	resExtRelease  = MFVarGetFloat (_MDOutResExtractableReleaseID, itemID, 0.0);

	if ((resCapacity = MFVarGetFloat (_MDInResCapacityID, itemID, 0.0)) > 0.0) {
		            dt = MFModelGet_dt ();
		prevResStorage = MFVarGetFloat (_MDOutResStorageID,            itemID, 0.0);
		targetLowFlow  = _MDInResTargetLowFlowID  != MFUnset ? MFVarGetFloat (_MDInResTargetLowFlowID,  itemID, discharge) : discharge;
		targetHighFlow = _MDInResTargetHighFlowID != MFUnset ? MFVarGetFloat (_MDInResTargetHighFlowID, itemID, discharge) : discharge;

		if (targetLowFlow > discharge)
	   		resRelease = (targetLowFlow - discharge) * dt / 1e9 < prevResStorage ? targetLowFlow : discharge + prevResStorage * 1e9 / dt;
		else if (discharge - targetHighFlow)
			resRelease = (discharge - targetHighFlow) * dt / 1e9 < (resCapacity - prevResStorage) ? targetLowFlow : discharge - (resCapacity - prevResStorage) * 1e9 / dt;
		else
			resRelease = discharge;

		resStorage = prevResStorage + (discharge - resRelease) * dt / 1e9;
		resExtRelease = resRelease > discharge ? resRelease - discharge + (resExtRelease < discharge ? resExtRelease : discharge) : 0.0;
	}
	MFVarSetFloat (_MDOutResStorageID,            itemID, resStorage); 
	MFVarSetFloat (_MDOutResStorageChgID,         itemID, resStorageChg); 
	MFVarSetFloat (_MDOutResInflowID,             itemID, resInflow);
	MFVarSetFloat (_MDOutResReleaseID,            itemID, resRelease);
	MFVarSetFloat (_MDOutResExtractableReleaseID, itemID, resExtRelease);
}

static void _MDReservoirSNL (int itemID) {
// Input
	float discharge;           // Current discharge [m3/s]
	float natInflow;           // Naturalized long-term mean daily inflow [m3/s]
	float natMeanInflow;       // Naturalzied long-term mean annual inflow [m3/s]
	float nonIrrDemand;        // Non-irrigational water demand [m3/s]
	float nonIrrMeanDemand;    // Non-irrigational mean annual water demand [m3/s]
	float irrDemand;           // irrigational water demand [m3/s]
	float irrMeanDemand;       // irrigational mean annual water demand [m3/s]
	float resCapacity;         // Reservoir capacity [km3]
	float resInitStorage;      // Reference storage dictatink actual release ratio [km3]
// Output
	float resStorage    = 0.0; // Reservoir storage [km3]
	float resStorageChg = 0.0; // Reservoir storage change [km3/dt]
	float resInflow;           // Reservoir release [m3/s] 
	float resRelease;          // Reservoir release [m3/s] 
	float resExtRelease;       // Reservoir extractable release [m3/s]
// Local
	float prevResStorage;      // Previous reservoir storage [km3]
	float releaseTarget;       // Target reservoir release [m3/s]
	float dt;                  // Time step length [s]
	float alpha = 0.85;        // Adjusting factor
	float c;                   // Residency time [yr]
	float krls;                // release ratio

	resRelease     =
	resInflow      =
	discharge      = MFVarGetFloat (_MDInRouting_DischargeID,      itemID, 0.0);
	resExtRelease  = MFVarGetFloat (_MDOutResExtractableReleaseID, itemID, 0.0);

	if ((resCapacity = MFVarGetFloat (_MDInResCapacityID, itemID, 0.0)) > 0.0) {
		            dt = MFModelGet_dt ();
		prevResStorage = MFVarGetFloat (_MDOutResStorageID,            itemID, 0.0);
		     natInflow = MFVarGetFloat (_MDInResNatInflowID,           itemID, 0.0);
		 natMeanInflow = MFVarGetFloat (_MDInResNatMeanInflowID,       itemID, 0.0);
		  nonIrrDemand = MFVarGetFloat (_MDInNonIrrDemand,             itemID, 0.0);
	  nonIrrMeanDemand = MFVarGetFloat (_MDInNonIrrMeanDemand,         itemID, 0.0);
		     irrDemand = MFVarGetFloat (_MDInIrrDemand,                itemID, 0.0);
	     irrMeanDemand = MFVarGetFloat (_MDInIrrMeanDemand,            itemID, 0.0);
	    resInitStorage = MFVarGetFloat (_MDInResInitStorage,           itemID, 0.0);

		releaseTarget = irrMeanDemand <= 0.0 ? /* Non-irrigaitonal reservoirs */ natMeanInflow :
		/* Irrigational reservoirs */ (irrDemand <= 0.5 * natMeanInflow ? irrDemand + nonIrrMeanDemand :
						  	          (resInflow + 9.0 * (nonIrrDemand + irrDemand) / (nonIrrMeanDemand + irrMeanDemand)) / 10.0); 
		
		c = natMeanInflow > 0.0 ? resCapacity / (natMeanInflow * dt / 1e9) : 1.0;
		krls = resInitStorage / (alpha * resCapacity);

		resRelease = c <= 0.5 ? pow (c / 0.5,2.0) * krls * releaseTarget + (1.0 - pow(c / 0.5,2.0)) * natInflow : krls * releaseTarget; 

		resStorage = prevResStorage + (resInflow - resRelease) * dt / 1e9;
		if (resStorage > resCapacity) {
			   resRelease += (resStorage - resCapacity) * 1e9 / dt;
			   resStorage = resCapacity; 
		}
		else if (resStorage < 0.1 * resCapacity) {
			   resRelease = (prevResStorage - 0.1 * resCapacity) * 1e9 / dt;
			   resStorage = 0.1 * resCapacity; 
		}
		resStorageChg = resStorage - prevResStorage;
		resExtRelease = resRelease - nonIrrMeanDemand < irrDemand ? irrDemand - (resRelease - nonIrrMeanDemand) : irrDemand;
	}
	MFVarSetFloat (_MDOutResStorageID,            itemID, resStorage); 
	MFVarSetFloat (_MDOutResStorageChgID,         itemID, resStorageChg); 
	MFVarSetFloat (_MDOutResInflowID,             itemID, resInflow);
	MFVarSetFloat (_MDOutResReleaseID,            itemID, resRelease);
	MFVarSetFloat (_MDOutResExtractableReleaseID, itemID, resExtRelease);
}

enum { MDhelp, MDwisser, MDoptimized, MDsnl };

int MDReservoir_OperationDef () {
	int optID = MDwisser;
	const char *optStr, *options [ ] = { MFhelpStr, "Wisser", "optimized" "SNL" , (char *) NULL };

	if (_MDOutResReleaseID != MFUnset) return (_MDOutResReleaseID);

	MFDefEntering ("Reservoirs");
	if ((optStr = MFOptionGet (MDVarReservoir_Release)) != (char *) NULL) optID = CMoptLookup (options, optStr, true);
 	switch (optID) {
		default:     MFOptionMessage (MDVarReservoir_Release, optStr, options); return (CMfailed);
		case MDhelp: MFOptionMessage (MDVarReservoir_Release, optStr, options);
		case MDwisser:
			if (((_MDInRouting_DischargeID      = MDRouting_DischargeUptakeDef  ()) == CMfailed) ||
				((_MDInResUptakeID              = MDReservoir_UptakeDef         ()) == CMfailed) ||
				((_MDInAux_MeanDischargeID      = MDAux_MeanDischargeDef        ()) == CMfailed) ||
            	((_MDInResCapacityID            = MFVarGetID (MDVarReservoir_Capacity,           "km3",  MFInput,  MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutResStorageID            = MFVarGetID (MDVarReservoir_Storage,            "km3",  MFOutput, MFState, MFInitial))  == CMfailed) ||
                ((_MDOutResStorageChgID         = MFVarGetID (MDVarReservoir_StorageChange,      "km3",  MFOutput, MFState, MFBoundary)) == CMfailed) ||
			    ((_MDOutResInflowID             = MFVarGetID (MDVarReservoir_Inflow,             "m3/s", MFOutput, MFFlux,  MFBoundary)) == CMfailed) ||
			    ((_MDOutResReleaseID            = MFVarGetID (MDVarReservoir_Release,            "m3/s", MFOutput, MFFlux,  MFBoundary)) == CMfailed) ||
 			    ((_MDOutResExtractableReleaseID = MFVarGetID (MDVarReservoir_ExtractableRelease, "m3/s", MFRoute,  MFState, MFBoundary)) == CMfailed) ||
                (MFModelAddFunction (_MDReservoirWisser) == CMfailed)) return (CMfailed);
			break;
		case MDoptimized: break;
			if (((_MDInRouting_DischargeID      = MDRouting_DischargeUptakeDef  ()) == CMfailed) ||
				((_MDInResUptakeID              = MDReservoir_UptakeDef         ()) == CMfailed) ||
				((_MDInResTargetLowFlowID       = MDReservoir_TargetLowFlowDef  ()) == CMfailed) ||
				((_MDInResTargetHighFlowID      = MDReservoir_TargetHighFlowDef ()) == CMfailed) ||                
            	((_MDInResCapacityID            = MFVarGetID (MDVarReservoir_Capacity,           "km3",  MFInput,  MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutResStorageID            = MFVarGetID (MDVarReservoir_Storage,            "km3",  MFOutput, MFState, MFInitial))  == CMfailed) ||
                ((_MDOutResStorageChgID         = MFVarGetID (MDVarReservoir_StorageChange,      "km3",  MFOutput, MFState, MFBoundary)) == CMfailed) ||
			    ((_MDOutResInflowID             = MFVarGetID (MDVarReservoir_Inflow,             "m3/s", MFOutput, MFFlux,  MFBoundary)) == CMfailed) ||
			    ((_MDOutResReleaseID            = MFVarGetID (MDVarReservoir_Release,            "m3/s", MFOutput, MFFlux,  MFBoundary)) == CMfailed) ||
 			    ((_MDOutResExtractableReleaseID = MFVarGetID (MDVarReservoir_ExtractableRelease, "m3/s", MFRoute,  MFState, MFBoundary)) == CMfailed) ||
                (MFModelAddFunction (_MDReservoirOptimized) == CMfailed)) return (CMfailed);
		case MDsnl:
			if (((_MDInRouting_DischargeID      = MDRouting_DischargeUptakeDef  ()) == CMfailed) ||
            	((_MDInResCapacityID            = MFVarGetID (MDVarReservoir_Capacity,           "km3",  MFInput,  MFState, MFBoundary)) == CMfailed) ||
			    ((_MDInResNatInflowID           = MFVarGetID (MDVarReservoir_NatInflow,          "m3/s", MFInput,  MFFlux,  MFBoundary)) == CMfailed) ||
			    ((_MDInResNatMeanInflowID       = MFVarGetID (MDVarReservoir_NatMeanInflow,      "m3/s", MFInput,  MFFlux,  MFBoundary)) == CMfailed) ||
            	((_MDInNonIrrDemand             = MFVarGetID (MDVarReservoir_NonIrrDemand,       "m3/s", MFInput,  MFFlux,  MFBoundary)) == CMfailed) ||
            	((_MDInNonIrrMeanDemand         = MFVarGetID (MDVarReservoir_NonIrrMeanDemand,   "m3/s", MFInput,  MFFlux,  MFBoundary)) == CMfailed) ||
            	((_MDInIrrDemand                = MFVarGetID (MDVarReservoir_IrrDemand,          "m3/s", MFInput,  MFFlux,  MFBoundary)) == CMfailed) ||
            	((_MDInIrrMeanDemand            = MFVarGetID (MDVarReservoir_IrrMeanDemand,      "m3/s", MFInput,  MFFlux,  MFBoundary)) == CMfailed) ||
            	((_MDInResInitStorage           = MFVarGetID (MDVarReservoir_InitStorage,        "km3",  MFInput,  MFState, MFBoundary)) == CMfailed) ||
                ((_MDOutResStorageID            = MFVarGetID (MDVarReservoir_Storage,            "km3",  MFOutput, MFState, MFInitial))  == CMfailed) ||
                ((_MDOutResStorageChgID         = MFVarGetID (MDVarReservoir_StorageChange,      "km3",  MFOutput, MFState, MFBoundary)) == CMfailed) ||
			    ((_MDOutResInflowID             = MFVarGetID (MDVarReservoir_Inflow,             "m3/s", MFOutput, MFFlux,  MFBoundary)) == CMfailed) ||
			    ((_MDOutResReleaseID            = MFVarGetID (MDVarReservoir_Release,            "m3/s", MFOutput, MFFlux,  MFBoundary)) == CMfailed) ||
                (MFModelAddFunction (_MDReservoirSNL) == CMfailed)) return (CMfailed);
			break;
	}
	MFDefLeaving ("Reservoirs");
	return (_MDOutResReleaseID); 
}
