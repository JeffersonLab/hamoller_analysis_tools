// File: pattern_asymmetry.C
//
// Usage in ROOT:
// root -l pattern_asymmetry.C
//
// or
// root -l
// .x pattern_asymmetry.C("input.root","treeName",100.0)

#include <TFile.h>
#include <TString.h>
#include <TStyle.h>
#include <TTree.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TCanvas.h>
#include <iostream>
#include <algorithm>

void pattern_asymmetry(int run,
                       const char* treename = "T",
                       double THRESHOLD = 100.0)
{
   gStyle->SetOptStat("rMe");
   gStyle->SetStatW(0.5);
   gStyle->SetStatX(0.99);
   gStyle->SetPadRightMargin(0.05);
   TString filename = Form("~/data/Rootfiles/LeftRightSumTree_%i.root", run);
    // Open file
    TFile* f = TFile::Open(filename, "READ");
    if (!f || f->IsZombie()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    // Get tree
    TTree* tree = (TTree*)f->Get(treename);
    if (!tree) {
        std::cerr << "Error: could not find tree " << treename << std::endl;
        return;
    }

    // Variables for branches
    Int_t hel;
    Int_t patpol, winnum=0;
    Double_t leftsum, leftsum_t;
    Double_t rightsum, rightsum_t;
    Bool_t coinc;
    Int_t patnum=0;
    vector<double>start_t;
    // Set branch addresses
    tree->SetBranchStatus("*",0);
    tree->SetBranchStatus("patnum",  1);  
    tree->SetBranchAddress("patnum",   &patnum);
    tree->SetBranchStatus("leftsum_t",  1); 
    tree->SetBranchStatus("rightsum_t",  1);
    tree->SetBranchAddress("leftsum_t",  &leftsum_t);
    tree->SetBranchAddress("rightsum_t", &rightsum_t);
    int prevpn=-1;
    tree->GetEntry(tree->GetEntries()-1);
    for(int i=0;i<patnum+1;++i)start_t.push_back(0);
    for(int i=0;i<tree->GetEntries();++i){
       tree->GetEntry(i);
       if(patnum!=prevpn){
	  start_t[patnum]=(std::min(abs(leftsum_t),abs(rightsum_t)));
       }
       prevpn = patnum;
    }
    std::cout<<"patnum: "<<patnum<<"  "<<start_t.size()<<std::endl;
    tree->SetBranchStatus("hel", 1);
    tree->SetBranchStatus("patpol",  1);  
    tree->SetBranchStatus("windownum",  1);  
    tree->SetBranchStatus("coinc",  1);   
    tree->SetBranchStatus("leftsum",  1); 
    tree->SetBranchStatus("rightsum",  1);
    tree->SetBranchAddress("leftsum",  &leftsum);
    tree->SetBranchAddress("rightsum", &rightsum);
    tree->SetBranchAddress("hel",      &hel);
    tree->SetBranchAddress("windownum",   &winnum);
    tree->SetBranchAddress("patpol",   &patpol);
    tree->SetBranchAddress("coinc",    &coinc);

    // Histograms
    TH1D* hAsymAll  = new TH1D("hAsymAll",
                               "Pattern Asymmetry;A;Counts",
                               200, -0.2, 0.2);

    TH1D* hAsymPol1 = new TH1D("hAsymPol1",
                               "Pattern Asymmetry (patpol==1);A;Counts",
                               200, -0.2, 0.2);

    TH1D* hAsymPol0 = new TH1D("hAsymPol0",
                               "Pattern Asymmetry (patpol==-1);A;Counts",
                               200, -0.2, 0.2);

    TH2D *hT = new TH2D("hT","Event Energy vs Time Inside Pattern",1000,0,0.14,1000,3000,9000);
    Long64_t nEntries = tree->GetEntries();

    // Initialize pattern tracking
    Int_t currentPatnum = -999999;
    Int_t currentPatpol = 0;

    int nHel1 = 0;
    int nHel0 = 0;
    double n[4] = {0,0,0,0};

    for (Long64_t i = 0; i < nEntries; i++) {

        tree->GetEntry(i);
	double t = min(abs(leftsum_t),abs(rightsum_t))-start_t[patnum];
	double energy = (leftsum>0 ? leftsum : 0) + (rightsum>0 ? rightsum : 0);
	double dt = abs(leftsum_t - rightsum_t);
	if(dt > 5e-9)continue;
	n[winnum]++;
	hT->Fill(t,energy);
	if(i%100000==0)
	   std::cout<<i<<" "<<energy<<" "<<t<<std::endl;
        // Initialize first pattern
        if (i == 0) {
            currentPatnum = patnum;
            currentPatpol = patpol;
        }

        // Check for pattern change
        if (patnum != currentPatnum) {

            // Compute asymmetry for completed pattern
            int total = nHel1 + nHel0;

            if (total > 0) {
                double asym =
                    double(nHel1 - nHel0) / double(total);

                hAsymAll->Fill(asym);

                if (currentPatpol == 1)
                    hAsymPol1->Fill(asym);

                if (currentPatpol == -1)
                    hAsymPol0->Fill(asym);
            }

            // Reset counters for new pattern
            nHel1 = 0;
            nHel0 = 0;

            currentPatnum = patnum;
            currentPatpol = patpol;
        }

        // Event selection
        if (coinc &&
            leftsum  > THRESHOLD &&
            rightsum > THRESHOLD)
        {
            if (hel == 1)
                nHel1++;

            if (hel == 0)
                nHel0++;
        }
    }

    // Process final pattern
    int total = nHel1 + nHel0;

    if (total > 0) {
        double asym =
            double(nHel1 - nHel0) / double(total);


        if (currentPatpol == 1){
            hAsymPol1->Fill(asym);
	    hAsymAll->Fill(asym);
	}
        if (currentPatpol == 0){
            hAsymPol0->Fill(asym);
	    hAsymAll->Fill(asym);
	}
    }

    // Draw histograms
    TCanvas* c1 = new TCanvas("c1", "Pattern Asymmetries", 1200, 400);
    c1->Divide(3,1);

    c1->cd(1);
    hAsymAll->Draw();

    c1->cd(2);
    hAsymPol1->SetLineColor(kBlue);
    hAsymPol1->Draw();

    c1->cd(3);
    hAsymPol0->SetLineColor(kRed);
    hAsymPol0->Draw();

    c1->Update();
    TCanvas* c2 = new TCanvas("c2", "c2", 1000, 600);
    hT->Draw("colz");
    // // Optional output file
    // TFile* fout = new TFile("pattern_asymmetry_output.root", "RECREATE");
    // hAsymAll->Write();
    // hAsymPol1->Write();
    // hAsymPol0->Write();
    // fout->Close();
    double n23=n[1]+n[2], n14 = n[0]+n[3];
    double err = 2*sqrt(n23*n14)/pow(n23+n14,1.5);
    std::cout<<n[0]<<" "<<n[1]<<" "<<n[2]<<" "<<n[3]<<"  Asym: "<<(n[0]+n[3]-n[1]-n[2])/(n[0]+n[3]+n[1]+n[2])<<"+/-"<<err<<std::endl;
    std::cout << "Done." << std::endl;
}
