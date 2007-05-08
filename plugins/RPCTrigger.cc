/** \file RPCTrigger.cc
 *
 *  $Date: 2007/04/05 06:46:32 $
 *  $Revision: 1.26 $
 *  \author Tomasz Fruboes
 */
#include "L1Trigger/RPCTrigger/interface/RPCTrigger.h"

// Configuration via eventsetup:
#include "CondFormats/DataRecord/interface/L1RPCConfigRcd.h"
#include "CondFormats/RPCObjects/interface/L1RPCConfig.h"


//#define ML_DEBUG 





RPCTrigger::RPCTrigger(const edm::ParameterSet& iConfig)
{
  produces<std::vector<L1MuRegionalCand> >("RPCb");
  produces<std::vector<L1MuRegionalCand> >("RPCf");
  

  m_firstRun = true;

  m_triggerDebug = iConfig.getUntrackedParameter("RPCTriggerDebug",0);
  
  // 0 - no debug
  // 2 - technical debug
  // 1 - human readable debug
  if ( m_triggerDebug != 1 && m_triggerDebug != 2)
     m_triggerDebug = 0;
   
 
}




RPCTrigger::~RPCTrigger(){ 
  delete m_pacTrigger;
  delete m_trigConfig;
}



void
RPCTrigger::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
 

  if (m_firstRun){ // IOV checking?
     m_firstRun = false;  
     edm::ESHandle<L1RPCConfig> conf;
     iSetup.get<L1RPCConfigRcd>().get(conf);
     const L1RPCConfig *rpcconf = conf.product();

     int ppt =  rpcconf->getPPT();

     //std::string patternsDirNameLocal = rpcconf->getDataDir();
     
     // Since fileInPath doesnt allow us to use directory we use this quick and dirty solution
     //edm::FileInPath fp(patternsDirNameLocal+"keepme.txt"); 
     //std::string patternsDirNameUnstriped = fp.fullPath();
     //std::string patternsDirName = patternsDirNameUnstriped.substr(0,patternsDirNameUnstriped.find_last_of("/")+1);
     
     /*
     switch (ppt){
        case 1:
          m_pacManager.init(patternsDirName, ONE_PAC_PER_TOWER); // TODO: read that from cfg
          break;
        case 12:
          m_pacManager.init(patternsDirName, _12_PACS_PER_TOWER);
          break;
        case 144:
          m_pacManager.init(patternsDirName, _144_PACS_PER_TOWER);
          break;
        default:
          throw cms::Exception("BadConfig")
                  << "PACsPerTower set to wrong value: " << ppt << "\n";
       }*/
       m_pacManager.init(rpcconf);
  
       m_trigConfig = new RPCBasicTrigConfig(&m_pacManager);
  
       m_trigConfig->setDebugLevel(m_triggerDebug);
  
       m_pacTrigger = new RPCPacTrigger(m_trigConfig);

      
  }

 
  // Build the trigger linksystem geometry;
  if (!m_theLinksystem.isGeometryBuilt()){

    edm::LogInfo("RPC") << "Building RPC links map for a RPCTrigger";
    edm::ESHandle<RPCGeometry> rpcGeom;
    iSetup.get<MuonGeometryRecord>().get( rpcGeom );     
    m_theLinksystem.buildGeometry(rpcGeom);
    edm::LogInfo("RPC") << "RPC links map for a RPCTrigger built";

  } 

  
  edm::Handle<RPCDigiCollection> rpcDigis;
