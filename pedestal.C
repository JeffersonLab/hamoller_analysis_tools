#include "Pedestal.h"
#include "TChain.h"
#include "TString.h"
#include "TSystem.h"
#include <iostream>
#include <vector>

using namespace std;

void pedestal(int run, const char* fnameform = "fadcV2_moller_analyzer_")
{

    // ----------------------------
    // Load TTrain data
    // ----------------------------
    TChain* tree = new TChain("T");
    for (int x = 0; x <= 100; ++x) {
        TString filename(Form("%s/%s%i",
                              gSystem->Getenv("HAMOLLER_ROOTFILE_DIR"),
                              fnameform, run));
        if (x > 0) filename += Form("_%i", x);
        filename += ".root";

        cout << filename << endl;

        if (gSystem->AccessPathName(filename)) break;

        tree->Add(filename);
    }

    Long64_t nEntries = tree->GetEntries();
    cout << nEntries << " total entries." << endl;
    if (nEntries == 0) {
        cout << "No entries found. Exiting." << endl;
        return;
    }
    Pedestal *ped = new Pedestal();
    ped->Process(tree,run);
}
