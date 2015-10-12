// Torc - Copyright 2011-2013 University of Southern California.  All Rights Reserved.
// $HeadURL: https://svn.code.sf.net/p/torc-isi/code/trunk/sandbox/PhysicalExample.cpp $
// $Id: PhysicalExample.cpp 16 2013-11-12 22:50:42Z nsteiner $

// This program is free software: you can redistribute it and/or modify it under the terms of the 
// GNU General Public License as published by the Free Software Foundation, either version 3 of the 
// License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See 
// the GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along with this program.  If 
// not, see <http://www.gnu.org/licenses/>.

/// \file
/// \This file is for Module level movement.

#include "torc/Physical.hpp"
#include "torc/Architecture.hpp"
#include "torc/Common.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <cstdlib>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

using namespace torc::common;
using namespace torc::architecture::xilinx;
using namespace torc::physical;
using namespace torc::architecture;
/// \brief Standard main() function.

//Funtion declarations
//This function is used to check the validity of one-time 
//CLB  post-move location.
void CheckCLBValidity(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, std::string siteType);

//This function is used to DSP movement on X direction(+/-)
void DSPMovementXplus(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, int D_Tx, int count, int X_pos, int Y_pos, int X_Dpos, int Y_Dpos, std::string siteName, std::string tileName);
		
void DSPMovementXminus(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, int D_Tx, int count,int X_pos, int Y_pos,int X_Dpos, int Y_Dpos, std::string siteName, std::string tileName);

//This function is used to DSP movement on Y direction(+/-)
void DSPMovementYplus(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, int D_Ty, int count,int Y_pos,int Y_Dpos, int b, std::string siteName, std::string tileName, int D_flag);

void DSPMovementYminus(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, int D_Ty, int count,int Y_pos,int Y_Dpos, int b, std::string siteName, std::string tileName, int D_flag);

//This function is used to RAMB movement on X direction(+/-)
void RAMBMovementXplus(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, int D_Tx, int count, int X_pos, int Y_pos, int X_Dpos, int Y_Dpos, std::string siteName, std::string tileName);

void RAMBMovementXminus(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, int D_Tx, int count, int X_pos, int Y_pos, int X_Dpos, int Y_Dpos, std::string siteName, std::string tileName);

//This function is used to RAMB movement on Y direction(+/-)
void RAMBMovementYplus(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, int D_Ty, int count,int Y_pos,int Y_Dpos,std::string siteName, std::string tileName);

void RAMBMovementYminus(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, int D_Ty, int count,int Y_pos,int Y_Dpos,std::string siteName, std::string tileName);



