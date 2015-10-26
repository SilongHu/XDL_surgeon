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
#include <map>

using namespace torc::common;
using namespace torc::architecture::xilinx;
using namespace torc::physical;
using namespace torc::architecture;
//using namespace torc::physical::Net;
/// \brief Standard main() function.

//Funtion declarations
//This function is used to check the validity of one-time 
//CLB  post-move location.
void CheckCLBValidity(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, std::string siteType,std::string tileName,std::map<std::string,std::string>& TILE_MAP);

//This function is used to DSP movement on X direction(+/-)
void DSPMovementXplus(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, int D_Tx, int count, int X_pos, int Y_pos, int X_Dpos, int Y_Dpos, std::string siteName, std::string tileName,std::map<std::string,std::string>& TILE_MAP);
		
void DSPMovementXminus(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, int D_Tx, int count,int X_pos, int Y_pos,int X_Dpos, int Y_Dpos, std::string siteName, std::string tileName,std::map<std::string,std::string>& TILE_MAP);

//This function is used to DSP movement on Y direction(+/-)
void DSPMovementYplus(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, int D_Ty, int count,int Y_pos,int Y_Dpos, int b, std::string siteName, std::string tileName, int D_flag, std::map<std::string,std::string>& TILE_MAP);

void DSPMovementYminus(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, int D_Ty, int count,int Y_pos,int Y_Dpos, int b, std::string siteName, std::string tileName, int D_flag, std::map<std::string,std::string>& TILE_MAP);

//This function is used to RAMB movement on X direction(+/-)
void RAMBMovementXplus(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, int D_Tx, int count, int X_pos, int Y_pos, int X_Dpos, int Y_Dpos, std::string siteName, std::string tileName,std::map<std::string,std::string>& TILE_MAP);

void RAMBMovementXminus(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, int D_Tx, int count, int X_pos, int Y_pos, int X_Dpos, int Y_Dpos, std::string siteName, std::string tileName,std::map<std::string,std::string>& TILE_MAP);

//This function is used to RAMB movement on Y direction(+/-)
void RAMBMovementYplus(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, int D_Ty, int count,int Y_pos,int Y_Dpos,std::string siteName, std::string tileName,std::map<std::string,std::string>& TILE_MAP);

void RAMBMovementYminus(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, int D_Ty, int count,int Y_pos,int Y_Dpos,std::string siteName, std::string tileName,std::map<std::string,std::string>& TILE_MAP);

void TIEOFFMovementX(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, int D_Tx, int X_pos, int Y_pos, int X_Dpos, int Y_Dpos, std::string siteName, std::string tileName,std::map<std::string,std::string>& TILE_MAP);

void TIEOFFMovementYplus(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, int D_Ty, int count,int Y_pos,int Y_Dpos,std::string siteName, std::string tileName,std::map<std::string,std::string>& TILE_MAP);

void TIEOFFMovementYminus(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, int D_Ty, int count,int Y_pos,int Y_Dpos,std::string siteName, std::string tileName,std::map<std::string,std::string>& TILE_MAP);

void NetsProcessing(DesignSharedPtr designPtr, const Sites &sites, const Tiles &tiles,std::map<std::string,std::string>& TILE_MAP, std::set<std::string>& OtherTileLocation_SET);

	std::map<std::string,std::string> TILE_MAP;
	std::set<std::string> OtherTileLocation_SET;	
