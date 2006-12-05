#ifndef L1RpcPatternsParserH
#define L1RpcPatternsParserH
/*******************************************************************************
*                                                                              *
*  Karol Bunkowski                                                             *
*  Warsaw University 2005                                                      *
*                                                                              *
*******************************************************************************/


#include <string>
#include <iostream>

#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>

#include "L1Trigger/RPCTrigger/src/RPCConst.h"
#include "L1Trigger/RPCTrigger/src/RPCPattern.h"
//#include "L1Trigger/RPCTrigger/src/RPCException.h"

XERCES_CPP_NAMESPACE_USE
class RPCPatternsParser : public DefaultHandler  {
public:
  //class SAX2PatHandler : public DefaultHandler {
  //public:
  //  SAX2PatHandler();
  //  ~SAX2PatHandler();

  //  void startElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname, const Attributes& attrs);
  //  void endElement (const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname);
  //};

  RPCPatternsParser();
  ~RPCPatternsParser();

  
  void startElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname, const Attributes& attrs);
  void endElement (const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname);

  void parse(std::string fileName);

  const L1RpcPatternsVec& getPatternsVec(const RPCConst::l1RpcConeCrdnts& coneCrds) const;

  struct TQuality {
    int m_QualityTabNumber;
    std::string m_FiredPlanes;
    short m_QualityValue;
  };

  typedef std::vector<TQuality> TQualityVec;

  const TQualityVec & getQualityVec() const{ //XXX - clean me!
    return m_QualityVec;
  };


private:
  //virtual void startElement(const XMLCh* const name, xercesc::AttributeList& attributes);

  //virtual void endElement(const XMLCh* const name);

  static int m_InstanceCount;

  std::string m_CurrElement;

  TQualityVec m_QualityVec;

  typedef std::map<RPCConst::l1RpcConeCrdnts, L1RpcPatternsVec> TPatternsVecsMap;

  TPatternsVecsMap m_PatternsVecsMap;
  
  TPatternsVecsMap::iterator m_CurPacIt;

  RPCPattern m_CurPattern;
};

#endif