//  iEvent.getByType(rpcDigis);
  iEvent.getByLabel("muonRPCDigis",rpcDigis);

  L1RpcLogConesVec ActiveCones = m_theLinksystem.getCones(rpcDigis);
  
  L1RpcTBMuonsVec2 finalMuons = m_pacTrigger->runEvent(ActiveCones);

  int maxFiredPlanes = 0;
  
  for (unsigned int i=0;i<ActiveCones.size();i++){
      int fpCnt = ActiveCones[i].getFiredPlanesCnt();
      if (fpCnt > maxFiredPlanes)
         maxFiredPlanes = fpCnt;
  }

  // Fill out the products
  // finalMuons[0]=barell, finalMuons[1]=endcap
  LogDebug("RPCTrigger") << "---Filling candindates in new event--- " 
                         << maxFiredPlanes << std::endl;
  
  std::vector<L1MuRegionalCand> RPCb = giveFinallCandindates(finalMuons[0],1);
  std::vector<L1MuRegionalCand> RPCf = giveFinallCandindates(finalMuons[1],3);;
    
  std::auto_ptr<std::vector<L1MuRegionalCand> > candBarell(new std::vector<L1MuRegionalCand>);
  candBarell->insert(candBarell->end(), RPCb.begin(), RPCb.end());
  
  std::auto_ptr<std::vector<L1MuRegionalCand> > candForward(new std::vector<L1MuRegionalCand>);
  candForward->insert(candForward->end(), RPCf.begin(), RPCf.end());
  
  iEvent.put(candBarell, "RPCb");
  iEvent.put(candForward, "RPCf");
  
}
///////////////////////////////////////////////////////////////////////////////
/**
 *
 * \brief Returns vector of L1MuRegionalCand (input of L1GMT)
 * \note - type is defined in L1MuRegionalCand 1 - barell, 3 - forward
 * \todo - we use offset value of 5 deegres. It should be stored centrally.
 *
 */
///////////////////////////////////////////////////////////////////////////////
std::vector<L1MuRegionalCand> RPCTrigger::giveFinallCandindates(L1RpcTBMuonsVec finalMuons, short type){

  std::vector<L1MuRegionalCand> RPCCand;
  
  for(unsigned int iMu = 0; iMu < finalMuons.size(); iMu++)
  {

    if (finalMuons[iMu].getPtCode()==0){
      continue; 
    } 

    L1MuRegionalCand l1Cand;
    
    
    l1Cand.setQualityPacked(finalMuons[iMu].getQuality());
    l1Cand.setPtPacked(finalMuons[iMu].getPtCode());
    
    l1Cand.setType(type); 
    
    int charge=finalMuons[iMu].getSign();
    
    if (charge == 0)  // negative
      l1Cand.setChargePacked(1);
    else  
      l1Cand.setChargePacked(0);
    
    //RPCConst::l1RpcConeCrdnts cone = finalMuons[iMu].getConeCrdnts();    
    
    /*
    int pac = cone.m_LogSector*12+cone.m_LogSegment;
    const float pi = 3.14159265;
    const float offset = 5*(2*pi/360); // redefinition! Defined also in RPCRingFromRolls::phiMapCompare
    float phi = 2*pi*pac/144-offset;
    if (phi<0)
      phi+=2*pi;
    
    l1Cand.setPhiValue(phi);
    */

    //Note: pac numbering begins at 5 deg and goes from 1 to 144.
    // we want phi values from 0 to 2.5 deg to be phiPacked=0 
    // max phiPacked value is 143 (see CMS IN 2004-022)
    int phiPacked = (finalMuons[iMu].getPhiAddr()+2)%144;
    l1Cand.setPhiPacked(phiPacked);
/*
    float eta = RPCConst::etaFromTowerNum(cone.m_Tower);
    l1Cand.setEtaValue(eta);
*/
    //Note: etaAddr is packed in special way: see CMS IN 2004-022
    signed short etaAddr = finalMuons[iMu].getEtaAddr()-16; // -16..16
    bool etaNegative = false;
    if (etaAddr < 0){
      etaNegative = true;
      etaAddr = ~(-etaAddr)+1; // convert to negative :)
    }

    etaAddr &= 63; // 6 bits only
         
    l1Cand.setEtaPacked(etaAddr);
    l1Cand.setChargeValid(true);

    /*    
    std::cout<< std::endl << "RBMuon::" << finalMuons[iMu].getEtaAddr() << " " 
             << finalMuons[iMu].getPhiAddr() << std::endl ;
    std::cout<< "cand " <<  l1Cand.eta_packed() << " " 
             << l1Cand.phi_packed() << std::endl ;
    //*/

    RPCCand.push_back(l1Cand);
        
    LogDebug("RPCTrigger") << "Found muonf of pt " << finalMuons[iMu].getPtCode()
        << " L1Charge " << l1Cand.charge_packed()
        << " ql " << l1Cand.quality()
        << " fp " << finalMuons[iMu].getFiredPlanes()
        << " b/f " << l1Cand.type_idx()
        << " phi " <<  l1Cand.phi_packed()
        << " eta " << l1Cand.eta_packed()
        //<< " eta l1 " << l1Cand.etaValue() // will drop out soon 
        << " killed " << finalMuons[iMu].wasKilled();
        
    
  }

  return RPCCand;
}
