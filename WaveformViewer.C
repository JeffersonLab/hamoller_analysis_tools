#include <TChain.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TGraph.h>
#include <TSystem.h>
#include <TPaveText.h>
#include <iostream>
#include <map>
#include <vector>
#include <fstream>

class WaveformViewer {
public:
   TCanvas* c;
   TChain* chain;
   Long64_t gEntry = 0, nEntries = 0, gIdx = 0;
   Int_t run_number = 0;
   // Branch variables
   Double_t samples[10000], chan[16];   // large enough buffer
   Int_t nsamp_total, nchan;
   vector<int>entries;
   
   WaveformViewer(int runnum, string fname="") {
      run_number = runnum;
      chain = new TChain("T"); // or your tree name
      c = new TCanvas("c", "Waveforms", 850, 1100);
      c->Divide(2, 4);

      // Example: adjust path pattern to your setup
      std::string base(gSystem->Getenv("HAMOLLER_ROOTFILE_DIR"));
      std::string filename = base + Form("/fadcV2_moller_analyzer_%d_*.root", runnum);
      for(int seg=0;seg<100;++seg){
	 for(int i=0;i<100;++i){
	    std::string filename = base;
	    if(i==0)
	       filename += Form("/fadcV2_moller_analyzer_%d.%d.root", runnum, seg);
	    else
	       filename += Form("/fadcV2_moller_analyzer_%d.%d_%d.root", runnum,seg, i);
	    if (gSystem->AccessPathName(filename.c_str())) break;
	    chain->Add(filename.c_str());
	 }
      }
      // Set branches
      chain->SetBranchStatus("*",0);
      chain->SetBranchStatus("M.cal.samples", 1);
      chain->SetBranchStatus("Ndata.M.cal.samples", 1);
      chain->SetBranchStatus("M.cal.chan", 1);
      chain->SetBranchStatus("Ndata.M.cal.chan", 1);
      chain->SetBranchAddress("M.cal.samples", samples);
      chain->SetBranchAddress("M.cal.chan", chan);
      chain->SetBranchAddress("Ndata.M.cal.chan", &nchan);
      chain->SetBranchAddress("Ndata.M.cal.samples", &nsamp_total);
      ifstream file(fname);
      int x;
      if (!file.is_open()) 
	 cerr << "No file of entries.\n";
      else
	 while (file >> x) {entries.push_back(x);cout<<x<<endl;}
      
   }
	     