//	std::map<std::string,std::string> m;
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
	std::cout<<"MAP size:  "<<TILE_MAP.size()<<std::endl;
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
if((siteType.compare("SLICEL")==0 || siteType.compare("SLICEM")==0 || siteType.compare("DSP48E1")==0 || siteType.compare("RAMB36E1")==0||siteType.compare("TIEOFF")==0)&&strs[1].empty()) 
	{
		if(strs[0][0]=='X'){

			if (strs[0][1]=='+'){
				// X plus
				if (siteType.compare("DSP48E1")==0){
				DSPMovementXplus(instancePtr, sites,tiles,D_Tx,count,X_pos,Y_pos,X_Dpos,Y_Dpos, siteName,tileName,TILE_MAP);
				}
				else if (siteType.compare("RAMB36E1")==0){
				RAMBMovementXplus(instancePtr, sites,tiles,D_Tx,count,X_pos,Y_pos,X_Dpos,Y_Dpos, siteName,tileName,TILE_MAP);
				}
				else if (siteType.compare("TIEOFF")==0){
				TIEOFFMovementX(instancePtr, sites,tiles,D_Tx,X_pos,Y_pos,X_Dpos,Y_Dpos, siteName,tileName,TILE_MAP);	
				}
				else{
				instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(a+count)));
				CheckCLBValidity(instancePtr, sites, tiles, siteType,tileName,TILE_MAP);
				}
			}

			else if (strs[0][1]=='-'){
				//X minus
				if (siteType.compare("DSP48E1")==0){
				DSPMovementXminus(instancePtr, sites,tiles,D_Tx,count,X_pos, Y_pos,X_Dpos,Y_Dpos, siteName,tileName,TILE_MAP);
				}
				else if (siteType.compare("RAMB36E1")==0){
				RAMBMovementXminus(instancePtr, sites,tiles,D_Tx,count,X_pos,Y_pos,X_Dpos,Y_Dpos, siteName,tileName,TILE_MAP);
				}
				else if (siteType.compare("TIEOFF")==0){
				TIEOFFMovementX(instancePtr, sites,tiles,D_Tx,X_pos,Y_pos,X_Dpos,Y_Dpos, siteName,tileName,TILE_MAP);	
				}
				else{
				instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(a-count)));
				CheckCLBValidity(instancePtr, sites, tiles, siteType,tileName,TILE_MAP);}
			}

			else
			{std::cout<< "Can not opreate the X location!"<<std::endl;}
			}

		else if(strs[0][0]=='Y'){

			if (strs[0][1]=='+'){
				//Y plus
				if(siteType.compare("DSP48E1")==0){
				DSPMovementYplus(instancePtr, sites, tiles, D_Ty, count,Y_pos, Y_Dpos,b,siteName, tileName, D_flag,TILE_MAP);
				}
				else if(siteType.compare("RAMB36E1")==0){
				RAMBMovementYplus(instancePtr, sites, tiles, D_Ty, count,Y_pos, Y_Dpos,siteName, tileName,TILE_MAP);
				}
				else if (siteType.compare("TIEOFF")==0){
				TIEOFFMovementYplus(instancePtr, sites,tiles,D_Ty,count,Y_pos,Y_Dpos, siteName,tileName,TILE_MAP);	
				}
				else{
				instancePtr->setSite(siteName.replace(Y_pos+1,siteName.length()-Y_pos,boost::lexical_cast<std::string>(b+count)));
				CheckCLBValidity(instancePtr, sites, tiles, siteType,tileName,TILE_MAP);
				}
			}
			else if (strs[0][1]=='-'){
				//Y minus
				if(siteType.compare("DSP48E1")==0){
				DSPMovementYminus(instancePtr, sites, tiles, D_Ty, count,Y_pos, Y_Dpos,b,siteName, tileName, D_flag,TILE_MAP);
				}
				else if(siteType.compare("RAMB36E1")==0){
				RAMBMovementYminus(instancePtr, sites, tiles, D_Ty, count,Y_pos, Y_Dpos,siteName, tileName,TILE_MAP);
				}
				else if (siteType.compare("TIEOFF")==0){
				TIEOFFMovementYminus(instancePtr, sites,tiles,D_Ty,count,Y_pos,Y_Dpos, siteName,tileName,TILE_MAP);	
				}
				else{
				instancePtr->setSite(siteName.replace(Y_pos+1,siteName.length()-Y_pos,boost::lexical_cast<std::string>(b-count)));
				CheckCLBValidity(instancePtr, sites, tiles, siteType,tileName,TILE_MAP);
				}
			}
			else
			{std::cout<< "Can not opreate the Y location!"<<std::endl;}
			}
		else
			{std::cout<< "Please Iuput the X or Y as first! "<< std::endl;}
	}
	// Combination Movement
	
	else if ((siteType.compare("SLICEL")==0 || siteType.compare("SLICEM")==0	|| siteType.compare("DSP48E1")==0 || siteType.compare("RAMB36E1")==0||siteType.compare("TIEOFF")==0)&&(!strs[1].empty()))
	{
		// X first, Y second
		int count2 = atoi(strs[1].substr(2).c_str());
		if(strs[0][0]=='X')
		{
			// First Part
			if(strs[0][1]=='+'){
				if (siteType.compare("DSP48E1")==0){
				DSPMovementXplus(instancePtr, sites,tiles,D_Tx,count,X_pos, Y_pos,X_Dpos,Y_Dpos, siteName,tileName,TILE_MAP);	
				}
				else if (siteType.compare("RAMB36E1")==0){
				RAMBMovementXplus(instancePtr, sites,tiles,D_Tx,count,X_pos,Y_pos,X_Dpos,Y_Dpos, siteName,tileName,TILE_MAP);
				}
				else if (siteType.compare("TIEOFF")==0){
				TIEOFFMovementX(instancePtr, sites,tiles,D_Tx,X_pos,Y_pos,X_Dpos,Y_Dpos, siteName,tileName,TILE_MAP);	
				}
				else{
				instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(a+count)));
				CheckCLBValidity(instancePtr, sites, tiles, siteType,tileName,TILE_MAP);
					}
			}
			else if(strs[0][1]=='-'){
				if (siteType.compare("DSP48E1")==0){
				DSPMovementXminus(instancePtr, sites,tiles,D_Tx,count,X_pos,Y_pos,X_Dpos,Y_Dpos, siteName,tileName,TILE_MAP);
				}
				else if (siteType.compare("RAMB36E1")==0){
				RAMBMovementXminus(instancePtr, sites,tiles,D_Tx,count,X_pos,Y_pos,X_Dpos,Y_Dpos, siteName,tileName,TILE_MAP);
				}
				else if (siteType.compare("TIEOFF")==0){
				TIEOFFMovementX(instancePtr, sites,tiles,D_Tx,X_pos,Y_pos,X_Dpos,Y_Dpos, siteName,tileName,TILE_MAP);	
				}
				else{
				instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(a-count)));
				CheckCLBValidity(instancePtr, sites, tiles, siteType,tileName,TILE_MAP);
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
						DSPMovementYplus(instancePtr, sites,tiles,D_Ty,count,Y_pos,Y_Dpos,b,siteName,tileName,D_flag,TILE_MAP);
						count = temp;
					}
					else if(siteType.compare("RAMB36E1")==0){
						temp = count;
						count = count2;
						std::string siteName = instancePtr->getSite();
						std::string tileName = instancePtr->getTile();
						RAMBMovementYplus(instancePtr, sites, tiles, D_Ty, count,Y_pos, Y_Dpos,siteName, tileName,TILE_MAP);
						count = temp;
					}
					else if (siteType.compare("TIEOFF")==0){
						temp = count;
						count = count2;
						std::string siteName = instancePtr->getSite();
						std::string tileName = instancePtr->getTile();
						TIEOFFMovementYplus(instancePtr, sites,tiles,D_Ty,count,Y_pos,Y_Dpos, siteName,tileName,TILE_MAP);	
						count = temp;
					}
					else{
					instancePtr->setSite(siteName.replace(Y_pos+1,siteName.length()-Y_pos,boost::lexical_cast<std::string>(b+count2)));
					CheckCLBValidity(instancePtr, sites, tiles, siteType,tileName,TILE_MAP);
					}
				}
				
				else if(strs[1][1]=='-'){
					
					if(siteType.compare("DSP48E1")==0){
						temp = count;
						count = count2;
						std::string siteName = instancePtr->getSite();
						std::string tileName = instancePtr->getTile();

						DSPMovementYminus(instancePtr, sites,tiles,D_Ty,count,Y_pos,Y_Dpos,b, siteName,tileName,D_flag,TILE_MAP);
						count = temp;
					}
					else if(siteType.compare("RAMB36E1")==0){
						temp = count;
						count = count2;
						std::string siteName = instancePtr->getSite();
						std::string tileName = instancePtr->getTile();
						RAMBMovementYminus(instancePtr, sites, tiles, D_Ty, count,Y_pos, Y_Dpos,siteName, tileName,TILE_MAP);
						count = temp;
					}
					else if (siteType.compare("TIEOFF")==0){
						temp = count;
						count = count2;
						std::string siteName = instancePtr->getSite();
						std::string tileName = instancePtr->getTile();
						TIEOFFMovementYminus(instancePtr, sites,tiles,D_Ty,count,Y_pos,Y_Dpos, siteName,tileName,TILE_MAP);	
						count = temp;
					}
					else{
					instancePtr->setSite(siteName.replace(Y_pos+1,siteName.length()-Y_pos,boost::lexical_cast<std::string>(b-count2)));
					CheckCLBValidity(instancePtr, sites, tiles, siteType,tileName,TILE_MAP);
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
				DSPMovementYplus(instancePtr, sites,tiles,D_Ty,count,Y_pos,Y_Dpos,b, siteName,tileName,D_flag,TILE_MAP);
				}
				else if(siteType.compare("RAMB36E1")==0){
				RAMBMovementYplus(instancePtr, sites,tiles,D_Ty,count,Y_pos,Y_Dpos, siteName,tileName,TILE_MAP);
				}
				else if (siteType.compare("TIEOFF")==0){
				TIEOFFMovementYplus(instancePtr, sites,tiles,D_Ty,count,Y_pos,Y_Dpos, siteName,tileName,TILE_MAP);	
				}
				else{
				instancePtr->setSite(siteName.replace(Y_pos+1,siteName.length()-Y_pos,boost::lexical_cast<std::string>(b+count)));
				CheckCLBValidity(instancePtr, sites, tiles, siteType,tileName,TILE_MAP);
				}
			}
			else if(strs[0][1]=='-'){
				if (siteType.compare("DSP48E1")==0){
				DSPMovementYminus(instancePtr, sites,tiles,D_Ty,count,Y_pos,Y_Dpos,b,siteName,tileName,D_flag,TILE_MAP);
				}
				else if(siteType.compare("RAMB36E1")==0){
				RAMBMovementYminus(instancePtr, sites,tiles,D_Ty,count,Y_pos,Y_Dpos, siteName,tileName,TILE_MAP);
				}
				else if (siteType.compare("TIEOFF")==0){
				TIEOFFMovementYminus(instancePtr, sites,tiles,D_Ty,count,Y_pos,Y_Dpos, siteName,tileName,TILE_MAP);	
				}
				else{
				instancePtr->setSite(siteName.replace(Y_pos+1,siteName.length()-Y_pos,boost::lexical_cast<std::string>(b-count)));
				CheckCLBValidity(instancePtr, sites, tiles, siteType,tileName,TILE_MAP);
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
					DSPMovementXplus(instancePtr, sites,tiles,D_Tx,count,X_pos, Y_pos,X_Dpos,Y_Dpos, siteName,tileName,TILE_MAP);
					count = temp;
					}
					else if (siteType.compare("RAMB36E1")==0){
					temp = count;
					count = count2;
					std::string siteName = instancePtr->getSite();
					std::string tileName = instancePtr->getTile();
					RAMBMovementXplus(instancePtr, sites,tiles,D_Tx,count,X_pos,Y_pos,X_Dpos,Y_Dpos, siteName,tileName,TILE_MAP);
					count = temp;
					}
					else if (siteType.compare("TIEOFF")==0){
					temp = count;
					count = count2;
					std::string siteName = instancePtr->getSite();
					std::string tileName = instancePtr->getTile();
					TIEOFFMovementX(instancePtr, sites,tiles,D_Tx,X_pos,Y_pos,X_Dpos,Y_Dpos, siteName,tileName,TILE_MAP);	
					count = temp;
					}
					else{
					instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(a+count2)));
					CheckCLBValidity(instancePtr, sites, tiles, siteType,tileName,TILE_MAP);
					}
				}
				else if(strs[1][1]=='-'){
					if(siteType.compare("DSP48E1")==0){
					temp = count;
					count = count2;
					std::string siteName = instancePtr->getSite();
					std::string tileName = instancePtr->getTile();
					DSPMovementXminus(instancePtr, sites,tiles,D_Tx,count,X_pos, Y_pos,X_Dpos,Y_Dpos, siteName,tileName,TILE_MAP);
					count = temp;
					}
					else if (siteType.compare("RAMB36E1")==0){
					temp = count;
					count = count2;
					std::string siteName = instancePtr->getSite();
					std::string tileName = instancePtr->getTile();
					RAMBMovementXminus(instancePtr, sites,tiles,D_Tx,count,X_pos,Y_pos,X_Dpos,Y_Dpos, siteName,tileName,TILE_MAP);
					count = temp;
					}
					else if (siteType.compare("TIEOFF")==0){
					temp = count;
					count = count2;
					std::string siteName = instancePtr->getSite();
					std::string tileName = instancePtr->getTile();
					TIEOFFMovementX(instancePtr, sites,tiles,D_Tx,X_pos,Y_pos,X_Dpos,Y_Dpos, siteName,tileName,TILE_MAP);	
					count = temp;
					}
					else{
					instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(a-count2)));
					CheckCLBValidity(instancePtr, sites, tiles, siteType,tileName,TILE_MAP);
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
	{
	std::string subtile = tileName.substr(X_Dpos-1);
	std::cout<<"subtile:  "<<subtile<<std::endl;
	OtherTileLocation_SET.insert(subtile);
	std::cout<<"This is no SLCIEL or SLICEM!"<<std::endl;

	}
	
	pInstance++;	
	}	



	NetsProcessing(designPtr, sites, tiles,TILE_MAP, OtherTileLocation_SET);
	// export the XDL design
	std::map<std::string,std::string>::iterator itt = TILE_MAP.begin();
	
	for(itt=TILE_MAP.begin();itt!=TILE_MAP.end();++itt){
	std::cout<<itt->first << " => "<<itt->second <<std::endl;
	}
	std::string outFileName = boost::filesystem::path(inFileName).replace_extension().string() 
		+ "_mod.xdl";
	std::fstream xdlExport(outFileName.c_str(), std::ios_base::out);
	torc::physical::XdlExporter fileExporter(xdlExport);
	fileExporter(designPtr);

	return 0;
}
//This function is used to check the validity of one-time  post-move location.
void CheckCLBValidity(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles,std::string siteType, std::string tileName,std::map<std::string,std::string>& TILE_MAP){
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
				std::string tileName_New = std::string(Name);
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
				//map<std::string,std::string> m =map_list_of(tileName,tileName_New); 
			  std::cout<< "MAp size: "<<TILE_MAP.size() <<std::endl;
				}
			}
		else{
			std::cout<<"The New CLB location does not exist!"<<std::endl;
			exit(0);}
	}
	
