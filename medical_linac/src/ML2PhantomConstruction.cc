//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// The code was written by :
//	^Claudio Andenna  claudio.andenna@ispesl.it, claudio.andenna@iss.infn.it
//      *Barbara Caccia barbara.caccia@iss.it
//      with the support of Pablo Cirrone (LNS, INFN Catania Italy)
//	with the contribute of Alessandro Occhigrossi*
//
// ^INAIL DIPIA - ex ISPESL and INFN Roma, gruppo collegato Sanità, Italy
// *Istituto Superiore di Sanità and INFN Roma, gruppo collegato Sanità, Italy
//  Viale Regina Elena 299, 00161 Roma (Italy)
//  tel (39) 06 49902246
//  fax (39) 06 49387075
//
// more information:
// http://g4advancedexamples.lngs.infn.it/Examples/medical-linac
//
//*******************************************************//

#include "ML2PhantomConstruction.hh"
#include "ML2PhantomConstructionMessenger.hh"
#include "G4SystemOfUnits.hh"

CML2PhantomConstruction::CML2PhantomConstruction(void): PVPhmWorld(0), sensDet(0)
{
	phantomContstructionMessenger=new CML2PhantomConstructionMessenger(this);
	idCurrentCentre=0;
}

CML2PhantomConstruction::~CML2PhantomConstruction(void)
{
	if (phantomName=="fullWater")
	{
		delete Ph_fullWater;
	}
	else if (phantomName=="boxInBox")
	{
		delete Ph_BoxInBox;
	}
	else if (phantomName=="SiDiode")
	{
		delete Ph_BoxInBox;
	}
}

CML2PhantomConstruction* CML2PhantomConstruction::instance = 0;

CML2PhantomConstruction* CML2PhantomConstruction::GetInstance(void)
{
  if (instance == 0)
    {
      instance = new CML2PhantomConstruction();
     
    }
  return instance;
}
bool CML2PhantomConstruction::design(void)
{
// switch between two different phantoms according to the main macro selection
	bool bPhanExists=false;

	std::cout << "I'm building "<< phantomName<<"  phantom"<< G4endl;

	if (phantomName=="fullWater")
	{
		Ph_fullWater=new CML2Ph_FullWater();bPhanExists=true;
		halfPhantomInsideSize=Ph_fullWater->getHalfContainerSize();
	}
	else if (phantomName=="boxInBox")
	{
		Ph_BoxInBox=new CML2Ph_BoxInBox();bPhanExists=true;
		halfPhantomInsideSize=Ph_BoxInBox->getHalfContainerSize();
	}
	else if (phantomName=="SiDiode")
	{
		Ph_SiDiode=new CML2Ph_SiDiode();bPhanExists=true;
		halfPhantomInsideSize=Ph_SiDiode->getHalfContainerSize()*2;
	}
 	if (centre.size()<1)
 	{addNewCentre(G4ThreeVector(0.,0.,0.));}
	return bPhanExists;
}
G4int CML2PhantomConstruction::getTotalNumberOfEvents()
{
	if (phantomName=="fullWater")
	{return Ph_fullWater->getTotalNumberOfEvents();}
	else if (phantomName=="boxInBox")
	{return Ph_BoxInBox->getTotalNumberOfEvents();}
	else if (phantomName=="SiDiode")
	{return Ph_SiDiode->getTotalNumberOfEvents();}
	return 0;
}

