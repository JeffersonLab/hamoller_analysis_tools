#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TGraph.h>
#include <TBranch.h>
#include <iostream>
#include <vector>
void test(int run){
   TString dir("/adaqfs/home/hamoller/data/Rootfiles");
   TFile *file = TFile::Open(Form("%s/fadcV2_moller_analyzer_%i.root", dir.Data(),run));
   ULong64_t evt_time;
   Int_t hel;
   UInt_t tbits;
   TTree *tree = (TTree*)file->Get("T");
   tree->SetBranchAddress("fEvtHdr.fEvtTime",&evt_time);
   tree->SetBranchAddress("fEvtHdr.fHelicity",&hel);
   tree->SetBranchAddress("fEvtHdr.fTrigBits",&tbits);

   for(int i=0; i<tree->GetEntries();++i){
      tree->GetEntry(i);
      if(i>100)break;
      cout<<evt_time<<" "<<hel<<" "<<tbits<<endl;
   }

   return;
}
