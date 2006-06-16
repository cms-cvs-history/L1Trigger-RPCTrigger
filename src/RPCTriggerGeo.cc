/** \file RPCTriggerGeo.cc
 *
 *  $Date: 2006/06/12 15:45:39 $
 *  $Revision: 1.7 $
 *  \author Tomasz Fruboes
 */

#include "Geometry/RPCGeometry/interface/RPCGeometry.h"
#include "Geometry/Records/interface/MuonGeometryRecord.h"

#include "L1Trigger/RPCTrigger/src/RPCTriggerGeo.h"
#include <Geometry/CommonTopologies/interface/RectangularStripTopology.h>
#include <Geometry/CommonTopologies/interface/TrapezoidalStripTopology.h>

#include "DataFormats/MuonDetId/interface/RPCDetId.h"


#include <vector>
#include <algorithm>
#include <map>

#include <cmath>

//#############################################################################
/**
 *
 * \brief Default constructor
 *
*/
//#############################################################################
RPCTriggerGeo::RPCTriggerGeo(){

  m_isGeometryBuilt=false;

}
//#############################################################################
/**
*
* \brief Checks, if we have built the geometry already
*
*/
//#############################################################################
bool RPCTriggerGeo::isGeometryBuilt(){ return m_isGeometryBuilt; }
//#############################################################################
/**
 * \brief Builds RpcGeometry
 * \note Code accessing geometry info is heavly based on
           CMSSW/Geometry/RPCGeometry/test/RPCGeometryAnalyzer.cc
 *
*/
//#############################################################################
void RPCTriggerGeo::buildGeometry(edm::ESHandle<RPCGeometry> rpcGeom){
 
  // Get some information for all RpcDetId`s; store it locally
  for(TrackingGeometry::DetContainer::const_iterator it = rpcGeom->dets().begin();
      it != rpcGeom->dets().end();
      it++)
  {
    
    if( dynamic_cast< RPCRoll* >( *it ) == 0 ) continue;
    RPCRoll* roll = dynamic_cast< RPCRoll*>( *it );
    addDet(roll);
    
  } // RpcDet loop end
  
  // Separete reference curls from others, should be done in previous step
  for(RPCCurlMap::iterator it = m_RPCCurlMap.begin();
      it!=m_RPCCurlMap.end();
      it++)
  {
    if ( (it->second).isRefPlane() ){
      m_refRPCCurlMap[it->first]=it->second;
    }
    else  
      m_otherRPCCurlMap[it->first]=it->second;
  }
  
  //loop over reference curls
  for ( RPCCurlMap::iterator itRefCurl=m_refRPCCurlMap.begin(); 
        itRefCurl != m_refRPCCurlMap.end();
        itRefCurl++)
  {
    //loop over other curls
    for ( RPCCurlMap::iterator itOtherCurl=m_otherRPCCurlMap.begin(); 
          itOtherCurl != m_otherRPCCurlMap.end();
          itOtherCurl++)
    {
      (itRefCurl->second).makeRefConnections(&(itOtherCurl->second));
    } //otherCurl loop end
  } // refCurl's loop end
  
  
  // Copy all stripConections into one place
  for ( RPCCurlMap::iterator it=m_refRPCCurlMap.begin(); 
        it != m_refRPCCurlMap.end();
        it++){
          
          RPCCurl::RPCLinks linksTemp = (it->second).giveConnections();
          m_links.insert(linksTemp.begin(),linksTemp.end() );
          
        }
        
  //std::cout<< "Connections for refs: " << m_links.size() << std::endl;
  
  for ( RPCCurlMap::iterator it=m_otherRPCCurlMap.begin(); 
        it != m_otherRPCCurlMap.end();
        it++){
          
          RPCCurl::RPCLinks linksTemp = (it->second).giveConnections();
          m_links.insert(linksTemp.begin(),linksTemp.end() );
  
  }
    
    
    
  m_isGeometryBuilt=true;
  //printCurlMapInfo();
}