void DSPMovementXplus(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, int D_Tx, int count,int X_pos, int Y_pos,
int X_Dpos, int Y_Dpos, std::string siteName, std::string tileName,std::map<std::string,std::string>& TILE_MAP){
	X_pos = siteName.find("X");
	Y_pos = siteName.find("Y");
	std::string tileName_New = tileName;
	X_Dpos = tileName_New.find("X");
	Y_Dpos = tileName_New.find("Y");
	switch (D_Tx+count)
		{
		case 9:
			//siteName.replace(3,1,"R");
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(0)));
		//	std::string tileName_New = tileName;//.replace(4,1,"R");
			tileName_New.replace(4,1,"R");
			instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx+count)));
			TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));						
			std::cout<< instancePtr->getSite()<<std::endl;
			std::cout<< instancePtr->getTile()<<std::endl;
			break;
		case 14:
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(1)));
		//	std::string tileName_New = tileName;
			tileName_New.replace(4,1,"L");
			instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx+count)));	
			TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));				
			std::cout<< instancePtr->getSite()<<std::endl;
			std::cout<< instancePtr->getTile()<<std::endl;
			break;
		case 25:
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(2)));
		//	std::string tileName_New = tileName;
			tileName_New.replace(4,1,"R");	
			instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx+count)));
			TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));	
			std::cout<< instancePtr->getSite()<<std::endl;
			std::cout<< instancePtr->getTile()<<std::endl;
			break;
		case 59:
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(3)));
		//	std::string tileName_New = tileName;	
			tileName_New.replace(4,1,"R");
			instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx+count)));		
			TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));		
			std::cout<< instancePtr->getSite()<<std::endl;
			std::cout<< instancePtr->getTile()<<std::endl;
			break;
		case 64:
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(4)));
		//	std::string tileName_New = tileName;		
			tileName_New.replace(4,1,"L");
			instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx+count)));		
			TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));					
			std::cout<< instancePtr->getSite()<<std::endl;
			std::cout<< instancePtr->getTile()<<std::endl;
			break;
		default:
			std::cout<<"DSP movement X plus Error!"<<std::endl;
			exit(0);
					}	
	}
	
void DSPMovementXminus(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, int D_Tx, int count,int X_pos, int Y_pos,
int X_Dpos, int Y_Dpos, std::string siteName, std::string tileName,std::map<std::string,std::string>& TILE_MAP){
	X_pos = siteName.find("X");
	Y_pos = siteName.find("Y");
	std::string tileName_New = tileName;
	X_Dpos = tileName_New.find("X");
	Y_Dpos = tileName_New.find("Y");
	switch (D_Tx-count)
		{
		case 9:
			//siteName.replace(3,1,"R");
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(0)));
			//std::string tileName_New = tileName;	
			tileName_New.replace(4,1,"R");
			instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx-count)));	
			TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));							
			std::cout<< instancePtr->getSite()<<std::endl;
			std::cout<< instancePtr->getTile()<<std::endl;
			break;
		case 14:
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(1)));
			//std::string tileName_New = tileName;	
			tileName_New.replace(4,1,"L");
			instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx-count)));		
			TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));				
			std::cout<< instancePtr->getSite()<<std::endl;
			std::cout<< instancePtr->getTile()<<std::endl;
			break;
		case 25:
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(2)));
		//	std::string tileName_New = tileName;	
			tileName_New.replace(4,1,"R");	
			instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx-count)));
			TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));		
			std::cout<< instancePtr->getSite()<<std::endl;
			std::cout<< instancePtr->getTile()<<std::endl;
			break;
		case 59:
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(3)));
		//	std::string tileName_New = tileName;	
			tileName_New.replace(4,1,"R");
			instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx-count)));		
			TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));		
			std::cout<< instancePtr->getSite()<<std::endl;
			std::cout<< instancePtr->getTile()<<std::endl;
			break;
		case 64:
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(4)));
		//	std::string tileName_New = tileName;	
			tileName_New.replace(4,1,"L");
			instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx-count)));			
			TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));			
			std::cout<< instancePtr->getSite()<<std::endl;
			std::cout<< instancePtr->getTile()<<std::endl;
			break;
		default:
			std::cout<<"DSP movement X minus Error!"<<std::endl;
			exit(0);
					}	
	}	
	
	
