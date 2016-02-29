
// OverlapTest
#include "SusyNtToys/OverlapTest.h"

// std
#include <iostream>

// ROOT
#include "TROOT.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TLine.h"
#include "TPad.h"
#include "TColor.h"

using namespace std;


/////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////
OverlapTest::OverlapTest() :
    m_orHisto(NULL),
    m_canvas(NULL),
    m_upper(NULL),
    m_lower(NULL),
    m_dbg(false),
    do_or_output(true),
    n_denom(0),
    n_nom_passBVeto(0),
    m_numberOR(0),
    m_orConfig(0),
    m_boostedLepton(false),
    m_removeCaloMuons(false),
    m_muonJetGhost(false),
    m_eleBJet(false),
    m_muoBJet(false),
    m_useJVT(false)
{
    m_lepsAtStage.assign(25, 0);
    m_passBVeto.assign(25, 0);
}
/////////////////////////////////////////////////////////////////
// TSelector Begin
/////////////////////////////////////////////////////////////////
void OverlapTest::Begin(TTree* /*tree*/)
{
    SusyNtAna::Begin(0);
    if(verbose()) cout << "[OverlapTest::Begin]" << endl;
}
/////////////////////////////////////////////////////////////////
// TSelector Init
/////////////////////////////////////////////////////////////////
void OverlapTest::Init(TTree* tree)
{
    SusyNtAna::Init(tree);

    // initialize histograms
    m_canvas = new TCanvas("can", "", 1200, 600);
    m_upper  = new TPad("upper", "upper", 0.0, 0.0, 1.0, 1.0);
    m_lower  = new TPad("lower", "lower", 0.0, 0.0, 1.0, 1.0);
    setPadDimensions();

    m_orHisto = new TH1F("orHisto", "", 11, 0, 11);
    m_orHisto->SetStats(0);
    m_orHisto->SetMaximum(2.0);
    m_orHisto->SetMinimum(0.0);
    m_orHisto->GetXaxis()->SetTitle("OR Procedure");
    m_orHisto->GetYaxis()->SetTitle("Yep");
    m_orHisto->Sumw2();

    

}
void OverlapTest::setPadDimensions()
{
    m_canvas->cd();
    float up_height = 0.55;
    float dn_height = 0.40;
    m_upper->SetPad(0.0, 1.0-up_height, 1.0, 1.0);
    m_lower->SetPad(0.0, 0.0, 1.0, dn_height);

    m_upper->SetTickx(0);
    m_lower->SetGrid(0);
    m_lower->SetTicky(0);

    m_upper->SetFrameFillColor(0);
    m_upper->SetFillColor(0);
    m_lower->SetLeftMargin(0.04);
    m_lower->SetRightMargin(0.01);
    m_lower->SetBottomMargin(0.5);
    m_lower->SetTopMargin(0.03);

    m_upper->SetLeftMargin(0.04);
    m_upper->SetRightMargin(0.01);

    m_upper->Draw();
    m_lower->Draw();
    m_canvas->Update();
}
/////////////////////////////////////////////////////////////////
// TSelector Process
/////////////////////////////////////////////////////////////////
Bool_t OverlapTest::Process(Long64_t entry)
{
    int or_proc = 0;

    GetEntry(entry);
    SusyNtAna::clearObjects();
   // SusyNtAna::selectObjects();
    OverlapTest::clearORContainers();

    m_chainEntry++;
    if(verbose() || m_chainEntry%100==0) {
        cout << "**** Processing entry " << setw(6) << m_chainEntry
            << " run " << setw(6) << nt.evt()->run
            << " event " << setw(7) << nt.evt()->eventNumber << " ****"
            << endl;
    }

    // get objects with analysis' baseline definition (no OR)
    OverlapTest::getBaselineObjects();
    n_denom += (m_baseElectrons.size()+m_baseMuons.size());

    if(!OverlapTest::passEventCleaning())
        return false;

    // require >=2 baseline (before OR) leptons
    if( !((m_baseElectrons.size() + m_baseMuons.size()) >= 2) )
        return false;

    ///////////////////////////////////////////////////////////////
    // #0: n0220 OR
    addObjectCopy();
    setORConfig(/*boosted*/false, /*calo*/true,
                /*ghost*/true, /*ele-b*/true, /*mu-b*/true, /*jvt*/true);
    OverlapTest::performOldOverlap(ORElectrons(or_proc), ORMuons(or_proc),
                                ORJets(or_proc));
    if(verbose()) printState(or_proc);
    m_lepsAtStage[or_proc] += (ORElectrons(or_proc).size()+
                                ORMuons(or_proc).size());
    or_proc++;



    ///////////////////////////////////////////////////////////////
    // #1: default OR
    //      > boosed lepton             = FALSE
    //      > calo muon removal         = TRUE
    //      > mu-jet ghost association  = TRUE
    //      > ele-bjet                  = TRUE
    //      > muo-bjet                  = TRUE
    //      > use JVT                   = TRUE
    addObjectCopy();
    setORConfig(/*boosted*/false, /*calo*/true,
                /*ghost*/true, /*ele-b*/true, /*mu-b*/true, /*jvt*/true);
    OverlapTest::performOverlap(ORElectrons(or_proc), ORMuons(or_proc),
                                    ORJets(or_proc));
    if(verbose()) printState(or_proc);
    m_lepsAtStage[or_proc] += (ORElectrons(or_proc).size()+
                                ORMuons(or_proc).size());
    or_proc++;

    ///////////////////////////////////////////////////////////////
    // #2
    //      > boosed lepton             = TRUE
    //      > calo muon removal         = TRUE
    //      > mu-jet ghost association  = TRUE
    //      > ele-bjet                  = TRUE
    //      > muo-bjet                  = TRUE
    //      > use JVT                   = TRUE
    addObjectCopy();
    setORConfig(/*boosted*/true, /*calo*/true,
                /*ghost*/true, /*ele-b*/true, /*mu-b*/true, /*jvt*/true);
    OverlapTest::performOverlap(ORElectrons(or_proc), ORMuons(or_proc),
                                    ORJets(or_proc));
    if(verbose()) printState(or_proc);
    m_lepsAtStage[or_proc] += (ORElectrons(or_proc).size()+
                                ORMuons(or_proc).size());
    or_proc++;

    ///////////////////////////////////////////////////////////////
    // #3
    //      > boosed lepton             = TRUE
    //      > calo muon removal         = TRUE
    //      > mu-jet ghost association  = TRUE
    //      > ele-bjet                  = TRUE
    //      > muo-bjet                  = TRUE
    //      > use JVT                   = FALSE
    addObjectCopy();
    setORConfig(/*boosted*/true, /*calo*/true,
                /*ghost*/true, /*ele-b*/true, /*mu-b*/true, /*jvt*/false);
    OverlapTest::performOverlap(ORElectrons(or_proc), ORMuons(or_proc),
                                    ORJets(or_proc));
    if(verbose()) printState(or_proc);
    m_lepsAtStage[or_proc] += (ORElectrons(or_proc).size()+
                                ORMuons(or_proc).size());
    or_proc++;

    ///////////////////////////////////////////////////////////////
    // #4
    //      > boosed lepton             = TRUE
    //      > calo muon removal         = TRUE
    //      > mu-jet ghost association  = TRUE
    //      > ele-bjet                  = FALSE 
    //      > muo-bjet                  = FALSE 
    //      > use JVT                   = TRUE
    addObjectCopy();
    setORConfig(/*boosted*/true, /*calo*/true,
                /*ghost*/true, /*ele-b*/false, /*mu-b*/false, /*jvt*/true);
    OverlapTest::performOverlap(ORElectrons(or_proc), ORMuons(or_proc),
                                    ORJets(or_proc));
    if(verbose()) printState(or_proc);
    m_lepsAtStage[or_proc] += (ORElectrons(or_proc).size()+
                                ORMuons(or_proc).size());
    or_proc++;

    ///////////////////////////////////////////////////////////////
    // #5
    //      > boosed lepton             = FALSE
    //      > calo muon removal         = TRUE
    //      > mu-jet ghost association  = TRUE
    //      > ele-bjet                  = FALSE 
    //      > muo-bjet                  = FALSE 
    //      > use JVT                   = TRUE
    addObjectCopy();
    setORConfig(/*boosted*/false, /*calo*/true,
                /*ghost*/true, /*ele-b*/false, /*mu-b*/false, /*jvt*/true);
    OverlapTest::performOverlap(ORElectrons(or_proc), ORMuons(or_proc),
                                    ORJets(or_proc));
    if(verbose()) printState(or_proc);
    m_lepsAtStage[or_proc] += (ORElectrons(or_proc).size()+
                                ORMuons(or_proc).size());
    or_proc++;

    ///////////////////////////////////////////////////////////////
    // #6
    //      > boosed lepton             = FALSE
    //      > calo muon removal         = FALSE 
    //      > mu-jet ghost association  = TRUE 
    //      > ele-bjet                  = TRUE  
    //      > muo-bjet                  = TRUE  
    //      > use JVT                   = TRUE 
    addObjectCopy();
    setORConfig(/*boosted*/false, /*calo*/false,
                /*ghost*/true, /*ele-b*/true, /*mu-b*/true, /*jvt*/true);
    OverlapTest::performOverlap(ORElectrons(or_proc), ORMuons(or_proc),
                                    ORJets(or_proc));
    if(verbose()) printState(or_proc);
    m_lepsAtStage[or_proc] += (ORElectrons(or_proc).size()+
                                ORMuons(or_proc).size());
    or_proc++;

    ///////////////////////////////////////////////////////////////
    // #7
    //      > boosed lepton             = FALSE
    //      > calo muon removal         = TRUE 
    //      > mu-jet ghost association  = TRUE 
    //      > ele-bjet                  = TRUE  
    //      > muo-bjet                  = TRUE  
    //      > use JVT                   = FALSE
    addObjectCopy();
    setORConfig(/*boosted*/false, /*calo*/true,
                /*ghost*/true, /*ele-b*/true, /*mu-b*/true, /*jvt*/false);
    OverlapTest::performOverlap(ORElectrons(or_proc), ORMuons(or_proc),
                                    ORJets(or_proc));
    if(verbose()) printState(or_proc);
    m_lepsAtStage[or_proc] += (ORElectrons(or_proc).size()+
                                ORMuons(or_proc).size());
    or_proc++;

    ///////////////////////////////////////////////////////////////
    // #8
    //      > boosed lepton             = FALSE
    //      > calo muon removal         = TRUE 
    //      > mu-jet ghost association  = TRUE 
    //      > ele-bjet                  = FALSE  
    //      > muo-bjet                  = FALSE  
    //      > use JVT                   = FALSE
    addObjectCopy();
    setORConfig(/*boosted*/false, /*calo*/true,
                /*ghost*/true, /*ele-b*/false, /*mu-b*/false, /*jvt*/false);
    OverlapTest::performOverlap(ORElectrons(or_proc), ORMuons(or_proc),
                                    ORJets(or_proc));
    if(verbose()) printState(or_proc);
    m_lepsAtStage[or_proc] += (ORElectrons(or_proc).size()+
                                ORMuons(or_proc).size());
    or_proc++;

    ///////////////////////////////////////////////////////////////
    // #9
    //      > boosed lepton             = FALSE
    //      > calo muon removal         = TRUE 
    //      > mu-jet ghost association  = TRUE 
    //      > ele-bjet                  = FALSE  
    //      > muo-bjet                  = TRUE  
    //      > use JVT                   = TRUE
    addObjectCopy();
    setORConfig(/*boosted*/false, /*calo*/true,
                /*ghost*/true, /*ele-b*/false, /*mu-b*/true, /*jvt*/true);
    OverlapTest::performOverlap(ORElectrons(or_proc), ORMuons(or_proc),
                                    ORJets(or_proc));
    if(verbose()) printState(or_proc);
    m_lepsAtStage[or_proc] += (ORElectrons(or_proc).size()+
                                ORMuons(or_proc).size());
    or_proc++;

    ///////////////////////////////////////////////////////////////
    // #10
    //      > boosed lepton             = FALSE
    //      > calo muon removal         = TRUE 
    //      > mu-jet ghost association  = TRUE 
    //      > ele-bjet                  = TRUE  
    //      > muo-bjet                  = FALSE  
    //      > use JVT                   = TRUE
    addObjectCopy();
    setORConfig(/*boosted*/false, /*calo*/true,
                /*ghost*/true, /*ele-b*/true, /*mu-b*/false, /*jvt*/true);
    OverlapTest::performOverlap(ORElectrons(or_proc), ORMuons(or_proc),
                                    ORJets(or_proc));
    if(verbose()) printState(or_proc);
    m_lepsAtStage[or_proc] += (ORElectrons(or_proc).size()+
                                ORMuons(or_proc).size());
    or_proc++;

    // now compare b-veto-ness w.r.t. n0220
    // ie. (number of events w/ baseline jets passing OR X with 0 b-tags)
    // over (number of events w/ baseline jets passing n0220 OR with 0 b-tags)
    n_nom_passBVeto += passBVeto(ORJets(0));
    for(int iOR = 0; iOR < or_proc; iOR++) {
        if(passBVeto(ORJets(iOR)))
            m_passBVeto[iOR]+=1;
    } // iOR

    do_or_output = false;
    return true;
}
/////////////////////////////////////////////////////////////////
// Set OR Config
/////////////////////////////////////////////////////////////////
void OverlapTest::setORConfig(bool doBoost, bool removeCaloMuons,
                    bool muJetGhost, bool eleBJet, bool muoBJet,
                    bool useJVT)
{
    m_boostedLepton = doBoost;
    m_removeCaloMuons = removeCaloMuons;
    m_muonJetGhost = muJetGhost;
    m_eleBJet = eleBJet;
    m_muoBJet = muoBJet;
    m_useJVT = useJVT;

    if(verbose() || do_or_output) {
        cout << "---------------------------------" << endl;
        cout << "  Set OR Precedure #" << m_orConfig << endl;
        cout << "    boosted lepton      : " << doBoost << endl;
        cout << "    remove calo muons   : " << removeCaloMuons << endl;
        cout << "    muon-jet ghost ass. : " << muJetGhost << endl;
        cout << "    ele-b-jet           : " << eleBJet << endl;
        cout << "    muo-b-jet           : " << muoBJet << endl;
        cout << "    use JVT             : " << useJVT << endl;
        cout << "---------------------------------" << endl;
        cout << endl;
    }


    m_orConfig++;
}

