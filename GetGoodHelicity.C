#include <TTrain.h>
#include <TChain.h>
#include <vector>
#include <Rtypes.h>
#include <iostream>
#include <string>
#include <cstdint>

using namespace std;

// =============================
// Constants
// =============================
const int nMAXPULSE = 4;//Maximum number of pile-up pulses per read out window
const int nCHAN = 16;//Number of FADC channels to read out
const int nMAXCHAN = nCHAN*nMAXPULSE;//Maximum number of channels to read out if pile-up is at maximum on all channels
const double THRESH = 10;//Software threshold for ADC channel to be considered active

// =============================
// Structures
// =============================

struct window_t {
    Long64_t start;
    Long64_t end;
    double   start_time;

    bool reportedHelicity;
    bool trueHelicity;
    bool hasQRT;
    bool valid;
};

struct pattern_t {
    Long64_t pattern_start;
    Long64_t pattern_end;

    vector<Long64_t> window_start;
    vector<Long64_t> window_end;

    vector<bool> helicity;

    int  polarity;    // +/-1 polarity of the helicity pattern
    bool complete;    //all expected helicity windows included in pattern
    bool sequenceOK;  //matches selected helicity pattern sequence
    bool timingOK;    //time between helicity windows matches expectation (no missed windows)
    bool predictedOK; //pattern polarity sequence matches prediction
};

// ===============================================
//Define the helicity pattern sequence being used.
// ===============================================

vector<int> GetBasePattern(const string &type)
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

// =======================================================
// Helicity predictor using Linear Feedback Shift Register
// 30-bit LFSR (taps 30,29,28,7)
// Polynomial: x^30 + x^29 + x^28 + x^7 + 1
// =======================================================

class LFSR30 {
public:
    uint32_t state = 0;
    bool seeded = false;

    void Seed(const vector<int>& bits)
    {
        state = 0;
        for(int i = 0; i < 30; i++) {
            state <<= 1;
            if(bits[i] > 0)
                state |= 1;
        }
        seeded = true;
    }

    int Next()
    {
        int newbit =
            ((state >> 29) ^
             (state >> 28) ^
             (state >> 27) ^
             (state >> 6)) & 1;

        state = ((state << 1) | newbit) & 0x3fffffff;

        // output from LSB
        return (state & 1) ? +1 : -1;
    }

    void Reset()
    {
        seeded = false;
        state = 0;
    }
};

// =====================================================================================================
// Main Helicity Decoder
// Arguments:
//            ch              TChain containing any number of ROOT TTrees
//            patternType     Options are "pair", "quartet", "octet", "hexoquad", "octoquad" or "toggle" 
//            delayWindows    Helicity delay in windows
//            Tsettle_freq    Helicity flip frequency in Hz
// ======================================================================================================