void DSPMovementYplus(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, int D_Ty, int count,int Y_pos,
int Y_Dpos, int b ,std::string siteName, std::string tileName, int D_flag,std::map<std::string,std::string>& TILE_MAP){	
	std::cout<<"D_Ty: "<<D_Ty<<"  b:"<<b<<std::endl;
	if(float((D_Ty/5)*2)==float(b)){ D_flag = 0;}
	else {D_flag = 1;}
//	std::cout<<D_flag<<std::endl;
	//I can also use the (D_Ty+count)%5 to check
	Y_pos = siteName.find("Y");
	Y_Dpos = tileName.find("Y");
	std::string tileName_New = tileName;
	instancePtr->setTile(tileName_New.replace(Y_Dpos+1,tileName_New.length()-Y_Dpos, boost::lexical_cast<std::string>(D_Ty+count)));
//	std::cout<<"after Y plus tileName: "<<instancePtr->getTile()<<std::endl;
	TileIndex new_tile_index = tiles.findTileIndex(instancePtr->getTile());
	//If exists, change the site
	if(!new_tile_index.isUndefined()){
		TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));	
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
int Y_Dpos, int b ,std::string siteName, std::string tileName, int D_flag, std::map<std::string,std::string>& TILE_MAP){	
	std::cout<<"D_Ty: "<<D_Ty<<"  b:"<<b<<std::endl;
	if(float((D_Ty/5)*2)==float(b)){ D_flag = 0;}
	else {D_flag = 1;}
	std::cout<<D_flag<<std::endl;
	//I can also use the (D_Ty+count)%5 to check
	std::string tileName_New = tileName;
	std::cout<<"TileName in Y minus: "<<tileName << std::endl;
	Y_pos = siteName.find("Y");
	Y_Dpos = tileName.find("Y");
	instancePtr->setTile(tileName_New.replace(Y_Dpos+1,tileName_New.length()-Y_Dpos, boost::lexical_cast<std::string>(D_Ty-count)));
	std::cout<<"after Y minus check tileName: "<<instancePtr->getTile()<<std::endl;
	TileIndex new_tile_index = tiles.findTileIndex(instancePtr->getTile());
	//If exists, change the site
	std::cout<<"SiteName in Y minus:  "<<siteName<<std::endl;
	if(!new_tile_index.isUndefined()){
		TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));	
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
int X_Dpos, int Y_Dpos, std::string siteName, std::string tileName, std::map<std::string,std::string>& TILE_MAP){
	X_pos = siteName.find("X");
	Y_pos = siteName.find("Y");
	std::string tileName_New = tileName;
	X_Dpos = tileName_New.find("X");
	Y_Dpos = tileName_New.find("Y");
	switch (D_Tx+count)
		{
		case 6:
			//siteName.replace(3,1,"R");
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(0)));
			tileName_New.replace(4,1,"L");
			instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx+count)));		
			TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));						
			std::cout<<"RAMB site: "<<instancePtr->getSite()<<std::endl;
			std::cout<<"RAMB tile: "<<instancePtr->getTile()<<std::endl;
			break;
		case 17:
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(1)));
			tileName_New.replace(4,1,"R");
			instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx+count)));	
			TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));					
			std::cout<<"RAMB site: "<< instancePtr->getSite()<<std::endl;
			std::cout<<"RAMB tile: "<< instancePtr->getTile()<<std::endl;
			break;
		case 22:
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(2)));
			tileName_New.replace(4,1,"L");	
			instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx+count)));
			TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));		
			std::cout<< "RAMB site: "<<instancePtr->getSite()<<std::endl;
			std::cout<< "RAMB tile: "<<instancePtr->getTile()<<std::endl;
			break;
		case 36:
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(3)));
			tileName_New.replace(4,1,"L");
			instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx+count)));		
			TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));		
			std::cout<<"RAMB site:" << instancePtr->getSite()<<std::endl;
			std::cout<<"RAMB tile:" << instancePtr->getTile()<<std::endl;
			break;
		case 56:
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(4)));
			tileName_New.replace(4,1,"L");
			instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx+count)));			
			TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));				
			std::cout<<"RAMB site: "<< instancePtr->getSite()<<std::endl;
			std::cout<<"RAMB tile: "<< instancePtr->getTile()<<std::endl;
			break;
		case 67:
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(5)));
			tileName_New.replace(4,1,"R");
			instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx+count)));			
			TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));				
			std::cout<<"RAMB site: "<< instancePtr->getSite()<<std::endl;
			std::cout<<"RAMB tile: "<< instancePtr->getTile()<<std::endl;
			break;
		default:
			std::cout<<"RAMB movement X plus Error!"<<std::endl;
			exit(0);
					}	
	}
	
void RAMBMovementXminus(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, int D_Tx, int count,int X_pos, int Y_pos,
int X_Dpos, int Y_Dpos, std::string siteName, std::string tileName, std::map<std::string,std::string>& TILE_MAP){
	X_pos = siteName.find("X");
	Y_pos = siteName.find("Y");
	std::string tileName_New = tileName;
	X_Dpos = tileName_New.find("X");
	Y_Dpos = tileName_New.find("Y");
	switch (D_Tx-count)
		{
		case 6:
			//siteName.replace(3,1,"R");
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(0)));
			tileName_New.replace(4,1,"L");
			instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx-count)));	
			TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));						
			std::cout<<"RAMB site: "<<instancePtr->getSite()<<std::endl;
			std::cout<<"RAMB tile: "<<instancePtr->getTile()<<std::endl;
			break;
		case 17:
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(1)));
			tileName_New.replace(4,1,"R");
			instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx-count)));			
			TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));		
			std::cout<<"RAMB site: "<< instancePtr->getSite()<<std::endl;
			std::cout<<"RAMB tile: "<< instancePtr->getTile()<<std::endl;
			break;
		case 22:
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(2)));
			tileName_New.replace(4,1,"L");	
			instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx-count)));
			TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));	
			std::cout<< "RAMB site: "<<instancePtr->getSite()<<std::endl;
			std::cout<< "RAMB tile: "<<instancePtr->getTile()<<std::endl;
			break;
		case 36:
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(3)));
			tileName_New.replace(4,1,"L");
			instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx-count)));		
			TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));	
			std::cout<<"RAMB site: "<< instancePtr->getSite()<<std::endl;
			std::cout<<"RAMB tile: "<< instancePtr->getTile()<<std::endl;
			break;
		case 56:
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(4)));
			tileName_New.replace(4,1,"L");
			instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx-count)));			
			TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));			
			std::cout<<"RAMB site: "<< instancePtr->getSite()<<std::endl;
			std::cout<<"RAMB tile: "<< instancePtr->getTile()<<std::endl;
			break;
		case 67:
			instancePtr->setSite(siteName.replace(X_pos+1, Y_pos-X_pos-1, boost::lexical_cast<std::string>(5)));
			tileName_New.replace(4,1,"R");
			instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx-count)));		
			TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));				
			std::cout<<"RAMB site: "<< instancePtr->getSite()<<std::endl;
			std::cout<<"RAMB tile: "<< instancePtr->getTile()<<std::endl;
			break;
		default:
			std::cout<<"RAMB movement X minus Error!"<<std::endl;
			exit(0);
					}	
	}
	
	