/////////////////////////////////////////////////////////////////
// Overlap Methods
/////////////////////////////////////////////////////////////////
void OverlapTest::performOverlap(ElectronVector& electrons,
                MuonVector& muons, JetVector& jets)
{
    // perform electron muon overlap
    ele_muon_overlap(electrons, muons);

    // perform jet electron overlap
    jet_electron_overlap(electrons, jets);

    // perform jet muon overlap
    jet_muon_overlap(muons, jets);

}

// electron-muon
void OverlapTest::ele_muon_overlap(ElectronVector& electrons,
                MuonVector& muons)
{
    if(removeCaloMuons())
        nttools().overlapTool().m_e_overlap(muons, electrons);

    nttools().overlapTool().e_m_overlap(electrons, muons);
}

// jet-electron
void OverlapTest::jet_electron_overlap(ElectronVector& electrons,
                JetVector& jets)
{
    nttools().overlapTool().j_e_overlap(electrons, jets,
                    0.2, doEleBJet()); 

    nttools().overlapTool().e_j_overlap(electrons, jets,
                    0.4, doBoostedLepton(), useJVT());
}

// muon-jet
void OverlapTest::jet_muon_overlap(MuonVector& muons, JetVector& jets)
{
    nttools().overlapTool().j_m_overlap(jets, muons,
                    0.2, doMuoBJet(), doMuonJetGhost());

    nttools().overlapTool().m_j_overlap(muons, jets,
                    0.4, doBoostedLepton(), useJVT());

}