// Main function starts here	
int main(int argc, char* argv[]) {
	
	if(argc != 3) {
	std::cerr<< "Usage: "<< argv[0] << " <XDL_FILE_NAME> <X_move or Y_move>(Using ',' to seperate X and Y) "<< std::endl;
	return -1;
	}
	//construct and intialized the device database
	(void)  argc;
	DirectoryTree directoryTree(argv[0]);
	DeviceDesignator designator("xc7z020clg484-1");
	DDB ddb(designator);
	
	//look up a site output
	const Sites& sites = ddb.getSites();
	const Tiles& tiles = ddb.getTiles();

//D_flag is used for DSP SITE location judgement
	int X_pos, Y_pos, X_Dpos, Y_Dpos,D_flag=-1;

	// import the XDL design
	std::string inFileName(argv[1]);
	std::fstream fileStream(inFileName.c_str());
	if(!fileStream.good()) {
	std::cout<< "The file does not exist! " << std::endl;
	return -1;
	}
	torc::physical::XdlImporter importer;
	importer(fileStream, inFileName);
	
	std::string Input(argv[2]);
	std::vector<std::string> strs;
		
	// Using the "," to spilt the Input if necessary
	boost::split(strs,Input,boost::is_any_of(","));
	// count means the first found number
	int count = atoi(strs[0].substr(2).c_str());
	int temp;
	
	// look up the design (and do something with it ...)
	DesignSharedPtr designPtr = importer.getDesignPtr();
		
	InstanceSharedPtrVector::const_iterator pInstance = designPtr->instancesBegin();
	InstanceSharedPtrVector::const_iterator eInstance = designPtr->instancesEnd();
	while(pInstance != eInstance)
	{
	InstanceSharedPtr instancePtr =*pInstance;
	std::string siteName = instancePtr->getSite();
	std::string siteType = instancePtr->getType();
	X_pos = siteName.find("X");
	Y_pos = siteName.find("Y");
	int a = atoi((siteName.substr(X_pos+1,Y_pos - X_pos-1)).c_str());
	int b = atoi((siteName.substr(Y_pos+1)).c_str());
	SiteIndex index_site = sites.findSiteIndex(instancePtr->getSite());
	const Site& site = sites.getSite(index_site);
	//From SITE to TILE, this part is used to DSP/RAMB movement
	TileIndex index_tile = site.getTileIndex();
	const TileInfo& TileInfo = tiles.getTileInfo(index_tile);
	const char* Name = TileInfo.getName();
	std::string tileName = std::string(Name);
	//The tile X/Y_pos is used for DSP and RAMB
	X_Dpos = tileName.find("X");
	Y_Dpos = tileName.find("Y");
	int D_Tx = atoi((tileName.substr(X_Dpos+1,Y_Dpos - X_Dpos-1)).c_str());
	int D_Ty = atoi((tileName.substr(Y_Dpos+1)).c_str());

	//Start moving, this is for the X or Y movement.
if((siteType.compare("SLICEL")==0 || siteType.compare("SLICEM")==0 || siteType.compare("DSP48E1")==0 || siteType.compare("RAMB36E1")==0)&&strs[1].empty()) 
	{
		if(strs[0][0]=='X'){

			if (strs[0][1]=='+'){
				// X plus
				if (siteType.compare("DSP48E1")==0){
				DSPMovementXplus(instancePtr, sites,tiles,D_Tx,count,X_pos,Y_pos,X_Dpos,Y_Dpos, siteName,tileName);
				}
				else if (siteType.compare("RAMB36E1")==0){
				RAMBMovementXplus(instancePtr, sites,tiles,D_Tx,count,X_pos,Y_pos,X_Dpos,Y_Dpos, siteName,tileName);
				}
				else{
				instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(a+count)));
				CheckCLBValidity(instancePtr, sites, tiles, siteType);
				}
			}

			else if (strs[0][1]=='-'){
				//X minus
				if (siteType.compare("DSP48E1")==0){
				DSPMovementXminus(instancePtr, sites,tiles,D_Tx,count,X_pos, Y_pos,X_Dpos,Y_Dpos, siteName,tileName);
				}
				else if (siteType.compare("RAMB36E1")==0){
				RAMBMovementXminus(instancePtr, sites,tiles,D_Tx,count,X_pos,Y_pos,X_Dpos,Y_Dpos, siteName,tileName);
				}
				else{
				instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(a-count)));
				CheckCLBValidity(instancePtr, sites, tiles, siteType);}
			}

			else
			{std::cout<< "Can not opreate the X location!"<<std::endl;}
			}

		else if(strs[0][0]=='Y'){

			if (strs[0][1]=='+'){
				//Y plus
				if(siteType.compare("DSP48E1")==0){
				DSPMovementYplus(instancePtr, sites, tiles, D_Ty, count,Y_pos, Y_Dpos,b,siteName, tileName, D_flag);
				}
				else if(siteType.compare("RAMB36E1")==0){
				RAMBMovementYplus(instancePtr, sites, tiles, D_Ty, count,Y_pos, Y_Dpos,siteName, tileName);
				}
				else{
				instancePtr->setSite(siteName.replace(Y_pos+1,siteName.length()-Y_pos,boost::lexical_cast<std::string>(b+count)));
				CheckCLBValidity(instancePtr, sites, tiles, siteType);
				}
			}
			else if (strs[0][1]=='-'){
				//Y minus
				if(siteType.compare("DSP48E1")==0){
				DSPMovementYminus(instancePtr, sites, tiles, D_Ty, count,Y_pos, Y_Dpos,b,siteName, tileName, D_flag);
				}
				else if(siteType.compare("RAMB36E1")==0){
				RAMBMovementYminus(instancePtr, sites, tiles, D_Ty, count,Y_pos, Y_Dpos,siteName, tileName);
				}
				else{
				instancePtr->setSite(siteName.replace(Y_pos+1,siteName.length()-Y_pos,boost::lexical_cast<std::string>(b-count)));
				CheckCLBValidity(instancePtr, sites, tiles, siteType);
				}
			}
			else
			{std::cout<< "Can not opreate the Y location!"<<std::endl;}
			}
		else
			{std::cout<< "Please Iuput the X or Y as first! "<< std::endl;}
	}
	// Combination Movement
	
	else if ((siteType.compare("SLICEL")==0 || siteType.compare("SLICEM")==0	|| siteType.compare("DSP48E1")==0 || siteType.compare("RAMB36E1")==0)&&(!strs[1].empty()))
	{
		// X first, Y second
		int count2 = atoi(strs[1].substr(2).c_str());
		if(strs[0][0]=='X')
		{
			// First Part
			if(strs[0][1]=='+'){
				if (siteType.compare("DSP48E1")==0){
				DSPMovementXplus(instancePtr, sites,tiles,D_Tx,count,X_pos, Y_pos,X_Dpos,Y_Dpos, siteName,tileName);	
				}
				else if (siteType.compare("RAMB36E1")==0){
				RAMBMovementXplus(instancePtr, sites,tiles,D_Tx,count,X_pos,Y_pos,X_Dpos,Y_Dpos, siteName,tileName);
				}
				else{
				instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(a+count)));
				CheckCLBValidity(instancePtr, sites, tiles, siteType);
					}
			}
			else if(strs[0][1]=='-'){
				if (siteType.compare("DSP48E1")==0){
				DSPMovementXminus(instancePtr, sites,tiles,D_Tx,count,X_pos,Y_pos,X_Dpos,Y_Dpos, siteName,tileName);
				}
				else if (siteType.compare("RAMB36E1")==0){
				RAMBMovementXminus(instancePtr, sites,tiles,D_Tx,count,X_pos,Y_pos,X_Dpos,Y_Dpos, siteName,tileName);
				}
				else{
				instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(a-count)));
				CheckCLBValidity(instancePtr, sites, tiles, siteType);
				}	
			}
			else
				{std::cout<< "Can not opreate the X location in combination!"<<std::endl;}
			//Second Part
			if(strs[1][0]=='Y')
			{
				if(strs[1][1]=='+'){
					if (siteType.compare("DSP48E1")==0){
						temp = count;
						count = count2;
						std::string siteName = instancePtr->getSite();
						std::string tileName = instancePtr->getTile();
						DSPMovementYplus(instancePtr, sites,tiles,D_Ty,count,Y_pos,Y_Dpos,b,siteName,tileName,D_flag);
						count = temp;
					}
					else if(siteType.compare("RAMB36E1")==0){
						temp = count;
						count = count2;
						std::string siteName = instancePtr->getSite();
						std::string tileName = instancePtr->getTile();
						RAMBMovementYplus(instancePtr, sites, tiles, D_Ty, count,Y_pos, Y_Dpos,siteName, tileName);
						count = temp;
					}
					else{
					instancePtr->setSite(siteName.replace(Y_pos+1,siteName.length()-Y_pos,boost::lexical_cast<std::string>(b+count2)));
					CheckCLBValidity(instancePtr, sites, tiles, siteType);
					}
				}
				
				else if(strs[1][1]=='-'){
					
					if(siteType.compare("DSP48E1")==0){
						temp = count;
						count = count2;
						std::string siteName = instancePtr->getSite();
						std::string tileName = instancePtr->getTile();

						DSPMovementYminus(instancePtr, sites,tiles,D_Ty,count,Y_pos,Y_Dpos,b, siteName,tileName,D_flag);
						count = temp;
					}
					else if(siteType.compare("RAMB36E1")==0){
						temp = count;
						count = count2;
						std::string siteName = instancePtr->getSite();
						std::string tileName = instancePtr->getTile();
						RAMBMovementYminus(instancePtr, sites, tiles, D_Ty, count,Y_pos, Y_Dpos,siteName, tileName);
						count = temp;
					}
					else{
					instancePtr->setSite(siteName.replace(Y_pos+1,siteName.length()-Y_pos,boost::lexical_cast<std::string>(b-count2)));
					CheckCLBValidity(instancePtr, sites, tiles, siteType);
					}
				}
			
				else
				{std::cout<< "Can not opreate the Y location in combination !"<<std::endl;}
			}
			else {std::cout<<"Error in the second part of INPUT!"<<std::endl;}
		}
		//Y first, X second
		else if(strs[0][0]=='Y')
		{
			//first part
			if(strs[0][1]=='+'){
				if(siteType.compare("DSP48E1")==0){
					DSPMovementYplus(instancePtr, sites,tiles,D_Ty,count,Y_pos,Y_Dpos,b, siteName,tileName,D_flag);
				}
				else if(siteType.compare("RAMB36E1")==0){
					RAMBMovementYplus(instancePtr, sites,tiles,D_Ty,count,Y_pos,Y_Dpos, siteName,tileName);
				}
				else{
				instancePtr->setSite(siteName.replace(Y_pos+1,siteName.length()-Y_pos,boost::lexical_cast<std::string>(b+count)));
				CheckCLBValidity(instancePtr, sites, tiles, siteType);
				}
			}
			else if(strs[0][1]=='-'){
				if (siteType.compare("DSP48E1")==0){
				DSPMovementYminus(instancePtr, sites,tiles,D_Ty,count,Y_pos,Y_Dpos,b,siteName,tileName,D_flag);
				}
				else if(siteType.compare("RAMB36E1")==0){
				RAMBMovementYminus(instancePtr, sites,tiles,D_Ty,count,Y_pos,Y_Dpos, siteName,tileName);
				}
				else{
				instancePtr->setSite(siteName.replace(Y_pos+1,siteName.length()-Y_pos,boost::lexical_cast<std::string>(b-count)));
				CheckCLBValidity(instancePtr, sites, tiles, siteType);
				}
			}
			else
			{std::cout<< "Can not opreate the Y location in combination !"<<std::endl;}
			//second part

			if(strs[1][0]=='X')
			{
				if(strs[1][1]=='+'){
					if(siteType.compare("DSP48E1")==0){
					temp = count;
					count = count2;
					std::string siteName = instancePtr->getSite();
					std::string tileName = instancePtr->getTile();
					DSPMovementXplus(instancePtr, sites,tiles,D_Tx,count,X_pos, Y_pos,X_Dpos,Y_Dpos, siteName,tileName);
					count = temp;
					}
					else if (siteType.compare("RAMB36E1")==0){
					temp = count;
					count = count2;
					std::string siteName = instancePtr->getSite();
					std::string tileName = instancePtr->getTile();
					RAMBMovementXplus(instancePtr, sites,tiles,D_Tx,count,X_pos,Y_pos,X_Dpos,Y_Dpos, siteName,tileName);
					count = temp;
					}
					else{
					instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(a+count2)));
					CheckCLBValidity(instancePtr, sites, tiles, siteType);
					}
				}
				else if(strs[1][1]=='-'){
					if(siteType.compare("DSP48E1")==0){
					temp = count;
					count = count2;
					std::string siteName = instancePtr->getSite();
					std::string tileName = instancePtr->getTile();
					DSPMovementXminus(instancePtr, sites,tiles,D_Tx,count,X_pos, Y_pos,X_Dpos,Y_Dpos, siteName,tileName);
					count = temp;
					}
					else if (siteType.compare("RAMB36E1")==0){
					temp = count;
					count = count2;
					std::string siteName = instancePtr->getSite();
					std::string tileName = instancePtr->getTile();
					RAMBMovementXminus(instancePtr, sites,tiles,D_Tx,count,X_pos,Y_pos,X_Dpos,Y_Dpos, siteName,tileName);
					count = temp;
					}
					else{
					instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(a-count2)));
					CheckCLBValidity(instancePtr, sites, tiles, siteType);
					}
				}
				else
				{std::cout<< "Can not opreate the X location!"<<std::endl;}
			}
			else {std::cout<<"Error in the second part(X) of INPUT!"<<std::endl;}
		}
		//X or Y first/second error
		else 
		{std::cout<<"Error in the combination of INPUT!"<<std::endl;}

	}
	
	else
	{std::cout<<"This is no SLCIEL or SLICEM!"<<std::endl;}
	
	pInstance++;	
	}	

	// export the XDL design
	std::string outFileName = boost::filesystem::path(inFileName).replace_extension().string() 
		+ ".mod.xdl";
	std::fstream xdlExport(outFileName.c_str(), std::ios_base::out);
	torc::physical::XdlExporter fileExporter(xdlExport);
	fileExporter(designPtr);

	return 0;
}
//This function is used to check the validity of one-time  post-move location.
void CheckCLBValidity(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles,std::string siteType){
		SiteIndex index_site = sites.findSiteIndex(instancePtr->getSite());
		std::cout<< "Index_site: " << index_site<<std::endl;
		std::cout<<" SITe:" << instancePtr->getSite()<<std::endl;
		if(!index_site.isUndefined()){
			const Site& site = sites.getSite(index_site);
			TileIndex index_tile = site.getTileIndex();
			const PrimitiveDef* primitivedef = site.getPrimitiveDefPtr();
			std::string PriDef = primitivedef->getName();//Get the new location's type(SLICEL or SLICEM)
			if((PriDef.compare("SLICEL")==0)&&(siteType.compare("SLICEM")==0)){
				std::cout<< "The SLICEM can not move to SLICEL!" << std::endl;
				exit(0);	
				} 
			else{
				const TileInfo& TileInfo = tiles.getTileInfo(index_tile);
				const char* Name = TileInfo.getName();
				instancePtr->setTile(Name);
			 // std::cout<< instancePtr->getName() <<std::endl;
				}
			}
		else{
			std::cout<<"The New CLB location does not exist!"<<std::endl;
			exit(0);}	
	}
	
