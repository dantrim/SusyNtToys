
// OverlapTest
#include "SusyNtToys/OverlapTest.h"

// std
#include <iostream>



using namespace std;


/////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////
OverlapTest::OverlapTest() :
    m_dbg(false),
    m_numberOR(0)
{
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
// TSelector Process
/////////////////////////////////////////////////////////////////
Bool_t OverlapTest::Process(Long64_t entry)
{
    GetEntry(entry);
    SusyNtAna::clearObjects();
    OverlapTest::clearORContainers();

    m_chainEntry++;
    if(verbose() || m_chainEntry%100==0) {
        cout << "**** Processing entry " << setw(6) << m_chainEntry
            << " run " << setw(6) << nt.evt()->run
            << " event " << setw(7) << nt.evt()->eventNumber << " ****" << endl;
    }

    // get objects with analysis' baseline definition
    OverlapTest::getBaselineObjects();

    if(!OverlapTest::passEventCleaning())
        return false;

    // add a copy for each type of OR to check
    addObjectCopy();
    addObjectCopy();



    return true;
}
/////////////////////////////////////////////////////////////////
// TSelector Terminate
/////////////////////////////////////////////////////////////////
void OverlapTest::Terminate()
{
    SusyNtAna::Terminate();
    if(verbose()) cout << "[OverlapTest::Terminate]" << endl;
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
    nttools().getBaselineObjects(m_preElectrons, m_preMuons, m_preJets, m_preTaus, m_prePhotons,
                    m_baseElectrons, m_baseMuons, m_baseJets, m_baseTaus, m_basePhotons);

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
