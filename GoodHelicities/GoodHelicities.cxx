#include <GoodHelicities.h>
#include <algorithm>
#include <iostream>
#include <TFile.h>
#include <TChain.h>
#include <TH1D.h>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <TSystem.h>

ClassImp(window_t)
ClassImp(GoodHelicities)



// =============================
// Constructor / Destructor
// =============================

GoodHelicities::GoodHelicities() : TObject()
{
}

GoodHelicities::~GoodHelicities()
{
}



// =============================
void GoodHelicities::ClearPatterns()
{//Empty the vector of HelPat_t variables with pattern information
    fPatterns.clear();
}

// =============================
void GoodHelicities::CreateTTrain(int run, const char* fnamebase)
{//Create TTrain from run number
   fRun = run;
   train = std::make_unique<TTrain>();
   std::cout<<"Setting up TTrain for run "<< run <<std::endl;
   
   for (int seg = 0; seg < 100; ++seg) {
      for (int x = 0; x < 100; ++x) {
	 TString filename(Form("%s/%s%i.%i",
			       gSystem->Getenv("HAMOLLER_ROOTFILE_DIR"),
			       fnamebase, run, seg));
	 if (x > 0) filename += Form("_%i", x);
	 filename += ".root";

	 if (gSystem->AccessPathName(filename)) break;
	 std::cout << filename << std::endl;

	 train->AddFile(filename, "T");
      }
   }

   Long64_t nEntries = train->GetEntries();
   std::cout << nEntries << " total entries." << std::endl;
   if (nEntries == 0) {
      std::cout << "No entries found. Exiting." << std::endl;
      return;
   }
}

// =============================
void GoodHelicities::SetPatternType(const std::string& type)
{//Set the helicity pattern: "octet", "quartet"...
   fPatternType = type;
   std::cout<<"Using pattern type "<<fPatternType<<std::endl;
}

// =============================
void GoodHelicities::SetDelayWindows(int d)
{//Number of helicity windows reported helicity is delayed.
    fDelayWindows = d;
}

// =============================
void GoodHelicities::SetRunNumber(int run)
{//Run number used to create and find files by name.
    fRun = run;
}

// =============================
void GoodHelicities::SetCoincWindow(double dt)
{//Absolute value of time difference between left and right to
 //be considered a coincidence pair.
    fCOINC_DT = dt;
}

// =============================
double GoodHelicities::GetCoincWindow() const
{//Absolute value of time difference between left and right to
 //be considered a coincidence pair.
    return fCOINC_DT;
}

void GoodHelicities::SetSumTimeDiff(double dt)
{//Time difference in nanoseconds to be considered part of 
 // the same event included in the left and right sums.
    fSUM_DT = dt;
}

double GoodHelicities::GetSumTimeDiff() const
{//Time difference in nanoseconds to be considered part of 
 // the same event included in the left and right sums.
     return fSUM_DT;
}

// =============================
void GoodHelicities::SetTsettleFrequency(double freq)
{//Helicity flip frequency
    fTsettleFreq = freq;
}

// =============================
double GoodHelicities::GetADCchanThreshold() const
{//ADC threshold for acceptance
   return fADCthreshold;
}

// =============================
void GoodHelicities::SetNSA(double nsa)
{//number of FADC samples after threshold crossing included in integral
   fNSA = nsa;
   fPEDFRAC = (fNSA+fNSB)/fNPED;
}

// =============================
void GoodHelicities::SetNSB(double nsb)
{//number of FADC samples included in pedestal integral
   fNSB = nsb;
   fPEDFRAC = (fNSA+fNSB)/fNPED;
}

// =============================
void GoodHelicities::SetNPED(double nped)
{//number of FADC samples after threshold crossing included in integral
   fNPED = nped;
   fPEDFRAC = (fNSA+fNSB)/fNPED;
}

// =============================
void GoodHelicities::SetADCchanThreshold(double thresh)
{//ADC threshold for acceptance
   fADCthreshold = thresh;
}

// =============================
const std::vector<HelPat_t>& GoodHelicities::GetPatterns() const
{//vector of HelPat_t variables with pattern information
    return fPatterns;
}

// =============================
// LFSR
// =============================

// =============================
void GoodHelicities::LFSR30::SeedBit(int bit)//feed one bit at a time
{
    if(seeded) return;  // already fully seeded
    state <<= 1;

    if(bit > 0) state |= 1;
    if(bit_count == 0) std::cout<<"Helicity seed: {";
    bit_count++;
    std::cout<<bit<<(bit_count < 30 ? ",":"}\n");
    if(bit_count >= 30){
       std::cout<<"Helicity seeding complete."<<std::endl;
       seeded = true;
    }
}

