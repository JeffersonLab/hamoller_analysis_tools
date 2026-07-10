#include "TTrain.h"
#include <iostream>

ClassImp(TTrain)

TTrain::TTrain()
    : fTotalEntries(0),
      fCurrentTreeNumber(-1),
      fCurrentTree(nullptr),
      fCurrentTreeStart(0),
      fCurrentTreeEnd(0),
      fLastGlobalEntry(-1)
{
}

TTrain::~TTrain()
{
    for (auto f : fFiles) {
        if (f) {
            f->Close();
            delete f;
        }
    }
}


bool TTrain::AddFile(const TString& filename, const TString& treename)
{
    if (gSystem->AccessPathName(filename)) {
        std::cout << "File does not exist: " << filename << std::endl;
        return false;
    }

    TFile* file = TFile::Open(filename, "READ");
    if (!file || file->IsZombie()) {
        std::cout << "Could not open file: " << filename << std::endl;
        return false;
    }

    TTree* tree = dynamic_cast<TTree*>(file->Get(treename));
    if (!tree) {
        std::cout << "Tree " << treename << " not found in "
                  << filename << std::endl;
        delete file;
        return false;
    }

    fFiles.push_back(file);
    fTrees.push_back(tree);
    fTreeOffsets.push_back(fTotalEntries);

    fTotalEntries += tree->GetEntries();

    return true;
}

void TTrain::SetUpTTrain(int run, const char* fnamebase, bool tchain)
{//Create TTrain from run number
   fRun = run;
   std::cout<<"Setting up TTrain for run "<< run <<std::endl;
   if(tchain) ch = new TChain("T");
   
   for (int seg = 0; seg < 100; ++seg) {
      for (int x = 0; x < 100; ++x) {
	 TString filename(Form("%s/%s%i.%i",
			       gSystem->Getenv("HAMOLLER_ROOTFILE_DIR"),
			       fnamebase, run, seg));
	 if (x > 0) filename += Form("_%i", x);
	 filename += ".root";

	 if (gSystem->AccessPathName(filename)) break;
	 std::cout << filename << std::endl;

	 AddFile(filename, "T");
	 if(tchain) ch->Add(filename);
      }
   }

   Long64_t nEntries = GetEntries();
   std::cout << nEntries << " total entries." << std::endl;
   if (nEntries == 0) {
      std::cout << "No entries found. Exiting." << std::endl;
      return;
   }
}


void TTrain::ApplyBranchAddresses()
{
    if (!fCurrentTree)
        return;

    for (auto& b : fBranches) {
        fCurrentTree->SetBranchAddress(b.name.c_str(), b.address);
    }
}

TString TTrain::GetCurrentFileName() const
{
    if (fCurrentTreeNumber < 0) return "";
    return fFiles[fCurrentTreeNumber]->GetName();
}

Long64_t TTrain::GetEntry(Long64_t globalEntry)
{
    if (globalEntry < 0 || globalEntry >= fTotalEntries)
        return 0;

    // Fast sequential case
    if (fCurrentTree &&
        globalEntry == fLastGlobalEntry + 1 &&
        globalEntry < fCurrentTreeEnd)
    {
        fLastGlobalEntry = globalEntry;

        Long64_t local = globalEntry - fCurrentTreeStart;

        return fCurrentTree->GetEntry(local);
    }

    // Otherwise update tree
    UpdateTree(globalEntry);

    fLastGlobalEntry = globalEntry;

    Long64_t local = globalEntry - fCurrentTreeStart;

    return fCurrentTree->GetEntry(local);
}

std::vector<TFile*> TTrain::GetVectorOfFiles()
{
   return fFiles;
}

void TTrain::SetBranchStatus(const char* bname, Bool_t status)
{
    // Store status internally
    fBranchStatus[bname] = status;

    // Apply immediately to current tree if it exists
    if (fCurrentTree) {
        fCurrentTree->SetBranchStatus(bname, status);
    }
}

void TTrain::UpdateTree(Long64_t globalEntry)
{
    // Binary search for correct tree
    auto it = std::upper_bound(
        fTreeOffsets.begin(),
        fTreeOffsets.end(),
        globalEntry
    );

    int treeIndex =
        std::distance(fTreeOffsets.begin(), it) - 1;

    if (treeIndex < 0)
        treeIndex = 0;

    if (treeIndex != fCurrentTreeNumber) {

        fCurrentTreeNumber = treeIndex;
        fCurrentTree = fTrees[treeIndex];
	fCurrentTree->SetMakeClass(1);
	//Without SetMakeClass(1):ROOT tries to use the object streamer
	//Split branches tied to THaEventHeader
	//Internal buffer mapping may fail silently. 64-bit leaves are especially sensitive
	//With SetMakeClass(1):
	//ROOT treats everything as raw leaves and No object streamer used
	//This allows complex leaves like fEvtHdr.fEvtTime whose structure may
	//change from file to file, to still have their branch addresses set correctly.
        fCurrentTreeStart = fTreeOffsets[treeIndex];

        if (treeIndex + 1 < (int)fTreeOffsets.size())
            fCurrentTreeEnd =
                fTreeOffsets[treeIndex + 1];
        else
            fCurrentTreeEnd = fTotalEntries;

	for (const auto& it : fBranchStatus) {
	   fCurrentTree->SetBranchStatus(it.first, it.second);
	}
	std::cout << "Switching to tree "
          << fCurrentTreeNumber
          << " file: "
          << fCurrentTree->GetCurrentFile()->GetName()
          << std::endl;	ApplyBranchAddresses();
    }
}