/// OLD OR (n0220 OR)
void OverlapTest::performOldOverlap(ElectronVector& electrons,
                        MuonVector& muons, JetVector& jets)
{

    // j_e_overlap with DR = 0.2 (remove jet) 
    if(!(electrons.size()==0 || jets.size()==0)) {
        for(int iEl=electrons.size()-1; iEl>=0; iEl--) {
            const Electron* e = electrons.at(iEl);
            for(int iJ=jets.size()-1; iJ>=0; iJ--){
                const Jet* j = jets.at(iJ);
                if(e->DeltaRy(*j) < 0.2) {
                    jets.erase(jets.begin()+iJ);
                }
            } // iJ
        } // iEl
    }

    // e_j_overlap with DR = 0.4 (remove electrons)
    if(!(electrons.size()==0 || jets.size()==0)) {;
        for(int iEl=electrons.size()-1; iEl>=0; iEl--){
            const Electron* e = electrons.at(iEl);
            for(size_t iJ=0; iJ<jets.size(); iJ++){
                const Jet* j = jets.at(iJ);
                if(e->DeltaRy(*j) < 0.4) {
                    electrons.erase(electrons.begin()+iEl);
                    break;
                }
            } // for(iJ)
        } // for(iEl)
    }

    // m_j_overlap with DR = 0.4
    if(!(muons.size()==0 || jets.size()==0)) {
        for(int iMu=muons.size()-1; iMu>=0; iMu--){
            const Muon* mu = muons.at(iMu);
            for(int iJ=jets.size()-1; iJ>=0; iJ--){
                const Jet* j = jets.at(iJ);
                int jet_nTrk = j->nTracks;
                if(mu->DeltaRy(*j) > 0.4) continue;
                if(jet_nTrk < 3) {
                    jets.erase(jets.begin() + iJ);
                }
                else {
                    muons.erase(muons.begin() + iMu);
                    break;
                }
            } // for(iJ)
        } // for(iMu)
    }

    // e_m_overlap with DR = 0.01 (remove both ele and muon)
    int nEl = electrons.size();
    int nMu = muons.size();
    if(!(nEl==0 || nMu==0)) {

        static std::set<const Electron*> electronsToRemove;
        static std::set<const Muon*> muonsToRemove;
        electronsToRemove.clear();
        muonsToRemove.clear();
        for(int iEl=0; iEl<nEl; iEl++){
            const Electron* e = electrons.at(iEl);
            for(int iMu=0; iMu<nMu; iMu++){
                const Muon* mu = muons.at(iMu);
                if(e->DeltaRy(*mu) < 0.01){
                    electronsToRemove.insert(e);
                    muonsToRemove.insert(mu);
                } // dR match
            } // iMu
        } // iEl
        // remove the flagged electrons
        if(electronsToRemove.size()){
            for(int iEl=nEl-1; iEl>=0; iEl--){
                if(electronsToRemove.find(electrons.at(iEl)) != electronsToRemove.end()){
                    electrons.erase(electrons.begin()+iEl);
                }
            }
        }
        // remove the flagged muons
        if(muonsToRemove.size()){
            for(int iMu=nMu-1; iMu>=0; iMu--){
                if(muonsToRemove.find(muons.at(iMu)) != muonsToRemove.end()){
                    muons.erase(muons.begin()+iMu);
                }
            }
        }
    }

    // e_e_overlap with DR = 0.05 remove ele with lower pT
    nEl=electrons.size();
    if(!(nEl<2)) {

        // Find all possible e-e pairings
        static std::set<const Electron*> electronsToRemove;
        electronsToRemove.clear();
        for(int iEl=0; iEl<nEl; iEl++){
            const Electron* ei = electrons.at(iEl);
            for(int jEl=iEl+1; jEl<nEl; jEl++){
                const Electron* ej = electrons.at(jEl);
                if(ei->DeltaRy(*ej) < 0.05){
                    if(ei->Pt() < ej->Pt()){
                        electronsToRemove.insert(ei);
                        break; // ei no longer exists for looping!
                    } else{
                        electronsToRemove.insert(ej);
                    }
                } // dR match
            } // jEl
        } // iEl
        // remove the flagged electrons
        for(int iEl=nEl-1; iEl>=0; iEl--){
            if(electronsToRemove.find(electrons.at(iEl)) != electronsToRemove.end()){
                electrons.erase(electrons.begin()+iEl);
            } // found one
        }
    }
}