void RAMBMovementYplus(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, int D_Ty, int count,int Y_pos,
int Y_Dpos,std::string siteName, std::string tileName, std::map<std::string,std::string>& TILE_MAP){	
	Y_pos = siteName.find("Y");
	std::string tileName_New = tileName;
	Y_Dpos = tileName_New.find("Y");
	instancePtr->setTile(tileName_New.replace(Y_Dpos+1,tileName_New.length()-Y_Dpos, boost::lexical_cast<std::string>(D_Ty+count)));
//	std::cout<<"after Y plus tileName: "<<instancePtr->getTile()<<std::endl;
	TileIndex new_tile_index = tiles.findTileIndex(instancePtr->getTile());
	//If exists, change the site
	if(!new_tile_index.isUndefined()){
			instancePtr->setSite(siteName.replace(Y_pos+1, siteName.length()-Y_pos,boost::lexical_cast<std::string>((D_Ty+count)/5)));
			TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));	
			std::cout<<"RAMB after Y site:"<< instancePtr->getSite()<<std::endl;
			std::cout<<"RAMB after Y tile:"<< instancePtr->getTile()<<std::endl;	
		}
	else{ std::cout<<"RAMB movement Y plus Error!"<<std::endl;
		exit(0);
	}
}	

void RAMBMovementYminus(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, int D_Ty, int count,int Y_pos,
int Y_Dpos,std::string siteName, std::string tileName,std::map<std::string,std::string>& TILE_MAP){	
	Y_pos = siteName.find("Y");
	std::string tileName_New = tileName;
	Y_Dpos = tileName_New.find("Y");
	instancePtr->setTile(tileName_New.replace(Y_Dpos+1,tileName_New.length()-Y_Dpos, boost::lexical_cast<std::string>(D_Ty-count)));
//	std::cout<<"after Y plus tileName: "<<instancePtr->getTile()<<std::endl;
	TileIndex new_tile_index = tiles.findTileIndex(instancePtr->getTile());
	//If exists, change the site
	if(!new_tile_index.isUndefined()){
			instancePtr->setSite(siteName.replace(Y_pos+1, siteName.length()-Y_pos,boost::lexical_cast<std::string>((D_Ty-count)/5)));
			TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));		
			std::cout<<"RAMB after Y site:"<< instancePtr->getSite()<<std::endl;
			std::cout<<"RAMB after Y tile:"<< instancePtr->getTile()<<std::endl;	
		}
	else{ std::cout<<"RAMB movement Y minus Error!"<<std::endl;
		exit(0);
	}
}

