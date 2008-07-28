// -*- C++ -*-
//
// Package:    RPCvsDTTiming
// Class:      RPCvsDTTiming
// 
/**\class RPCvsDTTiming RPCvsDTTiming.cc MyAna/RPCvsDTTiming/src/RPCvsDTTiming.cc

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
#include <sstream>
//
// class decleration
//

class RPCvsDTTiming : public edm::EDAnalyzer {
   public:
      explicit RPCvsDTTiming(const edm::ParameterSet&);
      ~RPCvsDTTiming();


   private:
      virtual void beginJob(const edm::EventSetup&) ;
      virtual void analyze(const edm::Event&, const edm::EventSetup&);
      virtual void endJob() ;
      void updateHist();


//      edm::InputTag m_dts;
//      edm::InputTag m_rpcs;
      edm::InputTag m_GMTsrc;
      int  m_towerMar;
      int  m_phiMar;
      typedef std::vector<L1MuRegionalCand>   L1MuRegionalCandCollection;

      // ----------member data ---------------------------


     struct TSimpleMuon {
        signed short bx;
        signed short phi;
        signed short tower;
     };

    TH2F * m_histo;
    int n_matched;
    int m_runnumber; 
    int n_ev;
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
RPCvsDTTiming::RPCvsDTTiming(const edm::ParameterSet& iConfig)
: n_ev(0)
{
   //now do what ever initialization is needed
  //m_rpcs  = iConfig.getParameter<edm::InputTag>("RPC");
  //m_dts  = iConfig.getParameter<edm::InputTag>("DT");

  m_GMTsrc = iConfig.getParameter<edm::InputTag>("GMTsrc");
  m_towerMar = iConfig.getParameter<int>("towerMar");
  m_phiMar = iConfig.getParameter<int>("phiMar");

  m_histo = new TH2F("a","DT vs RPC timing", 7, -3.5,3.5, 7,-3.5, 3.5);
  m_histo->GetXaxis()->SetTitle("RPC cand bx");
  m_histo->GetYaxis()->SetTitle("DT  cand bx");

  n_matched = 0;

}


RPCvsDTTiming::~RPCvsDTTiming()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called to for each event  ------------
void
RPCvsDTTiming::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{


  ++n_ev;
  bool updateInThisEvent = false;

  if ( n_ev % 10000==0) {
       std::cout << " " << n_ev;
       std::cout.flush();
  }

  m_runnumber = iEvent.id().run();
   using namespace edm;
   using namespace std;

     
  int rpcMinBx= 10;
  int dtMinBx = 10;

//   std::cout << "--------------------------------------" << std::endl;




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
     vector<L1MuRegionalCand> dtf_data = RRItr->getDTBXCands();

     L1MuRegionalCandCollection::const_iterator itDT = dtf_data.begin();

     for (;itDT != dtf_data.end();++itDT){
       if (itDT->pt_packed() !=1){
         //std::cout << "DT " << itDT->bx() << " " << itDT->pt_packed() << " " << itDT->eta_packed() << std::endl;
         TSimpleMuon mu;
         mu.bx = itDT->bx(); 

         if (mu.bx < dtMinBx) dtMinBx = mu.bx;

         mu.phi = itDT->phi_packed(); 
         mu.tower = itDT->eta_packed(); 
         if (mu.tower > 16){
             mu.tower = - ( (~mu.tower & 63) + 1);
         }
         dtMuons.push_back(mu);
       } // nonempty muons
     } // dt muons iter

     L1MuRegionalCandCollection::const_iterator itRPC = rpcb_data.begin();
     for (;itRPC != rpcb_data.end();++itRPC){
       if (itRPC->pt_packed() !=0){
         //std::cout << "RPC " << itRPC->bx() << " " << itRPC->pt_packed() << std::endl;
         TSimpleMuon mu;
         mu.bx = itRPC->bx();
         if (mu.bx < rpcMinBx) rpcMinBx = mu.bx;
         mu.phi = itRPC->phi_packed();
         mu.tower = itRPC->eta_packed();
         if (mu.tower > 16){
             mu.tower = - ( (~mu.tower & 63) + 1);
         }
         rpcMuons.push_back(mu);


       } // nonempty muons
     } // RPC muons iteration
  } // gmt records iteration


  for (vector<TSimpleMuon>::iterator itRPC= rpcMuons.begin(); itRPC != rpcMuons.end() ; ++itRPC){

    bool matched = false;
    int phiRPC = itRPC->phi;
    for (vector<TSimpleMuon>::iterator itDT= dtMuons.begin(); itDT != dtMuons.end() ; ++itDT){

      int phiDT = itDT->phi;
      int deltaPhi = 99;
      if (phiRPC < 5 && phiDT > 138)  {
        deltaPhi = phiRPC - (144 - phiDT);
      }
      else if  (phiRPC > 138  && phiDT < 5) {
        deltaPhi = phiDT - (144 - phiRPC);

      } else {
        deltaPhi = phiDT -  phiRPC;
        if (deltaPhi < 0) deltaPhi = -deltaPhi; 
      }

      int deltaTower = itRPC->tower - itRPC->tower;
      if (deltaTower<0) deltaTower = -deltaTower;

      if (!matched && deltaPhi <= m_phiMar && deltaTower <= m_towerMar){
        matched = true;
        /*
        std::cout << itDT->bx << " " << itRPC ->bx << std::endl;
        std::cout <<  "  -> " << itDT->tower << " " << itRPC -> tower << std::endl;
        std::cout <<  "     " << itDT->phi << " " << itRPC -> phi << std::endl << std::endl;
        */ 
        m_histo->Fill(itRPC->bx, itDT->bx);
        updateInThisEvent = true;
      }


    } //DT muons 
  } // RPC muons iter
  if (updateInThisEvent) {
    ++n_matched;
  
    if ( n_matched % 100 == 0 ){
      std::cout << "Updating hist after " <<  n_matched << " matched muons " << std::endl;
      std::cout.flush(); 
      updateHist();
    }
  }

}


// ------------ method called once each job just before starting event loop  ------------
void 
RPCvsDTTiming::beginJob(const edm::EventSetup&)
{
}

void RPCvsDTTiming::updateHist(){

   TCanvas c1;
   m_histo->Draw("BOX");
   std::stringstream nbox, ntext;
   nbox << "run_" << m_runnumber << "box.png" ;
   ntext << "run_" << m_runnumber << "text.png";

   c1.Print(nbox.str().c_str());

   gStyle->SetPaintTextFormat("3.2g");
   m_histo->SetMarkerSize(2);
   m_histo->DrawNormalized("TEXT",1);
   c1.Print(ntext.str().c_str());





}

// ------------ method called once each job just after ending the event loop  ------------
void 
RPCvsDTTiming::endJob() {

 updateHist();


}

//define this as a plug-in
DEFINE_FWK_MODULE(RPCvsDTTiming);
