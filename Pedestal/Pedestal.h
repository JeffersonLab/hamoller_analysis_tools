#ifndef PEDESTAL_H
#define PEDESTAL_H

#include <TObject.h>
#include <TTree.h>
#include <TChain.h>

class Pedestal : public TObject {
public:
    Pedestal();
    virtual ~Pedestal();

    void Process(TChain* ch, int runnum);

private:
    static const int fNCHAN = 16;
    static const int fNDET = 8;
    static const int fMAXLEN = fNCHAN*4;
    double fNSA = 9;  // Number of FADC samples after threshold crossing included in integral
    double fNSB = 4;  // Number of FADC samples before threshold crossing included in integral
    double fNPED = 4; // Number of FADC samples from start of window included in pedestal integral
    double fPEDFRAC = (fNSA+fNSB)/fNPED; // Multiply pedestal by this to subtract from FADC integral

    double fPedMean[fNDET];
    double fPedSigma[fNDET];


    void ComputeStats(TChain* ch);
    void BuildFriendTree(TChain* ch, int runnum);

    ClassDef(Pedestal,1)
};

#endif
