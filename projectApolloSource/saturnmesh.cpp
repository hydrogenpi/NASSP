/***************************************************************************
  This file is part of Project Apollo - NASSP
  Copyright 2004-2005 Jean-Luc Rocca-Serra, Mark Grant

  ORBITER vessel module: generic Saturn base class
  Saturn mesh code

  Project Apollo is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Project Apollo is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Project Apollo; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  See http://nassp.sourceforge.net/license/ for more details.

  **************************** Revision History ****************************
  *	$Log$
  *	Revision 1.68  2008/01/10 05:42:20  movieman523
  *	Hopefully fix Saturn 1b mass.
  *	
  *	Revision 1.67  2007/11/26 17:59:06  movieman523
  *	Assorted tidying up of state variable structures.
  *	
  *	Revision 1.66  2007/10/18 00:23:23  movieman523
  *	Primarily doxygen changes; minimal functional change.
  *	
  *	Revision 1.65  2007/07/17 14:33:09  tschachim
  *	Added entry and post landing stuff.
  *	
  *	Revision 1.64  2007/06/23 21:20:38  dseagrav
  *	LVDC++ Update: Now with Pre-IGM guidance
  *	
  *	Revision 1.63  2007/06/08 20:08:30  tschachim
  *	Kill apex cover vessel.
  *	
  *	Revision 1.62  2007/06/06 15:02:17  tschachim
  *	OrbiterSound 3.5 support, various fixes and improvements.
  *	
  *	Revision 1.61  2007/02/18 01:35:30  dseagrav
  *	MCC / LVDC++ CHECKPOINT COMMIT. No user-visible functionality added. lvimu.cpp/h and mcc.cpp/h added.
  *	
  *	Revision 1.60  2007/02/06 18:30:18  tschachim
  *	Bugfixes docking probe, CSM/LM separation. The ASTP stuff still needs fixing though.
  *	
  *	Revision 1.59  2007/01/28 17:04:26  tschachim
  *	Bugfix docking probe.
  *	
  *	Revision 1.58  2007/01/22 15:48:16  tschachim
  *	SPS Thrust Vector Control, RHC power supply, THC clockwise switch, bugfixes.
  *	
  *	Revision 1.57  2007/01/11 07:42:24  chode99
  *	Changed CreateAirfoil input data to accomodate updated airfoil model
  *	
  *	Revision 1.56  2006/12/16 01:18:53  tschachim
  *	Bugfix ph_ullage3 delete.
  *	
  *	Revision 1.55  2006/11/13 14:47:30  tschachim
  *	New SPS engine.
  *	New ProjectApolloConfigurator.
  *	Fixed and changed camera and FOV handling.
  *	
  *	Revision 1.54  2006/08/11 20:37:46  movieman523
  *	Added HasProbe flag for docking probe.
  *	
  *	Revision 1.53  2006/08/11 19:34:47  movieman523
  *	Added code to take the docking probe with the LES on a post-abort jettison.
  *	
  *	Revision 1.52  2006/06/27 18:22:55  movieman523
  *	Added 'drogues' sound.
  *	
  *	Revision 1.51  2006/06/25 21:19:45  movieman523
  *	Lots of Doxygen updates.
  *	
  **************************************************************************/

#include "Orbitersdk.h"
#include <stdio.h>
#include <math.h>
#include "OrbiterSoundSDK35.h"
#include "soundlib.h"

#include "resource.h"

#include "nasspdefs.h"
#include "nasspsound.h"

#include "toggleswitch.h"
#include "apolloguidance.h"
#include "dsky.h"
#include "csmcomputer.h"
#include "IMU.h"
#include "lvimu.h"

#include "saturn.h"
#include "tracer.h"

#include "LES.h"

MESHHANDLE hSM;
MESHHANDLE hSMRCS;
MESHHANDLE hSMRCSLow;
MESHHANDLE hSMSPS;
MESHHANDLE hSMPanel1;
MESHHANDLE hSMPanel2;
MESHHANDLE hSMPanel3;
MESHHANDLE hSMPanel4;
MESHHANDLE hSMPanel5;
MESHHANDLE hSMPanel6;
MESHHANDLE hSMhga;
MESHHANDLE hSMCRYO;
MESHHANDLE hSMSIMBAY;
MESHHANDLE hCM;
MESHHANDLE hCM2;
MESHHANDLE hCMP;
MESHHANDLE hCMInt;
MESHHANDLE hCMVC;
MESHHANDLE hCREW;
MESHHANDLE hFHO;
MESHHANDLE hFHC;
MESHHANDLE hCM2B;
MESHHANDLE hprobe;
MESHHANDLE hprobeext;
//MESHHANDLE hCMBALLOON;
MESHHANDLE hCRB;
MESHHANDLE hApollochute;
MESHHANDLE hCMB;
MESHHANDLE hChute30;
MESHHANDLE hChute31;
MESHHANDLE hChute32;
MESHHANDLE hFHC2;
MESHHANDLE hsat5tower;
MESHHANDLE hFHO2;
MESHHANDLE hCMPEVA;

extern void CoeffFunc(double aoa, double M, double Re ,double *cl ,double *cm  ,double *cd);

#define LOAD_MESH(var, name) var = oapiLoadMeshGlobal(name);

// "O2 venting" particle streams
PARTICLESTREAMSPEC o2_venting_spec = {
	0,		// flag
	0.3,	// size
	30,		// rate
	2,	    // velocity
	0.5,    // velocity distribution
	2,		// lifetime
	0.2,	// growthrate
	0.5,    // atmslowdown 
	PARTICLESTREAMSPEC::DIFFUSE,
	PARTICLESTREAMSPEC::LVL_FLAT, 1.0, 1.0,
	PARTICLESTREAMSPEC::ATM_FLAT, 1.0, 1.0
};