/////////////////////////////////////////////////////////////////
// draw the histo
/////////////////////////////////////////////////////////////////
void OverlapTest::drawHisto()
{
    cout << "leps stage size : " << m_lepsAtStage.size() << endl;

    m_canvas->cd();
    m_upper->cd();
    for(int il = 0; il < (int)m_lepsAtStage.size(); il++) {
        if(il >= m_orHisto->GetNbinsX()) break;
        cout << "setting bin " << il << " with "
             << m_lepsAtStage[il]*1.0/m_lepsAtStage[0] << endl;
             //<< m_lepsAtStage[il]*1.0/n_denom << endl;
        m_orHisto->SetBinContent(il+1, m_lepsAtStage[il]*1.0/m_lepsAtStage[0]);
        //m_orHisto->SetBinContent(il, m_lepsAtStage[il]*1.0/n_denom);
    }
    m_orHisto->Draw("hist");
    m_canvas->Update();

    TLine line(0, 1, 11, 1);
    line.SetNDC(false);
    line.SetLineColor(kRed);
    line.SetLineStyle(2);
    line.Draw("same");
    m_canvas->Update();
}

/////////////////////////////////////////////////////////////////
// TSelector Terminate
/////////////////////////////////////////////////////////////////
void OverlapTest::Terminate()
{
    SusyNtAna::Terminate();
    if(verbose()) cout << "[OverlapTest::Terminate]" << endl;

    drawHisto();
    // print the canvas
    m_canvas->SaveAs("test.eps");

    printBVetoInfo();


    // delete histos
    delete m_orHisto;
    delete m_upper;
    delete m_lower;
    delete m_canvas;
}
/////////////////////////////////////////////////////////////////
// clear
/////////////////////////////////////////////////////////////////
void OverlapTest::clearORContainers()
{
    m_orElectrons.clear();
    m_orMuons.clear();
    m_orJets.clear();

    m_numberOR = 0;
    m_orConfig = 0;
}

