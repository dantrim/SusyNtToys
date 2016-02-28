
// std/stl
#include <string>
#include <iostream>

// ROOT
#include "TChain.h"

// SusyNtuple
#include "SusyNtuple/ChainHelper.h"
#include "SusyNtuple/string_utils.h"

// SusyNtToys
#include "SusyNtToys/OverlapTest.h"


using namespace std;



void help()
{
    cout << " Options: "                                << endl;
    cout << "   -i input (file, list, or dir)"          << endl;
    cout << "        defaults: ''"                      << endl;
    cout << "   -n number of events to process"         << endl;
    cout << "        defaults: -1 (all events)"         << endl;

    cout << "  -d debug printout level"                 << endl;
    cout << "        defaults: 0 (quiet)"               << endl;

    cout << "  -h print this help"                      << endl;
}


int main(int argc, char** argv)
{
    int nEvt = -1;
    int dbg  = 0;
    string input;

    // read inputs
    for(int i = 1; i < argc; i++) {
        if          (strcmp(argv[i], "-n") == 0) nEvt = atoi(argv[++i]);
        else if     (strcmp(argv[i], "-d") == 0) dbg  = atoi(argv[++i]);
        else if     (strcmp(argv[i], "-i") == 0) input = argv[++i];
        else {
            help();
            return 0;
        }

    }


    if(input.empty()) {
        cout << "You must specify an input" << endl;
        return 1;
    }

    TChain* chain = new TChain("susyNt");
    ChainHelper::addInput(chain, input, dbg>0);
    Long64_t nEntries = chain->GetEntries();
    chain->ls();

    // build the selector
    OverlapTest* susyAna = new OverlapTest();
    susyAna->setDebug(dbg);
    susyAna->nttools().setAnaType(AnalysisType::Ana_Stop2L);
    if(nEvt<0) nEvt = nEntries;
    cout << endl;
    cout << "Total entries      : " << nEntries << endl;
    cout << "Entries to process : " << nEvt << endl;
    if(nEvt>0) chain->Process(susyAna, "", nEvt);

    cout << endl;
    cout << "OverlapTest job done" << endl;

    delete chain;
    return 0;

} // main