static PARTICLESTREAMSPEC let_exhaust = {
	0,		// flag
	0.5,	// size
	25.0,	// rate
	30.0,	// velocity
	0.1,	// velocity distribution
	0.25,	// lifetime
	0.1,	// growthrate
	0.0,	// atmslowdown 
	PARTICLESTREAMSPEC::EMISSIVE,
	PARTICLESTREAMSPEC::LVL_PSQRT, 0.5, 1.0,
	PARTICLESTREAMSPEC::ATM_PLIN, -0.3, 1.0
};

PARTICLESTREAMSPEC dyemarker_spec = {
	0,		// flag
	0.15,	// size
	15,	    // rate
	1,	    // velocity
	0.3,    // velocity distribution
	3,		// lifetime
	0.2,	// growthrate
	0.2,    // atmslowdown 
	PARTICLESTREAMSPEC::EMISSIVE,
	PARTICLESTREAMSPEC::LVL_FLAT, 1.0, 1.0,
	PARTICLESTREAMSPEC::ATM_FLAT, 1.0, 1.0
};


void SaturnInitMeshes()

{
	LOAD_MESH(hSM, "ProjectApollo/SM-core");
	LOAD_MESH(hSMRCS, "ProjectApollo/SM-RCSHI");
	LOAD_MESH(hSMRCSLow, "ProjectApollo/SM-RCSLO");
	LOAD_MESH(hSMSPS, "ProjectApollo/SM-SPS");
	LOAD_MESH(hSMPanel1, "ProjectApollo/SM-Panel1");
	LOAD_MESH(hSMPanel2, "ProjectApollo/SM-Panel2");
	LOAD_MESH(hSMPanel3, "ProjectApollo/SM-Panel3");
	LOAD_MESH(hSMPanel4, "ProjectApollo/SM-Panel4");
	LOAD_MESH(hSMPanel5, "ProjectApollo/SM-Panel5");
	LOAD_MESH(hSMPanel6, "ProjectApollo/SM-Panel6");
	LOAD_MESH(hSMhga, "ProjectApollo/SM-HGA");
	LOAD_MESH(hSMCRYO, "ProjectApollo/SM-CRYO");
	LOAD_MESH(hSMSIMBAY, "ProjectApollo/SM-SIMBAY");
	LOAD_MESH(hCM, "ProjectApollo/CM");
	LOAD_MESH(hCM2, "ProjectApollo/CM-Recov");
	LOAD_MESH(hCMP, "ProjectApollo/CM-CMP");
	LOAD_MESH(hCMInt, "ProjectApollo/CM-Interior");
	LOAD_MESH(hCMVC, "ProjectApollo/CM-VC");
	LOAD_MESH(hCREW, "ProjectApollo/CM-CREW");
	LOAD_MESH(hFHC, "ProjectApollo/CM-HatchC");
	LOAD_MESH(hFHO, "ProjectApollo/CM-HatchO");
	LOAD_MESH(hCM2B, "ProjectApollo/CMB-Recov");
	LOAD_MESH(hprobe, "ProjectApollo/CM-Probe");
	LOAD_MESH(hprobeext, "ProjectApollo/CM-ProbeExtended");
	LOAD_MESH(hCRB, "ProjectApollo/CM-CrewRecovery");
	LOAD_MESH(hCMB, "ProjectApollo/CMB");
	LOAD_MESH(hChute30, "ProjectApollo/Apollo_2chute");
	LOAD_MESH(hChute31, "ProjectApollo/Apollo_3chuteEX");
	LOAD_MESH(hChute32, "ProjectApollo/Apollo_3chuteHD");
	LOAD_MESH(hApollochute, "ProjectApollo/Apollo_3chute");
	LOAD_MESH(hFHC2, "ProjectApollo/CMB-HatchC");
	LOAD_MESH(hsat5tower, "ProjectApollo/BoostCover");
	LOAD_MESH(hFHO2, "ProjectApollo/CMB-HatchO");
	LOAD_MESH(hCMPEVA, "ProjectApollo/CM-CMPEVA");

	SURFHANDLE contrail_tex = oapiRegisterParticleTexture ("Contrail2");
	let_exhaust.tex = contrail_tex;
}

void Saturn::AddSM(double offset, bool showSPS)

{
	VECTOR3 mesh_dir=_V(0, SMVO, offset);

	AddMesh (hSM, &mesh_dir);

	if (LowRes)
		AddMesh(hSMRCSLow, &mesh_dir);
	else
		AddMesh (hSMRCS, &mesh_dir);

	AddMesh (hSMPanel1, &mesh_dir);
	AddMesh (hSMPanel2, &mesh_dir);
	AddMesh (hSMPanel3, &mesh_dir);

	if (!ApolloExploded)
		AddMesh (hSMPanel4, &mesh_dir);
	else
		AddMesh (hSMCRYO, &mesh_dir);

	AddMesh (hSMPanel5, &mesh_dir);
	AddMesh (hSMPanel6, &mesh_dir);
	AddMesh (hSMSIMBAY, &mesh_dir);

	if (showSPS) {
		mesh_dir = _V(0, SMVO, offset - 1.654);
		SPSidx = AddMesh(hSMSPS, &mesh_dir);
	}
}

void Saturn::ToggelHatch()

