#include "GoodHelicities.h"
#include "TTrain.h"
#include "TString.h"
#include "TSystem.h"
#include <iostream>
#include <vector>

using namespace std;

void buildLeftRightSumTree(int run, int ndelaywin = 8, const char* fnameform = "fadcV2_moller_analyzer_")
{

    // ----------------------------
    // Load TTrain data
    // ----------------------------
    TTrain* tree = new TTrain();
    tree->SetUpTTrain(run, fnameform);
    Long64_t nEntries = tree->GetEntries();
    cout << nEntries << " total entries." << endl;
    if (nEntries == 0) {
        cout << "No entries found. Exiting." << endl;
        return;
    }
    
    GoodHelicities *gh = new GoodHelicities();
    gh->SetDelayWindows(ndelaywin);
    gh->SetOverwrite(true);
    gh->FillLeftRightSumTree( tree, run );
}