void DSPMovementXplus(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, int D_Tx, int count,int X_pos, int Y_pos,
int X_Dpos, int Y_Dpos, std::string siteName, std::string tileName){
	X_pos = siteName.find("X");
	Y_pos = siteName.find("Y");
	X_Dpos = tileName.find("X");
	Y_Dpos = tileName.find("Y");
	switch (D_Tx+count)
		{
		case 9:
			//siteName.replace(3,1,"R");
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(0)));
			tileName.replace(4,1,"R");
			instancePtr->setTile(tileName.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx+count)));						
			std::cout<< instancePtr->getSite()<<std::endl;
			std::cout<< instancePtr->getTile()<<std::endl;
			break;
		case 14:
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(1)));
			tileName.replace(4,1,"L");
			instancePtr->setTile(tileName.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx+count)));				
			std::cout<< instancePtr->getSite()<<std::endl;
			std::cout<< instancePtr->getTile()<<std::endl;
			break;
		case 25:
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(2)));
			tileName.replace(4,1,"R");	
			instancePtr->setTile(tileName.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx+count)));
			std::cout<< instancePtr->getSite()<<std::endl;
			std::cout<< instancePtr->getTile()<<std::endl;
			break;
		case 59:
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(3)));
			tileName.replace(4,1,"R");
			instancePtr->setTile(tileName.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx+count)));		
			std::cout<< instancePtr->getSite()<<std::endl;
			std::cout<< instancePtr->getTile()<<std::endl;
			break;
		case 64:
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(4)));
			tileName.replace(4,1,"L");
			instancePtr->setTile(tileName.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx+count)));					
			std::cout<< instancePtr->getSite()<<std::endl;
			std::cout<< instancePtr->getTile()<<std::endl;
			break;
		default:
			std::cout<<"DSP movement X plus Error!"<<std::endl;
			exit(0);
					}	
	}
	