{
	ClearMeshes();

	UINT meshidx;
	VECTOR3 mesh_dir=_V(0,SMVO,30.25-12.25-21.5);
	meshidx = AddMesh (hSM, &mesh_dir);
	SetMeshVisibilityMode (meshidx, MESHVIS_VCEXTERNAL);

	//
	// Skylab SM and Apollo 7 have no HGA.
	//
	if (!NoHGA) {
		mesh_dir=_V(-1.308,-1.18,29.04-12.25-21.5);
		AddMesh (hSMhga, &mesh_dir);
	}

	mesh_dir=_V(0,0,34.4-12.25-21.5);

	// And the Crew
	if (Crewed) {
		cmpidx = AddMesh (hCMP, &mesh_dir);
		crewidx = AddMesh (hCREW, &mesh_dir);
		SetCrewMesh();
	} else {
		cmpidx = -1;
		crewidx = -1;
	}

	meshidx = AddMesh (hCMInt, &mesh_dir);
	SetMeshVisibilityMode (meshidx, MESHVIS_EXTERNAL);

	meshidx = AddMesh (hCMVC, &mesh_dir);
	SetMeshVisibilityMode (meshidx, MESHVIS_VC);
	VCMeshOffset = mesh_dir;

	//Don't Forget the Hatch
	if (HatchOpen){
		meshidx = AddMesh(hFHC, &mesh_dir);
		HatchOpen = false;
	}
	else{
		meshidx = AddMesh(hFHO, &mesh_dir);
		HatchOpen = true;
	}

	SetMeshVisibilityMode (meshidx, MESHVIS_VCEXTERNAL);

	meshidx = AddMesh (hCM, &mesh_dir);
	SetMeshVisibilityMode (meshidx, MESHVIS_VCEXTERNAL);

	if (HasProbe) {
		probeidx = AddMesh(hprobe, &mesh_dir);
		probeextidx = AddMesh(hprobeext, &mesh_dir);
		SetDockingProbeMesh();
	} else {
		probeidx = -1;
		probeextidx = -1;
	}
}

//
// Toggle hatch after splashdown
//

void Saturn::ToggelHatch2()

{
	HatchOpen = !HatchOpen;
	SetReentryMeshes();
}

void Saturn::ToggleEVA()

{
	UINT meshidx;

	//
	// EVA does nothing if we're unmanned.
	//

	if (!Crewed)
		return;

	ToggleEva = false;

	if (EVA_IP){
		EVA_IP =false;

		ClearMeshes();
		VECTOR3 mesh_dir=_V(0,SMVO,30.25-12.25-21.5);
		AddMesh (hSM, &mesh_dir);

		//
		// Skylab SM and Apollo 7 have no HGA.
		//
		if (!NoHGA) {
			mesh_dir=_V(-1.308,-1.18,29.042-12.25-21.5);
			AddMesh (hSMhga, &mesh_dir);
		}

		mesh_dir=_V(0,0,34.4-12.25-21.5);

		// And the Crew
		if (Crewed) {
			cmpidx = AddMesh (hCMP, &mesh_dir);
			crewidx = AddMesh (hCREW, &mesh_dir);
			SetCrewMesh();
		} else {
			cmpidx = -1;
			crewidx = -1;
		}

		meshidx = AddMesh (hCMInt, &mesh_dir);
		SetMeshVisibilityMode (meshidx, MESHVIS_EXTERNAL);

		meshidx = AddMesh (hCMVC, &mesh_dir);
		SetMeshVisibilityMode (meshidx, MESHVIS_VC);
		VCMeshOffset = mesh_dir;

		meshidx = AddMesh (hCM, &mesh_dir);
		SetMeshVisibilityMode (meshidx, MESHVIS_VCEXTERNAL);

		//Don't Forget the Hatch
		meshidx = AddMesh (hFHO, &mesh_dir);
		SetMeshVisibilityMode (meshidx, MESHVIS_VCEXTERNAL);

		HatchOpen = true;
		if (HasProbe) {
			probeidx = AddMesh(hprobe, &mesh_dir);
			probeextidx = AddMesh(hprobeext, &mesh_dir);
			SetDockingProbeMesh();
		} else {
			probeidx = -1;
			probeextidx = -1;
		}
	}
	else 
	{
		EVA_IP = true;

		ClearMeshes();
		VECTOR3 mesh_dir=_V(0,SMVO,30.25-12.25-21.5);
		AddMesh (hSM, &mesh_dir);
		mesh_dir=_V(-1.308,-1.18,29.042-12.25-21.5);
		AddMesh (hSMhga, &mesh_dir);

		mesh_dir=_V(0,0,34.4-12.25-21.5);

		// And the Crew, CMP is outside
		if (Crewed) {
			crewidx = AddMesh (hCREW, &mesh_dir);
			SetCrewMesh();
		} else {
			crewidx = -1;
		}
		cmpidx = -1;

		meshidx = AddMesh (hCMInt, &mesh_dir);
		SetMeshVisibilityMode (meshidx, MESHVIS_EXTERNAL);

		meshidx = AddMesh (hCMVC, &mesh_dir);
		SetMeshVisibilityMode (meshidx, MESHVIS_VC);
		VCMeshOffset = mesh_dir;

		meshidx = AddMesh (hCM, &mesh_dir);
		SetMeshVisibilityMode (meshidx, MESHVIS_VCEXTERNAL);

		//Don't Forget the Hatch
		meshidx = AddMesh (hFHO, &mesh_dir);
		SetMeshVisibilityMode (meshidx, MESHVIS_VCEXTERNAL);

		HatchOpen= true;

		if (HasProbe) {
			probeidx = AddMesh(hprobe, &mesh_dir);
			probeextidx = AddMesh(hprobeext, &mesh_dir);
			SetDockingProbeMesh();
		} else {
			probeidx = -1;
			probeextidx = -1;
		}

		VESSELSTATUS vs1;
		GetStatus(vs1);
		VECTOR3 ofs1 = _V(0,0.15,34.25-12.25-21.5);
		VECTOR3 vel1 = _V(0,0,0);
		VECTOR3 rofs1, rvel1 = {vs1.rvel.x, vs1.rvel.y, vs1.rvel.z};
		Local2Rel (ofs1, vs1.rpos);
		GlobalRot (vel1, rofs1);
		vs1.rvel.x = rvel1.x+rofs1.x;
		vs1.rvel.y = rvel1.y+rofs1.y;
		vs1.rvel.z = rvel1.z+rofs1.z;
		char VName[256]="";
		strcpy (VName, GetName()); strcat (VName, "-EVA");
		hEVA = oapiCreateVessel(VName,"ProjectApollo/EVA",vs1);
		oapiSetFocusObject(hEVA);
	}
}

void Saturn::SetupEVA()

