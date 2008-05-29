#ifndef L1Trigger_RPCTrigger_RPCConeBuilderFromES_h
#define L1Trigger_RPCTrigger_RPCConeBuilderFromES_h
// -*- C++ -*-
//
// Package:     RPCTrigger
// Class  :     RPCConeBuilderFromES
// 
/**\class RPCConeBuilderFromES RPCConeBuilderFromES.h L1Trigger/RPCTrigger/interface/RPCConeBuilderFromES.h

 Description: <one line class summary>

 Usage:
    <usage>

*/
//
// Original Author:  
//         Created:  Mon Mar  3 13:34:15 CET 2008
// $Id: RPCConeBuilderFromES.h,v 1.1 2008/03/03 14:34:57 fruboes Exp $
//

#include "CondFormats/DataRecord/interface/L1RPCConeBuilderRcd.h"
#include "CondFormats/RPCObjects/interface/L1RPCConeBuilder.h"

#include "DataFormats/RPCDigi/interface/RPCDigiCollection.h"
#include "L1Trigger/RPCTrigger/interface/RPCLogCone.h" 

#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/Event.h"


// system include files

// user include files

// forward declarations

class RPCConeBuilderFromES
{

   public:
      RPCConeBuilderFromES();
      virtual ~RPCConeBuilderFromES();
      L1RpcLogConesVec getConesFromES(edm::Handle<RPCDigiCollection> rpcDigis, 
                                      edm::ESHandle<L1RPCConeBuilder> coneBuilder, int bx);
      // ---------- const member functions ---------------------

      // ---------- static member functions --------------------

      // ---------- member functions ---------------------------

   private:
      //RPCConeBuilderFromES(const RPCConeBuilderFromES&); // stop default

      //const RPCConeBuilderFromES& operator=(const RPCConeBuilderFromES&); // stop default

      // ---------- member data --------------------------------

};


#endif
