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
#include <GoodHelicities.h>
#include <map>
#include <TCanvas.h>
#include <TMultiGraph.h>
#include <THaEvent.h>
#include <TKey.h>
#include <sstream>

const Int_t MAXFILES = 100;//maximum number of file segments for a run
const Int_t nDET = 8;//number of PMTs in calorimeter
const Double_t NSA = 40, NSB = 12, NPED = 4;//values from config file
const Double_t PEDFRAC = (NSA+NSB)/(4*NPED);//multiply pedestal by this for pulse integral pedestal
const Double_t COINC = 4;//maximum separation in nanoseconds to be considered a coincidence
const Double_t TIME_OFFSET = -3.5;//average L-R difference in nanoseconds between coincidences on left and right
const Double_t THRESH = 2500;//minimum left or right sum (ADC integral units) to be used in a Moller coincidence
/////////////////////////////////////////////////////////////////////////////////////
//Function to predict helicity after collecting helicity sign from 30 patterns.    //
//The first thirty windows are used to set a 30 bit shift register after which     //
//the sign of the helicity can be predicted as the XOR of bits 30, 29, 28 and 7.   //
//If hRead = 0 or 1 it is interpreted as helicity and is used to set the initial   //
//shift register state. Otherwise, if hread = 2, it is used to predict the next    //
//helicity state.                                                                  //
/////////////////////////////////////////////////////////////////////////////////////
UInt_t fgShreg = 0;
UInt_t RanBit(UInt_t hRead){
   UInt_t bit7 = (fgShreg & 0x00000040) != 0;
   UInt_t bit28 = (fgShreg & 0x08000000) != 0;
   UInt_t bit29 = (fgShreg & 0x10000000) != 0;
   UInt_t bit30 = (fgShreg & 0x20000000) != 0;
   UInt_t newbit = (bit30 ^ bit29 ^ bit28 ^ bit7) & 0x1;
   fgShreg = ( (hRead == 2 ? newbit : hRead) | (fgShreg << 1 )) & 0x3FFFFFFF;
   return newbit;
}

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
int getAsym(int run, int pat = 1, bool accidental = 0, int delay_win=8, double flip_freq = 29.56, const char* fnameform = "fadcV2_moller_analyzer_") {
   if( pat < 0 || pat > 5){
      cout<<"Usage: getAysm(run number, pattern type)\n"
	 "Invalid pattern type. Select from one of the following: \n"
	 "0 pair +- or opposite polarity (1 transition)\n"
	 "1 quartet +--+ or opposite polarity (2 transitions)\n"
	 "2 octet +--+-++- or opposite polarity(5 transitions)\n"
	 "3 toggle +-+-+-+-+-+-+-\n"
	 "4 hexo-quad +--++--++--+-++--++--++- or opposite polarity (13 transitions)\n"
	 "5 octo-quad +--++--++--++--+-++--++--++--++- or opposite polarity (17 transitions)"<<endl;
      exit(1);
   }
   
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
   for (int x = 0; x <= MAXFILES; ++x) {
      TString filename(Form("%s/%s%i", gSystem->Getenv("HAMOLLER_ROOTFILE_DIR"), fnameform,run));
      if(x>0)filename += Form("_%i",x);
      filename += ".root";
      cout<<filename<<endl;
      if (gSystem->AccessPathName(filename)) {
	 break; 
      }
      tree->AddFile(filename,"T"); 
   }
   Long64_t nEntries = tree->GetEntries();
   std::cout<<nEntries<< " total entries.\n";
   if(nEntries==0){
      cout<<"No entries found. Exiting."<<endl;
      return -1;
   }
   //Go to the start of the first good pattern. Look for the first new pattern meaning
   //1. QRT==TRUE
   //2. PREV_QRT==FALSE
   //..so advance until you find at least one QRT==FALSE then advance until QRT==TRUE
   //Note: QRT is JLab jargon for a signal that is TRUE during the first window of each
   //      pattern and FALSE for the remainder of the pattern.
   //////////////////////////////////////////////////////////////////////////////////// 

   Long64_t start = 0;
   ULong64_t tEvt = 0;
   int nCh, nADCch, filestart = 0;
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
   for (Long64_t i = 0; i < nEntries; ++i) {
      tree->GetEntry(i);
      if(nADCch > nCHANMAX){
	 cout<<"Too many readings. Exiting."<<endl;
	 exit(1);
      }
      bool foundqrt = 0, foundhel = 0;
      for (int ch : chan) {
	 if(ch == 9)foundqrt = 1;
	 if(ch == 10)foundhel = 1;
      }
      
      if(!foundqrt)foundqrtOff = 1;//find first QRT off period
      if(foundqrt && foundqrtOff){ //then find where QRT turns on again
	 start = i;
	 prevT = tEvt;
	 break; 
      }
      prev_qrt = foundqrt;
      prev_hel = foundhel;
   }
   cout<<"Start at entry "<<start<<" in file segment "<<tree->GetCurrentTreeNumber()<<endl;

   //Loop over tree extracting helicity and QRT. Start with first full QRT window.
   ///////////////////////////////////////////////////////////////////////////////
   double deltaT;
   prevT = tEvt;
   int nBad=0, nGood = 0;
   vector<Int_t>patt_sign;//pattern polarity Sign = 0(-), 1(+) or -9 unknown
   vector<Long64_t>vStart;//first entry of pattern after tsettle is inactive
   vector<Long64_t>vEnd;//last entry of pattern before tsettle is active
   vector<vector<Long64_t>>vTransEntry;//first entry after helicity transition inside pattern. Each row in this 2D vector contains
                                       //the full number of helicity transitions pat_transitions[pat] inside a pattern (2 for quartet).
   vector<Long64_t>vTransEntry_tmp;//single row in vTransEntry
   vector<vector<int>>vHelicity;//helicity during QRT and after each inter-pattern transition. Each row in this 2D vector contains the
                                //helicity sequence inside a pattern (eg. 101 or 010 for quartet)
   vector<int>vHelicity_tmp;//single row in vHelicity
   vector<double>qrtTimes;
   vector<bool>vGood;//was the pattern complete?
   int  nhelTrans = 0, nMiss = 0, prev_entry = start;
   prev_qrt = 0;
   for (Long64_t i = start; i < nEntries; ++i) {
      tree->GetEntry(i);
      if(nADCch > nCHANMAX){
	 cout<<"Too many entries. Exiting."<<endl;
	 exit(1);
      }
      int found_hel = 0, found_qrt = 0, found_tsettle = 0;
      deltaT = (tEvt - prevT) * 4.0e-9;//convert to seconds
      if(deltaT<0)cout<<i<<" "<<tEvt<<" "<<prevT<<endl;
      assert(deltaT >= 0);
      for( double ch : chan){
	 if((int)ch==8){found_tsettle = 1;}
	 if((int)ch==9){found_qrt = 1;}
	 if((int)ch==10){found_hel = 1;}
      }
      if(found_tsettle)continue;
      if(prev_hel != found_hel && !found_qrt){
	 //helicity transition inside a pattern i.e.
	 //not associated with the start of a pattern
	 vTransEntry_tmp.push_back(i);
	 if(vHelicity_tmp.size()>0)if(vHelicity_tmp.back()==(int)found_hel)cout<<"Yikes!"<<endl;
	 vHelicity_tmp.push_back((int)found_hel);
	 ++nhelTrans;
      }
      if( found_qrt && (prev_qrt != found_qrt || deltaT > maxDT) ){
	 //Located a new pattern start that meets one of two conditions 
	 //1. A transition from QRT=0 to 1 signaling the end of one pattern and the start of the next
	 //     - condition: found_qrt && (prev_qrt != found_qrt)
	 //2. Or missed QRTs due to low event rates (eg. beam trips)
	 //     - condition: found_qrt && (deltaT > maxDT) maxDT=maximum time btw pulses in consecutive QRT gates.
	 //     - deltaT > maxDT condition catches the unfortuitous possibility that no data were captured
	 //       from windows inside a pattern but only the start (QRT) windows. In this case no QRT=0
	 //       would have been recorded.
	 //////////////////////////////////////////////////////////////////////////////////////////////////
	 
	 //Start by dealing with condition #2
	 /////////////////////////////////////
	 while(deltaT > maxDT && qrtTimes.size() > 0){
	    //If there is a period of missing helicities due to a beam trip or other reason
	    //fill in best estimate of QRT start times for missing patterns and helicities.
	    if(patt_sign.size()>0){
	       vGood.push_back(false);
	       vEnd.push_back(prev_entry);
	       while((int)vHelicity_tmp.size()<= nTransPerPat){
		  vHelicity_tmp.push_back(-9);
		  cout<<"Missing helicity transitions after entry "<<i;
	       }
	       while((int)vTransEntry_tmp.size() <= nTransPerPat){
		  vTransEntry_tmp.push_back(-9);
		  cout<<"Inserting -9 "<<endl;
	       }
	       vHelicity.push_back(vHelicity_tmp);
	       vHelicity_tmp.clear();
	       vHelicity_tmp.push_back(found_hel);
	       vTransEntry.push_back(vTransEntry_tmp);
	       vTransEntry_tmp.clear();
	    }
	    patt_sign.push_back(-9);
	    vStart.push_back(-9);
	    qrtTimes.push_back(qrtTimes.back() + period);
	    deltaT -= period;
	    nMiss++;
	 }
	 //Condition #1 with a complete pattern
	 //////////////////////////////////////
	 if(nhelTrans == nTransPerPat && deltaT > minDT){
	    //reached end of good pattern with right number of helicity transitions and a QRT
	    qrtTimes.push_back(tEvt*4e-9);
	    //stay one entry behind since you can't evaluate completeness until end of pattern
	    if(patt_sign.size()>0){
	       vGood.push_back(true);
	       vEnd.push_back(prev_entry);
	       vHelicity.push_back(vHelicity_tmp);
	       vHelicity_tmp.clear();
	       vHelicity_tmp.push_back(found_hel);
	       vTransEntry.push_back(vTransEntry_tmp);
	       vTransEntry_tmp.clear();
	    }
	                                                
	    patt_sign.push_back(found_hel);
	    vStart.push_back(i);
	    nhelTrans = 0;
	    nGood++;
	    //Condition #1 with an incomplete pattern
	    /////////////////////////////////////////
	 }else{
	    cout<<"Missed helicities or issues with QRT before event "<<i<<" "<<nhelTrans<<" "<<nCh<<" "<<tEvt<<endl;
	    qrtTimes.push_back(tEvt*4e-9);
	    if(patt_sign.size()>0){
	       vEnd.push_back(prev_entry);
	       vGood.push_back(false);
	       //for incomplete patterns complete the helicity sequence by toggling between helicity states
	       while((int)vHelicity_tmp.size()<=pat_transitions[pat]){
		  vHelicity_tmp.push_back(vHelicity_tmp.back()==0?1:0);
		  cout<<"Filling in incomplete helicity sequence."<<endl;
	       }
	       vHelicity.push_back(vHelicity_tmp);
	       vHelicity_tmp.clear();
	       vHelicity_tmp.push_back(found_hel);
	       vTransEntry.push_back(vTransEntry_tmp);
	       vTransEntry_tmp.clear();
	    }
	    patt_sign.push_back(-9);
	    vStart.push_back(i);
	    nhelTrans = 0;
	    nBad++;
	 }
	 prevT = tEvt;
      }
      prev_qrt = found_qrt;
      prev_hel = found_hel;
      prev_entry = i;
   }

   vGood.push_back(false);
   vEnd.push_back(prev_entry);
   while((int)vHelicity_tmp.size()<=nTransPerPat){
      vHelicity_tmp.push_back(vHelicity_tmp.back()==0?1:0);
      cout<<"Filling in missing helicity transitions in last pattern based on known pattern sequence.";
   }
   while((int)vTransEntry_tmp.size()<=nTransPerPat){
      vTransEntry_tmp.push_back(-1);
      cout<<endl;
   }
   vHelicity.push_back(vHelicity_tmp);
   vTransEntry.push_back(vTransEntry_tmp);
   vHelicity_tmp.clear();
   vTransEntry_tmp.clear();
   cout<<"Found "<<nGood<<" complete helicity patterns. "<<nBad<<" had issues. "<<nMiss<<" were missing."<<endl;

   //////////////////////////////////////////
   //Verify the helicity sequence is correct.
   int startover = 0;
   cout<<"Verifying helicity sequence."<<endl;
   for( int i=0; i< (int)patt_sign.size();++i){
      UInt_t bit = 0;
      Int_t sign = patt_sign[i];
      if(sign < 0 || sign > 1)startover = i + 1;
      if(i < startover + 30){
	 cout<<i<<" "<<vStart[i]<<" "<<sign<<" "<<fgShreg<<" "<<RanBit(sign)<<" ";
	 for(int j : vHelicity[i])cout<<j;cout<<endl;
      }else if((bit=RanBit(2)) == (UInt_t)sign){
	 cout<<i<<" "<<vStart[i]<<" "<<vEnd[i]<<" sign:"<<sign<<" bit:"<<bit<<" good:"<<vGood[i];
	 cout<<" vHel:";
	 for(int j : vHelicity[i])cout<<j;cout<<endl;
	 continue;
      }else{
	 cout<<"Oops! "<<vStart[i]<<" "<<sign<<" "<<bit<<endl;
	 cout<<"Exiting. Issue with helicity prediction at i="<<i<<endl;
	 exit(1);
      }
   }  
   //Set up the plots we want to see.
   ////////////////////////////////////
   TH1D *hADCSumLeft = new TH1D("hADCSumLeft","Spectrum of ADC-Summed Left Channels",300,-2000,15000);
   hADCSumLeft->SetLineWidth(2);
   TH1D *hADCSumLeftPedc = new TH1D("hADCSumLeftPedc","Pedestal Subtracted Spectrum of ADC-Summed Left Channels",300,-2000,15000);
   hADCSumLeftPedc->SetLineWidth(2);
   TH1D *hADCLeftNIMsum = new TH1D("hADCLeftNIMsum","ADC Spectrum of NIM-Summed Left Channels",300,-2000,15000);
   hADCLeftNIMsum->SetLineColor(kRed);
   hADCLeftNIMsum->SetLineWidth(2);
   TH1D *hADCLeftNIMsumPedc = new TH1D("hADCLeftNIMsumPedc","Pedestal Subtracted ADC Spectrum of NIM-Summed Left Channels",300,-2000,15000);
   hADCLeftNIMsumPedc->SetLineColor(kRed);
   hADCLeftNIMsumPedc->SetLineWidth(2);
   TH1D *hADCSumRight = new TH1D("hADCSumRight","Spectrum of ADC-Summed Right Channels",300,-2000,15000);
   hADCSumRight->SetLineWidth(2);
   TH1D *hADCSumRightPedc = new TH1D("hADCSumRightPedc","Pedestal Subtracted Spectrum of ADC-Summed Right Channels",300,-2000,15000);
   hADCSumRightPedc->SetLineWidth(2);
   TH1D *hADCRightNIMsum = new TH1D("hADCRightNIMsum","ADC Spectrum of NIM-Summed Right Channels",300,-2000,15000);
   hADCRightNIMsum->SetLineColor(kRed);
   hADCRightNIMsum->SetLineWidth(2);
   TH1D *hADCRightNIMsumPedc = new TH1D("hADCRightNIMsumPedc","ADC Spectrum of NIM-Summed Right Channels",300,-2000,15000);
   hADCRightNIMsumPedc->SetLineColor(kRed);
   hADCRightNIMsumPedc->SetLineWidth(2);
   TH1D *hCoinc = new TH1D("hCoinc","Total Detector FADC Sum Spectrum for Coincidence Events",200,-2000,18000);
   hCoinc->SetLineColor(kBlack);
   hCoinc->SetLineWidth(2);
   TH1D *hCoincL = new TH1D("hCoincL","Detector FADC Sum Spectrum for Coincidence Events",300,-2000,15000);
   hCoincL->SetLineColor(kBlue);
   hCoincL->SetLineWidth(2);
   TH1D *hCoincR = new TH1D("hCoincR","Detector FADC Sum Spectrum for Coincidence Events",300,-2000,15000);
   hCoincR->SetLineColor(kRed);
   hCoincR->SetLineWidth(2);
   TH1D *hCoincDt = new TH1D("hCoincDt","Left-Right Detector #DeltaT for Coincidence Events",40,-100,100);
   hCoincDt->SetLineColor(kBlack);
   hCoincDt->SetLineWidth(2);
   TH2D *hCoincLR = new TH2D("hCoincLR","Right vs. Left Detector Energy",1000,THRESH,10000,1000,THRESH,10000);
   TH1D *hADC[nCHAN], *hADCpedc[nCHAN], *hT[nCHAN], *hPed[nCHAN];
   TH1D *hAsym = new TH1D("hAsym","Coincidence Scattering Asymmetry",400,-1,1);
   TH1D *hAsymL = new TH1D("hAsymL","Left Single-Arm Asymmetry",200,-0.5,0.5);
   TH1D *hAsymR = new TH1D("hAsymR","Right Single-Arm Asymmetry",200,-0.5,0.5);
   int color[nCHAN] = {kRed+1,kBlue+1,kGreen+2,kMagenta+1,kCyan+1,kOrange+7,kViolet,kAzure+2,
      kBlack,kBlack,kBlack,kBlack,kBlack,kBlack,kBlack,kBlack};

   for(int i=0;i<nCHAN;++i){
      if(i<nDET){
	 hADC[i] = new TH1D(Form("hADC%i",i),Form("ADC Spectrum of PMTs"),100,0,18000);
	 hADCpedc[i] = new TH1D(Form("hADCpedc%i",i),Form("Pedstal Subtracted ADC Spectrum of PMTs"),100,-2000,8000);
	 hT[i] = new TH1D(Form("hT%i",i),Form("Time Spectrum of PMTs"),250,-5,245);
	 hPed[i] = new TH1D(Form("hPed%i",i),Form("Pedestal Spectrum of PMTs"),300,0,5000);
      }else if(i==11){
	 hADC[i] = new TH1D(Form("hADC%i",i),Form("ADC Spectrum of Left PMT NIM Sum"),100,0,18000);
	 hADCpedc[i] = new TH1D(Form("hADCpedc%i",i),Form("Pedstal Subtracted ADC Spectrum of Left PMT Sum"),100,-2000,8000);
	 hT[i] = new TH1D(Form("hT%i",i),Form("Time Spectrum of Left PMT NIM Sum"),250,-5,245);
	 hPed[i] = new TH1D(Form("hPed%i",i),Form("Pedestal Spectrum of Left PMT NIM Sum"),300,0,5000);
      }else if(i==12){
	 hADC[i] = new TH1D(Form("hADC%i",i),Form("ADC Spectrum of Right PMT NIM Sum"),100,0,18000);
	 hADCpedc[i] = new TH1D(Form("hADCpedc%i",i),Form("Pedstal Subtracted ADC Spectrum of Right PMT Sum"),100,-2000,8000);
	 hT[i] = new TH1D(Form("hT%i",i),Form("Time Spectrum of Right PMT NIM Sum"),250,-5,245);	 hPed[i] = new TH1D(Form("hPed%i",i),Form("Pedestal Spectrum of Right PMT Sum"),300,0,5000);
      }else{
	 hADC[i] = new TH1D(Form("hADC%i",i),Form("ADC Spectrum of FADC Ch %i",i),100,0,18000);
	 hADCpedc[i] = new TH1D(Form("hADCpedc%i",i),Form("Pedstal Subtracted ADC Spectrum of FADC Ch %i",i),100,-2000,8000);
	 hT[i] = new TH1D(Form("hT%i",i),Form("Time Spectrum of FADC Ch %i",i),250,-5,245);
	 hPed[i] = new TH1D(Form("hPed%i",i),Form("Pedestal Spectrum of FADC Ch %i",i),300,0,5000);
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
   bool foundstart = 0, foundend = 0;
   double sum = 0;
   //Use vectors below to group in-time pulses into left and right sums and left/right coincidences
   vector<double>ladc, ladcpedc, ladc_t, radc ,radcpedc, radc_t,
                 lsum, lsumpedc, rsum, rsumpedc, lsum_t, rsum_t,
                 vAsym, vEvent;
   vector<int>radc_wfidx, ladc_wfidx;
   vector<vector<int>>rsum_wfidx, lsum_wfidx;
   int count = 0, idx = 0, nHplus = 0, nHminus = 0, nHLplus = 0, nHLminus = 0,nHRplus = 0, nHRminus = 0, np = 0, hel = 0;
   vector<int>seq;
   tree->SetBranchStatus("M.cal.adc", 1);
   tree->SetBranchAddress("M.cal.adc", adc);
   tree->SetBranchStatus("M.cal.ped", 1);
   tree->SetBranchAddress("M.cal.ped", ped);
   tree->SetBranchStatus("M.cal.adc_t", 1);
   tree->SetBranchAddress("M.cal.adc_t", adc_t);
   TString pdfName = Form("~/jonesdc/macros/%swaveforms%i.pdf",(char*)(accidental?"accidental_":""),run);
   TCanvas *c = new TCanvas("c", "c", 0, 0, 850, 1100);
   c->Divide(2,3);
   c->SetBatch(1);
   c->Print(pdfName + "[");  // open multi-page PDF
   int padCounter = 0, pagecounter = 0;
   for (Long64_t i = start; i < nEntries; ++i) {
      ladc.clear(); radc.clear(); ladc_t.clear(); radc_t.clear();//clear the temp coinc-finding arrays
      ladcpedc.clear(); radcpedc.clear(); lsumpedc.clear(); rsumpedc.clear();
      lsum.clear(); rsum.clear(); lsum_t.clear(); rsum_t.clear();
      ladc_wfidx.clear(); radc_wfidx.clear(); lsum_wfidx.clear(); rsum_wfidx.clear();
      //Advance until inside the Tstable part of a pattern
      while(i < vStart[idx]){
	 ++i;
	 foundstart = foundend = 0;
      }
      //When you pass the end of a pattern advance the pattern index...
      while(i > vEnd[idx]){
	 if(idx+1 < (int)vEnd.size())idx++;
	 foundstart = foundend = 0;
	 while(i < vStart[idx]){//..and advance through Tsettle to Tstable
	    ++i;
	 }
      }
      if(!vGood[idx])continue;
      int nPat = (int)vTransEntry[idx].size();
      while(np < nPat){//increment through helicity pattern as required
	 if(i>=vTransEntry[idx][np]){++np;}
	 else break;
      }
      if(idx+delay <= 0)continue;
      if(idx+delay < (int)patt_sign.size()){
	 hel = vHelicity[idx+delay][np];//assign actual helicity from delayed helicity
      }else break;//stop delay-reported patterns before end of file due to lacking helicity information
      //cout<<i<<" "<<vStart[idx]<" "vEnd[idx]<<" "hel<<endl;
      tree->GetEntry(i);
      int idl = 0, idr = 0;//left and right channel indices
      double sumleftpedc = 0, sumrightpedc = 0, leftsumnim = 0, rightsumnim = 0, leftsumnimped = 0, rightsumnimped = 0;
      bool trigR = 0, trigL = 0, trigC = 0, qrt = 0, tsettle_active = 0;

      //Ignore all events inside Tsettle
      for(int j = 0; j<nCh; ++j){
	 if(chan[j]==8)tsettle_active = true;
	 if(chan[j]>8)break;
      }
      if(i==vStart[idx])foundstart = 1;
      if(i==vEnd[idx])foundend = 1;
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
      if(i==5781){
	 cout<<i<<" ladc: ";
	 for(int v:ladc_wfidx)cout<<v<<" ";
	 cout<<endl;
	 cout<<i<<" radc: ";
	 for(int v:radc_wfidx)cout<<v<<" ";
	 cout<<endl;
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
      if(i==5781){
	 for(const auto& row : lsum_wfidx){
	    cout<<i<<" lsum: size="<<row.size();
	    for(const auto& v: row)cout<<"   "<<v<<" ";
	    cout<<endl;
	 }
	 for(const auto& row : rsum_wfidx){
	    cout<<i<<" rsum: size="<<row.size();
	    for(const auto& v: row)cout<<"   "<<v<<" ";
	    cout<<endl;
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
		  if(inside_sample_window && pagecounter < nPAGE-1){//print waveforms
		     tree->SetBranchStatus("M.cal.nsamples", 1);
		     tree->SetBranchAddress("M.cal.nsamples", &nsamples);
		     tree->SetBranchStatus("M.cal.samples", 1);
		     tree->SetBranchAddress("M.cal.samples", samples);
		     tree->SetBranchStatus("M.cal.samples_c", 1);
		     tree->SetBranchAddress("M.cal.samples_c", samples_ch);
		     tree->GetEntry(i);
		     TMultiGraph *mgl = new TMultiGraph();
		     TGraph *grl[4]; int col[4] = {kBlack, kRed, kBlue, kGreen+2};
		     TString ch_name = "";
		     bool onedrawn = 0;
		     for(int wf=0; wf<(int)lsum_wfidx[j].size(); ++wf){
			if(i==5781)cout<<"wf: "<<wf<<" "<<lsum_wfidx[j].size()<<" "<<j<<endl;
			if(wf==4)break;
			if(padCounter == 6) {
			   c->ForceUpdate();
			   c->Print(pdfName,"pdf");
			   pagecounter++;
			   if(pagecounter==nPAGE-1)break;
			   c->Clear();
			   c->Divide(2, 3);
			   padCounter = 0;
			}
			c->cd(padCounter + 1);
			//cout<<"Pad "<<padCounter +1<<endl;;
			grl[wf] = new TGraph((int)nsamples);
			grl[wf]->SetLineColor(col[wf]);
			int chj = (int)adcchan[lsum_wfidx[j][wf]];
			ch_name += Form("%i ", chj);
			int nwf = 0;
			double prevx = -1;
			for(double x : adcchan){
			   if((int)x < chj && x != prevx){++nwf; prevx = x;}
			   if((int)x >= chj)break;
			}
			for(int s=0; s<(int)nsamples; s++) {
			   if(samples_ch[nwf*(int)nsamples+s] != chj)
			      cout<<"Coding error. Wrong waveform plotted. "<<samples_ch[nwf*(int)nsamples+s]<<" "<<chj<<endl;
			   grl[wf]->SetPoint(s, s*4, samples[nwf*(int)nsamples+s]);
			}
			mgl->Add(grl[wf]);
		     }
		     padCounter++;
		     cout<<"Drawing left event "<<i<<" "<<lsum_wfidx[j].size()<<"ch "<<ch_name.Data()<<endl;
		     mgl->SetTitle(Form("Event %lld - Channel(s) %s;Time (ns);ADC",i, ch_name.Data()));
		     mgl->Draw("ALP");
		     c->Modified();
		     c->Update();
		     TMultiGraph *mgr = new TMultiGraph();
		     TGraph *grr[4];
		     ch_name = "";
		     onedrawn = 0;
		     for(int wf=0; wf<(int)rsum_wfidx[k].size(); ++wf){
			if(wf == 4)break;
			if(padCounter == 6) {
			   c->Print(pdfName, "pdf");
			   pagecounter++;
			   if(pagecounter==nPAGE-1)break;
			   c->Clear();
			   c->Divide(2, 3);
			   padCounter = 0;
			}
			c->cd(padCounter + 1);
			//cout<<"Pad "<<padCounter +1<<endl;;
			grr[wf] = new TGraph((int)nsamples);
			grr[wf]->SetLineColor(col[wf]);
			int chk = (int)adcchan[rsum_wfidx[k][wf]];
			ch_name += Form("%i ", chk);
			int nwf = 0;
			double prevx = -1;
			for(double x : adcchan){
			   if((int)x < chk && x != prevx){++nwf; prevx = x;}
			   if((int)x >= chk) break;
			}
			for(int s=0; s<(int)nsamples; s++) {
			   if(samples_ch[nwf*(int)nsamples+s] != chk)
			      cout<<"Coding error. Wrong waveform plotted. "<<samples_ch[nwf*(int)nsamples+s]<<" "<<chk<<endl;
			   grr[wf]->SetPoint(s, s*4, samples[nwf*(int)nsamples+s]);
			}
			mgr->Add(grr[wf]);
		     }
		     padCounter++;
		     cout<<"Drawing right event "<<i<<" "<<rsum_wfidx[k].size()<<"ch "<<ch_name.Data()<<endl;

		     mgr->SetTitle(Form("Event %lld - Channel(s) %s;Time (ns);ADC", i, ch_name.Data()));
		     mgr->Draw("ALP");
		     c->Modified();
		     c->Update();
		     tree->SetBranchStatus("M.cal.nsamples", 0);
		     tree->SetBranchStatus("M.cal.samples", 0);
		  }
	       }
	    }
	 }
      }

      //Process pattern level averages now
      ////////////////////////////////////      
      if(foundstart && foundend){
	 //cout<<endl;
	 double asym  = double(nHplus - nHminus)/double(nHplus + nHminus);
	 hAsym->Fill(asym);
	 vAsym.push_back(asym);
	 hAsymL->Fill(double(nHLplus-nHLminus)/double(nHLplus+nHLminus));
	 hAsymR->Fill(double(nHRplus-nHRminus)/double(nHRplus+nHRminus));
	 vEvent.push_back(i);
	 seq.push_back(hel);
	 cout<<asym<<": "<<nHLminus<<" "<<nHLplus<<" start="<<vStart[idx]<<" end="<<vEnd[idx]<<" hel="<<hel;
	 for(int x=1; x<(int)vHelicity[idx+delay].size(); ++x) cout<<" "<<vHelicity[idx+delay][x];
	 cout<<endl;
	 foundstart = foundend = nHplus = nHminus = nHLplus = nHLminus = nHRplus = nHRminus = np = 0;
      }
   }
   if(padCounter > 0)
      c->Print(pdfName,"pdf");

   c->Print(pdfName + "]","pdf");
// int st = 0;
   // for(int i=st;i<(int)seq.size();++i){
   //    if(i-st<=30)RanBit(seq[i]);
   //    else cout<<seq[i]<<" "<<RanBit(2)<<endl;
   // }
  
   //return hAsym->GetEntries();
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
   int lbin = nbins>50 ? hAsym->FindFirstBinAbove(0)-1: hAsym->GetMaximumBin()-25;
   int hbin = nbins>50 ? hAsym->FindLastBinAbove(0)+1: hAsym->GetMaximumBin()+25;
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