{
	UINT meshidx;

	if (EVA_IP){
		EVA_IP =true;

		ClearMeshes();
		VECTOR3 mesh_dir=_V(0,SMVO,30.25-12.25-21.5);
		AddMesh (hSM, &mesh_dir);

		//
		// Skylab SM and Apollo 7 have no HGA.
		//
		if (!NoHGA) {
			mesh_dir=_V(-1.308,-1.18,29.042-12.25-21.5);
			AddMesh (hSMhga, &mesh_dir);
		}

		mesh_dir=_V(0,0,34.4-12.25-21.5);

		// And the Crew, CMP is outside
		if (Crewed) {
			crewidx = AddMesh (hCREW, &mesh_dir);
			SetCrewMesh();
		} else {
			crewidx = -1;
		}
		cmpidx = -1;

		meshidx = AddMesh (hCMInt, &mesh_dir);
		SetMeshVisibilityMode (meshidx, MESHVIS_EXTERNAL);

		meshidx = AddMesh (hCMVC, &mesh_dir);
		SetMeshVisibilityMode (meshidx, MESHVIS_VC);
		VCMeshOffset = mesh_dir;

		meshidx = AddMesh (hCM, &mesh_dir);
		SetMeshVisibilityMode (meshidx, MESHVIS_VCEXTERNAL);

		//Don't Forget the Hatch
		meshidx = AddMesh (hFHO, &mesh_dir);
		SetMeshVisibilityMode (meshidx, MESHVIS_VCEXTERNAL);

		HatchOpen = true;

		if (HasProbe) {
			probeidx = AddMesh(hprobe, &mesh_dir);
			probeextidx = AddMesh(hprobeext, &mesh_dir);
			SetDockingProbeMesh();
		} else {
			probeidx = -1;
			probeextidx = -1;
		}
	}
}

void Saturn::SetCSMStage ()

{
	ClearMeshes();
    ClearThrusterDefinitions();

	//
	// Delete any dangling propellant resources.
	//

	if (ph_ullage1)
	{
		DelPropellantResource(ph_ullage1);
		ph_ullage1 = 0;
	}

	if (ph_ullage2)
	{
		DelPropellantResource(ph_ullage2);
		ph_ullage2 = 0;
	}

	if (ph_ullage3)
	{
		DelPropellantResource(ph_ullage3);
		ph_ullage3 = 0;
	}

	if(ph_3rd) {
		DelPropellantResource(ph_3rd);
		ph_3rd = 0;
	}

	if (ph_sep) {
		DelPropellantResource(ph_sep);
		ph_sep = 0;
	}

	if (ph_sep2) {
		DelPropellantResource(ph_sep2);
		ph_sep2 = 0;
	}

	SetSize(20);
	SetCOG_elev(3.5);
	SetEmptyMass(CM_Mass + SM_EmptyMass);

	// ************************* propellant specs **********************************
	if (!ph_sps) {
		ph_sps  = CreatePropellantResource(SM_FuelMass, SM_FuelMass); //SPS stage Propellant
	}

	if (ApolloExploded && !ph_o2_vent) {

		double tank_mass = CSM_O2TANK_CAPACITY / 500.0;

		ph_o2_vent = CreatePropellantResource(tank_mass, tank_mass); //SPS stage Propellant

		TankQuantities t;
		GetTankQuantities(t);

		SetPropellantMass(ph_o2_vent, t.O2Tank1QuantityKg + t.O2Tank2QuantityKg);
	}

	SetDefaultPropellantResource (ph_sps); // display SPS stage propellant level in generic HUD

	// *********************** thruster definitions ********************************

	const double CGOffset = 12.25+21.5-1.8+0.35;

	VECTOR3 m_exhaust_pos1= {0,0,-8.-STG2O};
	// orbiter main thrusters
	th_main[0] = CreateThruster(_V(0,0,-5.0), _V(0,0,1), SPS_THRUST , ph_sps, SPS_ISP);
	DelThrusterGroup(THGROUP_MAIN,true);
	thg_main = CreateThrusterGroup (th_main, 1, THGROUP_MAIN);

	AddExhaust (th_main[0], 20.0, 2.25, SMExhaustTex);
	SetPMI (_V(12,12,7));
	SetCrossSections (_V(40,40,14));
	SetCW (0.1, 0.3, 1.4, 1.4);
	SetRotDrag (_V(0.7,0.7,0.3));
	SetPitchMomentScale (0);
	SetBankMomentScale (0);
	SetLiftCoeffFunc (0);

	AddSM(30.25 - CGOffset, true);

	VECTOR3 mesh_dir;

	//
	// Skylab SM and Apollo 7 have no HGA.
	//
	if (!NoHGA) {
		mesh_dir=_V(-1.308,-1.18,29.042-CGOffset);
		AddMesh (hSMhga, &mesh_dir);
	}

	mesh_dir=_V(0, 0, 34.4 - CGOffset);

	UINT meshidx;
	meshidx = AddMesh (hCM, &mesh_dir);
	SetMeshVisibilityMode (meshidx, MESHVIS_VCEXTERNAL);

	// And the Crew
	if (Crewed) {
		cmpidx = AddMesh (hCMP, &mesh_dir);
		crewidx = AddMesh (hCREW, &mesh_dir);
		SetCrewMesh();
	} else {
		cmpidx = -1;
		crewidx = -1;
	}

	meshidx = AddMesh (hCMInt, &mesh_dir);
	SetMeshVisibilityMode (meshidx, MESHVIS_EXTERNAL);

	//Don't Forget the Hatch
	meshidx = AddMesh (hFHC, &mesh_dir);
	SetMeshVisibilityMode (meshidx, MESHVIS_VCEXTERNAL);

	meshidx = AddMesh (hCMVC, &mesh_dir);
	SetMeshVisibilityMode (meshidx, MESHVIS_VC);
	VCMeshOffset = mesh_dir;

	if (HasProbe) {
		probeidx = AddMesh(hprobe, &mesh_dir);
		probeextidx = AddMesh(hprobeext, &mesh_dir);
		SetDockingProbeMesh();
	} else {
		probeidx = -1;
		probeextidx = -1;
	}

	VECTOR3 dockpos = {0,0,35.90-CGOffset};
	VECTOR3 dockdir = {0,0,1};
	VECTOR3 dockrot = {0,1,0};
	SetDockParams(dockpos, dockdir, dockrot);

	AddRCSJets(-0.18, SM_RCS_THRUST);

	if (ApolloExploded) {
		VECTOR3 vent_pos = {0, 1.5, 30.25 - CGOffset};
		VECTOR3 vent_dir = {0.5, 1, 0};

		th_o2_vent = CreateThruster (vent_pos, vent_dir, 450.0, ph_o2_vent, 300.0);
		AddExhaustStream(th_o2_vent, &o2_venting_spec);
	}

	SetView(0.4 + 1.8 - 0.35);
	// **************************** NAV radios *************************************

	InitNavRadios (4);
	SetEnableFocus(true);
	EnableTransponder (true);

	OrbiterAttitudeToggle.SetActive(true);

	ThrustAdjust = 1.0;
}

