/////////////////////////////////////////
//Author:   Donald Jones	       //
//Employer: Jefferson Lab	       //
//Date:     November 2025	       //
/////////////////////////////////////////
/////////////////////////////////////////

#include <TFile.h>
#include <TTree.h>
#include <TTrain.h>
#include <TROOT.h>
#include <TPad.h>
#include "HelPat_t.h"
#include <TProfile.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TSystem.h>
#include <TLegend.h>
#include <TGraph.h>
#include <TBranch.h>
#include <TString.h>
#include <TLeaf.h>
#include <iostream>
#include <vector>
#include <utility>
#include <cassert>
#include <map>
#include <GoodHelicities.h>
#include <TCanvas.h>
#include <TMultiGraph.h>
#include <THaEvent.h>
#include <TKey.h>
#include <sstream>

const Int_t MAXFILES = 100;//maximum number of file segments for a run
const Int_t nDET = 8;//number of PMTs in calorimeter
const Double_t NSA = 40, NSB = 12, NPED = 4;//values from config file
const Double_t PEDFRAC = (NSA+NSB)/(4*NPED);//multiply pedestal by this for pulse integral pedestal
const Double_t COINC = 22;//maximum separation in nanoseconds to be considered a coincidence
const Double_t TIME_OFFSET = -3.5;//average L-R difference in nanoseconds between coincidences on left and right
const Double_t THRESH = 250;//minimum left or right sum (ADC integral units) to be used in a Moller coincidence

////////////////////////////////////////////////////////////////////////////////////////////////////
//Usage: getAsym(run, pat, delay, delay, flip_freq)                                               //
//         run- incrementing positive integer assigned to each run by CODA			  //
//         pat- pattern types. Valid values are 						  //
//              0 pair +- or opposite polarity (1 transition)					  //
//              1 quartet +--+ or opposite polarity (2 transitions)				  //
//              2 octet +--+-++- or opposite polarity(5 transitions)				  //
//              3 toggle +-+-+-+-+-+-+-								  //
//              4 hexo-quad +--++--++--+-++--++--++- or opposite polarity (13 transitions)        //
//              5 octo-quad +--++--++--++--+-++--++--++--++- or opposite polarity (17 transitions)//
// accidental-  choose out of coincidence timing to get accidental background                     //
//  delay_win-  number of helicity windows the reported helicity is delayed                       //
//  flip_freq-  helicity flip frequency in Hz                                                     //
////////////////////////////////////////////////////////////////////////////////////////////////////