void DSPMovementXminus(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, int D_Tx, int count,int X_pos, int Y_pos,
int X_Dpos, int Y_Dpos, std::string siteName, std::string tileName){
	X_pos = siteName.find("X");
	Y_pos = siteName.find("Y");
	X_Dpos = tileName.find("X");
	Y_Dpos = tileName.find("Y");
	switch (D_Tx-count)
		{
		case 9:
			//siteName.replace(3,1,"R");
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(0)));
			tileName.replace(4,1,"R");
			instancePtr->setTile(tileName.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx-count)));						
			std::cout<< instancePtr->getSite()<<std::endl;
			std::cout<< instancePtr->getTile()<<std::endl;
			break;
		case 14:
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(1)));
			tileName.replace(4,1,"L");
			instancePtr->setTile(tileName.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx-count)));				
			std::cout<< instancePtr->getSite()<<std::endl;
			std::cout<< instancePtr->getTile()<<std::endl;
			break;
		case 25:
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(2)));
			tileName.replace(4,1,"R");	
			instancePtr->setTile(tileName.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx-count)));
			std::cout<< instancePtr->getSite()<<std::endl;
			std::cout<< instancePtr->getTile()<<std::endl;
			break;
		case 59:
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(3)));
			tileName.replace(4,1,"R");
			instancePtr->setTile(tileName.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx-count)));		
			std::cout<< instancePtr->getSite()<<std::endl;
			std::cout<< instancePtr->getTile()<<std::endl;
			break;
		case 64:
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(4)));
			tileName.replace(4,1,"L");
			instancePtr->setTile(tileName.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx-count)));					
			std::cout<< instancePtr->getSite()<<std::endl;
			std::cout<< instancePtr->getTile()<<std::endl;
			break;
		default:
			std::cout<<"DSP movement X minus Error!"<<std::endl;
			exit(0);
					}	
	}	
	
	