void Saturn::SetDockingProbeMesh() {

	if (probeidx == -1 || probeextidx == -1)
		return;

	if (HasProbe) {
		if (dockingprobe.IsExtended()) {
			SetMeshVisibilityMode(probeidx, MESHVIS_NEVER);
			SetMeshVisibilityMode(probeextidx, MESHVIS_VCEXTERNAL);
		} else {
			SetMeshVisibilityMode(probeidx, MESHVIS_VCEXTERNAL);
			SetMeshVisibilityMode(probeextidx, MESHVIS_NEVER);
		}
	} else {
		SetMeshVisibilityMode(probeidx, MESHVIS_NEVER);
		SetMeshVisibilityMode(probeextidx, MESHVIS_NEVER);
	}
}

void Saturn::SetCrewMesh() {

	if (cmpidx != -1) {
		if (Crewed && (Crew->number == 1 || Crew->number >= 3)) {
			SetMeshVisibilityMode(cmpidx, MESHVIS_VCEXTERNAL);
		} else {
			SetMeshVisibilityMode(cmpidx, MESHVIS_NEVER);
		}
	}
	if (crewidx != -1) {
		if (Crewed && Crew->number >= 2) {
			SetMeshVisibilityMode(crewidx, MESHVIS_VCEXTERNAL);
		} else {
			SetMeshVisibilityMode(crewidx, MESHVIS_NEVER);
		}
	}
}

void Saturn::SetReentryStage ()

{
    ClearThrusters();
	ClearPropellants();

	//
	// Tell AGC the CM has seperated from the SM.
	//

	agc.SetInputChannelBit(030, 2, true);

	double EmptyMass = CM_EmptyMass + (LESAttached ? 2000.0 : 0.0);

	SetSize(6.0);
	if (ApexCoverAttached) {
		SetCOG_elev(1);
		SetTouchdownPoints(_V(0, -10, -1), _V(-10, 10, -1), _V(10, 10, -1));
	} else {
		SetCOG_elev(2.2);
		SetTouchdownPoints(_V(0, -10, -2.2), _V(-10, 10, -2.2), _V(10, 10, -2.2));
	}
	SetEmptyMass (EmptyMass);
	SetPMI (_V(12,12,7));
	//SetPMI (_V(1.5,1.35,1.35));
	SetCrossSections (_V(9.17,7.13,7.0));
	SetCW (5.5, 0.1, 3.4, 3.4);
	SetRotDrag (_V(0.07,0.07,0.003));
	SetSurfaceFrictionCoeff(1, 1);
	if (GetFlightModel() >= 1) {
		CreateAirfoil(LIFT_VERTICAL, _V(0.0,0.12,1.12), CoeffFunc, 3.5,11.95, 1.0);
    }

	SetReentryMeshes();
	if (ApexCoverAttached) {
		SetView(-0.15);
	} else {
		SetView(-1.35);
	}
	if (CMTex) SetReentryTexture(CMTex,1e6,5,0.7);

	AddRCS_CM(CM_RCS_THRUST);

	if (LESAttached) {
		if (!ph_let)
			ph_let  = CreatePropellantResource(1405.0);

		SetDefaultPropellantResource (ph_let); // display LET propellant level in generic HUD

		//
		// *********************** thruster definitions ********************************
		//

		VECTOR3 m_exhaust_pos1= _V(0.0, -0.5, TowerOffset-2.2);
		VECTOR3 m_exhaust_pos2= _V(0.0, 0.5, TowerOffset-2.2);
		VECTOR3 m_exhaust_pos3= _V(-0.5, 0.0, TowerOffset-2.2);
		VECTOR3 m_exhaust_pos4 = _V(0.5, 0.0, TowerOffset-2.2);

		//
		// Main thrusters.
		//

		th_let[0] = CreateThruster (m_exhaust_pos1, _V(0.0, 0.4, 0.7), THRUST_VAC_LET, ph_let, ISP_LET_VAC, ISP_LET_SL);
		th_let[1] = CreateThruster (m_exhaust_pos2, _V(0.0, -0.4, 0.7),  THRUST_VAC_LET, ph_let, ISP_LET_VAC, ISP_LET_SL);
		th_let[2] = CreateThruster (m_exhaust_pos3, _V(0.4, 0.0, 0.7), THRUST_VAC_LET, ph_let, ISP_LET_VAC, ISP_LET_SL);
		th_let[3] = CreateThruster (m_exhaust_pos4, _V(-0.4, 0.0, 0.7), THRUST_VAC_LET, ph_let, ISP_LET_VAC, ISP_LET_SL);

		//
		// Add exhausts
		//

		int i;
		for (i = 0; i < 4; i++)
		{
			AddExhaust (th_let[i], 8.0, 0.5, SIVBRCSTex);
			AddExhaustStream (th_let[i], &let_exhaust);
		}

		thg_let = CreateThrusterGroup (th_let, 4, THGROUP_MAIN);
	}

	VECTOR3 dockpos = {0, 0, 1.5};
	VECTOR3 dockdir = {0, 0, 1};
	VECTOR3 dockrot = {0, 1, 0};
	SetDockParams(dockpos, dockdir, dockrot);

	if (!DrogueS.isValid())
		soundlib.LoadMissionSound(DrogueS, DROGUES_SOUND);
}

