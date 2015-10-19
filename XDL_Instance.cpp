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
#include <set>

using namespace torc::common;
using namespace torc::architecture::xilinx;
using namespace torc::physical;
using namespace torc::architecture;
/// \brief Standard main() function.
/*
//Funtion declarations
*/

// Main function starts here	
int main(int argc, char* argv[]) {
	
	if(argc != 5) {
	std::cerr<< "Usage: "<< argv[0] << " <XDL_FILE_NAME> <Inst_Type>(CLB,DSP,BRAM) <Old_lLoc> <New_Loc> (Using 1,2 represents location) "<< std::endl;
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
//	int X_pos, Y_pos, X_Dpos, Y_Dpos,D_flag=-1;

	// import the XDL design
	std::string inFileName(argv[1]);
	std::fstream fileStream(inFileName.c_str());
	if(!fileStream.good()) {
	std::cout<< "The file does not exist! " << std::endl;
	return -1;
	}
	torc::physical::XdlImporter importer;
	importer(fileStream, inFileName);
	// what the type you want to move(CLB,DSP,RAMB)
	std::string Inst_Type(argv[2]);
	if (Inst_Type.compare("CLB")!=0 && Inst_Type.compare("DSP")!=0 && Inst_Type.compare("BRAM")!=0 )
	{std::cout<<"The type of instance does not exist"<<std::endl;
	exit(0);}

	std::string Old_Loc(argv[3]);
	std::vector<std::string> strs_old;
	// The old location you input
	boost::split(strs_old,Old_Loc,boost::is_any_of(","));

	int Old_X = atoi(strs_old[0].c_str());
	int Old_Y = atoi(strs_old[1].c_str());

	
	std::string New_Loc(argv[4]);
	std::vector<std::string> strs_new;
	// The new location
	boost::split(strs_new,New_Loc,boost::is_any_of(","));

	int New_X = atoi(strs_new[0].c_str());
	int New_Y = atoi(strs_new[1].c_str());
	// look up the design (and do something with it ...)
	DesignSharedPtr designPtr = importer.getDesignPtr();
		
	InstanceSharedPtrVector::const_iterator pInstance = designPtr->instancesBegin();
	InstanceSharedPtrVector::const_iterator eInstance = designPtr->instancesEnd();
	
//	InstanceSharedPtrVector::const_iterator first_carry;
	//This is for checking input with the existing CLB SET
	std::string CLB_input = "SLICE_X";
	CLB_input.append(boost::lexical_cast<std::string>(Old_X));
	CLB_input.append("Y");
	CLB_input.append(boost::lexical_cast<std::string>(Old_Y));

	std::string DSP_input = "DSP48_X";
	DSP_input.append(boost::lexical_cast<std::string>(Old_X));
	DSP_input.append("Y");
	DSP_input.append(boost::lexical_cast<std::string>(Old_Y));
	
	std::string BRAM_input = "RAMB36_X";
	BRAM_input.append(boost::lexical_cast<std::string>(Old_X));
	BRAM_input.append("Y");
	BRAM_input.append(boost::lexical_cast<std::string>(Old_Y));

	//This is the New Location
	std::string CLB_new = "SLICE_X";
	CLB_new.append(boost::lexical_cast<std::string>(New_X));
	CLB_new.append("Y");
	CLB_new.append(boost::lexical_cast<std::string>(New_Y));

	std::string DSP_new = "DSP48_X";
	DSP_new.append(boost::lexical_cast<std::string>(New_X));
	DSP_new.append("Y");
	DSP_new.append(boost::lexical_cast<std::string>(New_Y));
	
	std::string BRAM_new = "RAMB36_X";
	BRAM_new.append(boost::lexical_cast<std::string>(New_X));
	BRAM_new.append("Y");
	BRAM_new.append(boost::lexical_cast<std::string>(New_Y));

	std::string carry_shape="";	
	std::set<string> CLB_SET;
	std::multiset<string> CLB_SET_CARRY;
//	std::set<int>::iterator CLB_it;
	std::set<string> DSP_SET;
	std::set<string> DSP_SET_NEW;
//	std::set<int>::iterator DSP_it;
	std::set<string> BRAM_SET;
//	std::set<string> BRAM_SET_NEW;
//	std::set<int>::iterator BRAM_it;
	int Carry_Flag = 0;//, Carry_Operation = 0,Carry_Return = 0;//,
	int Carry_Count;// ,Carry_Move_Count=0;

//	InstanceSharedPtr first_carry;
	while(pInstance != eInstance)
	{
	InstanceSharedPtr instancePtr =*pInstance;
	std::string siteName = instancePtr->getSite();
	std::string siteType = instancePtr->getType();

	if((siteType.compare("SLICEL")==0 || siteType.compare("SLICEM")==0) && Inst_Type.compare("CLB")==0){
	
//	if(CLB_SET.count(siteName)==0){
//		CLB_SET.insert(siteName);
//		std::cout<<"Insert : " <<siteName<<std::endl;
	CLB_SET.insert(siteName);
	//if it is the carry chain.
	if(instancePtr->hasConfig("CARRY4")){
	//	ConfigMap::const_iterator pConfig = instancePtr->configBegin();
		std::cout<<siteName << " is carry!" << std::endl;
		ConfigMap::const_iterator eConfig = instancePtr->configEnd();
		eConfig--;
		std::string XDL_Shape = eConfig->second.getValue();
		std::string xdl_shape = XDL_Shape.substr(0,XDL_Shape.length()-2);	
		std::cout<< xdl_shape << std::endl;
		CLB_SET_CARRY.insert(xdl_shape);
		if(siteName.compare(CLB_input)==0){
			Carry_Flag = 1;
			std::cout<<"Carry find!"<<std::endl;
			carry_shape = xdl_shape;
			std::cout<<"Carry_shape:"<<carry_shape<<std::endl;
			
		}

	}

	else{
//	std::cout<<"you want to move: "<<CLB_input<<std::endl;}
//	CLB_it = CLB_SET.find(CLB_new);
	if(siteName.compare(CLB_input)==0 && CLB_SET.count(CLB_new)==0){
	//change the instance`
	//	CLB_SET.erase (CLB_SET.find(siteName));
		instancePtr->setSite(CLB_new);
		std::cout<<"siteName: "<<siteName<<std::endl;
		std::cout<<"new: "<<CLB_new<<std::endl;
		SiteIndex index_site = sites.findSiteIndex(instancePtr->getSite());
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
				}	
			}
		else{
			std::cout<<"The new location does not exist!"<<std::endl;
			exit(0);}
		}
	else if (CLB_SET.count(CLB_new)==1){
		std::cout<<"The New location of CLB is occupied!"<<std::endl;
		exit(0);
		}
	else{
		
		CLB_SET.insert(siteName);
		std::cout<<"CLB Insert : " <<siteName<<std::endl;
		}
	}
	}
	else if(siteType.compare("DSP48E1")==0 && Inst_Type.compare("DSP")==0){
	DSP_SET.insert(siteName);
	int X_pos = siteName.find("X");
	int Y_pos = siteName.find("Y");
	int a = atoi((siteName.substr(X_pos+1,Y_pos - X_pos-1)).c_str());
	int b = atoi((siteName.substr(Y_pos+1)).c_str());
	instancePtr->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1,boost::lexical_cast<std::string>(a+ New_X - Old_X)));
	Y_pos = siteName.find("Y");
	instancePtr->setSite(siteName.replace(Y_pos+1,siteName.length()-Y_pos,boost::lexical_cast<std::string>(b+New_Y-Old_Y)));
	std::cout<<"DSP changed SiteName: "<<siteName<<endl;
	SiteIndex DSP_index_site = sites.findSiteIndex(instancePtr->getSite());
	
	if(!DSP_index_site.isUndefined()){
		const Site& site = sites.getSite(DSP_index_site);
		TileIndex index_tile = site.getTileIndex();
		const TileInfo& TileInfo = tiles.getTileInfo(index_tile);
		const char* Name = TileInfo.getName();
		instancePtr->setTile(Name);
		DSP_SET_NEW.insert(siteName);

	}