void DSPMovementYplus(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, int D_Ty, int count,int Y_pos,
int Y_Dpos, int b ,std::string siteName, std::string tileName, int D_flag){	
	std::cout<<"D_Ty: "<<D_Ty<<"  b:"<<b<<std::endl;
	if(float((D_Ty/5)*2)==float(b)){ D_flag = 0;}
	else {D_flag = 1;}
//	std::cout<<D_flag<<std::endl;
	//I can also use the (D_Ty+count)%5 to check
	Y_pos = siteName.find("Y");
	Y_Dpos = tileName.find("Y");
	instancePtr->setTile(tileName.replace(Y_Dpos+1,tileName.length()-Y_Dpos, boost::lexical_cast<std::string>(D_Ty+count)));
//	std::cout<<"after Y plus tileName: "<<instancePtr->getTile()<<std::endl;
	TileIndex new_tile_index = tiles.findTileIndex(instancePtr->getTile());
	//If exists, change the site
	if(!new_tile_index.isUndefined()){
		switch (D_flag){
		case 0:
			instancePtr->setSite(siteName.replace(Y_pos+1, siteName.length()-Y_pos,boost::lexical_cast<std::string>((D_Ty+count)/5*2)));
			std::cout<<"after Ysite:"<< instancePtr->getSite()<<std::endl;
			std::cout<<"after Ytile:"<< instancePtr->getTile()<<std::endl;	
			break;
		case 1:
			instancePtr->setSite(siteName.replace(Y_pos+1, siteName.length()-Y_pos,boost::lexical_cast<std::string>((D_Ty+count)/5*2+1)));
			std::cout<<"after Ysite:"<<instancePtr->getSite()<<std::endl;
			std::cout<<"after Ytile:"<< instancePtr->getTile()<<std::endl;
			break;
		default:
			std::cout<< "DSP Flag Error!"<< std::endl;
			break;
		}
	}
	else{ std::cout<<"DSP movement Y plus Error!"<<std::endl;
		exit(0);
	}
}		