void Saturn::SetReentryMeshes() {

	ClearMeshes();

	UINT meshidx;
	VECTOR3 mesh_dir=_V(0,0,0);
	if (Burned)	{
		if (ApexCoverAttached) {
			meshidx = AddMesh (hCMB, &mesh_dir);
		} else {
			mesh_dir=_V(0, 0, -1.2);
			meshidx = AddMesh (hCM2B, &mesh_dir);
		}
	} else {
		if (ApexCoverAttached) {
			meshidx = AddMesh (hCM, &mesh_dir);
		} else {
			mesh_dir=_V(0, 0, -1.2);
			meshidx = AddMesh (hCM2, &mesh_dir);
		}
	}
	SetMeshVisibilityMode (meshidx, MESHVIS_VCEXTERNAL);

	if (LESAttached) {
		TowerOffset = 4.95;
		VECTOR3 mesh_dir_tower = mesh_dir + _V(0, 0, TowerOffset);

		meshidx = AddMesh (hsat5tower, &mesh_dir_tower);
		SetMeshVisibilityMode (meshidx, MESHVIS_VCEXTERNAL);
	}

	// And the Crew
	if (Crewed) {		
		cmpidx = AddMesh (hCMP, &mesh_dir);
		crewidx = AddMesh (hCREW, &mesh_dir);
		SetCrewMesh();
	} else {
		cmpidx = -1;
		crewidx = -1;
	}

	meshidx = AddMesh (hCMInt, &mesh_dir);
	SetMeshVisibilityMode (meshidx, MESHVIS_EXTERNAL);

	if (HatchOpen) {
		if (Burned) {
			meshidx = AddMesh (hFHO2, &mesh_dir);
		} else {
			meshidx = AddMesh (hFHO, &mesh_dir);
		}
	} else {
		if (Burned) {
			meshidx = AddMesh (hFHC2, &mesh_dir);
		} else {
			meshidx = AddMesh (hFHC, &mesh_dir);
		}
	}
	SetMeshVisibilityMode (meshidx, MESHVIS_VCEXTERNAL);

	meshidx = AddMesh (hCMVC, &mesh_dir);
	SetMeshVisibilityMode (meshidx, MESHVIS_VC);
	VCMeshOffset = mesh_dir;
}

void Saturn::StageSeven(double simt)

{
	if (CsmLmFinalSep1Switch.GetState() || CsmLmFinalSep2Switch.GetState()) {
		Undock(0);
		dockingprobe.SetEnabled(false);
	}

	if (!Crewed)
	{
		switch (StageState) {
		case 0:
			if (GetAltitude() < 185000) {
				SlowIfDesired();
				ActivateCMRCS();
				ActivateNavmode(NAVMODE_RETROGRADE);
				StageState++;
			}
			break;
		}
	}

	// Entry heat according to Orbiter reference manual
	double entryHeat = 0.5 * GetAtmDensity() * pow(GetAirspeed(), 3);
	if (entryHeat > 2e7 ) { // We 're looking wether the CM has burned or not
		Burned = true;
		SetReentryMeshes();

		AddRCS_CM(CM_RCS_THRUST);
		SetStage(CM_ENTRY_STAGE);
		SetView(-0.15);
	}
}

void Saturn::StageEight(double simt)

{
	SetTouchdownPoints(_V(0, -10, -2.2), _V(-10, 10, -2.2), _V(10, 10, -2.2));

	// Mark apex as detached
	ApexCoverAttached = false;
	SetReentryMeshes();

	AddRCS_CM(CM_RCS_THRUST, -1.2);
	SetView(-1.35);

	if (!Crewed)
	{
		switch (StageState) {
		case 0:
			if (GetAltitude() < 50000) {
				SlowIfDesired();
				DeactivateNavmode(NAVMODE_RETROGRADE);
				DeactivateCMRCS();
				StageState++;
			}
			break;
		}
	}

	SetApexCoverLight(true);

	//
	// Create the apex cover vessel
	//
	VECTOR3 posOffset = _V(0, 0, 0);
	VECTOR3 velOffset = _V(0, 0, 3);

	VESSELSTATUS vs;
	GetStatus(vs);
	Local2Rel(posOffset, vs.rpos);
	VECTOR3 vog;
	GlobalRot(velOffset, vog);
	vs.rvel += vog;

	char VName[256]="";
	GetApolloName(VName);
	strcat(VName, "-APEX");
	if (Burned) {
		hApex = oapiCreateVessel(VName,"ProjectApollo/CMBapex", vs);
	} else {
		hApex = oapiCreateVessel(VName,"ProjectApollo/CMapex", vs);
	}

	// New stage
	SetStage(CM_ENTRY_STAGE_TWO);
}

void Saturn::SetChuteStage1()
{
	SetSize(15);
	SetCOG_elev(2.2);
	SetTouchdownPoints(_V(0, -10, -2.2), _V(-10, 10, -2.2), _V(10, 10, -2.2));
	SetEmptyMass(CM_EmptyMass);
	ClearAirfoilDefinitions();
	SetPMI(_V(20,20,12));
	SetCrossSections(_V(2.8,2.8,80.0));
	SetCW(1.0, 1.5, 1.4, 1.4);
	SetRotDrag(_V(0.7,0.7,1.2));
	SetSurfaceFrictionCoeff(1, 1);
	if (GetFlightModel() >= 1)
	{
		SetPitchMomentScale(-5e-3);
		SetBankMomentScale(-5e-3);
	}
	SetLiftCoeffFunc(0);
    ClearExhaustRefs();
    ClearAttExhaustRefs();

	SetReentryMeshes();
	
	AddRCS_CM(CM_RCS_THRUST, -1.2);
	SetView(-1.35);

	DeactivateNavmode(NAVMODE_KILLROT);

	SetDrogueDeployLight(true);
}

