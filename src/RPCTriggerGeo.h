#ifndef RPCTrigger_RPCTriggerGeo_h
#define RPCTrigger_RPCTriggerGeo_h

/** \class RPCTriggerGeo
 *
 * \brief Class describing RPC trigger geometry
 *  
 * Aim: easly convert RPCdetId.firedStrip to loghit/logcone
 * \author Tomasz Fruboes 
 *
 */

#include <FWCore/Framework/interface/ESHandle.h> // Handle to read geometry
#include "FWCore/Framework/interface/Event.h"
#include "DataFormats/MuonDetId/interface/RPCDetId.h"
#include "DataFormats/RPCDigi/interface/RPCDigiCollection.h"

#include <Geometry/RPCGeometry/interface/RPCGeometry.h>
#include <Geometry/Records/interface/MuonGeometryRecord.h>

#include "L1Trigger/RPCTrigger/src/RPCDetInfo.h"
#include "L1Trigger/RPCTrigger/src/RPCRingFromRolls.h"

#include "L1Trigger/RPCTrigger/src/L1RpcLogCone.h" 
#include "set"

class RPCTriggerGeo {
  public:
    RPCTriggerGeo();
    ~RPCTriggerGeo() {};
    
    void buildGeometry(edm::ESHandle<RPCGeometry> rpcGeom);
    bool isGeometryBuilt();
  
    L1RpcLogConesVec getCones(edm::Handle<RPCDigiCollection> rpcDigis);
       

  private:
    void addDet(RPCRoll* roll);
    void printRingFromRollsMapInfo();
    int etaToTower(float eta);
                              
    bool m_isGeometryBuilt; ///< Determines if geometry is built allready
    typedef std::map<uint32_t, RPCRingFromRolls> RPCRingFromRollsMap;
    // Probably the curls should be local 
    RPCRingFromRollsMap m_RPCRingFromRollsMap; ///< Stores all curls
    RPCRingFromRollsMap m_refRPCRingFromRollsMap; ///< Stores refernce curls
    RPCRingFromRollsMap m_otherRPCRingFromRollsMap; ///< Stores other curls
    
    //L1RpcLogConesVec 	m_activeCones;
    
    RPCRingFromRolls::RPCLinks m_links; ///< Translates rpcdigi into loghits
    
    std::set<uint32_t> m_detsToIngore; ///< Temp. solution to the problem of not connected dets (in ORCA)
};
#endif
