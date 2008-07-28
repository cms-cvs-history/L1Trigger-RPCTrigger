// -*- C++ -*-
//
// Package:    TwoMuonTime
// Class:      TwoMuonTime
// 
/**\class TwoMuonTime TwoMuonTime.cc MyAna/TwoMuonTime/src/TwoMuonTime.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Tomasz Maciej Frueboes
//         Created:  Tue Jun 17 11:47:33 CEST 2008
// $Id$
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/L1GlobalMuonTrigger/interface/L1MuRegionalCand.h"
#include "DataFormats/L1GlobalMuonTrigger/interface/L1MuGMTReadoutCollection.h"


#include "TH2F.h"
#include "TCanvas.h"
#include <TStyle.h>


#include <map>
#include <sstream>

// class decleration
//

class TwoMuonTime : public edm::EDAnalyzer {
   public:
      explicit TwoMuonTime(const edm::ParameterSet&);
      ~TwoMuonTime();


   private:
      virtual void beginJob(const edm::EventSetup&) ;
      virtual void analyze(const edm::Event&, const edm::EventSetup&);
      virtual void endJob() ;


//      edm::InputTag m_dts;
//      edm::InputTag m_rpcs;
      edm::InputTag m_GMTsrc;
      int m_runnumber;   
 
      typedef std::vector<L1MuRegionalCand>   L1MuRegionalCandCollection;

      // ----------member data ---------------------------


     struct TSimpleMuon {
        signed short bx;
        signed short phi;
        signed short tower;
        signed short wheel;
        signed short topbottom; // 1 top, -1 bottom
        bool used;
     };

    typedef std::map<std::string, TH2F*> THistMap;
    THistMap m_histMap;
    int getWheel(int tower);



};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
TwoMuonTime::TwoMuonTime(const edm::ParameterSet& iConfig)

{

  m_GMTsrc = iConfig.getParameter<edm::InputTag>("GMTsrc");



}


TwoMuonTime::~TwoMuonTime()
{
 

}


//
// member functions
//

// ------------ method called to for each event  ------------
void
TwoMuonTime::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
   using namespace edm;
   using namespace std;


//   std::cout << "--------------------------------------" << std::endl;

   m_runnumber = iEvent.id().run();



   edm::Handle<L1MuGMTReadoutCollection> pCollection;
   iEvent.getByLabel(m_GMTsrc, pCollection);
   
   if (!pCollection.isValid()) {
     std::cout << "can't find L1MuGMTReadoutCollection with label "
                               << m_GMTsrc.label() ;
     return;
   }

   L1MuGMTReadoutCollection const* gmtrc = pCollection.product();
   vector<L1MuGMTReadoutRecord> gmt_records = gmtrc->getRecords();
   vector<L1MuGMTReadoutRecord>::const_iterator RRItr;


   vector<TSimpleMuon> rpcMuons;
   vector<TSimpleMuon> dtMuons;

   for( RRItr = gmt_records.begin() ;
        RRItr != gmt_records.end() ;
        RRItr++ ) 
   {


     //cout << "Readout Record " << RRItr->getBxInEvent() << std::endl;
     vector<L1MuRegionalCand> rpcb_data = RRItr->getBrlRPCCands();

     L1MuRegionalCandCollection::const_iterator itRPC = rpcb_data.begin();
     for (;itRPC != rpcb_data.end();++itRPC){
       if (itRPC->pt_packed() !=0){
         //std::cout << "RPC " << itRPC->bx() << " " << itRPC->pt_packed() << std::endl;

         TSimpleMuon mu;
         mu.bx = itRPC->bx();
         mu.phi = itRPC->phi_packed();
         mu.tower = itRPC->eta_packed();
         mu.wheel = getWheel(mu.tower);
         mu.topbottom = 1; // top
         if (mu.phi > 72) mu.topbottom = -1; //bottom
      
         mu.used = false;
         if (mu.tower > 16){
             mu.tower = - ( (~mu.tower & 63) + 1);
         }
         rpcMuons.push_back(mu);


       } // nonempty muons
     } // RPC muons iteration
  } // gmt records iteration


  if (rpcMuons.size()<2) return;

//  std:: cout << "##### " << std::endl;
  for (vector<TSimpleMuon>::iterator itRPC= rpcMuons.begin(); itRPC != rpcMuons.end() ; ++itRPC){

    if (itRPC->used) continue;

    vector<TSimpleMuon>::iterator itRPC2 = itRPC;
    ++itRPC2;

    for (; itRPC2 != rpcMuons.end() ; ++itRPC2){
       
      if ( itRPC2->used ) continue;
      if ( itRPC->topbottom*itRPC2->topbottom > 0) continue;


      // tower matching
      //int towerDelta = std::abs(itRPC->tower-itRPC2->tower);
      //if (towerDelta > 1) continue;  // tower margin

      // two neighbour wheels, or same wheel
      if ( std::abs(itRPC->wheel-itRPC2->wheel)>1 ) continue;
      

      vector<TSimpleMuon>::iterator itUp = itRPC;
      vector<TSimpleMuon>::iterator itDown = itRPC2;

      if ( itRPC->phi > 72 ) {
         itUp = itRPC2;
         itDown = itRPC;
      }

      std::stringstream histName;
    
      histName << "wh"<< itUp->wheel << "up_wh" << itDown->wheel << "down";


      // phi cut
      //if (phiUp < 24) continue;


      // top-bottom phi matching
      //int phiDownExpected = 143-phiUp;
      //if ( std::abs(phiDownExpected-phiDown) > 10) continue; // phiMargin

      if (m_histMap.find(histName.str())==m_histMap.end() ) { // allocate new histo
         TH2F *  histo = new TH2F(histName.str().c_str(), histName.str().c_str(), 7, -3.5,3.5, 7,-3.5, 3.5);
         histo->GetXaxis()->SetTitle("up muon bx");
         histo->GetYaxis()->SetTitle("down muon bx");
         m_histMap[histName.str()] = histo;
      }
      
     std::cout << itUp->bx << " " << itDown->bx
                   << "|"  << itUp->tower << " " << itDown->tower
                   << "|"  << itUp->phi   << " " << itDown->phi 
                   << std::endl;

      (m_histMap[histName.str()])->Fill(itUp->bx,itDown->bx);
     
      itRPC->used = true;
      itRPC2->used = true;

      

    }
  } // RPC muons iter


}


// Calculate wheel. Lame method, should use digis, not muons
int TwoMuonTime::getWheel(int tower){

  int ret = 0;
  int sign = (tower > 0 ? 1 : -1 );
  tower = (tower > 0 ? tower : -tower );

  if (tower < 2) ret = 0;  // 0,1
  else if (tower < 5) ret = 1; // 2,3, 4   
  else ret = 2; // 5+   


  return ret*sign;

}


void 
TwoMuonTime::beginJob(const edm::EventSetup&)
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
TwoMuonTime::endJob() {


   THistMap::iterator it;
   for ( it = m_histMap.begin(); it != m_histMap.end(); ++it) {

     std::stringstream nameBox, nameText;
     nameBox << "run_" << m_runnumber  << it->first << "_box.png";
     nameText <<"run_" << m_runnumber << it->first << "_text.png";

     TCanvas c1;
     it->second->Draw("BOX");
     c1.Print(nameBox.str().c_str());

     gStyle->SetPaintTextFormat("3.2g");
     it->second->SetMarkerSize(2);
     it->second->DrawNormalized("TEXT",1);
     c1.Print(nameText.str().c_str());
   }






}

//define this as a plug-in
DEFINE_FWK_MODULE(TwoMuonTime);