void Saturn::SetChuteStage2()
{
	SetSize(22);
	SetCOG_elev(2.2);
	SetTouchdownPoints(_V(0, -10, -2.2), _V(-10, 10, -2.2), _V(10, 10, -2.2));
	SetEmptyMass (CM_EmptyMass);
	SetPMI (_V(20,20,12));
	SetCrossSections (_V(2.8,2.8,140.0));
	SetCW (1.0, 1.5, 1.4, 1.4);
	SetRotDrag (_V(0.7,0.7,1.2));
	SetSurfaceFrictionCoeff(1, 1);
	if (GetFlightModel() >= 1)
	{
		SetPitchMomentScale (-5e-3);
		SetBankMomentScale (-5e-3);
	}
	SetLiftCoeffFunc(0);
    ClearExhaustRefs();
    ClearAttExhaustRefs();

	SetReentryMeshes();

	AddRCS_CM(CM_RCS_THRUST, -1.2);
	SetView(-1.35);
}

void Saturn::SetChuteStage3()
{
	SetSize(22);
	SetCOG_elev(2.2);
	SetTouchdownPoints(_V(0, -10, -2.2), _V(-10, 10, -2.2), _V(10, 10, -2.2));
	SetEmptyMass (CM_EmptyMass);
	SetPMI(_V(20,20,12));
	SetCrossSections(_V(2.8,2.8,480.0));
	SetCW(0.7, 1.5, 1.4, 1.4);
	SetRotDrag(_V(0.7,0.7,1.2));
	SetSurfaceFrictionCoeff(1, 1);
	if (GetFlightModel() >= 1)
	{
		SetPitchMomentScale(-5e-3);
		SetBankMomentScale(-5e-3);
	}
	SetLiftCoeffFunc (0);
    ClearExhaustRefs();
    ClearAttExhaustRefs();

	SetReentryMeshes();

	AddRCS_CM(CM_RCS_THRUST, -1.2);
	SetView(-1.35);
}

void Saturn::SetChuteStage4()
{
	SetSize(22);
	SetCOG_elev(2.2);
	SetTouchdownPoints(_V(0, -10, -2.2), _V(-10, 10, -2.2), _V(10, 10, -2.2));
	SetEmptyMass(CM_EmptyMass);
	SetPMI(_V(20,20,12));
	SetCrossSections (_V(2.8,2.8,3280.0));
	SetCW (0.7, 1.5, 1.4, 1.4);
	SetRotDrag(_V(0.7, 0.7, 1.2));
	SetSurfaceFrictionCoeff(1, 1);
	if (GetFlightModel() >= 1)
	{
		SetPitchMomentScale (-5e-3);
		SetBankMomentScale (-5e-3);
	}
	SetLiftCoeffFunc(0);
    ClearExhaustRefs();
    ClearAttExhaustRefs();

	SetReentryMeshes();

	AddRCS_CM(CM_RCS_THRUST, -1.2);
	SetView(-1.35);

	SetMainDeployLight(true);
}

void Saturn::SetSplashStage()
{
	SetSize(6.0);
	SetCOG_elev(2.2);
	SetTouchdownPoints(_V(0, -10, -2.2), _V(-10, 10, -2.2), _V(10, 10, -2.2));
	SetEmptyMass(CM_EmptyMass);
	SetPMI(_V(20,20,12));
	SetCrossSections(_V(2.8,2.8,7.0));
	SetCW(0.5, 1.5, 1.4, 1.4);
	SetRotDrag(_V(0.7,0.7,1.2));
	SetSurfaceFrictionCoeff(1, 1);
	if (GetFlightModel() >= 1)
	{
		SetPitchMomentScale (-5e-3);
		SetBankMomentScale (-5e-3);
	}
	SetLiftCoeffFunc(0);
    ClearExhaustRefs();
    ClearAttExhaustRefs();

	SetReentryMeshes();

	AddRCS_CM(CM_RCS_THRUST, -1.2);

	dyemarker_spec.tex = oapiRegisterParticleTexture("ProjectApollo/Dyemarker");
	AddParticleStream(&dyemarker_spec, _V(-0.5, 1.5, -2), _V(-0.8660254, 0.5, 0), els.GetDyeMarkerLevelRef());

	SetView(-1.35);
}

void Saturn::SetRecovery()

{
	SetSize(10.0);
	SetCOG_elev(2.2);
	SetTouchdownPoints(_V(0, -10, -2.2), _V(-10, 10, -2.2), _V(10, 10, -2.2));
	SetEmptyMass(CM_EmptyMass);
	SetPMI(_V(20,20,12));
	SetCrossSections(_V(2.8,2.8,7.0));
	SetCW(0.5, 1.5, 1.4, 1.4);
	SetRotDrag(_V(0.7,0.7,1.2));
	SetSurfaceFrictionCoeff(1, 1);
	if (GetFlightModel() >= 1)
	{
		SetPitchMomentScale (-5e-3);
		SetBankMomentScale (-5e-3);
	}
	SetLiftCoeffFunc(0);
    ClearExhaustRefs();
    ClearAttExhaustRefs();

	// Meshes
	ClearMeshes();

	UINT meshidx;
	VECTOR3 mesh_dir=_V(0,0,-1.2);
	if (Burned)	{
		meshidx = AddMesh (hCM2B, &mesh_dir);
	} else {
		meshidx = AddMesh (hCM2, &mesh_dir);
	}
	SetMeshVisibilityMode (meshidx, MESHVIS_VCEXTERNAL);

	HatchOpen = true;
	if (Burned)	{
		meshidx = AddMesh (hFHO2, &mesh_dir);
	} else {
		meshidx = AddMesh (hFHO, &mesh_dir);
	}
	SetMeshVisibilityMode (meshidx, MESHVIS_VCEXTERNAL);

	meshidx = AddMesh (hCMInt, &mesh_dir);
	SetMeshVisibilityMode (meshidx, MESHVIS_EXTERNAL);

	meshidx = AddMesh (hCMVC, &mesh_dir);
	SetMeshVisibilityMode (meshidx, MESHVIS_VC);
	VCMeshOffset = mesh_dir;

	if (Crewed) {
		mesh_dir =_V(2.7,1.8,-1.5);
		meshidx = AddMesh (hCRB, &mesh_dir);
		SetMeshVisibilityMode (meshidx, MESHVIS_VCEXTERNAL);
	}

	AddRCS_CM(CM_RCS_THRUST, -1.2);

	dyemarker_spec.tex = oapiRegisterParticleTexture("ProjectApollo/Dyemarker");
	AddParticleStream(&dyemarker_spec, _V(-0.5, 1.5, -2), _V(-0.8660254, 0.5, 0), els.GetDyeMarkerLevelRef());

	SetView(-1.35);
}