void DSPMovementYminus(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, int D_Ty, int count,int Y_pos,
int Y_Dpos, int b ,std::string siteName, std::string tileName, int D_flag){	
	std::cout<<"D_Ty: "<<D_Ty<<"  b:"<<b<<std::endl;
	if(float((D_Ty/5)*2)==float(b)){ D_flag = 0;}
	else {D_flag = 1;}
	std::cout<<D_flag<<std::endl;
	//I can also use the (D_Ty+count)%5 to check
	std::cout<<"TileName in Y minus: "<<tileName << std::endl;
	Y_pos = siteName.find("Y");
	Y_Dpos = tileName.find("Y");
	instancePtr->setTile(tileName.replace(Y_Dpos+1,tileName.length()-Y_Dpos, boost::lexical_cast<std::string>(D_Ty-count)));
	std::cout<<"after Y minus check tileName: "<<instancePtr->getTile()<<std::endl;
	TileIndex new_tile_index = tiles.findTileIndex(instancePtr->getTile());
	//If exists, change the site
	std::cout<<"SiteName in Y minus:  "<<siteName<<std::endl;
	if(!new_tile_index.isUndefined()){
		switch (D_flag){
		case 0:
			instancePtr->setSite(siteName.replace(Y_pos+1, siteName.length()-Y_pos,boost::lexical_cast<std::string>((D_Ty-count)/5*2)));
			std::cout<<"after Ysite:"<< instancePtr->getSite()<<std::endl;
			std::cout<<"after Ytile:"<< instancePtr->getTile()<<std::endl;	
			break;
		case 1:
			instancePtr->setSite(siteName.replace(Y_pos+1, siteName.length()-Y_pos,boost::lexical_cast<std::string>((D_Ty-count)/5*2+1)));
			std::cout<<"after Ysite:"<< instancePtr->getSite()<<std::endl;
			std::cout<<"after Ysite:"<< instancePtr->getTile()<<std::endl;
			break;
		default:
			std::cout<< "DSP Flag Error!"<< std::endl;
			break;
		}
	}
	else{ std::cout<<"DSP movement Y minus Error!"<<std::endl;
		exit(0);
	}
}		