// =============================
void GoodHelicities::LFSR30::Seed(const std::vector<int>& bits)//feed all 30 bits at once
{
    state = 0;
    for(int i=0;i<30;i++){
        state <<= 1;
        if(bits[i] > 0) state |= 1;
    }
    seeded = true;
}

// =============================
int GoodHelicities::LFSR30::Next()
{
    int newbit =
        ((state >> 29) ^
         (state >> 28) ^
         (state >> 27) ^
         (state >> 6)) & 1;
    state = ((state << 1) | newbit) & 0x3fffffff;
    //std::cout<<"Next helicity: "<<((state & 1) ? +1 : -1)<<std::endl;
    return (state & 1) ? +1 : -1;
}

// =============================
void GoodHelicities::LFSR30::Reset()
{
    std::cout<<"Resetting helicity seed at ";
    seeded = false;
    state = 0;
    bit_count = 0;
}

// =============================
std::vector<int> GoodHelicities::GetBasePattern(const std::string &type) const
{
    if(type=="pair")        return {+1,-1};
    if(type=="quartet")     return {+1,-1,-1,+1};
    if(type=="octet")       return {+1,-1,-1,+1, -1,+1,+1,-1};
    if(type=="hexoquad")    return {+1,-1,-1,+1, +1,-1,-1,+1, +1,-1,-1,+1,
                                    -1,+1,+1,-1, -1,+1,+1,-1, -1,+1,+1,-1};
    if(type=="octoquad")    return {+1,-1,-1,+1, +1,-1,-1,+1, +1,-1,-1,+1, +1,-1,-1,+1,
				    -1,+1,+1,-1, -1,+1,+1,-1, -1,+1,+1,-1, -1,+1,+1,-1 };
    if(type=="toggle")      return {}; // handled separately
    return {};
}

// =============================
void GoodHelicities::SavePatterns(int run)
{  //Save helicity pattern information to a TTree and store in a root file.
   /////////////////////////////////////////////////////////////////////////
   
   if(fPatterns.size()==0){
      std::cout<<"No patterns found."<<std::endl;
      return;
   }
   std::cout<<"Saving helicity pattern information to TTree."<<std::endl;
   TFile *f = TFile::Open(Form("%s/patterns/helicity_patterns_%i.root",
			       gSystem->Getenv("HAMOLLER_ROOTFILE_DIR"),run), "RECREATE");
   f->cd();// make sure we're in the file
   TTree *t = new TTree("TPat", "Helicity Patterns");
   HelPat_t pat;
   t->Branch("pattern", &pat);
   for (const auto& p : fPatterns) {
      pat = p;
      t->Fill();
   }
   t->Write("", TObject::kOverwrite);// writes the new tree
   f->Close();
}

// =============================
void GoodHelicities::FillLeftRightSumTree(int runnum, const char* fnamebase)
{
   fRun = runnum;
   CreateTTrain(runnum, fnamebase);
   FillLeftRightSumTree(train.get(), runnum);
}

// =============================
void GoodHelicities::SetOverwrite(bool over)
{  //Set to overwrite existing TTrees of helicity, pedestal or left/right sums. Default is true.
   fOverwrite = over;
}