void Saturn::SetAbortStage ()

{
	SetReentryStage();

	ABORT_IND = true;
	OrbiterAttitudeToggle.SetState(false);
}

bool Saturn::clbkLoadGenericCockpit ()

{
	TRACESETUP("Saturn::clbkLoadGenericCockpit");

	//
	// VC-only in engineering camera view.
	//

	if (viewpos == SATVIEW_ENG1 || viewpos == SATVIEW_ENG2 || viewpos == SATVIEW_ENG3)
		return false;

	SetCameraRotationRange(0.0, 0.0, 0.0, 0.0);
	SetCameraDefaultDirection(_V(0.0, 0.0, 1.0));
	InVC = false;
	InPanel = false;

	SetView();
	return true;
}

//
// Generic function to jettison the escape tower.
//

void Saturn::JettisonLET(bool UseMain, bool AbortJettison)

{
	//
	// Don't do anything if the tower isn't attached!
	//
	if (!LESAttached)
		return;

	//
	// If the jettison motor fails and we're trying to
	// use it for the jettison, return.
	//
	// We'll always give them one way to jettison the LES as
	// being unable to jettison it is fatal.
	//
	if (!UseMain && LaunchFail.LESJetMotorFail)
		return;

	//
	// Otherwise jettison the LES.
	//
	VECTOR3 ofs1 = _V(0.0, 0.0, TowerOffset);
	VECTOR3 vel1 = _V(0.0,0.0,0.5);

	VESSELSTATUS vs1;
	GetStatus (vs1);

	vs1.eng_main = vs1.eng_hovr = 0.0;

	//
	// We must set status to zero to ensure the LET is in 'free flight'. Otherwise if we jettison
	// on the pad, the LET thinks it's on the ground!
	//

	vs1.status = 0;

	VECTOR3 rofs1, rvel1 = {vs1.rvel.x, vs1.rvel.y, vs1.rvel.z};

	Local2Rel (ofs1, vs1.rpos);

	GlobalRot (vel1, rofs1);

	vs1.rvel.x = rvel1.x+rofs1.x;
	vs1.rvel.y = rvel1.y+rofs1.y;
	vs1.rvel.z = rvel1.z+rofs1.z;

	TowerJS.play();
	TowerJS.done();

	char VName[256];

	GetApolloName(VName);
	strcat (VName, "-TWR");

	hesc1 = oapiCreateVessel(VName, "ProjectApollo/LES", vs1);
	LESAttached = false;

	LESSettings LESConfig;

	LESConfig.SettingsType.word = 0;
	LESConfig.SettingsType.LES_SETTINGS_GENERAL = 1;
	LESConfig.SettingsType.LES_SETTINGS_ENGINES = 1;
	LESConfig.SettingsType.LES_SETTINGS_THRUST = 1;

	//
	// Pressing the LES jettison button fires the main LET engine. The TWR JETT
	// switches jettison the LES and fire the jettison engines.
	//
	// If the LES jettison button is pressed before using the TWR JETT switches,
	// the explosive bolts won't fire, so the main LET motor will fire while
	// still attached to the CM!
	//
	// See: AOH 2.9.4.8.4
	//

	LESConfig.FireMain = UseMain;

	LESConfig.MissionTime = MissionTime;
	LESConfig.Realism = Realism;
	LESConfig.VehicleNo = VehicleNo;
	LESConfig.LowRes = LowRes;
	LESConfig.ProbeAttached = AbortJettison && HasProbe;
	LESConfig.ISP_LET_SL = ISP_LET_SL;
	LESConfig.ISP_LET_VAC = ISP_LET_VAC;
	LESConfig.THRUST_VAC_LET = THRUST_VAC_LET;

	//
	// If this is the CSM abort stage, we need to transfer fuel information from
	// the CSM LET.
	//
	// Usually this will be zero, so you'd better use the right jettison button!
	//
	// \todo At  some point we should expand this so that we can jettison the LES
	// while the main abort motor is running.
	//

	if (ph_let)
	{
		LESConfig.MainFuelKg = GetPropellantMass(ph_let);
		LESConfig.SettingsType.LES_SETTINGS_MAIN_FUEL = 1;
	}

	LES *les_vessel = (LES *) oapiGetVesselInterface(hesc1);
	les_vessel->SetState(LESConfig);

	//
	// AOH SECS page 2.9-8 says that in the case of an abort, the docking probe is pulled away
	// from the CM by the LES when it's jettisoned.
	//
	if (AbortJettison)
	{
		dockingprobe.SetEnabled(false);
		HasProbe = false;
	}
	else
	{
		//
		// Enable docking probe because the tower is gone
		//
		dockingprobe.SetEnabled(HasProbe);			
	}

	ConfigureStageMeshes(stage);

	if (Crewed)
	{
		SwindowS.play();
	}
	SwindowS.done();

	SetLESMotorLight(true);
}