   void PlotEntry(Long64_t entry) {
      if(entry < 0) entry = 0;
      if(entry >= chain->GetEntries())entry = chain->GetEntries() - 1;
      gStyle->SetLabelSize(0.05, "X");  // X-axis labels
      gStyle->SetLabelSize(0.05, "Y");  // Y-axis labels
      gEntry= entry;
      chain->GetEntry(entry);
      c->SetTitle(Form("Run %i, Entry %lli",run_number, entry));
      //clear content from canvas if it exists
      for(int i=1;i<=8;++i){c->cd(i);gPad->Clear();}
      
      // --- Infer number of samples per channel ---
      // Assume equal length blocks
      int npmtchan = 0;
      while (chan[npmtchan] < 8) npmtchan++; // find number of PMT channels that fired

      if (npmtchan == 0) {
	 std::cout << "No PMT channels in this event\n";
      }
      int trig[3] = {0,0,0}, n = npmtchan-1;
      while (++n < nchan){ // find trigger channels that fired
	 if(chan[n]==13)trig[0]=1;//coinc
	 if(chan[n]==14)trig[1]=1;//left
	 if(chan[n]==15)trig[2]=1;//right
      }
      int Nsamp = nsamp_total/nchan;
      std::cout<<"trig: {"<<trig[0]<<","<<trig[1]<<","<<trig[2]<<"}\n";
      std::cout<<"Run "<<run_number<<". Event "<<entry<<" with "<<Nsamp<<" samples per waveform."<<std::endl;
      // --- Build map: channel -> waveform ---
      std::map<int, std::vector<double>> waveforms;

      for (int i = 0; i < npmtchan; i++) {
	 int ch = chan[i];
	 if (ch < 0 || ch > 7) break;

	 std::vector<double> wf(Nsamp);
	 for (int j = 0; j < Nsamp; j++) {
	    wf[j] = samples[i * Nsamp + j];
	 }
	 waveforms[ch] = wf;
      }
      for (int i = 0; i < nchan; i++) {
	 int ch = chan[i];
	 if (ch < 13) continue;

	 std::vector<double> wf(Nsamp);
	 for (int j = 0; j < Nsamp; j++) {
	    wf[j] = samples[i * Nsamp + j];
	 }
	 waveforms[ch] = wf;
      }

      // --- helper: "No data" ---
      auto drawNoData = [](int ch) {
	 TPaveText* txt = new TPaveText(0.3, 0.45, 0.7, 0.55, "NDC");
	 txt->SetFillStyle(0);   // transparent
	 txt->SetBorderSize(0);  // no border
	 txt->SetShadowColor(0); // no shadow
	 txt->AddText("");
	 txt->Draw();

	 // PMT label
	 TPaveText* label = new TPaveText(0.8, 0.8, 0.95, 0.95, "NDC");
	 label->SetFillStyle(0);
	 label->SetBorderSize(0);
	 label->SetShadowColor(0);
	 label->AddText(Form("PMT %d", ch));
	 label->Draw();
      };

      // --- loop over pads ---
      for (int row = 0; row < 4; row++) {

	 // LEFT: channels 0–3
	 int chL = row;
	 c->cd(2*row + 1);
	 gPad->SetMargin(0.08, 0.01, 0.08, 0.01); // small margins

	 if (waveforms.count(chL)) {
            auto& wf = waveforms[chL];
            TGraph* g = new TGraph(wf.size());

            for (size_t i = 0; i < wf.size(); i++)
	       g->SetPoint(i, i, wf[i]);

            g->SetTitle(""); // no title
            g->Draw("AL");

            // PMT label
            TPaveText* label = new TPaveText(0.8, 0.8, 0.95, 0.95, "NDC");
            label->SetFillStyle(0);
            label->SetBorderSize(0);
            label->SetShadowColor(0);
            label->AddText(Form("PMT %d", chL));
            label->Draw();

	 } else {
            drawNoData(chL);
	 }

	 // RIGHT: channels 4–7
	 int chR = row + 4;
	 c->cd(2*row + 2);
	 gPad->SetMargin(0.08, 0.01, 0.08, 0.01);

	 if (waveforms.count(chR)) {
            auto& wf = waveforms[chR];
            TGraph* g = new TGraph(wf.size());

            for (size_t i = 0; i < wf.size(); i++)
	       g->SetPoint(i, i, wf[i]);

            g->SetTitle("");
            g->Draw("ALP");

            TPaveText* label = new TPaveText(0.8, 0.8, 0.95, 0.95, "NDC");
            label->SetFillStyle(0);
            label->SetBorderSize(0);
            label->SetShadowColor(0);
            label->AddText(Form("PMT %d", chR));
            label->Draw();

	 } else {
            drawNoData(chR);
	 }
      }
      c->cd(7);//left trigger
      if(trig[1]>0){
            auto& wf = waveforms[14];
            TGraph* g = new TGraph(wf.size());

            for (size_t i = 0; i < wf.size(); i++)
	       g->SetPoint(i, i, wf[i]);

            g->SetTitle("");
	    g->SetLineWidth(2);
            g->Draw("ALP");

            TPaveText* label = new TPaveText(0.1, 0.85, 0.25, 0.95, "NDC");
            label->SetFillStyle(0);
            label->SetBorderSize(0);
            label->SetShadowColor(0);
            label->AddText(Form("Left Trig"));
            label->Draw();
      }
      c->cd(8);
      if(trig[2]>0){
            auto& wf = waveforms[15];
            TGraph* g = new TGraph(wf.size());

            for (size_t i = 0; i < wf.size(); i++)
	       g->SetPoint(i, i, wf[i]);

            g->SetTitle("");
	    g->SetLineWidth(2);
            g->Draw("ALP");

            TPaveText* label = new TPaveText(0.1, 0.85, 0.27, 0.95, "NDC");
            label->SetFillStyle(0);
            label->SetBorderSize(0);
            label->SetShadowColor(0);
            label->AddText(Form("Right Trig"));
            label->Draw();
      }
      if(trig[0]>0){
            auto& wf = waveforms[13];
            TGraph* g = new TGraph(wf.size());

            for (size_t i = 0; i < wf.size(); i++)
	       g->SetPoint(i, i, wf[i]);

            g->SetTitle("");
	    g->SetLineColor(kRed);
	    g->SetLineWidth(2);
	    if(trig[2]>0)
	       g->Draw("SAMELP");
	    else
	       g->Draw("ALP");

            TPaveText* label = new TPaveText(0.1, 0.75, 0.27, 0.85, "NDC");
            label->SetFillStyle(0);
            label->SetBorderSize(0);
            label->SetShadowColor(0);
	    label->SetTextColor(kRed);
            label->AddText(Form("Coinc Trig"));
            label->Draw();
	    c->cd(7);
	    if(trig[1]>0){
	       g->Draw("SAMELP");
	       label->Draw();
	    }else
	       g->Draw("ALP");
      }
      c->Update();
   }

   void Back(bool by_index = false){
      int ent;
      if(by_index && gIdx > 0)
	 ent = entries[--gIdx];
      PlotEntry(ent);
   }

   void Next(bool by_index = false){
      int ent = gEntry + 1;
      if(by_index && gIdx < (int)entries.size()-1)
	 ent = entries[++gIdx];
      PlotEntry(ent);
   }
};
