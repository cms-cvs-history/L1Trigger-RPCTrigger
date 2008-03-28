// -*- C++ -*-
//
// Package:     RPCTrigger
// Class  :     RPCConeBuilderFromES
// 
// Implementation:
//     <Notes on implementation>
//
// Original Author:  
//         Created:  Mon Mar  3 13:34:20 CET 2008
// $Id$
//

// system include files

// user include files
#include "L1Trigger/RPCTrigger/interface/RPCConeBuilderFromES.h"


//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
RPCConeBuilderFromES::RPCConeBuilderFromES()
{
}

// RPCConeBuilderFromES::RPCConeBuilderFromES(const RPCConeBuilderFromES& rhs)
// {
//    // do actual copying here;
// }

RPCConeBuilderFromES::~RPCConeBuilderFromES()
{
}
L1RpcLogConesVec RPCConeBuilderFromES::getConesFromES(edm::Handle<RPCDigiCollection> rpcDigis, 
                                edm::ESHandle<L1RPCConeBuilder> coneBuilder)
{
  std::vector<RPCLogHit> logHits;
  
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
      if (digiIt->bx()!=0)
        continue;
      
      
      std::pair<L1RPCConeBuilder::TStripConVec::const_iterator, L1RPCConeBuilder::TStripConVec::const_iterator> 
          itPair = coneBuilder->getConVec(rawId,digiIt->strip());
      
      L1RPCConeBuilder::TStripConVec::const_iterator it = itPair.first;
      for (; it!=itPair.second;++it){
        logHits.push_back( RPCLogHit(it->m_tower, it->m_PAC, it->m_logplane, it->m_logstrip) );
      }
      
    }
    
    
  }
  
  L1RpcLogConesVec ActiveCones;

  std::vector<RPCLogHit>::iterator p_lhit;
  for (p_lhit = logHits.begin(); p_lhit != logHits.end(); p_lhit++){
    bool hitTaken = false;
    L1RpcLogConesVec::iterator p_cone;
    for (p_cone = ActiveCones.begin(); p_cone != ActiveCones.end(); p_cone++){
      hitTaken = p_cone->addLogHit(*p_lhit);
      if(hitTaken)
        break;
    }

    if(!hitTaken) {
      RPCLogCone newcone(*p_lhit);
      newcone.setIdx(ActiveCones.size());
      ActiveCones.push_back(newcone);
    }
  }// for loghits

  return ActiveCones;
  
}
