#include <TFile.h>
#include <TTree.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include <cstdlib>

int main() {
    // --- Configuration ---
    int run_number = 1234;  // example run number
    std::string fileName = "LeftRightSumTree_" + std::to_string(run_number) + ".root";
    const size_t maxFileSize = 20ULL * 1024 * 1024 * 1024; // 20 GB
    
    // --- Open file and create tree ---
    TFile* file = TFile::Open(fileName.c_str(), "RECREATE");
    TTree* tree = new TTree("T", "Left/Right Detector Sums Tree");

    // --- Leaves using vectors ---
    std::vector<double> ADC;
    std::vector<double> ADC_t;
    std::vector<int> ADC_chan;
    int nChan;  // optional, can store vector size
    double Lsum, Lsum_t;
    double Rsum, Rsum_t;
    bool coinc;

    tree->Branch("nChan", &nChan, "nChan/I");       // optional
    tree->Branch("ADC", &ADC);
    tree->Branch("ADC_t", &ADC_t);
    tree->Branch("ADC_chan", &ADC_chan);
    tree->Branch("Lsum", &Lsum, "Lsum/D");
    tree->Branch("Lsum_t", &Lsum_t, "Lsum_t/D");
    tree->Branch("Rsum", &Rsum, "Rsum/D");
    tree->Branch("Rsum_t", &Rsum_t, "Rsum_t/D");
    tree->Branch("coinc", &coinc, "coinc/O");

    // --- Set maximum tree size ---
    tree->SetMaxTreeSize(maxFileSize);

    // --- Fill tree with events ---
    const int N = 100000;  // number of events
    const double coincidence_window = 5.0; // ns

    for (int evt = 0; evt < N; ++evt) {
        // --- Clear vectors per event ---
        ADC.clear();
        ADC_t.clear();
        ADC_chan.clear();

        // --- Example: Randomly decide number of hits ---
        nChan = rand() % 9;  // 0-8 channels
        if (nChan == 0) nChan = 1; // avoid empty events

        // --- Reset sums ---
        Lsum = -99999;
        Lsum_t = -99999;
        Rsum = -99999;
        Rsum_t = -99999;
        double Ltime_sum = 0;
        double Rtime_sum = 0;
        int Lcount = 0;
        int Rcount = 0;

        // --- Fill vectors ---
        for (int i = 0; i < nChan; ++i) {
            int ch = rand() % 8; // channel 0-7
            double adc_val = (rand() % 1000) / 10.0;   // example ADC
            double time_val = (rand() % 1000) / 100.0; // example time

            ADC.push_back(adc_val);
            ADC_t.push_back(time_val);
            ADC_chan.push_back(ch);

            // Update sums
            if (ch <= 3) {  // left detector
                if (Lsum == -99999) Lsum = 0;
                Lsum += adc_val;
                Ltime_sum += time_val;
                Lcount++;
            } else {        // right detector
                if (Rsum == -99999) Rsum = 0;
                Rsum += adc_val;
                Rtime_sum += time_val;
                Rcount++;
            }
        }

        if (Lcount > 0) Lsum_t = Ltime_sum / Lcount;
        if (Rcount > 0) Rsum_t = Rtime_sum / Rcount;

        // Determine coincidence
        if (Lcount > 0 && Rcount > 0) {
            coinc = std::abs(Lsum_t - Rsum_t) <= coincidence_window;
        } else {
            coinc = false;
        }

        // Optional: store vector size as nChan
        nChan = ADC.size();

        tree->Fill();

        if (evt % 10000 == 0) std::cout << "Filled event " << evt << std::endl;
    }

    // --- Write and close ---
    tree->Write();
    file->Close();

    std::cout << "Saved TTree to " << fileName << std::endl;
    return 0;
}