//	if (siteName.compare(DSP_input)==0 && DSP_SET.count(DSP_new)==0){
	//	}	
/*	else if(DSP_SET.count(DSP_new)==1){
		std::cout<<"The New location of DSP is occupied!"<<std::endl;
		exit(0);
		}*/
	else{
	//	DSP_SET.insert(siteName);
		std::cout<<"DSP Movement Error: New location does not exist "<<siteName<<std::endl;
		exit(0);
		}

	}

	else if(siteType.compare("RAMB36E1")==0&&Inst_Type.compare("BRAM")==0){
	BRAM_SET.insert(siteName);
	if (siteName.compare(BRAM_input)==0 && BRAM_SET.count(BRAM_new)==0){			
	//	BRAM_SET.erase (BRAM_SET.find(siteName));

		instancePtr->setSite(BRAM_new);
		std::cout<<"BRAM siteName: "<<siteName<<std::endl;
		std::cout<<"BRAM new: "<<CLB_new<<std::endl;
		SiteIndex BRAM_index_site = sites.findSiteIndex(instancePtr->getSite());
		if(!BRAM_index_site.isUndefined()){	
		//	BRAM_SET.insert(BRAM_new);	
			const Site& site = sites.getSite(BRAM_index_site);
			TileIndex index_tile = site.getTileIndex();
			const TileInfo& TileInfo = tiles.getTileInfo(index_tile);
			const char* Name = TileInfo.getName();
			instancePtr->setTile(Name);

		}
		else{
			std::cout<<"The BRAM you input does not exist"<<std::endl;
			exit(0);}
	}
	else if(BRAM_SET.count(BRAM_new)==1){
		std::cout<<"The new location of BRAM is occupied"<<std::endl;
		exit(0);
	}
	else{
		BRAM_SET.insert(siteName);
		std::cout<<"BRAM insert: "<<siteName <<std::endl;
	}
	

	}

	else{
	std::cout<< "This is not what we want to move! "<< std::endl;
	}
	pInstance++;

	}	
	
	std::cout<<"after first iterator: carry_shape "<<carry_shape<<std::endl;
	//after moving DSP,check it
	if((Inst_Type.compare("DSP")==0) && (DSP_SET.count(DSP_input)==0 || DSP_SET_NEW.count(DSP_new)==0)){
	std::cout<<"The DSP you want to change is not in XDL"<<std::endl;
	exit(0);
	}
	else if (Inst_Type.compare("BRAM")==0 &&BRAM_SET.count(BRAM_input)==1){
		BRAM_SET.erase (BRAM_SET.find(BRAM_input));
		BRAM_SET.insert(BRAM_new);
		}
	else if (Inst_Type.compare("BRAM")==0 && BRAM_SET.count(BRAM_input)==0){
	std::cout<<"The BRAM you want to move is not in XDL"<<std::endl;
	exit(0);
	}
	else if(Inst_Type.compare("CLB")==0 && CLB_SET.count(CLB_input)==1 && Carry_Flag ==0 ){
		CLB_SET.erase (CLB_SET.find(CLB_input));
		CLB_SET.insert(CLB_new);
		}
	else if(Inst_Type.compare("CLB")==0 && CLB_SET.count(CLB_input)==0 && Carry_Flag ==0){	
	std::cout<<"The CLB you want to move is not in XDL"<<std::endl;
	exit(0);
	}
	else if(Carry_Flag == 1){
	
	InstanceSharedPtrVector::const_iterator pInstance2 = designPtr->instancesBegin();
	InstanceSharedPtrVector::const_iterator eInstance2 = designPtr->instancesEnd();
	//dealing with the multiset, delete all the element except carry_shape
	for (std::multiset<string>::iterator it = CLB_SET_CARRY.begin(); it!= CLB_SET_CARRY.end();++it){
		std::cout<< (*it).compare(carry_shape)<<std::endl;
		std::cout<<"IT:"<< *it<<std::endl;
		std::cout<<"Carry_shape:"<< carry_shape<<std::endl;

		if((*it).compare(carry_shape) != 0 ){

		std::multiset<string>::iterator newit = it;
		CLB_SET_CARRY.erase(newit);
//		std::cout<<"delete extra carry:"<<*newit<<std::endl;}
		}		
	}
//	for (std::multiset<string>::iterator itt = CLB_SET_CARRY.begin();itt!=CLB_SET_CARRY.end();++itt){
//	std::cout<<*itt <<std::endl;	
//	}
	Carry_Count = static_cast<int>(CLB_SET_CARRY.size());
	while(pInstance2 != eInstance2)
	{
	InstanceSharedPtr instancePtr2 =*pInstance2;
	std::string siteName = instancePtr2->getSite();
	std::string siteType = instancePtr2->getType();

	if(instancePtr2->hasConfig("CARRY4")){
	//	ConfigMap::const_iterator pConfig = instancePtr->configBegin();
	//	std::cout<<siteName << " is carry!" << std::endl;
		ConfigMap::const_iterator eConfig = instancePtr2->configEnd();
		eConfig--;
		std::string XDL_Shape = eConfig->second.getValue();
		std::string xdl_shape = XDL_Shape.substr(0,XDL_Shape.length()-2);	
	//	std::cout<< xdl_shape << std::endl;
	//	CLB_SET_CARRY.insert(xdl_shape);
		if(static_cast<int>(CLB_SET_CARRY.count(xdl_shape))== Carry_Count){
		//	Carry_Flag = 1;
		//	std::cout<<"Carry find!"<<std::endl;
		CLB_SET.erase(siteName);
		int X_pos = siteName.find("X");
		int Y_pos = siteName.find("Y");
		int a = atoi((siteName.substr(X_pos+1,Y_pos - X_pos-1)).c_str());
		int b = atoi((siteName.substr(Y_pos+1)).c_str());
		std::cout<<"a: "<<a<<"   b:"<<b<<std::endl;
		instancePtr2->setSite(siteName.replace(X_pos+1,Y_pos-X_pos-1,boost::lexical_cast<std::string>(a+ New_X - Old_X)));
		std::cout<<"CARRY changed SiteName: "<<siteName<<endl;
		Y_pos = siteName.find("Y");
		std::cout<<"length: "<<siteName.length()<<"  Y_pos:"<<Y_pos<<std::endl;
		instancePtr2->setSite(siteName.replace(Y_pos+1,siteName.length()-Y_pos,boost::lexical_cast<std::string>(b+New_Y-Old_Y)));
		std::cout<<"CARRY changed SiteName: "<<siteName<<endl;
		SiteIndex index_site = sites.findSiteIndex(instancePtr2->getSite());
		std::cout<< "carry Index_site: " << index_site<<std::endl;
		std::cout<<" carry SITe:" << instancePtr2->getSite()<<std::endl;
		if(CLB_SET.count(instancePtr2->getSite())==1){
		std::cout<<"The new location has been occupied!"<<std::endl;
		exit(0);
		}
			if(!index_site.isUndefined()){
				const Site& site = sites.getSite(index_site);
				TileIndex index_tile = site.getTileIndex();
				const PrimitiveDef* primitivedef = site.getPrimitiveDefPtr();
				std::string PriDef = primitivedef->getName();//Get the new location's type(SLICEL or SLICEM)
				if((PriDef.compare("SLICEL")==0)&&(siteType.compare("SLICEM")==0)){
					std::cout<< "The SLICEM can not move to SLICEL in Carry!" << std::endl;
					exit(0);	
					} 
				else{
					const TileInfo& TileInfo = tiles.getTileInfo(index_tile);
					const char* Name = TileInfo.getName();
					instancePtr2->setTile(Name);
					CLB_SET.insert(instancePtr2->getSite());
			 // std::cout<< instancePtr->getName() <<std::endl;
					}
				}
			else{
				std::cout<<"The New CLB-CARRY location does not exist!"<<std::endl;
				exit(0);}	
			
			}
		else{std::cout<<"This is not the carry what we want to move!"<<std::endl;}

		}

		pInstance2++;

		}
	
	}

	else{
	std::cout<<"Unknown Error!"<<std::endl;
	}
	
	


	// export the XDL design
	std::string outFileName = boost::filesystem::path(inFileName).replace_extension().string() 
		+ ".mod.xdl";
	std::fstream xdlExport(outFileName.c_str(), std::ios_base::out);
	torc::physical::XdlExporter fileExporter(xdlExport);
	fileExporter(designPtr);

	return 0;
}