using namespace std;
int getAsym(int run, int pat = 1, bool accidental = 0, int delay_win=8, double flip_freq = 29.56, const char* fnamebase = "fadcV2_moller_analyzer_") {
   if( pat < 0 || pat > 5){
      cout<<"Usage: getAysm(run number, pattern type)\n"
	 "Invalid pattern type. Select from one of the following: \n"
	 "0 pair +- or opposite polarity (1 transition)\n"
	 "1 quartet +--+ or opposite polarity (2 transitions)\n"
	 "2 octet +--+-++- or opposite polarity(5 transitions)\n"
	 "3 toggle +-+-+-+-+-+-+-\n"
	 "4 hexoquad +--++--++--+-++--++--++- or opposite polarity (13 transitions)\n"
	 "5 octoquad +--++--++--++--+-++--++--++--++- or opposite polarity (17 transitions)"<<endl;
      exit(1);
   }
   const string pattype[6] = {"pair","quartet","octet","toggle","hexoquad","octoquad"};
   const int nCHAN = 16;//max number of pulses per event to read out. Up to 4 per physical channel.
   const int nPULSEMAX = 4;//maximum number of pulses firmware will find per event
   const int nCHANMAX = nCHAN*nPULSEMAX;//max number of pulses per event to read out. Up to 4 per physical channel.
   const int nSAMP = 60;//number of samples per waveform
   const int nSAMPMAX = nCHANMAX*nSAMP;//max number waveform samples
   const int pat_len[6] = {2,4,8,2,24,32};//quartet = 4, octet = 8 etc
   const int patlen = pat_len[pat];
   const double period = pat_len[pat]/flip_freq;//period of pattern in seconds
   const int pat_transitions[6] = {1,2,5,1,13,17};//the number of helicity state changes per pattern
   const int nTransPerPat = pat_transitions[pat];
   const double minDT = (patlen - 1.0)/double(patlen)*period;
   const double maxDT = (patlen + 1.0)/double(patlen)*period;
   const int delay = delay_win/patlen;//delay in number of helicity patterns
   const int nPAGE = 30;//number of PDF pages of waveforms to print

   //Set up a vector of TTrees with all file segments.
   //Check if Root Files exist and if the tree is in them. If so, add them to the vector.
   ////////////////////////////////////////////////////////////////////////////////////// 
   TTrain *tree = new TTrain();
   tree->SetUpTTrain(run, fnamebase, 0);
   Long64_t nEntries = tree->GetEntries();
   std::cout<<nEntries<< " total entries.\n";
   if(nEntries==0){
      cout<<"No entries found. Exiting."<<endl;
      return -1;
   }


   //Get helicity and pattern information.
   ////////////////////////////////////////
   GoodHelicities *gh = new GoodHelicities();
   gh->SetPatternType(pattype[pat]);
   gh->SetTsettleFrequency(flip_freq);
   gh->SetDelayWindows(delay_win);
   gh->FindGoodHelicities(tree);
   const vector<HelPat_t>& patterns = gh->GetPatterns();
   
   
   //Set up the plots we want to see.
   ////////////////////////////////////
   TH1D *hADCSumLeft = new TH1D("hADCSumLeft",Form("Spectrum of ADC-Summed Left Channels Run %i", run),300,-2000,15000);
   hADCSumLeft->SetLineWidth(2);
   TH1D *hADCSumLeftPedc = new TH1D("hADCSumLeftPedc",Form("Pedestal Subtracted Spectrum of ADC-Summed Left Channels Run %i", run),300,-2000,15000);
   hADCSumLeftPedc->SetLineWidth(2);
   TH1D *hADCLeftNIMsum = new TH1D("hADCLeftNIMsum",Form("ADC Spectrum of NIM-Summed Left Channels Run %i", run),300,-2000,15000);
   hADCLeftNIMsum->SetLineColor(kRed);
   hADCLeftNIMsum->SetLineWidth(2);
   TH1D *hADCLeftNIMsumPedc = new TH1D("hADCLeftNIMsumPedc",Form("Pedestal Subtracted ADC Spectrum of NIM-Summed Left Channels Run %i", run),300,-2000,15000);
   hADCLeftNIMsumPedc->SetLineColor(kRed);
   hADCLeftNIMsumPedc->SetLineWidth(2);
   TH1D *hADCSumRight = new TH1D("hADCSumRight",Form("Spectrum of ADC-Summed Right Channels Run %i", run),300,-2000,15000);
   hADCSumRight->SetLineWidth(2);
   TH1D *hADCSumRightPedc = new TH1D("hADCSumRightPedc",Form("Pedestal Subtracted Spectrum of ADC-Summed Right Channels Run %i", run),300,-2000,15000);
   hADCSumRightPedc->SetLineWidth(2);
   TH1D *hADCRightNIMsum = new TH1D("hADCRightNIMsum",Form("ADC Spectrum of NIM-Summed Right Channels Run %i", run),300,-2000,15000);
   hADCRightNIMsum->SetLineColor(kRed);
   hADCRightNIMsum->SetLineWidth(2);
   TH1D *hADCRightNIMsumPedc = new TH1D("hADCRightNIMsumPedc",Form("ADC Spectrum of NIM-Summed Right Channels Run %i", run),300,-2000,15000);
   hADCRightNIMsumPedc->SetLineColor(kRed);
   hADCRightNIMsumPedc->SetLineWidth(2);
   TH1D *hCoinc = new TH1D("hCoinc",Form("Total Detector FADC Sum Spectrum for Coincidence Events Run %i", run),200,-2000,18000);
   hCoinc->SetLineColor(kBlack);
   hCoinc->SetLineWidth(2);
   TH1D *hCoincL = new TH1D("hCoincL",Form("Detector FADC Sum Spectrum for Coincidence Events Run %i", run),300,-2000,15000);
   hCoincL->SetLineColor(kBlue);
   hCoincL->SetLineWidth(2);
   TH1D *hCoincR = new TH1D("hCoincR",Form("Detector FADC Sum Spectrum for Coincidence Events Run %i", run),300,-2000,15000);
   hCoincR->SetLineColor(kRed);
   hCoincR->SetLineWidth(2);
   TH1D *hCoincDt = new TH1D("hCoincDt",Form("Left-Right Detector #DeltaT for Coincidence Events Run %i", run),40,-100,100);
   hCoincDt->SetLineColor(kBlack);
   hCoincDt->SetLineWidth(2);
   TH2D *hCoincLR = new TH2D("hCoincLR",Form("Right vs. Left Detector Energy Run %i", run),1000,THRESH,10000,1000,THRESH,10000);
   TH1D *hADC[nCHAN], *hADCpedc[nCHAN], *hT[nCHAN], *hPed[nCHAN];
   TH1D *hAsym = new TH1D("hAsym",Form("Coincidence Scattering Asymmetry Run %i", run),800,-1,1);
   TH1D *hAsymL = new TH1D("hAsymL",Form("Left Single-Arm Asymmetry Run %i", run),200,-0.5,0.5);
   TH1D *hAsymR = new TH1D("hAsymR",Form("Right Single-Arm Asymmetry Run %i", run),200,-0.5,0.5);
   int color[nCHAN] = {kRed+1,kBlue+1,kGreen+2,kMagenta+1,kCyan+1,kOrange+7,kViolet,kAzure+2,
      kBlack,kBlack,kBlack,kBlack,kBlack,kBlack,kBlack,kBlack};

   for(int i=0;i<nCHAN;++i){
      if(i<nDET){
	 hADC[i] = new TH1D(Form("hADC%i",i),Form("ADC Spectrum of PMTs"),100,0,18000);
	 hADCpedc[i] = new TH1D(Form("hADCpedc%i",i),Form("Pedstal Subtracted ADC Spectrum of PMTs Run %i", run),100,-2000,8000);
	 hT[i] = new TH1D(Form("hT%i",i),Form("Time Spectrum of PMTs Run %i", run),250,-5,245);
	 hPed[i] = new TH1D(Form("hPed%i",i),Form("Pedestal Spectrum of PMTs"),300,0,5000);
      }else if(i==11){
	 hADC[i] = new TH1D(Form("hADC%i",i),Form("ADC Spectrum of Left PMT NIM Sum Run %i", run),100,0,18000);
	 hADCpedc[i] = new TH1D(Form("hADCpedc%i",i),Form("Pedstal Subtracted ADC Spectrum of Left PMT Sum Run %i", run),100,-2000,8000);
	 hT[i] = new TH1D(Form("hT%i",i),Form("Time Spectrum of Left PMT NIM Sum Run %i", run),250,-5,245);
	 hPed[i] = new TH1D(Form("hPed%i",i),Form("Pedestal Spectrum of Left PMT NIM Sum"),300,0,5000);
      }else if(i==12){
	 hADC[i] = new TH1D(Form("hADC%i",i),Form("ADC Spectrum of Right PMT NIM Sum Run %i", run),100,0,18000);
	 hADCpedc[i] = new TH1D(Form("hADCpedc%i",i),Form("Pedstal Subtracted ADC Spectrum of Right PMT Sum Run %i", run),100,-2000,8000);
	 hT[i] = new TH1D(Form("hT%i",i),Form("Time Spectrum of Right PMT NIM Sum Run %i", run),250,-5,245);	 hPed[i] = new TH1D(Form("hPed%i",i),Form("Pedestal Spectrum of Right PMT Sum"),300,0,5000);
      }else{
	 hADC[i] = new TH1D(Form("hADC%i",i),Form("ADC Spectrum of FADC Ch %i Run %i", i, run),100,0,18000);
	 hADCpedc[i] = new TH1D(Form("hADCpedc%i",i),Form("Pedstal Subtracted ADC Spectrum of FADC Ch %i Run %i", i, run),100,-2000,8000);
	 hT[i] = new TH1D(Form("hT%i",i),Form("Time Spectrum of FADC Ch %i Run %i", i, run),250,-5,245);
	 hPed[i] = new TH1D(Form("hPed%i",i),Form("Pedestal Spectrum of FADC Ch %i Run %i", i, run),300,0,5000);
      }
      hADC[i]->SetLineColor(color[i]);
      hADC[i]->SetLineWidth(2);
      hADCpedc[i]->SetLineColor(color[i]);
      hADCpedc[i]->SetLineWidth(2);
      hT[i]->SetLineColor(color[i]);
      hT[i]->SetLineWidth(2);
      hPed[i]->SetLineColor(color[i]);
      hPed[i]->SetLineWidth(2);
   }

   //Now loop over the tree forming asymmetries and ADC and time distributions.
   ////////////////////////////////////////////////////////////////////////////
   double sum = 0;
   //Use vectors below to group in-time pulses into left and right sums and left/right coincidences
   vector<double>ladc, ladcpedc, ladc_t, radc ,radcpedc, radc_t,
                 lsum, lsumpedc, rsum, rsumpedc, lsum_t, rsum_t,
                 vAsym, vEvent;
   vector<int>radc_wfidx, ladc_wfidx;
   vector<vector<int>>rsum_wfidx, lsum_wfidx;
   int count = 0, idx = 0, nHplus = 0, nHminus = 0, nHLplus = 0, nHLminus = 0,nHRplus = 0, nHRminus = 0, np = 0, hel = 0;
   vector<int>seq;
   Long64_t start = 0;
   ULong64_t tEvt = 0;
   int nCh, nADCch;
   double adc[nCHANMAX], adc_t[nCHANMAX], chan[nCHANMAX], ped[nCHANMAX], adcchan[nCHANMAX];
   bool foundqrtOff = 0, prev_hel=-1, prev_qrt=-1;
   double prevT=0;
   tree->SetBranchStatus("*", 0);
   tree->SetBranchStatus("M.cal.chan", 1);
   tree->SetBranchAddress("M.cal.chan", chan);
   tree->SetBranchStatus("M.cal.adc_chan", 1);
   tree->SetBranchAddress("M.cal.adc_chan", adcchan);
   tree->SetBranchStatus("Ndata.M.cal.chan", 1);
   tree->SetBranchAddress("Ndata.M.cal.chan", &nCh);
   tree->SetBranchStatus("Ndata.M.cal.adc_chan", 1);
   tree->SetBranchAddress("Ndata.M.cal.adc_chan", &nADCch);
   tree->SetBranchStatus("fEvtHdr.fEvtTime", 1);
   tree->SetBranchAddress("fEvtHdr.fEvtTime", &tEvt);
   tree->SetBranchStatus("M.cal.adc", 1);
   tree->SetBranchAddress("M.cal.adc", adc);
   tree->SetBranchStatus("M.cal.ped", 1);
   tree->SetBranchAddress("M.cal.ped", ped);
   tree->SetBranchStatus("M.cal.adc_t", 1);
   tree->SetBranchAddress("M.cal.adc_t", adc_t);

   for(auto& patt: patterns){
      nHplus = nHminus = nHLplus = nHLminus = nHRplus = nHRminus = np = 0;
      for (Long64_t i = patt.pattern_start; i <= patt.pattern_end; ++i) {
	 if(!patt.good_pattern)continue;

	 ladc.clear(); radc.clear(); ladc_t.clear(); radc_t.clear();//clear the temp coinc-finding arrays
	 ladcpedc.clear(); radcpedc.clear(); lsumpedc.clear(); rsumpedc.clear();
	 lsum.clear(); rsum.clear(); lsum_t.clear(); rsum_t.clear();
	 ladc_wfidx.clear(); radc_wfidx.clear(); lsum_wfidx.clear(); rsum_wfidx.clear();

	 int nPat = (int)patt.helicity.size();
	 while(np < nPat){//increment through helicity pattern as required
	    if(i > patt.window_end[np])
	       if(np < (int)patt.window_end.size()-1){
		  ++np;
	       }else{
		  cout<<"Something is wrong. Passed end of pattern without finding entry. Exiting"<<endl;
		  break;
	       }
	    else break;
	 }
	 hel = patt.helicity[np];//assign actual helicity (not delayed helicity)
	 tree->GetEntry(i);
	 int idl = 0, idr = 0;//left and right channel indices
	 double sumleftpedc = 0, sumrightpedc = 0, leftsumnim = 0, rightsumnim = 0, leftsumnimped = 0, rightsumnimped = 0;
	 bool trigR = 0, trigL = 0, trigC = 0, qrt = 0, tsettle_active = 0;

	 //Ignore all events inside Tsettle
	 for(int j = 0; j<nCh; ++j){
	    if(chan[j]==8)tsettle_active = true;
	    if(chan[j]>8)break;
	 }
	 if(tsettle_active) continue;

	 //Find pedestal for each channel
	 double pedest[nCHAN] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	 for(int j = 0; j<nCh; ++j){
	    pedest[(int)chan[j]] = ped[j];
	    hPed[(int)chan[j]]->Fill(ped[j]);
	 }
	 
	 //Fill histograms with good events channel by channel.
	 //Don't worry about coincidences yet.
	 ///////////////////////////////////////////////////////
	 for(int j = 0; j<nADCch; ++j){
	    if(nADCch>12&&0){
	       cout<<adcchan[j]<<" "<<adc_t[j]<<" "<<adc[j]-pedest[(int)adcchan[j]]<<" ";
	       if(j==nADCch-1)cout<<i<<endl;
	    }
	 
	    int ch = adcchan[j];
	    switch(ch){
	    case 0:
	    case 1:
	    case 2:
	    case 3:
	       hADC[ch]->Fill(adc[j]);
	       hADCpedc[ch]->Fill(adc[j]-pedest[ch]*PEDFRAC);
	       hT[ch]->Fill(adc_t[j]);
	       ladc.push_back(adc[j]);
	       ladc_wfidx.push_back(j);
	       ladcpedc.push_back(adc[j]-pedest[ch]*PEDFRAC);
	       ladc_t.push_back(adc_t[j]);
	       sumleftpedc += adc[j]-pedest[ch]*PEDFRAC;	
	       break;
	    case 4:
	    case 5:
	    case 6:
	    case 7:
	       hADC[ch]->Fill(adc[j]);
	       hADCpedc[ch]->Fill(adc[j]-pedest[ch]*PEDFRAC);
	       hT[ch]->Fill(adc_t[j]);
	       radc_wfidx.push_back(j);
	       radc.push_back(adc[j]);
	       radcpedc.push_back(adc[j]-pedest[ch]*PEDFRAC);
	       radc_t.push_back(adc_t[j]);
	       sumrightpedc += adc[j]-pedest[ch]*PEDFRAC;	
	       break;
	    case 8:
	       tsettle_active = true;
	       break;
	    case 9:
	       qrt = true;
	       break;
	    case 10:
	       //delayed helicity 
	       break;
	    case 11:
	       leftsumnim += adc[j];
	       leftsumnimped += pedest[ch]*PEDFRAC;
	       break;
	    case 12:
	       rightsumnim += adc[j];
	       rightsumnimped += pedest[ch]*PEDFRAC;
	       break;
	    case 13:
	       trigC = true;
	       break;
	    case 14:
	       trigL = true;
	       break;
	    case 15:
	       trigR = true;
	       break;
	    }
	 }
	 if(tsettle_active){
	    cout<<"Yikes! Tsettle active."<<endl;//catch unexpected behavior
	    exit(1);
	 }
	 if(leftsumnim > THRESH){
	    hADCLeftNIMsum->Fill(leftsumnim);
	    hADCLeftNIMsumPedc->Fill(leftsumnim-leftsumnimped);
	 }
	 if(rightsumnim > THRESH){
	    hADCRightNIMsum->Fill(rightsumnim);
	    hADCRightNIMsumPedc->Fill(rightsumnim-rightsumnimped);
	 }

	 //Aggregate coincident left pulses (PMTs 1-4) into total pulse sum.
	 ///////////////////////////////////////////////////////////////////
	 int n = 0; double sum = 0, sumpedc = 0, t = 0;
	 for(size_t j=0;j<ladc_t.size();++j){
	    //don't use the same pulse again if it has already been included in another sum
	    if(ladc_t[j] == 0)continue;
	    int chj = adcchan[ladc_wfidx[j]];
	    if(chj < 0 || chj > 3)
	       cout<<"Error. Invalid PMTs on the left: "<<adcchan[ladc_wfidx[j]]<<endl;
	    lsum_wfidx.push_back({});//initialize empty row
	    lsum_wfidx.back().push_back(ladc_wfidx[j]);
	    sum = ladc[j]; sumpedc = ladcpedc[j], t = ladc_t[j]; n = 1;
	    for(size_t k=j+1; k<ladc_t.size(); ++k){
	       int chk = adcchan[ladc_wfidx[k]];
	       if(abs(ladc_t[k]-ladc_t[j]) <= COINC){
		  if(chk < 0 || chk > 3)
		     cout<<"Error. Invalid PMTs on the left: "<<chk<<endl;
		  lsum_wfidx.back().push_back(ladc_wfidx[k]);
		  sum += ladc[k];
		  sumpedc += ladcpedc[k];
		  t += ladc_t[k];
		  ladc_t[k] = 0;
		  ++n;
	       }
	    }
	    if(sumpedc > THRESH){
	       hADCSumLeft->Fill(sum);
	       hADCSumLeftPedc->Fill(sumpedc);
	       lsum_t.push_back(t/(double)n);//average time of all pulses included
	       lsum.push_back(sum);
	       lsumpedc.push_back(sumpedc);
	    }else{
	       //The pulse channel information must be stored before knowing if it
	       //passes threshold, so erase the row of the vector if it doesn't.
	       lsum_wfidx.pop_back();   
	    }
	 }
      
	 //Aggregate coincident right pulses (PMTs 5-8) into total pulse sum.
	 ////////////////////////////////////////////////////////////////////
	 n = 0; sum = 0; sumpedc = 0; t = 0;
	 for(size_t j=0;j<radc_t.size();++j){
	    //don't use the same pulse again if it has already been included in another sum
	    if(radc_t[j] == 0)continue;
	    int chj = adcchan[radc_wfidx[j]];
	    if(chj < 4 || chj > 7)
	       cout<<"Error. Invalid PMTs on the right: "<<chj<<endl;
	    rsum_wfidx.push_back({});//initialize empty row
	    rsum_wfidx.back().push_back(radc_wfidx[j]);
	    sum = radc[j]; sumpedc = radcpedc[j]; t = radc_t[j]; n = 1;
	    for(size_t k=j+1;k<radc_t.size();++k){
	       if(abs(radc_t[k]-radc_t[j]) <= COINC){
		  sum += radc[k];
		  rsum_wfidx.back().push_back(radc_wfidx[k]);
		  int chk = adcchan[radc_wfidx[k]];
		  if(chk < 4 || chk > 7)
		     cout<<"Error. Invalid PMTs on the right: "<<chk<<endl;
		  sumpedc += radcpedc[k];
		  t += radc_t[k];
		  radc_t[k] = 0;
		  ++n;
	       }
	    }
	    if(sumpedc > THRESH){
	       hADCSumRight->Fill(sum);
	       hADCSumRightPedc->Fill(sumpedc);
	       rsum_t.push_back(t/(double)n);//average time of all pulses included
	       rsum.push_back(sum);
	       rsumpedc.push_back(sumpedc);
	    }else{
	       //The pulse channel information must be stored before knowing if it
	       //passes threshold, so erase the row of the vector if it doesn't.
	       rsum_wfidx.pop_back();
	    }
	 }
     
	 //Get single arm asymmetry information
	 ///////////////////////////////////////
	 for(size_t j=0; j < lsum_t.size(); ++j){
	    if(lsumpedc[j] > THRESH){
	       if(hel == 1)++nHLplus;
	       else ++nHLminus;
	    }
	 }
	 for(size_t j=0; j < rsum_t.size(); ++j){
	    if(rsumpedc[j] > THRESH){
	       if(hel == 1)++nHRplus;
	       else ++nHRminus;
	    }
	 }
      
	 //Now find coincidences
	 ////////////////////////
	 for(size_t j=0; j < lsum_t.size(); ++j){
	    for(size_t k=0; k < rsum_t.size(); ++k){
	       bool good_timing = accidental ? (abs((lsum_t[j]-rsum_t[k])-TIME_OFFSET) <= 15*COINC) &&
		  (abs((lsum_t[j]-rsum_t[k])-TIME_OFFSET) >= COINC) : (abs((lsum_t[j]-rsum_t[k])-TIME_OFFSET) <= COINC);
	       if(good_timing){
		  if(lsumpedc[j] > THRESH && rsumpedc[k] > THRESH){
		     hCoincL->Fill(lsumpedc[j]);
		     hCoincR->Fill(rsumpedc[k]);
		     hCoinc->Fill(lsumpedc[j]+rsumpedc[k]);
		     double dt = lsum_t[j] - rsum_t[k];
		     hCoincDt->Fill(dt);
		     hCoincLR->Fill(lsumpedc[j], rsumpedc[k]);
		     //if(!trigC)std::cout<<"Entry "<<i<<std::endl;
		     if(hel == 1)nHplus++;
		     if(hel == 0)nHminus++;
		  
		  
		     double samples[nSAMPMAX], samples_ch[nSAMPMAX], nsamples;
		     bool inside_sample_window = accidental ? dt >= 30 && dt <= 35 : good_timing;

		  }
	       }
	    }
	 }
      }

      //Process pattern level averages now
      ////////////////////////////////////      
      //cout<<endl;
      double denom = double(nHplus + nHminus);
      if(denom > 0){
	 double asym  = double(nHplus - nHminus)/denom;
	 hAsym->Fill(asym);
	 vAsym.push_back(asym);
	 vEvent.push_back(patt.pattern_start);
	 cout<<asym<<": "<<nHminus<<" "<<nHplus<<" start="<<patt.pattern_start
	     <<" end="<<patt.pattern_end<<" hel="<<hel;
	 for(int x=1; x<(int)patt.helicity.size(); ++x) cout<<" "<<patt.helicity[x];
	 cout<<endl;
      }
      denom = double(nHLplus + nHLminus);
      if(denom > 0){
	 double asymL  = double(nHLplus - nHLminus)/denom;
	 hAsymL->Fill(asymL);
      }
      denom = double(nHRplus + nHRminus);
      if(denom > 0){
	 double asymR  = (denom > 0) ? double(nHRplus - nHRminus)/denom : 0.0;
	 hAsymR->Fill(asymR);
      }

   }
   
   TLegend *legADC_L = new TLegend(0.8, 0.6, 0.98, 0.774); // Position of the legend
   TLegend *legADC_R = new TLegend(0.8, 0.6, 0.98, 0.774); // Position of the legend
   TLegend *legADC = new TLegend(0.8, 0.6, 0.98, 0.774); // Position of the legend
   legADC->AddEntry(hADCSumLeft,"Sum FADC","l");
   legADC->AddEntry(hADCLeftNIMsum,"NIM Sum","l");
   TLegend *legCoinc = new TLegend(0.8, 0.6, 0.98, 0.774); // Position of the legend
   legCoinc->AddEntry(hCoincL,"Coinc Left","l");
   legCoinc->AddEntry(hCoincR,"Coinc Right","l");

   //Raw ADC spectra
   /////////////////
   int width = 1500;
   int height = 900;//1100
   TCanvas *cADC = new TCanvas("cADC","cADC",0,0,width,height);
   cADC->Divide(2,2);
   cADC->cd(1);
   bool onedrawn = 0;
   for(int i=0; i<4; ++i){
      if(hADC[i]->GetEntries()>0){
	 cout<<"Drawing ADC "<<i<<" with "<<hADC[i]->GetEntries()<<" entries."<<endl;
	 hADC[i]->Draw(onedrawn ? "sames" : "");
	 legADC_L->AddEntry(hADC[i],Form("PMT %i",i),"l");
	 onedrawn = 1;
      }
   }
   legADC_L->Draw();
   gPad->Update();
   cADC->cd(2);
   onedrawn = 0;
   for(int i=4;i<8;++i){
      if(hADC[i]->GetEntries()>0){
	 cout<<"Drawing ADC "<<i<<" with "<<hADC[i]->GetEntries()<<" entries."<<endl;
	 hADC[i]->Draw(onedrawn ? "sames" : "");
	 legADC_R->AddEntry(hADC[i], Form("PMT %i",i), "l");
	 onedrawn = 1;
      }
   }
   legADC_R->Draw();
   gPad->Update();
   cADC->cd(3);
   hADCLeftNIMsum->Draw();
   hADCSumLeft->Draw("sames");
   legADC->Draw();
   gPad->Update();
   cADC->cd(4);
   hADCRightNIMsum->Draw();
   hADCSumRight->Draw("sames");
   legADC->Draw();
   gPad->Update();
   cADC->Update();

   //Pedestal-subtracted spectra
   /////////////////////////////
   TCanvas *cADCpedc = new TCanvas("cADCpedc","cADCpedc",0,0,width,height);
   cADCpedc->Divide(2,2);
   cADCpedc->cd(1);
   onedrawn = 0;
   for(int i=0; i<4; ++i){
      if(hADCpedc[i]->GetEntries()>0){
	 cout<<"Drawing ADC "<<i<<" with "<<hADCpedc[i]->GetEntries()<<" entries."<<endl;
	 hADCpedc[i]->Draw(onedrawn ? "sames" : "");
	 onedrawn = 1;
      }
   }
   legADC_L->Draw();
   gPad->Update();
   cADCpedc->cd(2);
   onedrawn = 0;
   for(int i=4;i<8;++i){
      if(hADCpedc[i]->GetEntries()>0){
	 cout<<"Drawing ADC "<<i<<" with "<<hADCpedc[i]->GetEntries()<<" entries."<<endl;
	 hADCpedc[i]->Draw(onedrawn ? "sames" : "");
	 onedrawn = 1;
      }
   }
   legADC_R->Draw();
   gPad->Update();
   cADCpedc->cd(3);
   hADCLeftNIMsumPedc->Draw();
   hADCSumLeftPedc->Draw("sames");
   legADC->Draw();
   gPad->Update();
   cADCpedc->cd(4);
   hADCRightNIMsumPedc->Draw();
   hADCSumRightPedc->Draw("sames");
   legADC->Draw();
   gPad->Update();
   cADCpedc->Update();

   //Pedestal spectra
   //////////////////
   TCanvas *cPed = new TCanvas("cPed","cPed",0,0,width,700);
   cPed->Divide(2,1);
   cPed->cd(1);
   hPed[0]->Draw();
   for(int i=1;i<4;++i)
      if(hPed[i]->GetEntries()>0) hPed[i]->Draw("sames");
   hPed[11]->Draw("same");
   cPed->cd(2);
   hPed[4]->Draw();
   for(int i=5;i<8;++i)
      if(hPed[i]->GetEntries()>0) hPed[i]->Draw("sames");
   hPed[12]->Draw("same");
   cPed->Update();

   //Timing plots
   //////////////
   TCanvas *cT = new TCanvas("cT","cT",0,0,width,700);
   cT->Divide(2,1);
   cT->cd(1);
   hT[0]->Draw();
   for(int i=1;i<4;++i)
      if(hT[i]->GetEntries()>0) hT[i]->Draw("sames");
   legADC->Draw();
   cT->cd(2);
   hT[4]->Draw();
   for(int i=5;i<8;++i)
      if(hT[i]->GetEntries()>0) hT[i]->Draw("sames");
    legADC->Draw();
    cT->Update();

   //Coincidence events
   /////////////////////
   TCanvas *cCoinc = new TCanvas("cCoinc","Coincidences",0,0,width,height);
   cCoinc->Divide(2,2);
   cCoinc->cd(1);
   hCoincL->Draw();
   hCoincR->Draw("same");
   legCoinc->Draw();
   cCoinc->cd(2);
   hCoinc->Draw();
   cCoinc->cd(3);
   hCoincDt->Draw();
   cCoinc->Update();
   cCoinc->cd(4)->SetLogz();
   gPad->SetRightMargin(0.15);
   hCoincLR->Draw("colz");
   cCoinc->Update();
   TCanvas *cAsym = new TCanvas("cAsym","cAsym",0,0,1400,1000);
   cAsym->Divide(2,2);
   cAsym->cd(1);
   hAsym->Draw();
   int nbins = hAsym->FindLastBinAbove(0)-hAsym->FindFirstBinAbove(0);
   int lbin = hAsym->FindFirstBinAbove(0)-10;
   int hbin = hAsym->FindLastBinAbove(0)+10;
   hAsym->GetXaxis()->SetRange(lbin, hbin);
   gPad->Update();
   cAsym->cd(2);
   TGraph *grAsym = new TGraph((int)vAsym.size(),vEvent.data(),vAsym.data());
   grAsym->SetMarkerStyle(6);
   grAsym->SetTitle("Asymmetry versus Event Number");
   grAsym->Draw("ap");
   grAsym->GetYaxis()->SetTitle("Asymmetry");
   grAsym->GetXaxis()->SetTitle("Event");
   cAsym->cd(3);
   hAsymL->Draw();
   cAsym->cd(4);
   hAsymR->Draw();
   //file->Close();
   //delete tree;
   return 0;
}