bool CML2PhantomConstruction::Construct(G4VPhysicalVolume *PVWorld,
        G4int saving_in_ROG_Voxels_every_events, G4int seed,
        G4String ROGOutFile, G4bool bSaveROG, G4bool bOV)
{
	idVolumeName=0;
	bOnlyVisio=bOV;
// a call to select the right phantom
	if(design())
	{
		phantomContstructionMessenger->SetReferenceWorld(bOV);	// create the phantom-world box
		G4Material *Vacuum=G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR");

		G4Box *phmWorldB = new G4Box("phmWorldG", halfPhantomInsideSize.getX(), halfPhantomInsideSize.getY(), halfPhantomInsideSize.getZ());
		G4LogicalVolume *phmWorldLV = new G4LogicalVolume(phmWorldB, Vacuum, "phmWorldL", 0, 0, 0);
		G4VisAttributes* simpleAlSVisAtt= new G4VisAttributes(G4Colour::White());
		simpleAlSVisAtt->SetVisibility(false);
// 		simpleAlSVisAtt->SetForceWireframe(false);
		phmWorldLV->SetVisAttributes(simpleAlSVisAtt);
	
		// G4ThreeVector zTrans(0, 0,200*mm);

		PVPhmWorld= new G4PVPlacement(0, G4ThreeVector(0.,0.,0.), "phmWorldPV", phmWorldLV, PVWorld, false, 0);
	
	// create the actual phantom
		if (phantomName=="fullWater")
		{
			Ph_fullWater->Construct(PVPhmWorld, saving_in_ROG_Voxels_every_events, seed, ROGOutFile, bSaveROG);
			sensDet=Ph_fullWater->getSensDet();
			createPhysicalVolumeNamesList(Ph_fullWater->getPhysicalVolume());
			Ph_fullWater->writeInfo();
		}
		else if (phantomName=="boxInBox")
		{
			Ph_BoxInBox->Construct(PVPhmWorld, saving_in_ROG_Voxels_every_events, seed, ROGOutFile, bSaveROG);
			sensDet=Ph_BoxInBox->getSensDet();
			createPhysicalVolumeNamesList(Ph_BoxInBox->getPhysicalVolume());
			Ph_BoxInBox->writeInfo();
		}
		else if (phantomName=="SiDiode")
		{
			Ph_SiDiode->Construct(PVPhmWorld, saving_in_ROG_Voxels_every_events, seed, ROGOutFile, bSaveROG);
			sensDet=Ph_SiDiode->getSensDet();
			createPhysicalVolumeNamesList(Ph_SiDiode->getPhysicalVolume());
			Ph_SiDiode->writeInfo();
		}
		// I create the data base volumeName-volumeID in the sensitive detector 

		sensDet->setVolumeNameIdLink(volumeNameIdLink);
	}
	else
	{
		return false;
	}
	return true;
}
void CML2PhantomConstruction::createPhysicalVolumeNamesList(G4String  *matNames, G4int nMatNames)
{
	SvolumeNameId svnid;
	for (int i=0;i< nMatNames; i++)
	{
		svnid.volumeId=i;
		svnid.volumeName=matNames[i];
		volumeNameIdLink.push_back(svnid);
	}
}

void CML2PhantomConstruction::createPhysicalVolumeNamesList(G4VPhysicalVolume  *PV)
{
	int nLVD1;
	nLVD1=(int) PV->GetLogicalVolume()->GetNoDaughters();
	SvolumeNameId svnid;
		std::cout << "PV in name: " <<PV->GetName() << G4endl;
	if (nLVD1>0)
	{
		for (int i=0; i <nLVD1; i++)
		{
			createPhysicalVolumeNamesList(PV->GetLogicalVolume()->GetDaughter(i));
		}
		idVolumeName++;
		svnid.volumeId=idVolumeName;
		svnid.volumeName=PV->GetLogicalVolume()->GetMaterial()->GetName();
		volumeNameIdLink.push_back(svnid);
		std::cout << "physical volume name: " <<svnid.volumeName << G4endl;
	}
	else
	{
		idVolumeName++;
		svnid.volumeId=idVolumeName;
		svnid.volumeName=PV->GetLogicalVolume()->GetMaterial()->GetName();
		volumeNameIdLink.push_back(svnid);
		std::cout << "physical volume name: " <<svnid.volumeName << G4endl;
	}
}
bool  CML2PhantomConstruction::applyNewCentre()
{
	if (idCurrentCentre <(int) centre.size())
	{
		currentCentre=centre[idCurrentCentre];
		applyNewCentre(currentCentre);
		idCurrentCentre++;
		return true;
	}
	return false;
}
void CML2PhantomConstruction::writeInfo()
{
	if (!bOnlyVisio)
	{std::cout <<"Actual centre: "<<idCurrentCentre<<"/"<<centre.size() <<"  "<< G4endl;}
	std::cout <<"Phantom and its ROG centre: " << currentCentre<< G4endl;
}
void CML2PhantomConstruction::applyNewCentre(G4ThreeVector ctr)
{
	if (sensDet!=0)
	{	
		currentCentre=ctr;
		G4GeometryManager::GetInstance()->OpenGeometry();
		PVPhmWorld->SetTranslation(ctr);
		sensDet->GetROgeometry()->GetROWorld()->GetLogicalVolume()->GetDaughter(0)->SetTranslation(ctr);
		sensDet->resetVoxelsSingle();
		G4GeometryManager::GetInstance()->CloseGeometry();
		G4RunManager::GetRunManager()->GeometryHasBeenModified();
	}
}
G4String CML2PhantomConstruction::getCurrentTranslationString()
{
	char cT[5];
	G4int cTI;
	G4String translationName;
	cTI=(G4int)((currentCentre.getX()/mm));
	sprintf(cT,"%d",cTI);
	translationName="_TrX"+G4String(cT)+"_";
	cTI=(G4int)((currentCentre.getY()/mm));
	sprintf(cT,"%d",cTI);
	translationName+="Y"+G4String(cT)+"_";
	cTI=(G4int)((currentCentre.getZ()/mm));
	sprintf(cT,"%d",cTI);
	translationName+="Z"+G4String(cT);
	return translationName;
}