/////////////////////////////////////////////////////////////////
// passEventCleaning
/////////////////////////////////////////////////////////////////
bool OverlapTest::passEventCleaning()
{
    int flags = nt.evt()->cutFlags[NtSys::NOM];

    // grl
    if(!nttools().passGRL(flags) )                  return false;
    // lar
    if(!nttools().passLarErr(flags) )               return false;
    // tile error
    if(!nttools().passTileErr(flags) )              return false;
    // ttc veto
    if(!nttools().passTTC(flags) )                  return false;
    // event w/ vtx
    if(!nttools().passGoodVtx(flags) )              return false;
    // bad muon
    if(!nttools().passBadMuon(m_preMuons) )         return false;
    // cosmic muon
    if(!nttools().passCosmicMuon(m_baseMuons) )     return false;
    // jet cleaning
    if(!nttools().passJetCleaning(m_baseJets) )     return false;

    return true;
}

/////////////////////////////////////////////////////////////////
// getBaselineObjects
/////////////////////////////////////////////////////////////////
void OverlapTest::getBaselineObjects()
{
    // get pre objects
    nttools().getPreObjects(&nt, NtSys::NOM, m_preElectrons, m_preMuons,
                    m_preJets, m_preTaus, m_prePhotons);

    // get baseline objects (this is before OR)
    nttools().getBaselineObjects(m_preElectrons, m_preMuons, m_preJets,
                    m_preTaus, m_prePhotons,
                    m_baseElectrons, m_baseMuons, m_baseJets,
                    m_baseTaus, m_basePhotons);

}
/////////////////////////////////////////////////////////////////
// addObjectCopy
/////////////////////////////////////////////////////////////////
void OverlapTest::addObjectCopy()
{
    m_numberOR++;

    ElectronVector tmpEle; tmpEle.clear();
    MuonVector tmpMuo;     tmpMuo.clear();
    JetVector tmpJet;      tmpJet.clear();

    for(int ie = 0; ie < (int)m_baseElectrons.size(); ie++) {
        tmpEle.push_back(m_baseElectrons.at(ie));
    }
    for(int im = 0; im < (int)m_baseMuons.size(); im++) {
        tmpMuo.push_back(m_baseMuons.at(im));
    }
    for(int ij = 0; ij < (int)m_baseJets.size(); ij++) {
        tmpJet.push_back(m_baseJets.at(ij));
    }

    m_orElectrons.push_back(tmpEle);
    m_orMuons.push_back(tmpMuo);
    m_orJets.push_back(tmpJet);

}
/////////////////////////////////////////////////////////////////
// Get the objects for OR
/////////////////////////////////////////////////////////////////
ElectronVector& OverlapTest::ORElectrons(int proc)
{
    if(proc > (m_numberOR - 1)) {
        cout << "[OverlapTest::ORElectrons]    "
             << "Attempting to access OR electron at index "
             << proc << " when number of OR procedures is "
             << m_numberOR << endl;
        exit(1);
    }

    return m_orElectrons[proc];
}
MuonVector& OverlapTest::ORMuons(int proc)
{
    if(proc > (m_numberOR - 1)) {
        cout << "[OverlapTest::ORMuons]    "
             << "Attempting to access OR muons at index "
             << proc << " when number of OR procedures is "
             << m_numberOR << endl;
        exit(1);
    }

    return m_orMuons[proc];
}
JetVector& OverlapTest::ORJets(int proc)
{
    if(proc > (m_numberOR - 1)) {
        cout << "[OverlapTest::ORJets]    "
             << "Attempting to access OR jets at index "
             << proc << " when number of OR procedures is "
             << m_numberOR << endl;
        exit(1);
    }

    return m_orJets[proc];
}
/////////////////////////////////////////////////////////////////
// b-tagging
/////////////////////////////////////////////////////////////////
bool OverlapTest::passBVeto(JetVector& jets)
{
    bool there_are_no_btags = true;
    for(int ij = 0; ij < (int)jets.size(); ij++){
        if(nttools().jetSelector().isB(jets[ij])) {
            there_are_no_btags = false;
            break;
        }
    }
    return there_are_no_btags;
}
/////////////////////////////////////////////////////////////////
// print the state of the OR
/////////////////////////////////////////////////////////////////
void OverlapTest::printState(int proc)
{
    if(proc > (m_numberOR-1)) {
        cout << "[OverlapTest::printState]    "
        << "Attempting to access containers at index "
        << proc << " when number of OR procedures is "
        << m_numberOR << endl;
        exit(1);
    }

    cout << "----------------------------------" << endl;
    cout << "   OR PROCEDURE # " << proc << endl;
    cout << " electrons : " << m_baseElectrons.size()
         << "  --> " << ORElectrons(proc).size() << endl;
    cout << " muons     : " << m_baseMuons.size()
         << "  --> " << ORMuons(proc).size() << endl;
    cout << " jets      : " << m_baseJets.size()
         << " --> " << ORJets(proc).size() << endl;
    cout << "----------------------------------" << endl;

}
void OverlapTest::printBVetoInfo()
{
    cout << "-------------------" << endl;
    cout << "B-Veto Efficiencies" << endl;
    cout << endl;
    for(int i = 0; i < (int)m_passBVeto.size(); i++){
        if(i >= m_orHisto->GetNbinsX()) continue;
        cout << " > OR # " << i << "   " << setw(4)
             << m_passBVeto[i]*1.0 / n_nom_passBVeto
             << endl;
    } // i
}