void RAMBMovementXplus(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, int D_Tx, int count,int X_pos, int Y_pos,
int X_Dpos, int Y_Dpos, std::string siteName, std::string tileName){
	X_pos = siteName.find("X");
	Y_pos = siteName.find("Y");
	X_Dpos = tileName.find("X");
	Y_Dpos = tileName.find("Y");
	switch (D_Tx+count)
		{
		case 6:
			//siteName.replace(3,1,"R");
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(0)));
			tileName.replace(4,1,"L");
			instancePtr->setTile(tileName.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx+count)));						
			std::cout<<"RAMB site: "<<instancePtr->getSite()<<std::endl;
			std::cout<<"RAMB tile: "<<instancePtr->getTile()<<std::endl;
			break;
		case 17:
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(1)));
			tileName.replace(4,1,"R");
			instancePtr->setTile(tileName.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx+count)));				
			std::cout<<"RAMB site: "<< instancePtr->getSite()<<std::endl;
			std::cout<<"RAMB tile: "<< instancePtr->getTile()<<std::endl;
			break;
		case 22:
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(2)));
			tileName.replace(4,1,"L");	
			instancePtr->setTile(tileName.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx+count)));
			std::cout<< "RAMB site: "<<instancePtr->getSite()<<std::endl;
			std::cout<< "RAMB tile: "<<instancePtr->getTile()<<std::endl;
			break;
		case 36:
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(3)));
			tileName.replace(4,1,"L");
			instancePtr->setTile(tileName.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx+count)));		
			std::cout<<"RAMB site:" << instancePtr->getSite()<<std::endl;
			std::cout<<"RAMB tile:" << instancePtr->getTile()<<std::endl;
			break;
		case 56:
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(4)));
			tileName.replace(4,1,"L");
			instancePtr->setTile(tileName.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx+count)));					
			std::cout<<"RAMB site: "<< instancePtr->getSite()<<std::endl;
			std::cout<<"RAMB tile: "<< instancePtr->getTile()<<std::endl;
			break;
		case 67:
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(5)));
			tileName.replace(4,1,"R");
			instancePtr->setTile(tileName.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx+count)));					
			std::cout<<"RAMB site: "<< instancePtr->getSite()<<std::endl;
			std::cout<<"RAMB tile: "<< instancePtr->getTile()<<std::endl;
			break;
		default:
			std::cout<<"RAMB movement X plus Error!"<<std::endl;
			exit(0);
					}	
	}
	