void TIEOFFMovementX(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, int D_Tx,int X_pos, int Y_pos,
int X_Dpos, int Y_Dpos, std::string siteName, std::string tileName,std::map<std::string,std::string>& TILE_MAP){
	X_pos = siteName.find("X");
	Y_pos = siteName.find("Y");
	//std::map<std::string,std::string>::iterator check;
	std::string tileName_New = tileName;
	std::string CLB_tile;
	X_Dpos = tileName_New.find("X");
	Y_Dpos = tileName_New.find("Y");
	std::string subtile = tileName_New.substr(5);
	std::cout<<"sub: "<<subtile<<" tilename: "<<tileName_New<<std::endl;
	std::string check1 = "CLBLM_R"; check1.append(subtile);
	std::cout<<"check1: "<<check1<<std::endl;
	std::string check2 = "CLBLM_L"; check2.append(subtile);	
	std::string check3 = "CLBLL_R"; check3.append(subtile);
	std::string check4 = "CLBLL_L"; check4.append(subtile);
	int Mark,Xc_Dpos,Yc_Dpos;
	//check = TILE_MAP.find(tilename);
	if (TILE_MAP.find(check1)!=TILE_MAP.end()){
		Mark = 1;
	}
	else if (TILE_MAP.find(check2)!=TILE_MAP.end()){
		Mark = 2;
	}
	else if (TILE_MAP.find(check3)!=TILE_MAP.end()){
		Mark = 3;
	}
	else if (TILE_MAP.find(check4)!=TILE_MAP.end()){
		Mark = 4;
	}
	else{ Mark = 0;}
	
	switch (Mark)
		{
		case 1:
			CLB_tile = TILE_MAP.find(check1)->second;
			Xc_Dpos = CLB_tile.find("X");
			Yc_Dpos = CLB_tile.find("Y");
			D_Tx = atoi((CLB_tile.substr(Xc_Dpos+1,Yc_Dpos - Xc_Dpos-1)).c_str());	
			if((D_Tx%2==0)&&(D_Tx>=0)&&(D_Tx<=9)){
				tileName_New.replace(4,1,"L");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}	
			else if((D_Tx%2==0)&&(D_Tx>=10)&&(D_Tx<=13)){
				tileName_New.replace(4,1,"L");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+1)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			else if((D_Tx%2==0)&&(D_Tx>=14)&&(D_Tx<=25)){
				tileName_New.replace(4,1,"L");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+2)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			else if((D_Tx%2==0)&&(D_Tx>=26)&&(D_Tx<=59)){
				tileName_New.replace(4,1,"L");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+3)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			else if((D_Tx%2==0)&&(D_Tx>=60)&&(D_Tx<=63)){
				tileName_New.replace(4,1,"L");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+4)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			else if((D_Tx%2==0)&&(D_Tx>=64)&&(D_Tx<=73)){
				tileName_New.replace(4,1,"L");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+5)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			
			else if((D_Tx%2==1)&&(D_Tx>=0)&&(D_Tx<=9)){
				tileName_New.replace(4,1,"R");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}	
			else if((D_Tx%2==1)&&(D_Tx>=10)&&(D_Tx<=13)){
				tileName_New.replace(4,1,"R");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+1)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			else if((D_Tx%2==1)&&(D_Tx>=14)&&(D_Tx<=25)){
				tileName_New.replace(4,1,"R");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+2)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			else if((D_Tx%2==1)&&(D_Tx>=26)&&(D_Tx<=59)){
				tileName_New.replace(4,1,"R");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+3)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			else if((D_Tx%2==1)&&(D_Tx>=60)&&(D_Tx<=63)){
				tileName_New.replace(4,1,"R");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+4)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			else if((D_Tx%2==1)&&(D_Tx>=64)&&(D_Tx<=73)){
				tileName_New.replace(4,1,"R");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+5)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			else{std::cout<<"TIEOFF X move error"<<std::endl;exit(0);}
				
			std::cout<<"TIEOFF moved site:"<< instancePtr->getSite()<<std::endl;
			std::cout<<"TIEOFF moved tile:"<< instancePtr->getTile()<<std::endl;
			break;
		case 2:
			CLB_tile = TILE_MAP.find(check2)->second;
		 	Xc_Dpos = CLB_tile.find("X");
			Yc_Dpos = CLB_tile.find("Y");
			D_Tx = atoi((CLB_tile.substr(Xc_Dpos+1,Yc_Dpos - Xc_Dpos-1)).c_str());	
			if((D_Tx%2==0)&&(D_Tx>=0)&&(D_Tx<=9)){
				tileName_New.replace(4,1,"L");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}	
			else if((D_Tx%2==0)&&(D_Tx>=10)&&(D_Tx<=13)){
				tileName_New.replace(4,1,"L");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+1)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			else if((D_Tx%2==0)&&(D_Tx>=14)&&(D_Tx<=25)){
				tileName_New.replace(4,1,"L");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+2)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			else if((D_Tx%2==0)&&(D_Tx>=26)&&(D_Tx<=59)){
				tileName_New.replace(4,1,"L");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+3)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			else if((D_Tx%2==0)&&(D_Tx>=60)&&(D_Tx<=63)){
				tileName_New.replace(4,1,"L");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+4)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			else if((D_Tx%2==0)&&(D_Tx>=64)&&(D_Tx<=73)){
				tileName_New.replace(4,1,"L");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+5)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			
			else if((D_Tx%2==1)&&(D_Tx>=0)&&(D_Tx<=9)){
				tileName_New.replace(4,1,"R");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}	
			else if((D_Tx%2==1)&&(D_Tx>=10)&&(D_Tx<=13)){
				tileName_New.replace(4,1,"R");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+1)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			else if((D_Tx%2==1)&&(D_Tx>=14)&&(D_Tx<=25)){
				tileName_New.replace(4,1,"R");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+2)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			else if((D_Tx%2==1)&&(D_Tx>=26)&&(D_Tx<=59)){
				tileName_New.replace(4,1,"R");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+3)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			else if((D_Tx%2==1)&&(D_Tx>=60)&&(D_Tx<=63)){
				tileName_New.replace(4,1,"R");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+4)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			else if((D_Tx%2==1)&&(D_Tx>=64)&&(D_Tx<=73)){
				tileName_New.replace(4,1,"R");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+5)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			else{std::cout<<"TIEOFF X move error"<<std::endl;exit(0);}
				
			std::cout<<"TIEOFF moved site:"<< instancePtr->getSite()<<std::endl;
			std::cout<<"TIEOFF moved tile:"<< instancePtr->getTile()<<std::endl;
			break;
		case 3:
			CLB_tile = TILE_MAP.find(check3)->second;
			Xc_Dpos = CLB_tile.find("X");
			Yc_Dpos = CLB_tile.find("Y");
			D_Tx = atoi((CLB_tile.substr(Xc_Dpos+1,Yc_Dpos - Xc_Dpos-1)).c_str());	
			if((D_Tx%2==0)&&(D_Tx>=0)&&(D_Tx<=9)){
				tileName_New.replace(4,1,"L");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}	
			else if((D_Tx%2==0)&&(D_Tx>=10)&&(D_Tx<=13)){
				tileName_New.replace(4,1,"L");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+1)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			else if((D_Tx%2==0)&&(D_Tx>=14)&&(D_Tx<=25)){
				tileName_New.replace(4,1,"L");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+2)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			else if((D_Tx%2==0)&&(D_Tx>=26)&&(D_Tx<=59)){
				tileName_New.replace(4,1,"L");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+3)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			else if((D_Tx%2==0)&&(D_Tx>=60)&&(D_Tx<=63)){
				tileName_New.replace(4,1,"L");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+4)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			else if((D_Tx%2==0)&&(D_Tx>=64)&&(D_Tx<=73)){
				tileName_New.replace(4,1,"L");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+5)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			
			else if((D_Tx%2==1)&&(D_Tx>=0)&&(D_Tx<=9)){
				tileName_New.replace(4,1,"R");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}	
			else if((D_Tx%2==1)&&(D_Tx>=10)&&(D_Tx<=13)){
				tileName_New.replace(4,1,"R");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+1)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			else if((D_Tx%2==1)&&(D_Tx>=14)&&(D_Tx<=25)){
				tileName_New.replace(4,1,"R");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+2)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			else if((D_Tx%2==1)&&(D_Tx>=26)&&(D_Tx<=59)){
				tileName_New.replace(4,1,"R");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+3)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			else if((D_Tx%2==1)&&(D_Tx>=60)&&(D_Tx<=63)){
				tileName_New.replace(4,1,"R");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+4)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			else if((D_Tx%2==1)&&(D_Tx>=64)&&(D_Tx<=73)){
				tileName_New.replace(4,1,"R");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+5)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			else{std::cout<<"TIEOFF X move error"<<std::endl;exit(0);}
				
			std::cout<<"TIEOFF moved site:"<< instancePtr->getSite()<<std::endl;
			std::cout<<"TIEOFF moved tile:"<< instancePtr->getTile()<<std::endl;
			break;
		case 4:
			CLB_tile = TILE_MAP.find(check4)->second;
			Xc_Dpos = CLB_tile.find("X");
			Yc_Dpos = CLB_tile.find("Y");
			D_Tx = atoi((CLB_tile.substr(Xc_Dpos+1,Yc_Dpos - Xc_Dpos-1)).c_str());	
			if((D_Tx%2==0)&&(D_Tx>=0)&&(D_Tx<=9)){
				tileName_New.replace(4,1,"L");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}	
			else if((D_Tx%2==0)&&(D_Tx>=10)&&(D_Tx<=13)){
				tileName_New.replace(4,1,"L");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+1)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			else if((D_Tx%2==0)&&(D_Tx>=14)&&(D_Tx<=25)){
				tileName_New.replace(4,1,"L");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+2)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			else if((D_Tx%2==0)&&(D_Tx>=26)&&(D_Tx<=59)){
				tileName_New.replace(4,1,"L");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+3)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			else if((D_Tx%2==0)&&(D_Tx>=60)&&(D_Tx<=63)){
				tileName_New.replace(4,1,"L");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+4)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			else if((D_Tx%2==0)&&(D_Tx>=64)&&(D_Tx<=73)){
				tileName_New.replace(4,1,"L");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+5)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			
			else if((D_Tx%2==1)&&(D_Tx>=0)&&(D_Tx<=9)){
				tileName_New.replace(4,1,"R");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}	
			else if((D_Tx%2==1)&&(D_Tx>=10)&&(D_Tx<=13)){
				tileName_New.replace(4,1,"R");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+1)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			else if((D_Tx%2==1)&&(D_Tx>=14)&&(D_Tx<=25)){
				tileName_New.replace(4,1,"R");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+2)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			else if((D_Tx%2==1)&&(D_Tx>=26)&&(D_Tx<=59)){
				tileName_New.replace(4,1,"R");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+3)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			else if((D_Tx%2==1)&&(D_Tx>=60)&&(D_Tx<=63)){
				tileName_New.replace(4,1,"R");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+4)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			else if((D_Tx%2==1)&&(D_Tx>=64)&&(D_Tx<=73)){
				tileName_New.replace(4,1,"R");
				instancePtr->setTile(tileName_New.replace(X_Dpos+1,Y_Dpos-X_Dpos-1, boost::lexical_cast<std::string>(D_Tx)));
				instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1, boost::lexical_cast<std::string>(D_Tx+5)));
				TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));
			}
			else{std::cout<<"TIEOFF X move error"<<std::endl;exit(0);}
				
			std::cout<<"TIEOFF moved site:"<< instancePtr->getSite()<<std::endl;
			std::cout<<"TIEOFF moved tile:"<< instancePtr->getTile()<<std::endl;
			break;
		case 0:
			std::cout<<"TIEOFF can not find the identical CLB"<<std::endl;
			break;
		default:
			std::cout<<"TIEOFF unknown error!"<<std::endl;
			exit(0);
					}	
	}

