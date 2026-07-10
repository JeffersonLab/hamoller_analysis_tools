#ifndef HELPAT_T_H
#define HELPAT_T_H

#include <vector>
#include <Rtypes.h>

struct HelPat_t {
   Long64_t pattern_start{0};         // Entry number of pattern start (first event after Tsettle).
   Long64_t pattern_end{0};           // Entry number of pattern end (last event before Tsettle).

   std::vector<Long64_t> window_start;// Array of entry numbers of helicity window starts
                                      // i.e. first entry after Tsettle.

   std::vector<Long64_t> window_end;  // Array of entry numbers of helicity window ends
                                      // i.e. last event before next Tsettle.

   std::vector<bool> helicity;        // Array of true helicities (not delayed)

   int  polarity{0};                  // Sign of helicity pattern.

   bool complete{false};              // Has correct number of helicity states from given pattern.

   bool sequenceOK{false};            // Has helicity sequence correct for given pattern.

   bool windowtimingOK{false};        // Timing between all helicity windows inside current pattern
                                      // consistent with 1/TsettleFreq.

   bool patterntimingOK{false};       // Timing between this pattern and the previous one AND this
                                      // pattern and the next both consistent with
                                      // nWindowsPerPattern/TsettleFreq.

   bool predictedOK{false};           // Sequence of random helicity patterns consistent with
                                      // helicity random sequence generator.

   bool helicityFound{false};         // True helicity of each window in pattern found from looking
                                      // ahead N-delay windows verified timing difference consistent
                                      // with expected time delay from nWindPerPat/TsettleFreq.
   
   bool good_pattern{false};          // Pattern good to use:
                                      // (complete && sequenceOK && windowtimingOK &&
                                      // patterntimingOK && predictedOK && helicityVerified)
   
   ClassDef(HelPat_t, 1);
};

#endif
