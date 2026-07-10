#include "TTrain.h"
#include "GoodHelicities.h"
#include "HelPat_t.h"
#include "TString.h"
#include "TSystem.h"
#include <iostream>
#include <vector>

using namespace std;

void printGoodHelicities(int run,
                         const string& pattype = "quartet",
                         double flip_freq = 29.5596,
                         int ndelay = 8,
                         const char* fnameform = "fadcV2_moller_analyzer_")
{

    // ----------------------------
    // Load TTrain data
    // ----------------------------
    TTrain* tree = new TTrain();
    for (int x = 0; x <= 100; ++x) {
        TString filename(Form("%s/%s%i",
                              gSystem->Getenv("HAMOLLER_ROOTFILE_DIR"),
                              fnameform, run));
        if (x > 0) filename += Form("_%i", x);
        filename += ".root";

        cout << filename << endl;

        if (gSystem->AccessPathName(filename)) break;

        tree->AddFile(filename, "T");
    }

    Long64_t nEntries = tree->GetEntries();
    cout << nEntries << " total entries." << endl;
    if (nEntries == 0) {
        cout << "No entries found. Exiting." << endl;
        return;
    }
    
    // ----------------------------
    // Initialize GoodHelicities
    // ----------------------------
    GoodHelicities* gh = new GoodHelicities();
    gh->SetPatternType(pattype);
    gh->SetTsettleFrequency(flip_freq);
    gh->SetDelayWindows(ndelay);

    gh->FindGoodHelicities(tree);

    // ----------------------------
    // Get patterns and print
    // ----------------------------
    const vector<HelPat_t>& patterns = gh->GetPatterns();

    int i = 0;
    for (const auto& pat : patterns) {
 
        cout << i++ << " "
             << pat.pattern_start << " "
             << pat.pattern_end << " "
	     << (pat.polarity > 0 ? "+":"-")
             << abs(pat.polarity) << " ";
	if(pat.good_pattern){
	   cout<<"Good"<<endl;
	}else{
	   cout<<"Failed pattern checks. "
	       << pat.complete<<pat.sequenceOK
	       << pat.windowtimingOK<<pat.patterntimingOK
	       << pat.predictedOK<<pat.helicityFound
	       << endl;
	}
    }
    gh->FillLeftRightSumTree(tree,run);
}
