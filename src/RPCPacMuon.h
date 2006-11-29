/*******************************************************************************
*                                                                              *
*  Karol Bunkowski                                                             *
*  Warsaw University 2002                                                      *
*                                                                              *
*******************************************************************************/

#ifndef L1RpcPacMuonH
#define L1RpcPacMuonH
/** \class RPCPacMuon
 *
 * Muon candidate found by m_PAC for one LogCone. Containes the compare operators
 * used during sorting inside the m_PAC. The m_PAC sets for muon its cone coordinates.
 * \author Karol Bunkowski (Warsaw)
 */

#include "L1Trigger/RPCTrigger/src/RPCConst.h"
#include "L1Trigger/RPCTrigger/src/RPCMuon.h"
#include "L1Trigger/RPCTrigger/src/RPCPattern.h"
//------------------------------------------------------------------------------

class RPCPacMuon: public RPCMuon {
public:
  
  RPCPacMuon();
  
  RPCPacMuon(const RPCPattern& pattern, int quality, unsigned short firedPlanes);

  void setAll(const RPCPattern& pattern, int quality, unsigned short firedPlanes);

  void setPatternNum(int patternNum);

  bool operator < (const RPCPacMuon& pacMuon) const;

  bool operator > (const RPCPacMuon& pacMuon) const;

  bool operator == (const RPCPacMuon& pacMuon) const;

private:
};
#endif

