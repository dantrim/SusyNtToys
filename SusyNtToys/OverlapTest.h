// -*- c++ -*-
#ifndef SUSYNTTOYS_OVERLAPTEST_H
#define SUSYNTTOYS_OVERLAPTEST_H

// SusyNtuple
#include "SusyNtuple/SusyNtAna.h"
#include "SusyNtuple/SusyNtTools.h"

// ROOT
class TH1F;
class TCanvas;
class TPad;

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
        ElectronVector& ORElectrons(int ORprocedure);
        MuonVector& ORMuons(int ORprocedure);
        JetVector& ORJets(int ORprocedure);

        // basic event cleaning
        bool passEventCleaning();

        // fill the objects before OR
        void getBaselineObjects();

        // get a copy of the original baseline object vectors
        void addObjectCopy();

        // TSelector Methods
        virtual void Begin(TTree *tree);
        virtual void Init(TTree *tree);
        virtual Bool_t Process(Long64_t entry);
        virtual void Terminate();

        // OVERLAP METHODS
        void performOverlap(ElectronVector& electrons, MuonVector& muons,
                            JetVector& jets);
        void ele_muon_overlap(ElectronVector& electrons, MuonVector& muons);
        void jet_electron_overlap(ElectronVector& electrons, JetVector& jets);
        void jet_muon_overlap(MuonVector& muons, JetVector& jets);

        // OR from n0220
        void performOldOverlap(ElectronVector& electrons, MuonVector& muons,
                            JetVector& jets);


        // Set the OR configuration
        void setORConfig(bool doBoosted, bool removeCaloMuons,
                         bool muonJetGhost, bool eleBJet,
                         bool muoBJet, bool useJVT);
        
        bool doBoostedLepton() { return m_boostedLepton; }
        bool removeCaloMuons() { return m_removeCaloMuons; }
        bool doMuonJetGhost() { return m_muonJetGhost; }
        bool doEleBJet() { return m_eleBJet; }
        bool doMuoBJet() { return m_muoBJet; }
        bool useJVT() { return m_useJVT; }

        // b-tagging stuff
        bool passBVeto(JetVector& jets);

        void printState(int proc);
        void printBVetoInfo();


        ////////////////////////////////////////////
        // HISTOGRAMS
        ////////////////////////////////////////////
        TH1F* m_orHisto;
        TCanvas* m_canvas;
        TPad* m_upper;
        TPad* m_lower;

        void setPadDimensions();
        void drawHisto();

    protected :
        bool m_dbg;
        bool do_or_output;
        int n_denom;
        int n_nom_passBVeto;
        std::vector<int> m_passBVeto;
        /**
            number of baseline container copies
            (i.e. number of different OR procedures)
        */
        int m_numberOR;
        int m_orConfig;

        /**
            configuration toggles
        */
        bool m_boostedLepton;
        bool m_removeCaloMuons;
        bool m_muonJetGhost;
        bool m_eleBJet;
        bool m_muoBJet;
        bool m_useJVT;

        std::vector<ElectronVector>     m_orElectrons;
        std::vector<MuonVector>         m_orMuons;
        std::vector<JetVector>          m_orJets;

        std::vector<int>                m_lepsAtStage;


}; // end class


#endif