void TIEOFFMovementYplus(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, int D_Ty, int count,int Y_pos,
int Y_Dpos,std::string siteName, std::string tileName, std::map<std::string,std::string>& TILE_MAP){	
	Y_pos = siteName.find("Y");
	std::string tileName_New = tileName;
	Y_Dpos = tileName_New.find("Y");
	std::string subtile = tileName_New.substr(5);
	std::cout<<"sub: "<<subtile<<" tilename: "<<tileName_New<<std::endl;
	std::string check1 = "CLBLM_R"; check1.append(subtile);
	std::cout<<"check1: "<<check1<<std::endl;
	std::string check2 = "CLBLM_L"; check2.append(subtile);	
	std::string check3 = "CLBLL_R"; check3.append(subtile);
	std::string check4 = "CLBLL_L"; check4.append(subtile);	
	if (TILE_MAP.find(check1)!=TILE_MAP.end() || TILE_MAP.find(check2)!=TILE_MAP.end() || TILE_MAP.find(check3)!=TILE_MAP.end() || TILE_MAP.find(check4)!=TILE_MAP.end()){
		instancePtr->setTile(tileName_New.replace(Y_Dpos+1,tileName_New.length()-Y_Dpos, boost::lexical_cast<std::string>(D_Ty+count)));
		TileIndex new_tile_index = tiles.findTileIndex(instancePtr->getTile());
		if(!new_tile_index.isUndefined()){
			instancePtr->setSite(siteName.replace(Y_pos+1, siteName.length()-Y_pos,boost::lexical_cast<std::string>((D_Ty+count))));
			TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));	
			std::cout<<"TIEOFF after Y site:"<< instancePtr->getSite()<<std::endl;
			std::cout<<"TIEOFF after Y tile:"<< instancePtr->getTile()<<std::endl;	
			}
		else{ std::cout<<"TIEOFF movement Y plus Error!"<<std::endl;
			exit(0);
		}
	}
	else{std::cout<<"TIEOFF can not find the identical CLB"<<std::endl;}
}

void TIEOFFMovementYminus(InstanceSharedPtr instancePtr, const Sites &sites, const Tiles &tiles, int D_Ty, int count,int Y_pos,
int Y_Dpos,std::string siteName, std::string tileName, std::map<std::string,std::string>& TILE_MAP){	
	Y_pos = siteName.find("Y");
	std::string tileName_New = tileName;
	Y_Dpos = tileName_New.find("Y");
	std::string subtile = tileName_New.substr(5);
	std::cout<<"sub: "<<subtile<<" tilename: "<<tileName_New<<std::endl;
	std::string check1 = "CLBLM_R"; check1.append(subtile);
	std::cout<<"check1: "<<check1<<std::endl;
	std::string check2 = "CLBLM_L"; check2.append(subtile);	
	std::string check3 = "CLBLL_R"; check3.append(subtile);
	std::string check4 = "CLBLL_L"; check4.append(subtile);	
	if (TILE_MAP.find(check1)!=TILE_MAP.end() || TILE_MAP.find(check2)!=TILE_MAP.end() || TILE_MAP.find(check3)!=TILE_MAP.end() || TILE_MAP.find(check4)!=TILE_MAP.end()){
		instancePtr->setTile(tileName_New.replace(Y_Dpos+1,tileName_New.length()-Y_Dpos, boost::lexical_cast<std::string>(D_Ty-count)));
		TileIndex new_tile_index = tiles.findTileIndex(instancePtr->getTile());
		if(!new_tile_index.isUndefined()){
			instancePtr->setSite(siteName.replace(Y_pos+1, siteName.length()-Y_pos,boost::lexical_cast<std::string>((D_Ty-count))));
			TILE_MAP.insert(std::pair<std::string,std::string>(tileName,tileName_New));	
			std::cout<<"TIEOFF after Y site:"<< instancePtr->getSite()<<std::endl;
			std::cout<<"TIEOFF after Y tile:"<< instancePtr->getTile()<<std::endl;	
			}
		else{ std::cout<<"TIEOFF movement Y minus Error!"<<std::endl;
			exit(0);
		}
	}
	else{std::cout<<"TIEOFF can not find the identical CLB"<<std::endl;}
}