void RAMBMovementXminus(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, int D_Tx, int count,int X_pos, int Y_pos,
int X_Dpos, int Y_Dpos, std::string siteName, std::string tileName){
	X_pos = siteName.find("X");
	Y_pos = siteName.find("Y");
	X_Dpos = tileName.find("X");
	Y_Dpos = tileName.find("Y");
	switch (D_Tx-count)
		{
		case 6:
			//siteName.replace(3,1,"R");
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(0)));
			tileName.replace(4,1,"L");
			instancePtr->setTile(tileName.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx-count)));						
			std::cout<<"RAMB site: "<<instancePtr->getSite()<<std::endl;
			std::cout<<"RAMB tile: "<<instancePtr->getTile()<<std::endl;
			break;
		case 17:
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(1)));
			tileName.replace(4,1,"R");
			instancePtr->setTile(tileName.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx-count)));				
			std::cout<<"RAMB site: "<< instancePtr->getSite()<<std::endl;
			std::cout<<"RAMB tile: "<< instancePtr->getTile()<<std::endl;
			break;
		case 22:
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(2)));
			tileName.replace(4,1,"L");	
			instancePtr->setTile(tileName.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx-count)));
			std::cout<< "RAMB site: "<<instancePtr->getSite()<<std::endl;
			std::cout<< "RAMB tile: "<<instancePtr->getTile()<<std::endl;
			break;
		case 36:
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(3)));
			tileName.replace(4,1,"L");
			instancePtr->setTile(tileName.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx-count)));		
			std::cout<<"RAMB site: "<< instancePtr->getSite()<<std::endl;
			std::cout<<"RAMB tile: "<< instancePtr->getTile()<<std::endl;
			break;
		case 56:
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(4)));
			tileName.replace(4,1,"L");
			instancePtr->setTile(tileName.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx-count)));					
			std::cout<<"RAMB site: "<< instancePtr->getSite()<<std::endl;
			std::cout<<"RAMB tile: "<< instancePtr->getTile()<<std::endl;
			break;
		case 67:
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(5)));
			tileName.replace(4,1,"R");
			instancePtr->setTile(tileName.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx-count)));					
			std::cout<<"RAMB site: "<< instancePtr->getSite()<<std::endl;
			std::cout<<"RAMB tile: "<< instancePtr->getTile()<<std::endl;
			break;
		default:
			std::cout<<"RAMB movement X minus Error!"<<std::endl;
			exit(0);
					}	
	}
	
	
void RAMBMovementYplus(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, int D_Ty, int count,int Y_pos,
int Y_Dpos,std::string siteName, std::string tileName){	
	Y_pos = siteName.find("Y");
	Y_Dpos = tileName.find("Y");
	instancePtr->setTile(tileName.replace(Y_Dpos+1,tileName.length()-Y_Dpos, boost::lexical_cast<std::string>(D_Ty+count)));
//	std::cout<<"after Y plus tileName: "<<instancePtr->getTile()<<std::endl;
	TileIndex new_tile_index = tiles.findTileIndex(instancePtr->getTile());
	//If exists, change the site
	if(!new_tile_index.isUndefined()){
			instancePtr->setSite(siteName.replace(Y_pos+1, siteName.length()-Y_pos,boost::lexical_cast<std::string>((D_Ty+count)/5)));
			std::cout<<"RAMB after Y site:"<< instancePtr->getSite()<<std::endl;
			std::cout<<"RAMB after Y tile:"<< instancePtr->getTile()<<std::endl;	
		}
	else{ std::cout<<"RAMB movement Y plus Error!"<<std::endl;
		exit(0);
	}
}	

void RAMBMovementYminus(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, int D_Ty, int count,int Y_pos,
int Y_Dpos,std::string siteName, std::string tileName){	
	Y_pos = siteName.find("Y");
	Y_Dpos = tileName.find("Y");
	instancePtr->setTile(tileName.replace(Y_Dpos+1,tileName.length()-Y_Dpos, boost::lexical_cast<std::string>(D_Ty-count)));
//	std::cout<<"after Y plus tileName: "<<instancePtr->getTile()<<std::endl;
	TileIndex new_tile_index = tiles.findTileIndex(instancePtr->getTile());
	//If exists, change the site
	if(!new_tile_index.isUndefined()){
			instancePtr->setSite(siteName.replace(Y_pos+1, siteName.length()-Y_pos,boost::lexical_cast<std::string>((D_Ty-count)/5)));
			std::cout<<"RAMB after Y site:"<< instancePtr->getSite()<<std::endl;
			std::cout<<"RAMB after Y tile:"<< instancePtr->getTile()<<std::endl;	
		}
	else{ std::cout<<"RAMB movement Y minus Error!"<<std::endl;
		exit(0);
	}
}		