//#############################################################################
/**
 *
 * \brief Adds detID to the collection
 *
*/
//#############################################################################
void RPCTriggerGeo::addDet(RPCRoll* roll){

  RPCDetInfo detInfo(roll);

  // This two curls werent connected anywhere in ORCA. Filtered out for consitency with ORCA.
  if ( (detInfo.getCurlId() == 2108) ||(detInfo.getCurlId() == 2008) ){
    return;
  }
  
  
  if( m_RPCCurlMap.find(detInfo.getCurlId()) != m_RPCCurlMap.end() ){ // Curl allready in map

     m_RPCCurlMap[detInfo.getCurlId()].addDetId(detInfo);

  } else {  // add a new curl
    
    RPCCurl newCurl;
    newCurl.addDetId(detInfo);
    m_RPCCurlMap[detInfo.getCurlId()]=newCurl;

  }

}
//#############################################################################
/**
 *
 * \brief Builds cones from digis
 * \note Based on L1RpcConeBuilder from ORCA
 *
 */
//#############################################################################
L1RpcLogConesVec RPCTriggerGeo::getCones(edm::Handle<RPCDigiCollection> rpcDigis){

  std::vector<L1RpcLogHit> logHits;
    
// Build cones from digis
  RPCDigiCollection::DigiRangeIterator detUnitIt;
  for (detUnitIt=rpcDigis->begin();
       detUnitIt!=rpcDigis->end();
       ++detUnitIt)
  {
    const RPCDetId& id = (*detUnitIt).first;
    
    int rawId = id.rawId();
    
    const RPCDigiCollection::Range& range = (*detUnitIt).second;
    
    for (RPCDigiCollection::const_iterator digiIt = range.first;
         digiIt!=range.second;
         ++digiIt)
    {
      RPCCurl::stripCords sc;
      sc.detRawId=rawId;
      sc.stripNo=digiIt->strip();
      sc.isVirtual=false;
      
      // Find strip in map
      if (m_links.find(sc)!=m_links.end()){
      
        RPCCurl::RPCConnectionsVec stripCons = m_links[sc];
        //Build loghits
        for (RPCCurl::RPCConnectionsVec::iterator it = stripCons.begin();
             it != stripCons.end();
             it++)
        {
          
          logHits.push_back( L1RpcLogHit(it->tower, it->PAC, it->logplane, it->posInCone) );
             
             
        }
      
      } 
      // should throw exception
      else {  // Strip not in map :/
        std::cout<< "Strip " << sc.stripNo 
            << " of det " << sc.detRawId
            << " not present in map"
            << std::endl;
      }
    } // for digiCollection
  }// for detUnits

  
  // Build cones
  L1RpcLogConesVec ActiveCones;
  
  vector<L1RpcLogHit>::iterator p_lhit;
  for (p_lhit = logHits.begin(); p_lhit != logHits.end(); p_lhit++){
    bool hitTaken = false;
    L1RpcLogConesVec::iterator p_cone;
    for (p_cone = ActiveCones.begin(); p_cone != ActiveCones.end(); p_cone++){
      hitTaken = p_cone->AddLogHit(*p_lhit);
      if(hitTaken)
        break;
    }

    if(!hitTaken) {
      L1RpcLogCone newcone(*p_lhit);
      newcone.SetIdx(ActiveCones.size());
      ActiveCones.push_back(newcone);
    }
  }// for loghits
  
  return ActiveCones;
}
//#############################################################################
/**
*
* \brief Util function to print rpcChambersMap contents
*
*/
//#############################################################################
void RPCTriggerGeo::printCurlMapInfo(){ // XXX - Erase ME
  
  //*
  for ( RPCCurlMap::iterator it=m_refRPCCurlMap.begin(); it != m_refRPCCurlMap.end(); it++){
    std::cout << "------------------------------"<< std::endl;
    std::cout << "CurlId " << (it->first) << " " << std::endl;
    (it->second).printContents();
  }
  for ( RPCCurlMap::iterator it=m_otherRPCCurlMap.begin(); it != m_otherRPCCurlMap.end(); it++){
    std::cout << "------------------------------"<< std::endl;
    std::cout << "CurlId " << (it->first) << " " << std::endl;
    (it->second).printContents();
  }
  //*/
  
  std::cout<< "No of refs: " << m_refRPCCurlMap.size() << std::endl;
  std::cout<< "No of others: " << m_otherRPCCurlMap.size() << std::endl;
  std::cout<< "Connections: " << m_links.size() << std::endl;

}
