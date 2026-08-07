/////////////////////////////////////////
//Author:   Addison Arcuri	       //
//Employer: Temple University	       //
//Date:     April 2025   	       //
/////////////////////////////////////////

#include <TTree.h>
#include <TH1D.h>
#include <iostream>
#include <TCanvas.h>
#include <TStyle.h>
#include "TTrain.h"
#include <chrono>

void peakFit(TH1D *hppmtn, int binNum, int gausFitWidth){
   if(hppmtn->GetEntries()>0){
      hppmtn->GetXaxis()->SetRange(hppmtn->FindBin(1500),binNum);
      hppmtn->Fit("gaus","R","",hppmtn->GetBinCenter(hppmtn->GetMaximumBin())-gausFitWidth,hppmtn->GetBinCenter(hppmtn->GetMaximumBin())+gausFitWidth);
      hppmtn->GetXaxis()->SetRange(0,0);
   }
}

void adcSpectraV2(int run, int pmtsActive=8,int binNum=3000, int histRange=12000){
   auto start = std::chrono::high_resolution_clock::now();
   
   const int maxChan = 46; //4x8 PMTs, 4xL/R Sum, 1x6 Helicities/Triggers
   const double sampleRatio = 13.0/4.0; //Fraction to multipy pedestals by
   const int gausFitWidth = 200; //How far left and right of second peak to fit
   const vector<int> pmtOrder = {0,4,1,5,2,6,3,7}; //When displaying 2 PMTs, display 0,4; etc.
   
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
   }

   Long64_t nTreeEntries = tree->GetEntries();
   std::cout << nTreeEntries << " total entries." << std::endl;
   if (nTreeEntries == 0) {
      std::cout << "No entries found. Exiting." << std::endl;
      return;
   }
   
   /*const char* fnameform = "fadcV2_moller_analyzer_";
   
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
   }// End TTrain

   Long64_t nTreeEntries = tree->GetEntries();
   cout<<"nTreeEntries: "<<nTreeEntries<<endl;
   if (nTreeEntries == 0) {
      cout << "No entries found. Exiting." << endl;
      return;
   }*/

   tree->SetBranchStatus("*", 0);
   
   double adc[maxChan];
   tree->SetBranchStatus("M.cal.adc", 1);
   tree->SetBranchAddress("M.cal.adc", adc);
   
   double adc_chan[maxChan];
   tree->SetBranchStatus("M.cal.adc_chan", 1);
   tree->SetBranchAddress("M.cal.adc_chan", adc_chan);
   int nadc_chan = 0;
   tree->SetBranchStatus("Ndata.M.cal.adc_chan", 1);
   tree->SetBranchAddress("Ndata.M.cal.adc_chan",&nadc_chan);
   
   double ped[16];
   tree->SetBranchStatus("M.cal.ped", 1);
   tree->SetBranchAddress("M.cal.ped",ped);
   
   vector<TH1D*> vhPMTs = {}; //Only create as many hists as needed
   for(int i=0;i<pmtsActive;i++){
      vhPMTs.push_back(new TH1D(Form("PMT_%i",pmtOrder.at(i)),Form("PMT %i",pmtOrder.at(i)),binNum,0,histRange));
   }

   vector<TH1D*> vhPeds = {}; //Only create as many hists as needed
   for(int i=0;i<pmtsActive;i++){
      vhPeds.push_back(new TH1D(Form("Ped_%i",pmtOrder.at(i)),Form("Ped %i",pmtOrder.at(i)),binNum,0,histRange));
   }

   vector<TH1D*> vhPPMTs = {}; //Only create as many hists as needed
   for(int i=0;i<pmtsActive;i++){
      vhPPMTs.push_back(new TH1D(Form("Ped_PMT_%i",pmtOrder.at(i)),Form("Ped PMT %i",pmtOrder.at(i)),binNum,0,histRange));
   }
 
   for(int nEvent=0;nEvent<nTreeEntries;nEvent++){ //Loop over all events
      tree->GetEntry(nEvent);
      int j=0; // j is the CHANNEL index, i is the ADC index
      for(int i=0; i<nadc_chan; i++){ //Loop over adc_chans
	 if(i>0){
	    if((int)adc_chan[i]!=(int)adc_chan[i-1]){j++;}
	 }	       
	 switch ((int)adc_chan[i]){
	 case 0:
	    vhPMTs.at(0)->Fill(adc[i]);
	    vhPeds.at(0)->Fill(ped[j]*sampleRatio);
	    vhPPMTs.at(0)->Fill(adc[i]-ped[j]*sampleRatio);
	    break;
	 case 4:
	    vhPMTs.at(1)->Fill(adc[i]);
	    vhPeds.at(1)->Fill(ped[j]*sampleRatio);
	    vhPPMTs.at(1)->Fill(adc[i]-ped[j]*sampleRatio);
	    break;
	 case 1:
	    if(pmtsActive>2){
	       vhPMTs.at(2)->Fill(adc[i]);
	       vhPeds.at(2)->Fill(ped[j]*sampleRatio);
	       vhPPMTs.at(2)->Fill(adc[i]-ped[j]*sampleRatio);
	    }
	    break;
	 case 5:
	    if(pmtsActive>2){
	       vhPMTs.at(3)->Fill(adc[i]);
	       vhPeds.at(3)->Fill(ped[j]*sampleRatio);
	       vhPPMTs.at(3)->Fill(adc[i]-ped[j]*sampleRatio);
	    }
	    break;
	 case 2:
	    if(pmtsActive>4){
	       vhPMTs.at(4)->Fill(adc[i]);
	       vhPeds.at(4)->Fill(ped[j]*sampleRatio);
	       vhPPMTs.at(4)->Fill(adc[i]-ped[j]*sampleRatio);
	    }
	    break;
	 case 6:
	    if(pmtsActive>4){
	       vhPMTs.at(5)->Fill(adc[i]);
	       vhPeds.at(5)->Fill(ped[j]*sampleRatio);
	       vhPPMTs.at(5)->Fill(adc[i]-ped[j]*sampleRatio);
	    }
	    break;
	 case 3:
	    if(pmtsActive>6){
	       vhPMTs.at(6)->Fill(adc[i]);
	       vhPeds.at(6)->Fill(ped[j]*sampleRatio);
	       vhPPMTs.at(6)->Fill(adc[i]-ped[j]*sampleRatio);
	    }
	    break;
	 case 7:
	    if(pmtsActive>6){
	       vhPMTs.at(7)->Fill(adc[i]);
	       vhPeds.at(7)->Fill(ped[j]*sampleRatio);
	       vhPPMTs.at(7)->Fill(adc[i]-ped[j]*sampleRatio);
	    }
	    break;
	 } //End of switch      
      } //End of loop over adc_chans
   } // End of loop over events
   
   //Global stats box settings
   gStyle->SetPadLeftMargin(0.125);
   gStyle->SetOptFit(1011); //Display prob, fit parameters, errors
   gStyle->SetOptStat("en");//Entry number, name
   gStyle->SetStatX(0.875); //Unit is fraction of divided canvas
   gStyle->SetStatW(0.240); //Unit is fraction of Canvas
   gStyle->SetStatY(0.875); //Unit is fraction of divided canvas
   gStyle->SetStatH(0.200); //Unit is fraction of Canvas

   //Pedestal & PMT Graphs
   TCanvas *cpmts = new TCanvas(Form("%d_PMTs_Raw",run),Form("FADC Run %d Raw PMT Readouts",run),900,280+840/8*pmtsActive);
   cpmts->Divide(2,pmtsActive/2,0.0005,0.0005);
   for(int i=0;i<pmtsActive;i++){
      cpmts->cd(i+1);
      vhPMTs.at(i)->SetTitle(Form("Run %i Raw PMT %i Readouts",run,pmtOrder.at(i)));
      vhPMTs.at(i)->Draw("");
   }

   TCanvas *cpeds = new TCanvas(Form("%d_Peds",run),Form("FADC Run %d Pedestal Readouts",run),900,280+840/8*pmtsActive);
   cpeds->Divide(2,pmtsActive/2,0.0005,0.0005);
   for(int i=0;i<pmtsActive;i++){
      cpeds->cd(i+1);
      vhPeds.at(i)->SetTitle(Form("Run %i PMT %i Pedestals",run,pmtOrder.at(i)));
      vhPeds.at(i)->Draw("");
   }

   TCanvas *cppmts = new TCanvas(Form("%d_PMTs_Ped",run),Form("FADC Run %d Ped. PMT Readouts",run),900,280+840/8*pmtsActive);
   cppmts->Divide(2,pmtsActive/2,0.0005,0.0005);
   for(int i=0;i<pmtsActive;i++){
      cppmts->cd(i+1);
      vhPPMTs.at(i)->SetTitle(Form("Run %i Ped PMT %i Readouts",run,pmtOrder.at(i)));
      peakFit(vhPPMTs.at(i),binNum,gausFitWidth);
      vhPPMTs.at(i)->Draw("");
   }

   auto end = std::chrono::high_resolution_clock::now();
   auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start);
   std::cout<<"Time: "<<duration.count()<<" microseconds"<<std::endl;
}