void NetsProcessing(DesignSharedPtr designPtr, const Sites &sites, const Tiles &tiles, std::map<std::string,std::string>& TILE_MAP, std::set<std::string>& OtherTileLocation_SET){
	NetSharedPtrVector::const_iterator pNets = designPtr -> netsBegin();
	NetSharedPtrVector::const_iterator eNets = designPtr -> netsEnd();	
	std::map<std::string,std::string>::iterator it;
	//The new PIP set
	std::set<std::string> AddPIP_SET;
	//Other INT type set
	std::set<std::string> OtherINT_SET;
	//RIO type set, to check the same tile location in same net
	std::set<std::string> RIO_SET;
	std::cout<<"Working on Nets... count: "<< designPtr->getNetCount() << std::endl;
	std::map<std::string,std::string>::iterator itt = TILE_MAP.begin();
	int X_T,Y_T,Tieoff_X;
	for(itt=TILE_MAP.begin();itt!=TILE_MAP.end();++itt){
	std::cout<<itt->first << " => "<<itt->second <<std::endl;
	}
	while(pNets != eNets){
		NetSharedPtr netPtr = *pNets;
		int mark;
		std::cout<<" Net: "<<netPtr->getName() <<std::endl;
//		std::cout<<"Net Type: "<<netPtr->getNetType() <<std::endl;	
		//go over all the pips in the net
		Net::PipConstIterator pPips = netPtr->pipsBegin();
		Net::PipConstIterator ePips = netPtr->pipsEnd();
		while(pPips != ePips){
	//	EPipDirection indirection = pPips ->getDirection();
		TileName tilename = pPips->getTileName();
		it = TILE_MAP.find(tilename);
		/*std::cout<<"we should delete: "<<tilename<<" also IT: "<<it->first<<std::endl;
	X_pos = siteName.find("X");
	Y_pos = siteName.find("Y");
	int a = atoi((siteName.substr(X_pos+1,Y_pos - X_pos-1)).c_str());
		RoutethroughSharedPtr route = pPips -> getRoutethroughPtr();
		WireName sourcename = pPips -> getSourceWireName();
		WireName sinkname = pPips -> getSinkWireName();*/
		//exist
		if(it!=TILE_MAP.end()){// && netPtr->removePip(*pPips)){
		std::cout<<"we should delete: "<<tilename<<" also IT: "<<it->first<<" and its second: "<<it->second<<std::endl;
		RoutethroughSharedPtr route = pPips -> getRoutethroughPtr();
		WireName sourcename = pPips -> getSourceWireName();
		WireName sinkname = pPips -> getSinkWireName();
		torc::physical::Pip newpip = Factory::newPip(it->second,sourcename,sinkname, ePipUnidirectionalBuffered,route);
		if(netPtr->removePip(*pPips)){
	//	std::cout<<"delete pip !"<<std::endl;
		}	
		//std::cout<<"Pips name:  "<< newpip.getTileName()<<std::endl;
		netPtr->addPip(*(&newpip));
		AddPIP_SET.insert(it->second);
		pPips--;
			
		}
		else{
		std::string subtieoff = tilename.substr(5);
		std::string subother = tilename.substr(0,3);//INT
		std::cout<<"subOther: "<<subother<<std::endl;
		std::cout<<"sub: "<<subtieoff<<" tilename: "<<tilename<<std::endl;
		std::string check1 = "CLBLM_R"; check1.append(subtieoff);
		std::cout<<"check1: "<<check1<<std::endl;
		std::string check2 = "CLBLM_L"; check2.append(subtieoff);	
		std::string check3 = "CLBLL_R"; check3.append(subtieoff);
		std::string check4 = "CLBLL_L"; check4.append(subtieoff);	
		std::string TIEOFF_L = "INT_L";
		std::string TIEOFF_R = "INT_R";
		std::string New_CLB;
		std::string New_TIE;
		if (TILE_MAP.find(check1)!=TILE_MAP.end()){
		
		X_T = check1.find("X");
		Y_T = check1.find("Y");
		Tieoff_X = atoi((check1.substr(X_T+1,Y_T - X_T-1)).c_str());

		std::cout<<"we should delete: "<<tilename<<std::endl;
		RoutethroughSharedPtr route = pPips -> getRoutethroughPtr();
		WireName sourcename = pPips -> getSourceWireName();
		WireName sinkname = pPips -> getSinkWireName();
		New_CLB = TILE_MAP.find(check1)->second;
		New_CLB = New_CLB.substr(7);
		if(Tieoff_X % 2 == 0){
		New_TIE = TIEOFF_L.append(New_CLB);
		std::cout<<"the new tieoff is: "<<New_TIE<<std::endl;
		}
		else{
		New_TIE = TIEOFF_R.append(New_CLB);
		std::cout<<"the new tieoff is: "<<New_TIE<<std::endl;
		}
		
		torc::physical::Pip newpip = Factory::newPip(New_TIE,sourcename,sinkname, ePipUnidirectionalBuffered,route);
		if(netPtr->removePip(*pPips)){
		std::cout<<"delete pip !"<<std::endl;
		}	
		std::cout<<"Pips name:  "<< newpip.getTileName()<<std::endl;
		netPtr->addPip(*(&newpip));
		AddPIP_SET.insert(New_TIE);
		pPips--;
		
		}
		else if(TILE_MAP.find(check2)!=TILE_MAP.end()){

		X_T = check2.find("X");
		Y_T = check2.find("Y");
		Tieoff_X = atoi((check2.substr(X_T+1,Y_T - X_T-1)).c_str());
		std::cout<<"we should delete: "<<tilename<<std::endl;
		RoutethroughSharedPtr route = pPips -> getRoutethroughPtr();
		WireName sourcename = pPips -> getSourceWireName();
		WireName sinkname = pPips -> getSinkWireName();
		New_CLB = TILE_MAP.find(check2)->second;
		New_CLB = New_CLB.substr(7);
		if(Tieoff_X % 2 == 0){
		New_TIE = TIEOFF_L.append(New_CLB);
		std::cout<<"the new tieoff is: "<<New_TIE<<std::endl;
		}
		else{
		New_TIE = TIEOFF_R.append(New_CLB);
		std::cout<<"the new tieoff is: "<<New_TIE<<std::endl;
		}
		torc::physical::Pip newpip = Factory::newPip(New_TIE,sourcename,sinkname, ePipUnidirectionalBuffered,route);
		if(netPtr->removePip(*pPips)){
		std::cout<<"delete pip !"<<std::endl;
		}	
		std::cout<<"Pips name:  "<< newpip.getTileName()<<std::endl;
		netPtr->addPip(*(&newpip));
		AddPIP_SET.insert(New_TIE);
		pPips--;
		
		}

		else if( TILE_MAP.find(check3)!=TILE_MAP.end()){
		std::cout<<"we should delete: "<<tilename<<std::endl;
		X_T = check3.find("X");
		Y_T = check3.find("Y");
		Tieoff_X = atoi((check3.substr(X_T+1,Y_T - X_T-1)).c_str());
		RoutethroughSharedPtr route = pPips -> getRoutethroughPtr();
		WireName sourcename = pPips -> getSourceWireName();
		WireName sinkname = pPips -> getSinkWireName();
		New_CLB = TILE_MAP.find(check3)->second;
		New_CLB = New_CLB.substr(7);
		if(Tieoff_X % 2 == 0){
			New_TIE = TIEOFF_L.append(New_CLB);
			std::cout<<"the new tieoff is: "<<New_TIE<<std::endl;
		}
		else{
			New_TIE = TIEOFF_R.append(New_CLB);
			std::cout<<"the new tieoff is: "<<New_TIE<<std::endl;
		}
	
		
		torc::physical::Pip newpip = Factory::newPip(New_TIE,sourcename,sinkname, ePipUnidirectionalBuffered,route);
			if(netPtr->removePip(*pPips)){
		std::cout<<"delete pip !"<<std::endl;
		}	
		std::cout<<"Pips name:  "<< newpip.getTileName()<<std::endl;
		netPtr->addPip(*(&newpip));
		AddPIP_SET.insert(New_TIE);
		pPips--;

		}
		else if( TILE_MAP.find(check4)!=TILE_MAP.end()){	
		X_T = check4.find("X");
		Y_T = check4.find("Y");
		Tieoff_X = atoi((check4.substr(X_T+1,Y_T - X_T-1)).c_str());
		std::cout<<"we should delete: "<<tilename<<std::endl;
		RoutethroughSharedPtr route = pPips -> getRoutethroughPtr();
		WireName sourcename = pPips -> getSourceWireName();
		WireName sinkname = pPips -> getSinkWireName();
		New_CLB = TILE_MAP.find(check4)->second;
		New_CLB = New_CLB.substr(7);
		if(Tieoff_X % 2 == 0){
			New_TIE = TIEOFF_L.append(New_CLB);
			std::cout<<"the new tieoff is: "<<New_TIE<<std::endl;
		}
		else{
			New_TIE = TIEOFF_R.append(New_CLB);
			std::cout<<"the new tieoff is: "<<New_TIE<<std::endl;
		}
	torc::physical::Pip newpip = Factory::newPip(New_TIE,sourcename,sinkname, ePipUnidirectionalBuffered,route);
		if(netPtr->removePip(*pPips)){
		std::cout<<"delete pip !"<<std::endl;
		}	
		std::cout<<"Pips name:  "<< newpip.getTileName()<<std::endl;
		netPtr->addPip(*(&newpip));
		AddPIP_SET.insert(New_TIE);
		pPips--;
		}

		else if(subother.compare("INT")==0 && TILE_MAP.find(check1)==TILE_MAP.end()&&TILE_MAP.find(check2)==TILE_MAP.end()&&TILE_MAP.find(check3)==TILE_MAP.end()&&TILE_MAP.find(check4)==TILE_MAP.end()&&AddPIP_SET.find(tilename)==AddPIP_SET.end()){
		if ( OtherTileLocation_SET.find(subtieoff)!=OtherTileLocation_SET.end()){
		OtherINT_SET.insert(subtieoff);
		std::cout<<"This pip: "<<tilename<<" stay here now"<<std::endl;
		mark = 1;
		}
		else{
		if(netPtr->removePip(*pPips)){std::cout<<"delete pip: "<< tilename<<std::endl;}
		pPips--;
			}
		}
		else if(subother.compare("RIO")==0){
			X_T = tilename.find("X");
			RIO_SET.insert(tilename.substr(X_T-1));
			std::cout<<"part RIO: "<<tilename.substr(X_T-1)<<std::endl;
			std::cout<<"This is RIO pip: "<<tilename<<std::endl;
		}


		else{std::cout<<"This is :"<<tilename<<", we do not need move!"<<std::endl;}

		}		
	//	std::cout<<"Pips:  "<<route->getValue()<<std::endl;
		pPips++;
		}
		//second iterator
		if(mark==1){
		mark = 0;
		Net::PipConstIterator pPips2 = netPtr->pipsBegin();
		Net::PipConstIterator ePips2 = netPtr->pipsEnd();	
		while(pPips2 != ePips2){
		TileName tilename = pPips2->getTileName();
		std::string subtieoff = tilename.substr(5);//tile location
			if(OtherINT_SET.find(subtieoff)!=OtherINT_SET.end()){
				if (RIO_SET.find(subtieoff)!=RIO_SET.end()){
					std::cout<<"This pip: "<<tilename<<" stay here forever"<<std::endl;
					}
				else{
					if(netPtr->removePip(*pPips2)){std::cout<<"delete pip: "<< tilename<<std::endl;}
					pPips2--;
					}
				}
		pPips2++;
		}
		OtherINT_SET.clear();
		RIO_SET.clear();
		}
		
	pNets++;
	}
}		