// =============================
void GoodHelicities::FillLeftRightSumTree(TTrain *tr, int runnum)
{  // Each entry in the TTrain contains an event comprising one or more channels.
   // A channel is read out if it is above threshold at any time during the readout window.
   // The readout window is typically in the 150-250ns long and usually is chosen to start
   // several tens of ns before the trigger. Up to 4 pulses per channel can be found by the
   // firmware in a given event. This method groups pulses in the left detector and right
   // detector into left/right events and left/right sums if they lie within fSUM_DT ns of
   // each other. It also checks for coincidences between the left and right sums if they
   // are within fCOINC_DT ns of each other.
   /////////////////////////////////////////////////////////////////////////////////////////

    //If helicities and patterns not yet found, find them.
    ///////////////////////////////////////////////////////
    if(fPatterns.size()==0){

       //Finding the helicity information in an existing TTree
       //or overwrite existing information in TTree?
       ////////////////////////////////////////////////////////

       //*Try to find it in an existing TTree
       //////////////////////////////////////
       if(!fOverwrite){
	  TTree* trPat;
	  TFile fpat(Form("%s/patterns/helicity_patterns_%i.root",
		       gSystem->Getenv("HAMOLLER_ROOTFILE_DIR"),runnum), "READ");


	  //*** If pattern TFile not found, make it.
	  if (fpat.IsZombie()) {
	     std::cout << "Existing helicity tree file not found." 
		       <<" Finding helicities windows and patterns."<<std::endl;
	     FindGoodHelicities(tr);

          //*** Otherwise find TTree in the TFile.
	  }else{                           
	     trPat = (TTree*)fpat.Get("TPat");
	  }

	  //*** If pattern TTree not found, make it.
	  if (!trPat) {
	     std::cout << "Pattern TTree not found. Creating it."<<std::endl;
	     FindGoodHelicities(tr);

	  //*** If TFile and TTree found extract pattern info
	  }else{                   
	     HelPat_t* pat = nullptr;
	     trPat->SetBranchAddress("pattern", &pat);
	     Long64_t nentries = trPat->GetEntries();
	     for (Long64_t i = 0; i < nentries; i++) {
		trPat->GetEntry(i);
		if (!pat) continue;
		fPatterns.push_back(*pat);  // copy into vector
	     }
	  }

       //*Or find the pattern information and save it to a TTree.
       //////////////////////////////////////////////////////////	  
       }else{
	  std::cout<<"Finding helicities windows and patterns."<<std::endl;
	  FindGoodHelicities(tr);
       }
    }
    std::cout<<fPatterns.size()<<" patterns found for run "<<runnum<<std::endl;
    Double_t samples[fNMAXCHAN * 100]; 
    Int_t nsamp_total;
    Long64_t nentries = tr->GetEntries();

    //Open file and create tree for writing
    ////////////////////////////////////////
    std::string fileName(gSystem->Getenv("HAMOLLER_ROOTFILE_DIR"));
    fileName.append("/LeftRightSumTree_");
    fileName += std::to_string(runnum) + ".root";
    const size_t maxFileSize = 20ULL * 1024 * 1024 * 1024; // 20 GB

    TFile* file = TFile::Open(fileName.c_str(), "RECREATE");
    TTree* tree = new TTree("T", "Left/Right Detector Sums Tree");
    tree->SetMaxTreeSize(maxFileSize);

    bool coinc;
    int hel, patnum, Nlchan, Nrchan, patpol, windnum, event, pileup_order, nTotalChan, trig;
    double leftsum, rightsum, leftsum_t, rightsum_t;
    std::vector<double>lfadc, lfadc_t, lfadc_chan, lped;
    std::vector<double>rfadc, rfadc_t, rfadc_chan, rped;
    tree->Branch("event", &event, "event/I")->SetTitle("Event/trigger number in run.");
    tree->Branch("hel", &hel, "hel/I")->SetTitle("Helicity of current window");
    tree->Branch("patpol", &patpol, "patpol/I")->SetTitle("Pattern polarity (+/-1)");
    tree->Branch("patnum", &patnum, "patnum/I")->SetTitle("Pattern number");
    tree->Branch("windownum", &windnum, "windownum/I")->SetTitle("Window number inside pattern: 0 to patlen-1");
    tree->Branch("nleftchan", &Nlchan, "nleftchan/I")->SetTitle("Number of channels in left events");
    tree->Branch("leftadc", &lfadc)->SetTitle("ADC integrals for left events.");
    tree->Branch("leftadc_t", &lfadc_t)->SetTitle("ADC times(ns) in look back window for left events.");
    tree->Branch("leftadc_chan", &lfadc_chan)->SetTitle("Channel numbers for left hits.");
    tree->Branch("leftadc", &lfadc)->SetTitle("ADC integrals for nchan events.");
    tree->Branch("leftadc_t", &lfadc_t)->SetTitle("ADC times(ns) in look back window for nchan events.");
    tree->Branch("leftsum", &leftsum, "Lsum/D")->SetTitle("Sum of ADC integrals in left detector.");
    tree->Branch("leftsum_t", &leftsum_t, "Lsum_t/D")->SetTitle("Average ADC time for events in left detector.");
    tree->Branch("nrightchan", &Nrchan, "nrightchan/I")->SetTitle("Number of channels in right events");
    tree->Branch("rightadc", &rfadc)->SetTitle("ADC integrals for right events.");
    tree->Branch("rightadc_t", &rfadc_t)->SetTitle("ADC times(ns) in look back window for right events.");
    tree->Branch("rightadc_chan", &rfadc_chan)->SetTitle("Channel numbers for right hits.");
    tree->Branch("rightadc", &rfadc)->SetTitle("ADC integrals for nchan events.");
    tree->Branch("rightadc_t", &rfadc_t)->SetTitle("ADC times(ns) in look back window for nchan events.");
    tree->Branch("rightsum", &rightsum, "Rsum/D")->SetTitle("Sum of ADC integrals in right detector.");
    tree->Branch("rightsum_t", &rightsum_t, "Rsum_t/D")->SetTitle("Average ADC time for events in right detector.");
    tree->Branch("coinc", &coinc, "coinc/O")->SetTitle("Left/Right coincidence event?");
    tree->Branch("nTotalChan", &nTotalChan, "nTotalChan/I")->SetTitle("Total number of channels read out for this event");
    tree->Branch("pileup_order", &pileup_order, "pileup_order/I")->SetTitle("For multiple pileup pulses inside an event where is this one in the order?");
    tree->Branch("trig", &trig, "trig/I")->SetTitle("Trigger information bits: 001= coinc trigger, 010=leftsum trigger, 100=rightsum trigger");
    Double_t adc[fNMAXCHAN], adc_t[fNMAXCHAN], chan[fNMAXCHAN], ped[fNCHAN], pedchan[fNCHAN];
    Int_t nchan, npedchan;
    ULong64_t tEvt;

    tr->SetBranchStatus("*", 0);
    tr->SetBranchStatus("M.cal.adc", 1);
    tr->SetBranchStatus("M.cal.adc_t", 1);
    tr->SetBranchStatus("M.cal.adc_chan", 1);
    tr->SetBranchStatus("Ndata.M.cal.adc_chan", 1);
    tr->SetBranchStatus("fEvtHdr.fEvtTime", 1);
    tr->SetBranchStatus("M.cal.ped", 1);
    tr->SetBranchStatus("M.cal.chan", 1);
    tr->SetBranchStatus("Ndata.M.cal.chan", 1);
    tr->SetBranchAddress("Ndata.M.cal.chan", &npedchan);
    tr->SetBranchAddress("M.cal.ped", ped);
    tr->SetBranchAddress("M.cal.chan", pedchan);
    tr->SetBranchAddress("fEvtHdr.fEvtTime", &tEvt);
    tr->SetBranchAddress("M.cal.adc", adc);
    tr->SetBranchAddress("M.cal.adc_t", adc_t);
    tr->SetBranchAddress("M.cal.adc_chan", chan);
    tr->SetBranchAddress("Ndata.M.cal.adc_chan", &nchan);

    //Access Pedestal Friend Tree for corrected pedestals if available
    ///////////////////////////////////////////////////////////////////
    std::string fname = std::string(gSystem->Getenv("HAMOLLER_ROOTFILE_DIR"))
        + "/PedestalFriend_" + std::to_string(runnum) + ".root";
    TFile* filefr = TFile::Open(fname.c_str(), "READ");
    bool raw_ped = false;
    Double_t ped_corr[fNMAXCHAN];
    Int_t ped_corr_chan[fNMAXCHAN];
    Int_t nped_corr;
    Bool_t ped_changed[fNMAXCHAN];

    //Use corrected pedestal if: 1. the file exists and 2. the tree exists inside the file and
    //                           3. the tree has the corrects number of entries.
    ///////////////////////////////////////////////////////////////////////////////////////////
    TTree* trPed;
    if (!filefr || filefr->IsZombie()) {//1. Check if file exists
       std::cout << "Error opening corrected pedestal friend file. Going with raw pedestals."<<std::endl;
       raw_ped = true;
    }else{//2. Check if tree exists
       trPed = (TTree*)filefr->Get("T");
       if (!trPed) {
	  std::cout <<"Pedestal friend TTree not found. Going with raw pedestals."<<std::endl;
	  raw_ped = true;
       }else if(trPed->GetEntries()!=tr->GetEntries()){//3. Check if number of entries correct
	  std::cout<<"Number of entries in Pedestal Friend tree not equal to main event TTree."
		   <<" Going with raw pedestals."<<std::endl;
	  raw_ped = true;
       }else{//All good. Going with corrected pedestals.
	  std::cout << "Entries: " << trPed->GetEntries() << std::endl;
	  trPed->SetBranchAddress("nped_corr",&nped_corr);
	  trPed->SetBranchAddress("ped_corr", ped_corr);
	  trPed->SetBranchStatus("ped_changed", 0);
	  trPed->SetBranchAddress("ped_corr_chan", ped_corr_chan);
       }

    }

    patnum = -1;
    int idx[fNMAXCHAN]={};
    std::cout<<"Building left/right sum tree."<<std::endl;
    
    //Loop over TTrain using helicity and pattern information from fPatterns
    /////////////////////////////////////////////////////////////////////////
    for(const HelPat_t& pat : fPatterns){
       ++patnum;
       if(!pat.good_pattern){
	  std::cout<<"Skipping pattern "<<patnum<<std::endl;
	  continue;//skip patterns that aren't good
       }
       std::cout<<"Processing pattern "<<patnum<<" of "<<fPatterns.size()<<std::endl;
       patpol = pat.polarity;//polarity of pattern +/-1
       //Loop over each helicity window inside the pattern
       ////////////////////////////////////////////////////
       for (size_t win = 0; win < pat.helicity.size(); win++){
	  hel = pat.helicity[win];
	  windnum = win;
          //Process TTrain entries contained in a single helicity window
	  ///////////////////////////////////////////////////////////////
	  for(Long64_t i = pat.window_start[win];i<=pat.window_end[win];++i){
	     tr->GetEntry(i);
	     event = (Int_t)i;
	     //Get the pedestals for the PMT channels
	     /////////////////////////////////////////
	     double pedest[(int)fNDET] = {};
	     if( raw_ped ){
		for (int k = 0; k < npedchan; k++) {
		   int ch = (int)pedchan[k];
		   if(ch >= fNDET)break;
		   pedest[ch] = ped[k]*fPEDFRAC;
		}
	     }else{
		trPed->GetEntry(i);
		for (int k = 0; k < nped_corr; k++) {
		   int ch = (int)ped_corr_chan[k];
		   if(ch >= fNDET) break;
		   pedest[ch] = ped_corr[k];
		}
	     }

	     
	     // Record active trigger channels
	     trig = 0;
	     for (int k = 0; k < npedchan; ++k){
		if(pedchan[k]==13)trig |= (1 << 0);
		if(pedchan[k]==14)trig |= (1 << 1);
		if(pedchan[k]==15){
		   trig |= (1 << 2);
		   break;
		}
	     }
	     
	     //Start by sorting arrays in time order by creating a time-ordered index
	     /////////////////////////////////////////////////////////////////////////
	     if(nchan > fNMAXCHAN){
		std::cout<<"Error. Number of channels firing "<<nchan
			 <<" exceeds maximum. Exiting."<<std::endl;
		return;
	     }
	     
	     // PMT channels
	     int nch = 0;
	     for (int k = 0; k < nchan; ++k){
		if(chan[k]>=fNDET)break;
		++nch;
		idx[k] = k;
	     }
	     //Sort PMT pulses by time	
	     std::sort(idx, idx + nch, [&](int x, int y) {
		return adc_t[x] < adc_t[y];
	     });
	     if(i%100000==0&&0){
		std::cout<<i<<" "<<pat.window_end[win]<<" "<<nchan<<std::endl;
		for(int x=0;x<nch;++x)std::cout<<idx[x]<<" "<<adc_t[idx[x]]<<", ";
		std::cout<<std::endl;
	     }
	     bool used[fNMAXCHAN] = {};
	     pileup_order = 0;
	     nTotalChan = nchan;
	     for(int j=0;  j<nch; ++j){
		rightsum=leftsum=rightsum_t=leftsum_t=0;
		int nl = 0, nr = 0;
		lfadc.clear(); lfadc_t.clear(); lfadc_chan.clear();
		rfadc.clear(); rfadc_t.clear(); rfadc_chan.clear();

		int chj = (int)chan[idx[j]];
		if(used[idx[j]])continue; //don't use a pulse already included in another event
		if(chj < fNDET/2.0){
		   if(pedest[chj]/fPEDFRAC>4000 && 0)std::cout<<i<<" ch:"<<chj<<" adc["<<idx[j]<<"]: "
					  <<adc[idx[j]]<<" ped:"<<pedest[chj]/fPEDFRAC<<std::endl;
		   leftsum = adc[idx[j]]-pedest[chj];
		   leftsum_t = adc_t[idx[j]]-fMeanTrigTime[chj];
		   lfadc.push_back(adc[idx[j]]-pedest[chj]);
		   lfadc_t.push_back(adc_t[idx[j]]);
		   lfadc_chan.push_back((double)chj);
		   nl++;
		}else if(chj < fNDET){
		   if(pedest[chj]/fPEDFRAC>4000)std::cout<<i<<" ch:"<<chj<<" adc["<<idx[j]<<"]: "
					  <<adc[idx[j]]<<" ped:"<<pedest[chj]/fPEDFRAC<<std::endl;
		   rightsum = adc[idx[j]]-pedest[chj];
		   rightsum_t = adc_t[idx[j]]-fMeanTrigTime[chj];
		   rfadc.push_back(adc[idx[j]]-pedest[chj]);
		   rfadc_t.push_back(adc_t[idx[j]]);
		   rfadc_chan.push_back((double)chj);
		   nr++;
		}
		
		for(int k = j+1; k<nch; k++){
		   int chk = (int)chan[idx[k]];
		   if(chk > fNDET-1) break;//continue;
		   double dt = (adc_t[idx[k]]-fMeanTrigTime[chk]) - (adc_t[idx[j]]-fMeanTrigTime[chj]);
		   if( abs(dt) <= fSUM_DT ){
		      if(chk < fNDET/2.0){
			 if(pedest[chk]/fPEDFRAC>4000)std::cout<<i<<" ch:"<<chk<<" adc["<<idx[k]<<"]: "
						<<adc[idx[k]]<<" ped:"<<pedest[chk]/fPEDFRAC<<std::endl;
			 leftsum += adc[idx[k]]-pedest[chk]; //sum ADCs
			 leftsum_t += adc_t[idx[k]]-fMeanTrigTime[chk]; //average time
			 lfadc.push_back(adc[idx[k]]-pedest[chk]);
			 lfadc_t.push_back(adc_t[idx[k]]);
			 lfadc_chan.push_back(chk);
			 used[idx[k]] = true; //mark as already used
			 nl++;
		      }else if(chk < fNDET){
			 if(pedest[chk]/fPEDFRAC>4000)std::cout<<i<<" ch:"<<chk<<" adc["<<idx[k]<<"]: "
						<<adc[idx[k]]<<" ped:"<<pedest[chk]/fPEDFRAC<<std::endl;
			 rightsum += adc[idx[k]]-pedest[chk]; //sum ADCs    
			 rightsum_t += adc_t[idx[k]]-fMeanTrigTime[chk]; //average time
			 rfadc.push_back(adc[idx[k]]-pedest[chk]);
			 rfadc_t.push_back(adc_t[idx[k]]);
			 rfadc_chan.push_back(chk);
			 used[idx[k]] = true; //mark as already used
			 nr++;
		      }
		   }
		}//end of k-loop over event
		//
		
		//Convert sum times to seconds from run start. Leave adc_t in ns from trigger.
		///////////////////////////////////////////////////////////////////////////////
		leftsum_t  /= double(nl);  //calculate average (in ns)
		rightsum_t /= double(nr);  //calculate average (in ns)
		coinc = (abs(leftsum_t - rightsum_t) < fCOINC_DT  ? 1:0);//check for coincidence
		leftsum_t  *= 1e-9;  //convert to seconds
		rightsum_t *= 1e-9;  //convert to seconds
		leftsum_t  += tEvt*4e-9; //time in seconds since beginning of run
		rightsum_t += tEvt*4e-9; //time in seconds since beginning of run
		Nlchan = lfadc.size();//record number of left channels summed in this event
		Nrchan = rfadc.size();//record number of right channels summed in this event
		
		// Check if above software threshold and fill TTree
		////////////////////////////////////////////////////
		if(leftsum >= fSumThreshold && rightsum >= fSumThreshold){
		   //Both above threshold. 
		   tree->Fill();
		   pileup_order++;
		}else if(leftsum >= fSumThreshold &&  rightsum < fSumThreshold){
		   //Only left above threshold
		   rightsum = kError;
		   rightsum_t = kError;
		   coinc = false;
		   tree->Fill();
		   pileup_order++;
		}else if(leftsum < fSumThreshold &&  rightsum >= fSumThreshold){
		   //Only right above threshold
		   leftsum = kError;
		   leftsum_t = kError;
		   coinc = false;
		   tree->Fill();
		   pileup_order++;
		}else{
		   leftsum = kError;
		   rightsum = kError;
		   rightsum_t = kError;
		   leftsum_t = kError;
		   coinc = false;
		   tree->Fill();
		}
		
	     }//end of j-loop over event
	  }//end of loop over entries inside single helicity window
       }//end of loop over single windows inside patterns
    }//end of loop over patterns

    // --- Write and close ---
    file->cd();
    tree->Write();
    file->Close();

    std::cout << "Saved TTree to " << fileName << std::endl;
}


