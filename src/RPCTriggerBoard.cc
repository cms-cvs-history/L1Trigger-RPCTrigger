/*******************************************************************************
*                                                                              *
*  Karol Bunkowski                                                             *
*  Warsaw University 2004                                                      *
*                                                                              *
*******************************************************************************/
#include "L1Trigger/RPCTrigger/src/RPCTriggerBoard.h"
#include "L1Trigger/RPCTrigger/src/RPCException.h"

#include<sstream>
//---------------------------------------------------------------------------
RPCTriggerBoard::RPCTriggerBoard(RPCTBGhostBuster* tbGhostBuster,
                RPCTriggerConfiguration* triggerConfig,
                int tbNum, int tcNum) 
{
  m_TBGhostBuster = tbGhostBuster;
  m_TriggerConfig = triggerConfig;
  m_TBNumber = tbNum;

  int towerMin=-16;
  int towerMax=-16;
  
  // Calculate tower boundaries for this TB
  for (int i=0;i <= tbNum; i++){ // Trigger Boards are numbered from 0 to 8
    towerMin=towerMax;
    towerMax+=m_TriggerConfig->getTowsCntOnTB(i);
  }
  towerMax--;
  
  for (int tower = towerMin; tower <= towerMax; tower++){
    for (int logSegment = 0; logSegment < 12; logSegment++){ // One logSegment = one PAC
      //m_pacs[cone.getTower()][cone.getLogSegment()]
      RPCConst::l1RpcConeCrdnts coneCrds;
      coneCrds.m_Tower=tower;  
      coneCrds.m_LogSector=tcNum;
      coneCrds.m_LogSegment=logSegment;
      
      const RPCPacData *pacData = m_TriggerConfig->getPac(coneCrds);
      RPCPac *pac = new RPCPac(pacData, tower, tcNum, logSegment); // one trigger crate covers one logsector
      m_pacs[tower].push_back(pac);
    
    }
  }
           
           
}

//---------------------------------------------------------------------------
bool RPCTriggerBoard::runCone(const RPCLogCone& cone)  {

    //RPCTBMuon tbMuon(m_TriggerConfig->getPac(cone.getConeCrdnts())->run(cone) );
    if(m_pacs.find(cone.getTower())==m_pacs.end()){
      std::stringstream s;
      s << "RPCTriggerBoard::runCone() wrong tower: " << cone.getTower() << " in TB=" << m_TBNumber;
      throw RPCException(s.str());
    }
    
    RPCTBMuon tbMuon(m_pacs[cone.getTower()][cone.getLogSegment()]->run(cone));  
  

    if(tbMuon.getCode() > 0) {
        m_PacsMuonsVec.push_back(tbMuon);
        if (m_TriggerConfig->getDebugLevel()!=0){
#ifndef _STAND_ALONE
	  LogDebug("RPCHwDebug") << "GB 0"
			         << tbMuon.printDebugInfo(m_TriggerConfig->getDebugLevel());
#else
	  std::cout << "GB 0"
  	  	    << tbMuon.printDebugInfo(m_TriggerConfig->getDebugLevel())
		    << std::endl;
#endif //_STAND_ALONE
        }
      return true;
    }
    else
      return false;
      
}
//---------------------------------------------------------------------------
L1RpcTBMuonsVec RPCTriggerBoard::runTBGB() { //4 muons or empty vector
  if(m_PacsMuonsVec.size() == 0)
    return L1RpcTBMuonsVec();

  L1RpcTBMuonsVec2 gbMuons(RPCConst::m_TOWERS_ON_TB_CNT, L1RpcTBMuonsVec());
  for(unsigned int iMu = 0; iMu < m_PacsMuonsVec.size(); iMu++) {
    int tbTower = m_TriggerConfig->getTowerNumOnTb(m_PacsMuonsVec[iMu].getConeCrdnts() );

    if(gbMuons[tbTower].size() == 0)
      gbMuons[tbTower].assign(RPCConst::m_SEGMENTS_IN_SECTOR_CNT, RPCTBMuon());

    gbMuons[tbTower][m_PacsMuonsVec[iMu].getLogSegment()] = m_PacsMuonsVec[iMu];
  }

  m_PacsMuonsVec.clear();
  return m_TBGhostBuster->run(gbMuons);
}
