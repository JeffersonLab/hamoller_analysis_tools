#include "Pedestal.h"

#include <TFile.h>
#include <TH1D.h>
#include <TSystem.h>
#include <iostream>
#include <cmath>

ClassImp(Pedestal)

Pedestal::Pedestal() {}

Pedestal::~Pedestal() {}

void Pedestal::Process(TChain* ch, int runnum)
{
    if(!ch){
        std::cout<<"No chain provided\n";
        return;
    }

    std::cout<<"[Pedestal] Computing statistics...\n";
    ComputeStats(ch);

    std::cout<<"[Pedestal] Building friend tree...\n";
    BuildFriendTree(ch, runnum);
}

///////////////////////////////////////////////////////////

void Pedestal::ComputeStats(TChain* ch)
{
   Double_t ped[32], pedchan[32];
    Int_t nped;

    ch->SetBranchStatus("*",0);
    ch->SetBranchStatus("M.cal.ped",1);
    ch->SetBranchStatus("M.cal.chan",1);
    ch->SetBranchStatus("Ndata.M.cal.chan",1);

    ch->SetBranchAddress("M.cal.ped", ped);
    ch->SetBranchAddress("M.cal.chan", pedchan);
    ch->SetBranchAddress("Ndata.M.cal.chan", &nped);

    TH1D* hPed[fNDET];
    for(int i=0;i<fNDET;i++){
        hPed[i] = new TH1D(Form("hPed_%d",i),"",200,0,10000);
    }

    Long64_t nentries = ch->GetEntries();

    for(Long64_t i=0;i<nentries;i++){
        ch->GetEntry(i);

        for(int k=0;k<nped;k++){
            int c = pedchan[k];
            if(c>=fNDET) continue;
            hPed[c]->Fill(ped[k]);
        }
    }

    for(int i=0;i<fNDET;i++){
        fPedMean[i]  = hPed[i]->GetMean();
        fPedSigma[i] = hPed[i]->GetStdDev();

        std::cout<<"Ch "<<i<<" mean="<<fPedMean[i]
                 <<" sigma="<<fPedSigma[i]<<"\n";
    }
}

///////////////////////////////////////////////////////////

void Pedestal::BuildFriendTree(TChain* ch, int runnum)
{
    Double_t ped[fMAXLEN], pedchan[fMAXLEN];
    Int_t nped;

    Double_t samples[fMAXLEN*100];
    Int_t nsamp;

    ch->SetBranchStatus("*",0);
    ch->SetBranchStatus("M.cal.ped",1);
    ch->SetBranchStatus("M.cal.chan",1);
    ch->SetBranchStatus("Ndata.M.cal.chan",1);

    ch->SetBranchAddress("M.cal.ped", ped);
    ch->SetBranchAddress("M.cal.chan", pedchan);
    ch->SetBranchAddress("Ndata.M.cal.chan", &nped);

    // Separate chain for heavy branch
    TChain ch_wave("T");
    for(int i=0;i<ch->GetListOfFiles()->GetEntries();i++){
        ch_wave.Add(ch->GetListOfFiles()->At(i)->GetTitle());
    }

    ch_wave.SetBranchStatus("*",0);
    ch_wave.SetBranchStatus("M.cal.samples",1);
    ch_wave.SetBranchStatus("Ndata.M.cal.samples",1);

    ch_wave.SetBranchAddress("M.cal.samples", samples);
    ch_wave.SetBranchAddress("Ndata.M.cal.samples", &nsamp);

    // Output
    std::string fname = std::string(gSystem->Getenv("HAMOLLER_ROOTFILE_DIR"))
        + "/pedestals/PedestalFriend_" + std::to_string(runnum) + ".root";

    TFile* fout = TFile::Open(fname.c_str(),"RECREATE");
    TTree* t = new TTree("T","Pedestal corrections");

    Double_t ped_corr[fMAXLEN];
    Int_t ped_corr_chan[fMAXLEN];
    Int_t nped_corr;
    Bool_t ped_changed[fMAXLEN];

    t->Branch("nped_corr",&nped_corr,"nped_corr/I");
    t->Branch("ped_corr", ped_corr, "ped_corr[nped_corr]/D");
    t->Branch("ped_changed", ped_changed, "ped_corr[nped_corr]/O");
    t->Branch("ped_corr_chan", ped_corr_chan, "ped_corr_chan[nped_corr]/I");

    Long64_t nentries = ch->GetEntries();

    for(Long64_t i=0;i<nentries;i++){
        ch->GetEntry(i);
	if(i%1000000==0)std::cout<<"\rProcessing "<<i<<" of "<<nentries<<std::flush;
        nped_corr = 0;
        for(int k=0;k<nped;k++){
            int c = pedchan[k];
            if(c>=fNDET) break;

            double val = ped[k];
            bool bad = fabs(val - fPedMean[c]) > 3*fPedSigma[c];
            double newped = val * fPEDFRAC;

            if(bad){
                ch_wave.GetEntry(i);

                int Nsamp = nsamp / nped;
                int offset = k * Nsamp;

                double sum = 0;
                for(int j=0;j<fNPED;j++){
                    sum += samples[offset + Nsamp - 1 - j];
                }

                newped = sum * fPEDFRAC;
            }
	    ped_changed[nped_corr] = (bad ? true : false);
            ped_corr[nped_corr] = newped;
            ped_corr_chan[nped_corr] = c;
            nped_corr++;
        }
        t->Fill();
    }

    t->Write();
    fout->Close();

    std::cout<<"Saved friend tree: "<<fname<<"\n";
}
