/////////////////////////////////////////
//Author:   Addison Arcuri	       //
//Employer: Temple University	       //
//Date:     April 2025   	       //
/////////////////////////////////////////
/////////////////////////////////////////
#include <TH1D.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <TCanvas.h>
#include <stdbool.h>
#include <TStyle.h>
#include <TPaveStats.h>
#include "TTrain.h"
#include <chrono>

void triggerTimes(int run, int pmtsActive=8, bool saveFlag=false, int binNum=480, int histRange=240){ //Default 0.5 ns bins
   auto start = std::chrono::high_resolution_clock::now();
   
   const int maxChan = 46; //I don't see this changing. If efficiency is lower consider some PMTs can't fire twice (helicities, perhaps triggers)
   const vector<int> pmtOrder = {0,4,1,5,2,6,3,7}; //When displaying 2 PMTs, display 0,4; etc.
   const int width = 20;

   std::ofstream myfile;
   if(saveFlag){
      myfile.open(Form("%iTriggerTimes.txt",run)); //Create output file
   }
   
   // -----------------------------
   // TTrain lifted from Don's code
   // -----------------------------
   const char* fnamebase = "fadcV2_moller_analyzer_";
   TTrain* tree = new TTrain();
   for (int seg = 0; seg < 100; ++seg) {
      for (int x = 0; x < 100; ++x) {
	 TString filename(Form("%s/%s%i.%i",
			       gSystem->Getenv("HAMOLLER_ROOTFILE_DIR"),
			       fnamebase, run, seg));
	 if (x > 0) filename += Form("_%i", x);
	 filename += ".root";

	 if (gSystem->AccessPathName(filename)) break;
	 std::cout << filename << std::endl;

	 tree->AddFile(filename, "T");
      }
   }//End TTrain

   Long64_t nTreeEntries = tree->GetEntries();
   cout<<"nTreeEntries: "<<nTreeEntries<<endl;
   if (nTreeEntries == 0) {
      cout << "No entries found. Exiting." << endl;
      return;
   }  

   tree->SetBranchStatus("*", 0);
   double adc_chan[maxChan];
   tree->SetBranchStatus("M.cal.adc_chan", 1);
   tree->SetBranchAddress("M.cal.adc_chan", adc_chan);
   int nadc_chan = 0;
   tree->SetBranchStatus("Ndata.M.cal.adc_chan", 1);
   tree->SetBranchAddress("Ndata.M.cal.adc_chan",&nadc_chan);
   double adc_t[maxChan];
   tree->SetBranchStatus("M.cal.adc_t", 1);
   tree->SetBranchAddress("M.cal.adc_t", adc_t);

   vector<TH1D*> vhPMTts = {};
   for(int i=0;i<8;i++){
      vhPMTts.push_back(new TH1D(Form("Time_PMT_%i",i),Form("PMT %i ADC Times",i),binNum,0,histRange));
   }
   
   for(int nEvent = 0;nEvent<=nTreeEntries;nEvent++){ //Loop over all events
      tree->GetEntry(nEvent);   
      for(int i=0; i<nadc_chan; i++){ //Loop over adc_chans	       
	 switch ((int)adc_chan[i]){
	 case 0:
	 case 4:
	    vhPMTts.at(adc_chan[i])->Fill(adc_t[i]);
	    break;
	 case 1:
	 case 5:
	    if(pmtsActive>2){vhPMTts.at(adc_chan[i])->Fill(adc_t[i]); }
	    break;
	 case 2:
	 case 6:
	    if(pmtsActive>4){vhPMTts.at(adc_chan[i])->Fill(adc_t[i]); }
	    break;
	 case 3:
	 case 7:
	    if(pmtsActive>6){vhPMTts.at(adc_chan[i])->Fill(adc_t[i]); }
	    break;
	 } //End of switch
      } //End of loop over adc_chans
   } // End of loop over events
   
   gStyle->SetPadLeftMargin(0.125);
   gStyle->SetOptStat("enm");//Entry number, name, mean   
   TCanvas *cpmtts = new TCanvas(Form("%d_PMTs_Times",run),Form("FADC Run %d ADC Times by PMT",run),900,280+840/8*pmtsActive);
   cpmtts->Divide(2,pmtsActive/2,0.0005,0.0005);

   for(int i=0;i<pmtsActive;i++){
      double peakMean = 0;
      int nPMT = pmtOrder.at(i);
      
      cpmtts->cd(i+1);
      vhPMTts.at(nPMT)->SetTitle(Form("Run %i PMT %i ADC Times",run,nPMT));
      
      if(vhPMTts.at(nPMT)->GetEntries()>0){
	 vhPMTts.at(nPMT)->GetXaxis()->SetRange(vhPMTts.at(nPMT)->GetMaximumBin()-width,vhPMTts.at(nPMT)->GetMaximumBin()+width);
	 peakMean = vhPMTts.at(nPMT)->GetMean();
	 cout<<"PMT "<<nPMT<<" Average Trigger Time: "<<peakMean<<" ns"<<endl;
	 if(saveFlag){myfile<<"PMT "<<nPMT<<": "<<peakMean<<" ns"<<endl; }
	 vhPMTts.at(nPMT)->GetXaxis()->SetRange(0,0);
      }
      
      vhPMTts.at(nPMT)->Draw("");
      gPad->Update();
      TPaveStats *st = (TPaveStats*)vhPMTts.at(nPMT)->FindObject("stats"); //Stats box position
      st->SetX1NDC(0.50);
      st->SetY1NDC(0.65);
      st->SetX2NDC(0.85);
      st->SetY2NDC(0.85);
      TPaveText *pt = new TPaveText(0.50,0.575,0.85,0.65,"ndc");
      pt->AddText(Form("Peak Mean %.2f",peakMean));
      pt->SetTextFont(42);
      pt->SetTextSize(0.05);
      pt->SetFillColor(0);
      pt->SetBorderSize(1);
      pt->Draw();
      gPad->Update();
   }
   if(saveFlag){myfile.close(); }

   auto end = std::chrono::high_resolution_clock::now();
   auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
   std::cout<<"Time: "<<duration.count()<<" milliseconds"<<std::endl;
}
