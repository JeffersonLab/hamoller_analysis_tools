#include <vector>
#include <iostream>
#include <GoodHelicities.h>
#include <TStyle.h>
#include <TTrain.h>
#include <TCanvas.h>
#include <TH1D.h>
#include <TSystem.h>
#include <TString.h>
using namespace std;
void AnalyzeWindows(int run)
{
   gStyle->SetStatFormat("6.9g");
   TTrain *T = new TTrain();
   std::cout<<"Setting up TTrain for run "<< run <<std::endl;
   const char* fnamebase = "fadcV2_moller_analyzer_";
   for (int x = 0; x <= 100; ++x) {
      TString filename(Form("%s/%s%i",
			    gSystem->Getenv("HAMOLLER_ROOTFILE_DIR"),
			    fnamebase, run));
      if (x > 0) filename += Form("_%i", x);
      filename += ".root";

      std::cout << filename << std::endl;

      if (gSystem->AccessPathName(filename)) break;

      T->AddFile(filename, "T");
   }

   Long64_t nEntries = T->GetEntries();
   std::cout << nEntries << " total entries." << std::endl;
   ULong64_t evtTime;
   GoodHelicities *gh = new GoodHelicities();
   gh->FindGoodHelicities(T);
   auto patterns = gh->GetPatterns();


   T->SetBranchStatus("*",0);
   T->SetBranchStatus("fEvtHdr.fEvtTime",1);
   T->SetBranchAddress("fEvtHdr.fEvtTime",&evtTime);

   std::vector<double> dt_pos[4];
   TH1D *h[4];
   for(int i=0;i<4;++i)
      h[i] = new TH1D(Form("h%i",i),Form("Quartet Window %i Length",i),200, 33000,3340);

   //Cycle over all patterns getting time ellapsed between first and last events
   /////////////////////////////////////////////////////////////////////////////
   for( auto &pat : patterns ) {
      for( int i=0; i<4; ++i ){
	 if(pat.window_start.size() < 4)continue;
	 Long64_t begin = pat.window_start[i];
	 Long64_t end   = pat.window_end[i];

	 T->GetEntry(begin);
	 ULong64_t tBegin = evtTime;

	 T->GetEntry(end);
	 ULong64_t tEnd = evtTime;

	 double dt = double(tEnd - tBegin)*4e-3;
	 cout<<dt<<" "<<tBegin<<" "<<tEnd<<endl;
	 h[i]->Fill(dt);
      }
   }
   TCanvas *c = new TCanvas("c", "c", 0, 0, 1400, 1000);
   c->Divide(2, 2);
   for( int i = 0; i < 4; ++i ){
      c->cd( i+1 );
      h[i]->Draw();
   }
   
}