vector<pattern_t> GetGoodHelicity(TTrain *ch,
                                  const string &patternType,
                                  int delayWindows = 8,
                                  double Tsettle_freq = 29.5596)
{
    vector<pattern_t> goodPatterns;
    vector<window_t> windows;

 
    // -----------------------------------------
    // Set Branch addresses for required values.
    // -----------------------------------------

    Double_t adc[nMAXCHAN], adc_t[nMAXCHAN], chan[nMAXCHAN];
    Int_t    nchan;
    ch->SetBranchStatus("*", 0);//Turn off all branches and only activate the ones you need
    ch->SetBranchStatus("M.cal.adc", 1);
    ch->SetBranchStatus("M.cal.adc_chan", 1);
    ch->SetBranchStatus("Ndata.M.cal.adc", 1);
    ch->SetBranchStatus("Ndata.M.cal.adc_t", 1);
    ch->SetBranchAddress("M.cal.adc", adc);
    ch->SetBranchAddress("M.cal.adc_t", adc_t);
    ch->SetBranchAddress("M.cal.adc_chan", chan);
    ch->SetBranchAddress("Ndata.M.cal.adc", &nchan);

    Long64_t nentries = ch->GetEntries();

    bool prevTsettle = true;
    bool inWindow = false;
    bool lostWindowDetected = false;

    vector<double> TsettleRiseTimes;

    window_t current;

    // =============================
    // Build windows from Tsettle
    // =============================

    for(Long64_t i=0;i<nentries;i++)
    {
        ch->GetEntry(i);

        bool Tsettle=false;
        bool QRT=false;
        bool helicityBit=false;
        double time_sec = 0;

        for(int k=0;k<nchan;k++)
        {
            int c = (int)chan[k];
            if(c==8 && adc[k]>0) { Tsettle=true; time_sec = adc_t[k]*4e-9;}
            if(c==9 && adc[k]>0) QRT=true;
            if(c==10) helicityBit = (adc[k]>0);
        }

	//Set prevTsettle and move to next entry so as not to trigger
	//false detection of a rising or falling edge Tsettle window.
	if(i == 0)
	{
	  prevTsettle = Tsettle;
	  continue;
	}
        // Detect rising edge of Tsettle → window end
        if(!prevTsettle && Tsettle)
        {
            TsettleRiseTimes.push_back(time_sec);

            if(TsettleRiseTimes.size()>1)
            {
                double dt =
                    TsettleRiseTimes.back() -
                    TsettleRiseTimes[TsettleRiseTimes.size()-2];

                double expected = 1.0/Tsettle_freq;

                if(dt > 1.5*expected)
                    lostWindowDetected = true;
            }
        }

        // Detect falling edge of Tsettle → window start
        if(prevTsettle && !Tsettle)
        {
            inWindow=true;
            current.start=i;
            current.start_time=time_sec;
            current.reportedHelicity=helicityBit;
            current.hasQRT=QRT;
            current.valid=true;
        }

        // Rising edge -> window end
        if(!prevTsettle && Tsettle && inWindow)
        {
            current.end=i-1;
            windows.push_back(current);
            inWindow=false;
        }

        prevTsettle=Tsettle;
    }

    if(windows.empty())
        return goodPatterns;

    // ===============================
    // Apply helicity delay correction
    // ===============================

    for(size_t i=0;i<windows.size();i++)
    {
        if(i+delayWindows < windows.size())
            windows[i].trueHelicity =
                windows[i+delayWindows].reportedHelicity;
        else
            windows[i].valid=false;
    }

    while(!windows.empty() && !windows.back().valid)
        windows.pop_back();

    if(windows.empty())
        return goodPatterns;

    // =============================
    // Skip until first QRT
    // =============================

    size_t firstQRT=0;
    while(firstQRT<windows.size() &&
          !windows[firstQRT].hasQRT)
        firstQRT++;

    if(firstQRT>=windows.size())
        return goodPatterns;

    vector<int> basePattern = GetBasePattern(patternType);
    int plen = (patternType=="toggle") ? 2 : basePattern.size();

    LFSR30 lfsr;
    vector<int> seedBuffer;
    double lastQRTtime=-1;

    size_t iwin=firstQRT;

    // =============================
    // Pattern loop
    // =============================

    while(iwin < windows.size())
    {
        if(!windows[iwin].hasQRT) {
            iwin++;
            continue;
        }

        if(iwin+plen > windows.size())
            break;

        pattern_t pat;
        pat.complete=true;
        pat.sequenceOK=true;
        pat.timingOK=true;

        pat.pattern_start=windows[iwin].start;
        pat.pattern_end=windows[iwin+plen-1].end;

        bool firstHel=windows[iwin].trueHelicity;
        pat.polarity = firstHel ? +1 : -1;

        // QRT spacing check
        double currentQRTtime = windows[iwin].start_time;

        if(lastQRTtime>0)
        {
            double dt = currentQRTtime - lastQRTtime;
            double maxSpacing =
                (plen+0.5)/Tsettle_freq;

            if(dt > maxSpacing)
                pat.timingOK=false;
        }

        if(lostWindowDetected)
        {
            pat.timingOK=false;
            lostWindowDetected=false;
        }

        lastQRTtime=currentQRTtime;

        // Pattern helicity validation
        for(int w=0;w<plen;w++)
        {
            pat.window_start.push_back(
                windows[iwin+w].start);
            pat.window_end.push_back(
                windows[iwin+w].end);

            bool expected;

            if(patternType=="toggle")
                expected = (w%2==0) ?
                           firstHel : !firstHel;
            else
                expected =
                    (basePattern[w]*pat.polarity>0);

            pat.helicity.push_back(expected);

            if(expected !=
               windows[iwin+w].trueHelicity)
                pat.sequenceOK=false;
        }

        // =============================
        // LFSR handling
        // =============================

        if(pat.complete &&
           pat.sequenceOK &&
           pat.timingOK)
        {
            if(!lfsr.seeded)
            {
                seedBuffer.push_back(pat.polarity);

                if(seedBuffer.size()==30)
                {
                    lfsr.Seed(seedBuffer);
                    seedBuffer.clear();
                }

                pat.predictedOK=true;
            }
            else
            {
                int predicted=lfsr.Next();

                if(predicted!=pat.polarity)
                {
                    lfsr.Reset();
                    seedBuffer.clear();
                    seedBuffer.push_back(
                        pat.polarity);
                    pat.predictedOK=false;
                }
                else
                {
                    pat.predictedOK=true;
                }
            }
        }
        else
        {
            lfsr.Reset();
            seedBuffer.clear();
            pat.predictedOK=false;
        }

        goodPatterns.push_back(pat);

        iwin += plen;
    }
    return goodPatterns;
}