// =============================
void GoodHelicities::FindGoodHelicities(int runnum, const char* fnamebase, bool pedsub)
{
   fRun = runnum;
   CreateTTrain(runnum, fnamebase);
   FindGoodHelicities(train.get(), pedsub);
}

// =============================
void GoodHelicities::FindGoodHelicities(TTrain *tr, bool ped_sub)
{//Usually you would not pedestal subract the Tsettle, helicity and QRT signals
 //since they are NIM level DC voltages, but included for flexibility.
    fPatterns.clear();
    if(!tr){
       std::cout<<"No TTrain found. Exiting."<<std::endl;
       return;
    }
    std::cout<<"Using pattern type "<<fPatternType.data()<<std::endl;
    std::cout<<"Using and a delayed helicity reporting of "<<fDelayWindows<<" windows"<<std::endl;
    std::cout<<"Using helicity flip frequency of "<<fTsettleFreq<<" Hz"<<std::endl;

    std::vector<window_t> windows;

    Double_t adc[fNMAXCHAN], chan[fNMAXCHAN], ped[fNCHAN], pedchan[fNCHAN];
    Int_t nchan, npedchan;
    ULong64_t tEvt;

    tr->SetBranchStatus("*",0);
    tr->SetBranchStatus("M.cal.adc",1);
    tr->SetBranchStatus("M.cal.adc_chan",1);
    tr->SetBranchStatus("Ndata.M.cal.adc",1);
    tr->SetBranchStatus("fEvtHdr.fEvtTime", 1);

    tr->SetBranchAddress("fEvtHdr.fEvtTime", &tEvt);
    tr->SetBranchAddress("M.cal.adc", adc);
    tr->SetBranchAddress("M.cal.adc_chan", chan);
    tr->SetBranchAddress("Ndata.M.cal.adc", &nchan);
    if(ped_sub){
       tr->SetBranchStatus("M.cal.ped",1);
       tr->SetBranchStatus("M.cal.chan",1);
       tr->SetBranchStatus("Ndata.M.cal.chan",1);
       tr->SetBranchAddress("Ndata.M.cal.chan", &npedchan);
       tr->SetBranchAddress("M.cal.ped", ped);
       tr->SetBranchAddress("M.cal.chan", pedchan);
    }
    Long64_t nentries = tr->GetEntries();
    bool prevTsettle=true;
    bool inWindow=false;
    double time_sec=0, prev_time_sec=0;
    window_t current;

    //Loop over TTrain finding all helicity windows and store them in window_t types
    /////////////////////////////////////////////////////////////////////////////////
    std::cout<<"1). Finding start and end of each helicity window."<<std::endl;
    for(Long64_t i=0;i<nentries;i++)
    {
        tr->GetEntry(i);
	double pedest[fNCHAN] = {};
        bool Tsettle=false;
        bool QRT=false;
        bool helicityBit=false;
	if(ped_sub){
	   for(int k=0; k<npedchan; k++)
	    {
	       int c = pedchan[k];
	       pedest[c] = ped[k]*fPEDFRAC; 
	    }
        }
        for(int k=0; k<nchan; k++)
        {
            int c=(int)chan[k];
	    // std::cout<<i<<" adc["<<c<<"]: "<<adc[k]<<" "<<pedest[c]<<" > "<<fADCthreshold<<std::endl;
            if(c==8 && adc[k]-pedest[c]>fADCthreshold)
	    {
	       Tsettle = true;
	       time_sec = tEvt * 4e-9;//only updated inside Tsettle
	    }
            if(c==9 && adc[k]-pedest[c]>fNIMthreshold) QRT = true;
            if(c==10) helicityBit = (adc[k]-pedest[c] > fNIMthreshold);
        }

        if(i==0){ prevTsettle=Tsettle; continue; }

	
	//Start time of helicity window
	//////////////////////////////////////////////
	if(prevTsettle && !Tsettle){//start of Tstable
	    inWindow = true;
            current.start = i;
            current.start_time = time_sec;//window time taken from last event inside Tsettle
            current.reportedHelicity = helicityBit;
            current.hasQRT = QRT;
	    prev_time_sec = !windows.empty() ? windows.back().start_time : 0;
	    if(prev_time_sec == 0 ||//check if any helicities are missed using time
	       ((time_sec - prev_time_sec < 1.1/fTsettleFreq) &&
		(time_sec - prev_time_sec > 0.9/fTsettleFreq))){
	       current.timingOK=true;
	       //std::cout<<windows.size()<<" good timing "<<prev_time_sec<<std::endl;
	    }else
	       current.timingOK=false;
        }

	
	//End time of helicity window
	///////////////////////////////////////////////
        if(!prevTsettle && Tsettle && inWindow){//end of Tstable
            current.end=i-1;
            windows.push_back(current);
            inWindow=false;
	    //reinitialize current window_t
	    current.start = 0;
	    current.end = 0;
	    current.start_time = 0;
	    current.hasQRT = false;
	    current.hasTsettle = false;
	    current.timingOK = false;
	    current.trueHelicityFound = false;
        }
        prevTsettle = Tsettle;
    }

    if(windows.empty()){
       std::cout<<"No good helicity windows."<<std::endl;
       return;
    }

    //Now find true helicity from reported helicity.
    ////////////////////////////////////////////////////////////////////////////////
    std::cout<<"2). Finding true helicity from reported (delayed) helicity over "
	     <<windows.size()<<" helicity windows."<<std::endl;
    for(size_t i=0;i<windows.size();i++){
       if(i+fDelayWindows<windows.size()){
            windows[i].trueHelicity =
                windows[i+fDelayWindows].reportedHelicity;
	    double dt = windows[i+fDelayWindows].start_time - windows[i].start_time;
	    if(dt < (fDelayWindows+1.0)/fTsettleFreq && dt > (fDelayWindows-1.0)/fTsettleFreq){
	       windows[i].trueHelicityFound = true;
	    }else{
	       std::cout<<"Missing helicity window at start entry(time)"
			<<" "<<windows[i].start<<"("<<windows[i].start_time<<"s)"
			<<" and "<<fDelayWindows<<" helicity windows ahead at entry(time): "
			<<windows[i+fDelayWindows].start<<"("<<windows[i+fDelayWindows].start_time<<"s)"
			<<". delta_t "<<windows[i+fDelayWindows].start_time-windows[i].start_time
			<<". Expected delta_t "<<fDelayWindows/fTsettleFreq<<"+/-10%"<<std::endl;
	    }
       }
    }

    //Finally, we can form helicity patterns including the helicity of each window
    //the start and end entries of the pattern as well as the pattern polarity.
    //////////////////////////////////////////////////////////////////////////////
    std::cout<<"3). Forming full patterns from helicity windows, checking full pattern"
	     <<" sequence present\nand verifying random pattern polarity sequence "
	     <<" matches generator sequence. If not, re-seed generator."<<std::endl;

    std::vector<int> basePattern = GetBasePattern(fPatternType);
    int plen = (fPatternType == "toggle") ? 2 : basePattern.size();
    LFSR30 hel_predictor;
    size_t iwin = 0;
    prev_time_sec = 0;
    int nPat = 0;
    while( iwin<windows.size() )
    {
        if(!windows[iwin].hasQRT){ iwin++; continue; }
        if(iwin+plen>windows.size()) break;
	nPat++;
	double time_sec = windows[iwin].start_time;
        HelPat_t pat;
	//Initialize as good pattern until evidence shows otherwise.
        pat.complete = true;
        pat.sequenceOK = true;
        pat.windowtimingOK = true;
        pat.patterntimingOK = true;
        pat.predictedOK = true;
        pat.good_pattern = true;
        pat.helicityFound = true;
        pat.pattern_start = windows[iwin].start;
        pat.pattern_end = windows[iwin+plen-1].end;

	//Check pattern timing is correct
	////////////////////////////////////////
	double dt = time_sec - prev_time_sec;
	if((dt > (plen + 1.)/fTsettleFreq || dt < (plen - 1.)/fTsettleFreq) && prev_time_sec>0){
	   pat.patterntimingOK = false;
	   if(!hel_predictor.seeded)std::cout<<"}. Seed incomplete."<<std::endl;
	   std::cout<<"Missing patterns detected at helicity window "<<iwin<<" current time "
		    <<time_sec<<" previous time "<<prev_time_sec<<" expected delta_t "
		    <<plen/fTsettleFreq<<std::endl;
	   fPatterns.back().patterntimingOK = false;//missing windows implies previous pattern
	                                            //may be affected by partial cutoff even if
	                                            //all helicities in pattern present
	   fPatterns.back().good_pattern = false;
	   hel_predictor.Reset();
	   std::cout<<"pattern start entry "<<pat.pattern_start
		    <<" and end "<<pat.pattern_end<<std::endl;
	   hel_predictor.SeedBit(pat.polarity>0);
	   pat.predictedOK = false;
	}

	// Check random pattern sequence is correct.
	////////////////////////////////////////////
        bool firstHel = windows[iwin].trueHelicity;
        pat.polarity = firstHel ? +1 : -1;
	if(hel_predictor.seeded == false){
	   hel_predictor.SeedBit(pat.polarity>0);
	   pat.predictedOK = false;
	   //std::cout<<nPat<<". Seeding  "<<pat.polarity<<std::endl;
	}else{
	   int pred = hel_predictor.Next();
	   pat.predictedOK = (pred == pat.polarity);
	   if(pred != pat.polarity)
	   {
	      if(!hel_predictor.seeded)std::cout<<"}. Seed incomplete."<<std::endl;
	      std::cout<<"Pattern polarity sequence not matching generator."<<std::endl;
	      fPatterns.back().complete = false;//out of caution mark previous pattern incomplete as well
	      hel_predictor.Reset();
	      std::cout<<"pattern start entry "<<pat.pattern_start
		    <<" and end "<<pat.pattern_end<<std::endl;
	      hel_predictor.SeedBit(pat.polarity>0);
	      pat.predictedOK = false;
	   }
	   //std::cout<<nPat<<" "<<pat.polarity<<" "<<pred<<std::endl;
	}
	

	// Fill full pattern with consecutive helicity window information.
	//////////////////////////////////////////////////////////////////
        for(int w=0; w<plen; w++)
        {
	    pat.windowtimingOK &= windows[iwin+w].timingOK;
	    pat.helicityFound &= windows[iwin+w].trueHelicityFound;
            pat.window_start.push_back(windows[iwin+w].start);
            pat.window_end.push_back(windows[iwin+w].end);

            bool expected = (fPatternType=="toggle")
                ? (w%2 == 0?firstHel:!firstHel)
                : (basePattern[w]*pat.polarity>0);

            pat.helicity.push_back(expected);

            if(expected != windows[iwin+w].trueHelicity)
                pat.sequenceOK = false;
	    if(w>0 && windows[iwin+w].hasQRT){//Should not encounter new pattern start unless
	       pat.complete = false;          //pattern missing one or more helicity windows.
	       iwin += w-plen;
	       break;
	    }
        }
	pat.good_pattern = pat.complete && pat.sequenceOK && pat.windowtimingOK && pat.patterntimingOK
	                && pat.helicityFound && pat.predictedOK;
        fPatterns.push_back(pat);
	prev_time_sec = time_sec;
	iwin+=plen;
    }
    if(!hel_predictor.seeded)std::cout<<"}. Seed incomplete at end of run."<<std::endl;
    if(fRun>0){
       SavePatterns(fRun);
    }
}
