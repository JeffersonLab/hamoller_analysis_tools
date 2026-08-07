/////////////////////////////////////////
//Author:   Addison Arcuri	       //
//Employer: Temple University	       //
//Date:     April 2025   	       //
/////////////////////////////////////////
/////////////////////////////////////////
#include <TH1D.h>
#include <vector>
#include <TCanvas.h>
#include <TMath.h>
#include <TStyle.h>
#include <TF1.h>
#include <TPaveStats.h>
#include "TTrain.h"
#include <chrono>

///////////////////////////////////////////////////////////////////////////////
//This is my current main function. Right now just trying to unpack the data.//
//Everything from here on out is my code, minus one marked case.             //
///////////////////////////////////////////////////////////////////////////////
void quickRatesV2(int run, int patternType=4, int preScale=0, double flipRate = 29.5596, double tStable = 33330.0, double tSettle = 500, int histEnd=200000){
   auto start = std::chrono::high_resolution_clock::now();
   
   const int maxChan = 46;
   const double sampleRatio = 13.0/4.0; //Fraction to multipy pedestals by. Peds are 4 sample sum, adc readout is 13 sample integral
   const int eventWidth = 1000; //Event width (and therefore bin size) in ns
   const double sumTime = 4.0; //Time difference threshold for adjacent pulse combination, in ns
   const double coinTime = 6.0; //Time difference threshold for coincidence counting, in ns
   const double coinThresh = 400.0; //Minimum adc for coincidence counting (Usually 4000)
   const double singleArmThresh = 40.0; //Minimum adc for single arm counting
   
   //const double flipRate = 240.001;//29.5596; //Flip rate in Hertz
   tStable = tStable*0.000001; //TStable window (in s)
   tSettle = tSettle*0.000001; //TSettle window (in s)
   const double adjFlipRate = 1/((1/flipRate)-tSettle); //Adjusted for tSettle
   
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
	 //std::cout << filename << std::endl;
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
   tree->SetUpTTrain(run, fnameform,0);

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

   double ped[maxChan];
   tree->SetBranchStatus("M.cal.ped", 1);
   tree->SetBranchAddress("M.cal.ped",ped);

   double adc_t[maxChan];
   tree->SetBranchStatus("M.cal.adc_t", 1);
   tree->SetBranchAddress("M.cal.adc_t", adc_t);
   
   ULong64_t eventTime;
   tree->SetBranchStatus("fEvtHdr.fEvtTime", 1);
   tree->SetBranchAddress("fEvtHdr.fEvtTime",&eventTime);
   
   vector<int> vPattState; //1 if Channel 10 is firing. Otherwise 0.
   vector<int> vPattStarts; //The starting indexes of each pattern
   vector<int> vPattEnds; //The ending indexes of each pattern helicity state before TSettle
  
   ///////////////////EVENT TIMING STUFF/////////////////////
   tree->GetEntry(0);
   double firstEventTime = eventTime;
   tree->GetEntry(nTreeEntries-1);
   double lastEventTime = eventTime;
   double runLength = (lastEventTime - firstEventTime)*4.0*1.0e-9;
   std::cout<<"Run length (s): "<<runLength<<std::endl;
   std::cout<<"Event Rate (Entries/s): "<<nTreeEntries/runLength<<std::endl;
   double pattEst = (nTreeEntries/runLength)/flipRate*(double)patternType;
   std::cout<<"Per Pattern Estimate (Counts/Pattern): "<<pattEst<<std::endl;
   int pattEstRound = ((int)pattEst/1000)*1000+1000;
   std::cout<<"Rounded Per Pattern Estimate (Counts/Pattern): "<<pattEstRound<<std::endl;
   
   int preScaleConv = (int)std::pow(2.0,(preScale-1))+1;
   
   int nBins;
   histEnd = histEnd + eventWidth - histEnd%eventWidth;
   nBins = histEnd/eventWidth;

   TH1D *hlevtt = new TH1D(Form("Run_%d_hlevtt", run),Form("Run %d: Left Event #DeltaTs", run),nBins,0,histEnd);
   hlevtt->GetXaxis()->SetTitle("#DeltaT (ns)");
   hlevtt->GetYaxis()->SetTitle("Number of Events");
   TF1 *fTimeFitLeft = new TF1("fTimeFitLeft", Form("[0]*ROOT::Math::gamma_pdf(x,%f,[1])", (double)preScaleConv),eventWidth+20,histEnd);
   int eventTimeLeft=0;
   int prevEventTimeLeft=0;
   
   TH1D *hrevtt = new TH1D(Form("Run_%d_hrevtt", run),Form("Run %d: Right Event #DeltaTs", run),nBins,0,histEnd);
   hrevtt->GetXaxis()->SetTitle("#DeltaT (ns)");
   hrevtt->GetYaxis()->SetTitle("Number of Events");
   TF1 *fTimeFitRight = new TF1("fTimeFitRight", Form("[0]*ROOT::Math::gamma_pdf(x,%f,[1])", (double)preScaleConv),eventWidth+20,histEnd);
   int eventTimeRight=0;
   int prevEventTimeRight=0;
   
   TH1D *hcevtt = new TH1D(Form("Run_%d_hcevtt", run),Form("Run %d: Coin Event #DeltaTs", run),nBins,0,histEnd);
   hcevtt->GetXaxis()->SetTitle("#DeltaT (ns)");
   hcevtt->GetYaxis()->SetTitle("Number of Events");
   TF1 *fTimeFitCoin = new TF1("fTimeFitCoin", Form("[0]*ROOT::Math::gamma_pdf(x,%f,[1])", (double)preScaleConv),eventWidth+20,histEnd);
   int eventTimeCoin=0;
   int prevEventTimeCoin=0;
   ///////////////////EVENT TIMING STUFF/////////////////////
   
   for(long treeEntry = 0; treeEntry<nTreeEntries; treeEntry++){
      tree->GetEntry(treeEntry);
      
      //Find the QRT channel and add to Pattern vectors. If not present assume Pattern state is 0
      for(int j=0;j<nadc_chan;j++){
	 if(adc_chan[j]==9){
	    vPattState.push_back(1);
	    break;
	 }
	 else if(j==nadc_chan-1){
	    vPattState.push_back(0);
	 }
      }
      //If the present event's pattern state is not the same as the previous event's, update the PattStart, PattEnd vectors.
      if(treeEntry>0){
	 if((vPattState[treeEntry]==1)&&(vPattState[treeEntry-1]==0)){
	    if(vPattStarts.size()>0){ vPattEnds.push_back(treeEntry-1); }
	    vPattStarts.push_back(treeEntry);
	 }
      }
   }
   vPattStarts.pop_back();
   
   // for(size_t i=0;i<vPattStarts.size();++i){
   //    cout<<vPattStarts[i]<<"   "<<vPattEnds[i]<<endl;
   // }
   // return;

   int rateBinNum = 300;
   TH1D *hlp = new TH1D("Patt_Left_Singles","Patt Left Singles",rateBinNum,0,pattEstRound);
   TH1D *hrp = new TH1D("Patt_Right_Singles","Patt Right Singles",rateBinNum,0,pattEstRound);
   TH1D *hcp = new TH1D("Patt_Coin_Counts","Patt Coin. Counts",rateBinNum,0,pattEstRound);
   double meanl=0, meanr=0, meanc=0, n=0;
   for(int nPatt = 0;nPatt<(int)vPattEnds.size();nPatt++){ //Loop over patterns
      std::cout<<"Pattern: "<<nPatt<<std::endl;
      int pattLefts = 0;
      int pattRights = 0;
      int pattCoins = 0;

      for(int nEvent = vPattStarts[nPatt];nEvent<=vPattEnds[nPatt];nEvent++){ //Loop over events of a given pattern
	 tree->GetEntry(nEvent);
	 std::cout<<"Event: "<<nEvent<<std::endl;
	    
	 vector<double> vPMT03p; //Lefts
	 vector<double> vPMT03p_t; //Left times
	 vector<double> vPMT47p; //Rights
	 vector<double> vPMT47p_t; //Right times
	 vector<double> vPMT03ps; //Summed Lefts	 
	 vector<double> vPMT03ps_t; //Summed Left times
	 vector<double> vPMT47ps; //Summed Rights
	 vector<double> vPMT47ps_t; //Summed Right times

	 int j=0; // j is the CHANNEL index, i is the ADC index
	 for(int i=0; i<nadc_chan; i++){ //Loop over adc_chans
	    if(i>0){
	       if((int)adc_chan[i]!=(int)adc_chan[i-1]){j++;}
	    }
	       
	    switch ((int)adc_chan[i]){
	    case 0:
	    case 1:
	    case 2:
	    case 3:
	       vPMT03p.push_back(adc[i]-ped[j]*sampleRatio);
	       vPMT03p_t.push_back(adc_t[i]);
	       std::cout<<"Pulse in Left"<<std::endl;
	       break;
	    case 4:
	    case 5:
	    case 6:
	    case 7:
	       vPMT47p.push_back(adc[i]-ped[j]*sampleRatio);
	       vPMT47p_t.push_back(adc_t[i]);
	       std::cout<<"Pulse in Right"<<std::endl;    
	       break;
	    default:
	       break;
	    } //End of switch
	       
	 } //End of loop over adc_chans

	 std::cout<<"vPMT03p_t.size():"<<(int)vPMT03p_t.size()<<std::endl;
	 for(int k=0;k<(int)vPMT03p_t.size();k++){ std::cout<<vPMT03p_t.at(k)<<std::flush; }
	 std::cout<<std::endl;
	 
	 std::vector<uint8_t> vPMT03p_u(vPMT03p.size(),0);
	 std::vector<uint8_t> vPMT47p_u(vPMT47p.size(),0);
	 int nSummedL = 0, nSummedR = 0;
	 //This is the code for combining left pulses originating from the same event
	 for(int k=0;k<(int)vPMT03p_t.size();k++){ //k is pulse index
	    if(vPMT03p_u[k]==1){continue; }
	    vPMT03p_u[k] = 1;
	    double runningSumP = vPMT03p[k];
	    double runningSumT = vPMT03p_t[k];
	    int numSummed = 1;
	    if(k<(int)vPMT03p_t.size()-1){
	       for(int l=k+1;l<(int)vPMT03p_t.size();l++){ //l is comparison index
		  if(vPMT03p_u[l]==1){continue; }
		  if(abs(vPMT03p_t[k] - vPMT03p_t[l])<=sumTime){
		     runningSumP = runningSumP + vPMT03p[l];
		     runningSumT = runningSumT + vPMT03p_t[l];
		     vPMT03p_u[l] = 1;
		     numSummed++;
		  }
	       }
	    }
	    nSummedL = numSummed;
	    if(runningSumP > singleArmThresh){
	       vPMT03ps.push_back(runningSumP);
	       vPMT03ps_t.push_back(runningSumT/(double)numSummed);
	    }
	 }
	 if(vPMT03ps_t.size()>0){
	    eventTimeLeft = eventTime;
	    hlevtt->Fill((eventTimeLeft-prevEventTimeLeft)*4);
	    prevEventTimeLeft=eventTimeLeft;
	 }
	 pattLefts = pattLefts + (int)vPMT03ps_t.size();
	 std::cout<<"vPMT03ps_t.size():"<<(int)vPMT03ps_t.size()<<std::endl;


	 //Right pulses
	 for(int k=0;k<(int)vPMT47p_t.size();k++){ //k is pulse index
	    if(vPMT47p_u[k]==1){continue; }
	    vPMT47p_u[k]=1;
	    double runningSumP = vPMT47p[k];
	    double runningSumT = vPMT47p_t[k];
	    int numSummed = 1;
	    if(k<(int)vPMT47p_t.size()-1){
	       for(int l=k+1;l<(int)vPMT47p_t.size();l++){ //l is comparison index
		  if(vPMT47p_u[l]==1){continue; }
		  if(abs(vPMT47p_t[k] - vPMT47p_t[l])<=sumTime){
		     runningSumP = runningSumP + vPMT47p[l];
		     runningSumT = runningSumT + vPMT47p_t[l];
		     vPMT47p_u[l] = 1;
		     numSummed++;
		  }
	       }
	    }
	    nSummedR = numSummed;
	    if(runningSumP > singleArmThresh){
	       vPMT47ps.push_back(runningSumP);
	       vPMT47ps_t.push_back(runningSumT/(double)numSummed);
	    }
	 }
	 if(vPMT47ps_t.size()>0){
	    eventTimeRight = eventTime;
	    hrevtt->Fill((eventTimeRight-prevEventTimeRight)*4);
	    prevEventTimeRight=eventTimeRight;
	 }
	 pattRights = pattRights + (int)vPMT47ps_t.size();
	 //if(nSummedL != nSummedR)cout<<"Different at event "<<nEvent<<endl;
	 //Coincidence counting
	 std::vector<uint8_t> vPMT03ps_u(vPMT03ps.size(),0);
	 std::vector<uint8_t> vPMT47ps_u(vPMT47ps.size(),0);
	 for(int k=0;k<(int)vPMT03ps_t.size();k++){
	    if(vPMT03ps_u[k]==1){continue; }
	    vPMT03ps_u[k] = 1;
	    for(int l=0;l<(int)vPMT47ps_t.size();l++){
	       if(vPMT47ps_u[l]==1){continue; }
	       if((abs(vPMT03ps_t[k] - vPMT47ps_t[l])<coinTime)&&(vPMT03ps[k]+vPMT47ps[l]>coinThresh)){
		  eventTimeCoin = eventTime;
		  hcevtt->Fill((eventTimeCoin-prevEventTimeCoin)*4);
		  prevEventTimeCoin=eventTimeCoin;
		  
		  pattCoins++;
		  vPMT47ps_u[l]=1;
		  break;
	       }
	    }
	 }
	 
      } // End of loop over events

      hlp->Fill(pattLefts);
      hrp->Fill(pattRights);
      hcp->Fill(pattCoins);
      meanl += pattLefts;
      meanr += pattRights;
      meanc += pattCoins;
      ++n;
   } // End of loop over patterns
   meanl /= n; meanr /= n; meanc /= n;
   gStyle->SetOptFit(111111);
   gStyle->SetStatX(0.525);
   gStyle->SetStatW(0.200);
   gStyle->SetStatY(0.850);
   gStyle->SetStatH(0.125);

   TCanvas *clevtt = new TCanvas(Form("%d_LeftEventTimeGraph", run),Form("Run %d Left Event Time Graph",run),0,0,1400,600); //Time Diff canvas & plotting
   clevtt->SetLeftMargin(0.150);
   clevtt->SetRightMargin(0.050);
   fTimeFitLeft->SetParameter(0,hlevtt->GetMean());
   fTimeFitLeft->SetParameter(1,hlevtt->GetMean());
   int fTimeFitLeftStatus = hlevtt->Fit(fTimeFitLeft,"R");
   hlevtt->Draw("");
   gPad->Update();
   TPaveStats *stl = (TPaveStats*)hlevtt->FindObject("stats"); //Stats box position
   stl->SetX1NDC(0.6);
   stl->SetY1NDC(0.5);
   stl->SetX2NDC(0.85);
   stl->SetY2NDC(0.85);
   stl->Draw();
   gPad->Update();
   //hlevtt->GetListOfFunctions()->FindObject("fTimeFitLeft");
   double betaParamLeft = 0.0;
   double betaParamRateLeft = 0.0;
   double betaParamErrLeft = 0.0;
   double betaParamErrRateLeft = 0.0;
   if(fTimeFitLeftStatus==0){
      betaParamLeft = fTimeFitLeft->GetParameter(1);
      betaParamRateLeft = (double)1000000000/betaParamLeft;
      betaParamErrLeft = fTimeFitLeft->GetParError(1);
      betaParamErrRateLeft = (betaParamRateLeft/betaParamLeft)*betaParamErrLeft;
   }
   
   TPaveText *rtl = new TPaveText(0.6,0.45,0.85,0.5,"blNDC"); //Additional box (Rate)
   rtl->SetBorderSize(1);
   rtl->SetFillColor(0);
   rtl->AddText(Form("p1 Rate (Hz)                  %.1f #pm %.1f",betaParamRateLeft,betaParamErrRateLeft));
   rtl->SetTextFont(43);
   rtl->SetTextSize(19);
   rtl->Draw();

   TCanvas *crevtt = new TCanvas(Form("%d_RightEventTimeGraph", run),Form("Run %d Right Event Time Graph",run),0,0,1400,600); //Time Diff canvas & plotting
   crevtt->SetLeftMargin(0.150);
   crevtt->SetRightMargin(0.050);
   fTimeFitRight->SetParameter(0,hrevtt->GetMean());
   fTimeFitRight->SetParameter(1,hrevtt->GetMean());
   int fTimeFitRightStatus = hrevtt->Fit(fTimeFitRight,"R");
   hrevtt->Draw("");
   gPad->Update();
   TPaveStats *str = (TPaveStats*)hrevtt->FindObject("stats"); //Stats box position
   str->SetX1NDC(0.6);
   str->SetY1NDC(0.5);
   str->SetX2NDC(0.85);
   str->SetY2NDC(0.85);
   str->Draw();
   gPad->Update();
   //hrevtt->GetListOfFunctions()->FindObject("fTimeFitRight");
   double betaParamRight = 0.0;
   double betaParamRateRight = 0.0;
   double betaParamErrRight = 0.0;
   double betaParamErrRateRight = 0.0;
   if(fTimeFitRightStatus==0){
      betaParamRight = fTimeFitRight->GetParameter(1);
      betaParamRateRight = (double)1000000000/betaParamRight;
      betaParamErrRight = fTimeFitRight->GetParError(1);
      betaParamErrRateRight = (betaParamRateRight/betaParamRight)*betaParamErrRight;
   }
   TPaveText *rtr = new TPaveText(0.6,0.45,0.85,0.5,"blNDC"); //Additional box (Rate)
   rtr->SetBorderSize(1);
   rtr->SetFillColor(0);
   rtr->AddText(Form("p1 Rate (Hz)                  %.1f #pm %.1f",betaParamRateRight,betaParamErrRateRight));
   rtr->SetTextFont(43);
   rtr->SetTextSize(19);
   rtr->Draw();

   TCanvas *ccevtt = new TCanvas(Form("%d_CoinEventTimeGraph", run),Form("Run %d Coin Event Time Graph",run),0,0,1400,600); //Time Diff canvas & plotting
   ccevtt->SetLeftMargin(0.150);
   ccevtt->SetRightMargin(0.050);
   fTimeFitCoin->SetParameter(0,hcevtt->GetMean());
   fTimeFitCoin->SetParameter(1,hcevtt->GetMean());
   int fTimeFitCoinStatus = hcevtt->Fit(fTimeFitCoin,"R");
   hcevtt->Draw("");
   gPad->Update();
   TPaveStats *stc = (TPaveStats*)hcevtt->FindObject("stats"); //Stats box position
   stc->SetX1NDC(0.6);
   stc->SetY1NDC(0.5);
   stc->SetX2NDC(0.85);
   stc->SetY2NDC(0.85);
   stc->Draw();
   gPad->Update();
   double betaParamCoin = 0.0;
   double betaParamRateCoin = 0.0;
   double betaParamErrCoin = 0.0;
   double betaParamErrRateCoin = 0.0;
   if(fTimeFitCoinStatus==0){
      betaParamCoin = fTimeFitCoin->GetParameter(1);
      betaParamRateCoin = (double)1000000000/betaParamCoin;
      betaParamErrCoin = fTimeFitCoin->GetParError(1);
      betaParamErrRateCoin = (betaParamRateCoin/betaParamCoin)*betaParamErrCoin;
   }
   TPaveText *rtc = new TPaveText(0.6,0.45,0.85,0.5,"blNDC"); //Additional box (Rate)
   rtc->SetBorderSize(1);
   rtc->SetFillColor(0);
   rtc->AddText(Form("p1 Rate (Hz)                  %.1f #pm %.1f",betaParamRateCoin,betaParamErrRateCoin));
   rtc->SetTextFont(43);
   rtc->SetTextSize(19);
   rtc->Draw();

   TCanvas *crate = new TCanvas(Form("%d_Rate_Plots", run),Form("FADC Run %d Raw Rate Plots", run),1600,500);
   crate->Divide(3,1);

   crate->cd(1);
   hlp->SetTitle(Form("%d Left Singles per Pattern",run));
   hlp->GetXaxis()->SetTitle("Lefts per Pattern");
   double leftMean = 0.0;
   double leftErr = 0.0;
   TF1 *fLeftGaus = new TF1("fLeftGaus", "gaus");
   int fLeftGausStatus = hlp->Fit(fLeftGaus);
   if(fLeftGausStatus==0){
      leftMean = fLeftGaus->GetParameter(1);
      leftErr = fLeftGaus->GetParError(1);
   }
   hlp->Draw("");
   
   crate->cd(2);
   hrp->SetTitle(Form("%d Right Singles per Pattern",run));
   hrp->GetXaxis()->SetTitle("Rights per Pattern");
   double rightMean = 0.0;
   double rightErr = 0.0;
   TF1 *fRightGaus = new TF1("fRightGaus", "gaus");
   int fRightGausStatus = hrp->Fit(fRightGaus);
   if(fRightGausStatus==0){
      rightMean = fRightGaus->GetParameter(1);
      rightErr = fRightGaus->GetParError(1);
   }
   hrp->Draw("");

   crate->cd(3);
   hcp->SetTitle(Form("%d Coin. Rate per Pattern",run));
   hcp->GetXaxis()->SetTitle("Coin. Rate per Pattern");
   double coinMean = 0.0;
   double coinErr = 0.0;
   TF1 *fCoinGaus = new TF1("fCoinGaus", "gaus");
   int fCoinGausStatus = hcp->Fit(fCoinGaus);
   if(fCoinGausStatus==0){
      coinMean = fCoinGaus->GetParameter(1);
      coinErr = fCoinGaus->GetParError(1);
   }
   hcp->Draw("");

   std::cout<<std::endl;
   std::cout<<"Left  rate from DelT fit: "<<betaParamRateLeft<<" ± "<<betaParamErrRateLeft<<" Hz"<<std::endl;
   std::cout<<"Right rate from DelT fit: "<<betaParamRateRight<<" ± "<<betaParamErrRateRight<<" Hz"<<std::endl;
   std::cout<<"Coin  rate from DelT fit: "<<betaParamRateCoin<<" ± "<<betaParamErrRateCoin<<" Hz"<<std::endl;
   std::cout<<"Left  rate from gaus fit: "<<leftMean*flipRate/(double)patternType<<" ± "<<leftErr*flipRate/(double)patternType<<" Hz"<<std::endl;
   std::cout<<"Right rate from gaus fit: "<<rightMean*flipRate/(double)patternType<<" ± "<<rightErr*flipRate/(double)patternType<<" Hz"<<std::endl;
   std::cout<<"Coin  rate from gaus fit: "<<coinMean*flipRate/(double)patternType<<" ± "<<coinErr*flipRate/(double)patternType<<" Hz"<<std::endl;
   printf("Left  rate from hist mean: %0.2f Hz\n",meanl*flipRate/(double)patternType);
   printf("Right  rate from hist mean: %0.2f Hz \n",meanr*flipRate/(double)patternType);
   printf("Coin  rate from hist mean: %0.2f Hz\n",meanc*flipRate/(double)patternType);

   auto end = std::chrono::high_resolution_clock::now();
   auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
   std::cout<<"Time: "<<duration.count()<<" milliseconds"<<endl;
   /*int nb = hcevtt->FindFirstBinAbove(hcevtt->GetEntries()/3.);
   int nmissing=0;
   for(int i=nb+5;i<hcevtt->GetNbinsX();i++){
      int num = (int)(double(i-nb)/(double)nb + 0.4);
      nmissing += hcevtt->GetBinContent(i)*num;
      if(hcevtt->GetBinContent(i)>0)cout<<i<<" "<<nb<< " "<<i<<" "<<num<<" "<<hcevtt->GetBinContent(i)<<endl;
   }
   cout<<nmissing<<" events missing out of "<<nTreeEntries<<" total events ("<<nmissing/double(nTreeEntries)*100<<"%)."<<endl;
   cout<<"Rates corrected for missing events:\n";
   double cor = 1+ nmissing/(double)hcevtt->GetEntries();
   printf("Left  rate from hist mean: %0.2f Hz\n",meanl*flipRate/(double)patternType*cor);
   printf("Right  rate from hist mean: %0.2f Hz \n",meanr*flipRate/(double)patternType*cor);
   printf("Coin  rate from hist mean: %0.2f Hz\n",meanc*flipRate/(double)patternType*cor);*/
}
