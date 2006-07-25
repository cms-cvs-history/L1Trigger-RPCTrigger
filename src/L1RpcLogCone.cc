/*******************************************************************************
*                                                                              *
*  Karol Bunkowski                                                             *
*  Warsaw University 2002                                                      *
*                                                                              *
*******************************************************************************/
#include "L1Trigger/RPCTrigger/src/L1RpcLogCone.h" 

#include <iostream>
#include <iomanip>
#include <sstream>

/** 
 *
 * Default constructor. No hits, no muon.
 *
*/
L1RpcLogCone::L1RpcLogCone(): 
    ConeCrdnts() 
{
  LogPlanesVec.assign(RPCParam::LOGPLANES_COUNT, TLogPlane() );
  MuonCode = 0;
  MuonSign = 0;
}
/**
 *
 * Constructor. Cone coordinates are set.
 *
*/

L1RpcLogCone::L1RpcLogCone(int tower, int logSector, int logSegment):
    ConeCrdnts(tower, logSector, logSegment) 
{
  LogPlanesVec.assign(RPCParam::LOGPLANES_COUNT, TLogPlane() );
  MuonCode = 0;
  MuonSign = 0;
}
/**
 *
 * Copying Constructor
 *
*/
L1RpcLogCone::L1RpcLogCone(const L1RpcLogHit &logHit) 
{
  LogPlanesVec.assign(RPCParam::LOGPLANES_COUNT, TLogPlane() );

  ConeCrdnts = logHit.GetConeCrdnts();

  MuonCode = 0;
  MuonSign = 0;

  SetLogStrip(logHit.getlogPlaneNumber() -1, logHit.getStripNumberInCone(), logHit.getDigiIdx());
}

string L1RpcLogCone::toString() const {
  std::ostringstream ostr;
  ostr << "\n       ======================> TOWER = ";
  ostr<<setw(2)<<ConeCrdnts.Tower<<", LogSector = "<<ConeCrdnts.LogSector<<",  LogSegment = "<<ConeCrdnts.LogSegment;
  ostr <<" <======================="<<endl;

  std::string spacer;

  for (int logPlane = RPCParam::LAST_PLANE; logPlane >= RPCParam::FIRST_PLANE; logPlane--) {
    ostr<<RPCParam::LOGPLANE_STR[logPlane]<<" ";
    spacer.assign((72 - RPCParam::LOGPLANE_SIZE[abs(ConeCrdnts.Tower)][logPlane])/2, ' ');
    ostr<<spacer;
  
    for(int i = RPCParam::LOGPLANE_SIZE[abs(ConeCrdnts.Tower)][logPlane]-1; i >=0; i--) {
      if(GetLogStripState(logPlane, i))
         ostr<<"X";
      else {
        if(i%8 == 0)
          ostr<<i%10;
        else
    ostr<<"."; 
      }  
    }  

    ostr<<endl;
  }
 
  ostr<<endl;
  return ostr.str();
}
// 
//#############################################################################################
//
//  Simple getters and setters
//
//#############################################################################################
L1RpcLogCone::TLogPlane L1RpcLogCone::GetLogPlane(int logPlane) const { return LogPlanesVec[logPlane]; }

///Gets fired strips count in given logPlane.
int L1RpcLogCone::GetHitsCnt(int logPlane) const { return LogPlanesVec[logPlane].size(); }

/// sets pt code of muon that fired the strips */
void L1RpcLogCone::SetMuonCode(int code) { MuonCode = code; }

/** @return pt code of muon that fired the strips */
int L1RpcLogCone::GetMuonCode() const { return MuonCode; }

void L1RpcLogCone::SetMuonSign(int sign) { MuonSign = sign; }

int L1RpcLogCone::GetMuonSign() const { return MuonSign; }

int L1RpcLogCone::GetTower() const { return ConeCrdnts.Tower; }

int L1RpcLogCone::GetLogSector() const { return ConeCrdnts.LogSector; }

int L1RpcLogCone::GetLogSegment() const { return ConeCrdnts.LogSegment; }

RPCParam::L1RpcConeCrdnts L1RpcLogCone::GetConeCrdnts() const { return ConeCrdnts; }

void L1RpcLogCone::SetIdx(int index) { Index = index; }

int L1RpcLogCone::GetIdx() const { return Index; }
  
void L1RpcLogCone::SetLogStrip(int logPlane, int logStripNum, int digiIdx) {
//LogPlanesVec[logPlane].insert(logStripNum);
//LogPlanesVec[logPlane].insert(TLogPlane::value_type(logStripNum, vector<int>() ) );
  LogPlanesVec[logPlane][logStripNum].push_back(digiIdx);
}

void L1RpcLogCone::SetLogStrip(int logPlane, int logStripNum) {
  LogPlanesVec[logPlane].insert(TLogPlane::value_type(logStripNum, vector<int>() ) );
}


bool L1RpcLogCone::GetLogStripState(int logPlane, unsigned int logStripNum)  const {
  return LogPlanesVec[logPlane].count(logStripNum);
}

  
/**
 *
 * Adds a loghit to a cone
 * 
*/
bool L1RpcLogCone::AddLogHit(const L1RpcLogHit &logHit) {
  
  if (ConeCrdnts.Tower == logHit.getTower() &&
      ConeCrdnts.LogSector == logHit.getLogSector() &&
      ConeCrdnts.LogSegment == logHit.getLogSegment()  ) 
  {
    SetLogStrip(logHit.getlogPlaneNumber()-1, logHit.getStripNumberInCone(), logHit.getDigiIdx());
    return true;
  }
  else
    return false;
}


vector<int> L1RpcLogCone::GetLogStripDigisIdxs(int logPlane, unsigned int logStripNum) const {
  TLogPlane::const_iterator it = LogPlanesVec[logPlane].find(logStripNum); 
  if(it != LogPlanesVec[logPlane].end() )
    return it->second;
  else
    return vector<int>();
}


bool L1RpcLogCone::IsPlaneFired(int logPlane) const {
  if(LogPlanesVec[logPlane].size() == 0)
    return false; 
  else
    return true;  
}

int L1RpcLogCone::GetFiredPlanesCnt() const{
  int firedPlanes = 0;
  for(int logPlane = RPCParam::FIRST_PLANE; logPlane < RPCParam::USED_PLANES_COUNT[abs(ConeCrdnts.Tower)]; logPlane++) {
    firedPlanes = firedPlanes + IsPlaneFired(logPlane);
  }
  return firedPlanes;
}


int L1RpcLogCone::PossibleTrigger() const {
  int triggerType = 0; //0 - trigger not possible
                        //1 - 3/4 (3 inner planes fired)
                        //2 - 4/6 (four palnes fired)
  int firedPlanes = 0;

  int logPlane = RPCParam::FIRST_PLANE;
  for( ; logPlane <= RPCParam::LOGPLANE4; logPlane++) {
    firedPlanes = firedPlanes + IsPlaneFired(logPlane);
  }
  if(firedPlanes >= 3)
    triggerType = 1;

  for( ; logPlane < RPCParam::USED_PLANES_COUNT[abs(ConeCrdnts.Tower)]; logPlane++) {
    firedPlanes = firedPlanes + IsPlaneFired(logPlane);
  }
  if(firedPlanes >= 4)
    triggerType = 2;

  return triggerType;
}
