//---------------------------------------------------------------------------

#include "L1Trigger/RPCTrigger/src/L1RpcTriggerCrate.h"

//---------------------------------------------------------------------------
L1RpcTriggerCrate::L1RpcTriggerCrate(L1RpcTCGhostBusterSorter* tcGhostBusterSorter,
    L1RpcTBGhostBuster* tbGhostBuster, L1RpcTriggerConfiguration* triggerConfig, int tcNum) {
  TCGhostBusterSorter = tcGhostBusterSorter;
  TriggerConfig = triggerConfig;
  WasMuon = false;

  TCNumber = tcNum;

  for(int iTB = 0; iTB < triggerConfig->GetTBsInTC(); iTB++)
    TBsVec.push_back(L1RpcTriggerBoard(tbGhostBuster, triggerConfig, iTB));
}

L1RpcTBMuonsVec L1RpcTriggerCrate::RunTCGBSorter() {
  if(!WasMuon)
    return L1RpcTBMuonsVec();

  L1RpcTBMuonsVec2 tbMuonsVec2;
  for(unsigned int iTB = 0; iTB < TBsVec.size(); iTB++) {
    tbMuonsVec2.push_back(TBsVec[iTB].RunTBGB() );
  }

  WasMuon = false;
  return TCGhostBusterSorter->Run(tbMuonsVec2);
}


/** Runs L1RpcTriggerBoard::RunCone() for every TB. Cheks, if any non empty muons were found*/
void L1RpcTriggerCrate::RunCone(const L1RpcLogCone& cone) {
  if(TBsVec[TriggerConfig->GetTBNum(cone.GetConeCrdnts())].RunCone(cone) )
    WasMuon = true;
};
