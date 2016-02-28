// -*- c++ -*-
#ifndef SUSYNTTOYS_OVERLAPTEST_H
#define SUSYNTTOYS_OVERLAPTEST_H

// SusyNtuple
#include "SusyNtuple/SusyNtAna.h"
#include "SusyNtuple/SusyNtTools.h"

// ROOT

// std/stl
#include <vector>


/// Overlap Procedure Comparator

class OverlapTest : public SusyNtAna
{
    public :
        OverlapTest();
        virtual ~OverlapTest(){};

        void setDebug(bool dbg = true) { m_dbg = dbg; }
        bool verbose() { return m_dbg; }

        int numberOR() { return m_numberOR; }

        void clearORContainers();

        // basic event cleaning
        bool passEventCleaning();

        // fill the objects before OR
        void getBaselineObjects();

        // get a copy of the original baseline object vectors
        void addObjectCopy();

        // TSelector Methods
        virtual void Begin(TTree *tree);
        virtual Bool_t Process(Long64_t entry);
        virtual void Terminate();

        // OVERLAP METHODS
        void performOverlap(ElectronVector& electrons, MuonVector& muons, JetVector& jets,
                            bool doBoostedLepton,
                            bool removeCaloMuons,
                            bool muonJetGhost,
                            bool eleBJet,
                            bool muoBJet);

        void ele_muon_overlap(ElectronVector& electrons, MuonVector& muons);
        void m_e_overlap(MuonVector& muons, ElectronVector& electrons); // calo tagging
        void e_m_overlap(ElectronVector& electrons, MuonVector& muons);
        void jet_electron_overlap(ElectronVector& electrons, JetVector& jets);
        void j_e_overlap(ElectronVector& electrons, JetVector& jets, double dR = 0.2, bool doBJetOR = true);
        void e_j_overlap(ElectronVector& electrons, JetVector& jets, double dR = 0.4, bool doBoosted = false);
        void jet_muon_overlap(MuonVector& muons, JetVector& jets);
        void j_m_overlap(JetVector& jets, MuonVector& muons, double dR = 0.2, bool doBJetOR = true, bool doGhost = true);
        void m_j_overlap(MuonVector& muons, JetVector& jets, double dR = 0.4, bool doBoosted = false);
        

    protected :
        bool m_dbg;
        int m_numberOR; /// number of baseline container copies (i.e. number of different OR procecdures)

        std::vector<ElectronVector>     m_orElectrons;
        std::vector<MuonVector>         m_orMuons;
        std::vector<JetVector>          m_orJets;


}; // end class


#endif
