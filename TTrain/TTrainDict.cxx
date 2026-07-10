// Do NOT change. Changes will be lost next time file is generated

#define R__DICTIONARY_FILENAME TTrainDict
#define R__NO_DEPRECATION

/*******************************************************************/
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#define G__DICTIONARY
#include "ROOT/RConfig.hxx"
#include "TClass.h"
#include "TDictAttributeMap.h"
#include "TInterpreter.h"
#include "TROOT.h"
#include "TBuffer.h"
#include "TMemberInspector.h"
#include "TInterpreter.h"
#include "TVirtualMutex.h"
#include "TError.h"

#ifndef G__ROOT
#define G__ROOT
#endif

#include "RtypesImp.h"
#include "TIsAProxy.h"
#include "TFileMergeInfo.h"
#include <algorithm>
#include "TCollectionProxyInfo.h"
/*******************************************************************/

#include "TDataMember.h"

// Header files passed as explicit arguments
#include "TTrain.h"

// Header files passed via #pragma extra_include

// The generated code does not explicitly qualify STL entities
namespace std {} using namespace std;

namespace ROOT {
   static void *new_TTrain(void *p = nullptr);
   static void *newArray_TTrain(Long_t size, void *p);
   static void delete_TTrain(void *p);
   static void deleteArray_TTrain(void *p);
   static void destruct_TTrain(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::TTrain*)
   {
      ::TTrain *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::TTrain >(nullptr);
      static ::ROOT::TGenericClassInfo 
         instance("TTrain", ::TTrain::Class_Version(), "TTrain.h", 16,
                  typeid(::TTrain), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::TTrain::Dictionary, isa_proxy, 4,
                  sizeof(::TTrain), alignof(::TTrain) );
      instance.SetNew(&new_TTrain);
      instance.SetNewArray(&newArray_TTrain);
      instance.SetDelete(&delete_TTrain);
      instance.SetDeleteArray(&deleteArray_TTrain);
      instance.SetDestructor(&destruct_TTrain);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::TTrain*)
   {
      return GenerateInitInstanceLocal(static_cast<::TTrain*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::TTrain*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

//______________________________________________________________________________
atomic_TClass_ptr TTrain::fgIsA(nullptr);  // static to hold class pointer

//______________________________________________________________________________
const char *TTrain::Class_Name()
{
   return "TTrain";
}

//______________________________________________________________________________
const char *TTrain::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::TTrain*)nullptr)->GetImplFileName();
}

//______________________________________________________________________________
int TTrain::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::TTrain*)nullptr)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *TTrain::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::TTrain*)nullptr)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *TTrain::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::TTrain*)nullptr)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
void TTrain::Streamer(TBuffer &R__b)
{
   // Stream an object of class TTrain.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(TTrain::Class(),this);
   } else {
      R__b.WriteClassBuffer(TTrain::Class(),this);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_TTrain(void *p) {
      return  p ? new(p) ::TTrain : new ::TTrain;
   }
   static void *newArray_TTrain(Long_t nElements, void *p) {
      return p ? new(p) ::TTrain[nElements] : new ::TTrain[nElements];
   }
   // Wrapper around operator delete
   static void delete_TTrain(void *p) {
      delete (static_cast<::TTrain*>(p));
   }
   static void deleteArray_TTrain(void *p) {
      delete [] (static_cast<::TTrain*>(p));
   }
   static void destruct_TTrain(void *p) {
      typedef ::TTrain current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::TTrain

namespace ROOT {
   static TClass *vectorlETTreemUgR_Dictionary();
   static void vectorlETTreemUgR_TClassManip(TClass*);
   static void *new_vectorlETTreemUgR(void *p = nullptr);
   static void *newArray_vectorlETTreemUgR(Long_t size, void *p);
   static void delete_vectorlETTreemUgR(void *p);
   static void deleteArray_vectorlETTreemUgR(void *p);
   static void destruct_vectorlETTreemUgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<TTree*>*)
   {
      vector<TTree*> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<TTree*>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<TTree*>", -2, "vector", 389,
                  typeid(vector<TTree*>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlETTreemUgR_Dictionary, isa_proxy, 0,
                  sizeof(vector<TTree*>), alignof(vector<TTree*>) );
      instance.SetNew(&new_vectorlETTreemUgR);
      instance.SetNewArray(&newArray_vectorlETTreemUgR);
      instance.SetDelete(&delete_vectorlETTreemUgR);
      instance.SetDeleteArray(&deleteArray_vectorlETTreemUgR);
      instance.SetDestructor(&destruct_vectorlETTreemUgR);
      static_assert(alignof(vector<TTree*>::value_type) <= 4096,
          "Class with alignment strictly greater than 4096 are currently not supported in CollectionProxy. "
          "Please report this case to the developers");
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<TTree*> >()));

      instance.AdoptAlternate(::ROOT::AddClassAlternate("vector<TTree*>","std::vector<TTree*, std::allocator<TTree*> >"));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const vector<TTree*>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlETTreemUgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const vector<TTree*>*>(nullptr))->GetClass();
      vectorlETTreemUgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlETTreemUgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlETTreemUgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) vector<TTree*> : new vector<TTree*>;
   }
   static void *newArray_vectorlETTreemUgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) vector<TTree*>[nElements] : new vector<TTree*>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlETTreemUgR(void *p) {
      delete (static_cast<vector<TTree*>*>(p));
   }
   static void deleteArray_vectorlETTreemUgR(void *p) {
      delete [] (static_cast<vector<TTree*>*>(p));
   }
   static void destruct_vectorlETTreemUgR(void *p) {
      typedef vector<TTree*> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class vector<TTree*>

namespace ROOT {
   static TClass *vectorlETTraincLcLBranchInfogR_Dictionary();
   static void vectorlETTraincLcLBranchInfogR_TClassManip(TClass*);
   static void *new_vectorlETTraincLcLBranchInfogR(void *p = nullptr);
   static void *newArray_vectorlETTraincLcLBranchInfogR(Long_t size, void *p);
   static void delete_vectorlETTraincLcLBranchInfogR(void *p);
   static void deleteArray_vectorlETTraincLcLBranchInfogR(void *p);
   static void destruct_vectorlETTraincLcLBranchInfogR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<TTrain::BranchInfo>*)
   {
      vector<TTrain::BranchInfo> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<TTrain::BranchInfo>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<TTrain::BranchInfo>", -2, "vector", 389,
                  typeid(vector<TTrain::BranchInfo>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlETTraincLcLBranchInfogR_Dictionary, isa_proxy, 0,
                  sizeof(vector<TTrain::BranchInfo>), alignof(vector<TTrain::BranchInfo>) );
      instance.SetNew(&new_vectorlETTraincLcLBranchInfogR);
      instance.SetNewArray(&newArray_vectorlETTraincLcLBranchInfogR);
      instance.SetDelete(&delete_vectorlETTraincLcLBranchInfogR);
      instance.SetDeleteArray(&deleteArray_vectorlETTraincLcLBranchInfogR);
      instance.SetDestructor(&destruct_vectorlETTraincLcLBranchInfogR);
      static_assert(alignof(vector<TTrain::BranchInfo>::value_type) <= 4096,
          "Class with alignment strictly greater than 4096 are currently not supported in CollectionProxy. "
          "Please report this case to the developers");
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<TTrain::BranchInfo> >()));

      instance.AdoptAlternate(::ROOT::AddClassAlternate("vector<TTrain::BranchInfo>","std::vector<TTrain::BranchInfo, std::allocator<TTrain::BranchInfo> >"));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const vector<TTrain::BranchInfo>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlETTraincLcLBranchInfogR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const vector<TTrain::BranchInfo>*>(nullptr))->GetClass();
      vectorlETTraincLcLBranchInfogR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlETTraincLcLBranchInfogR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlETTraincLcLBranchInfogR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) vector<TTrain::BranchInfo> : new vector<TTrain::BranchInfo>;
   }
   static void *newArray_vectorlETTraincLcLBranchInfogR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) vector<TTrain::BranchInfo>[nElements] : new vector<TTrain::BranchInfo>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlETTraincLcLBranchInfogR(void *p) {
      delete (static_cast<vector<TTrain::BranchInfo>*>(p));
   }
   static void deleteArray_vectorlETTraincLcLBranchInfogR(void *p) {
      delete [] (static_cast<vector<TTrain::BranchInfo>*>(p));
   }
   static void destruct_vectorlETTraincLcLBranchInfogR(void *p) {
      typedef vector<TTrain::BranchInfo> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class vector<TTrain::BranchInfo>

namespace ROOT {
   static TClass *vectorlETFilemUgR_Dictionary();
   static void vectorlETFilemUgR_TClassManip(TClass*);
   static void *new_vectorlETFilemUgR(void *p = nullptr);
   static void *newArray_vectorlETFilemUgR(Long_t size, void *p);
   static void delete_vectorlETFilemUgR(void *p);
   static void deleteArray_vectorlETFilemUgR(void *p);
   static void destruct_vectorlETFilemUgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<TFile*>*)
   {
      vector<TFile*> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<TFile*>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<TFile*>", -2, "vector", 389,
                  typeid(vector<TFile*>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlETFilemUgR_Dictionary, isa_proxy, 0,
                  sizeof(vector<TFile*>), alignof(vector<TFile*>) );
      instance.SetNew(&new_vectorlETFilemUgR);
      instance.SetNewArray(&newArray_vectorlETFilemUgR);
      instance.SetDelete(&delete_vectorlETFilemUgR);
      instance.SetDeleteArray(&deleteArray_vectorlETFilemUgR);
      instance.SetDestructor(&destruct_vectorlETFilemUgR);
      static_assert(alignof(vector<TFile*>::value_type) <= 4096,
          "Class with alignment strictly greater than 4096 are currently not supported in CollectionProxy. "
          "Please report this case to the developers");
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<TFile*> >()));

      instance.AdoptAlternate(::ROOT::AddClassAlternate("vector<TFile*>","std::vector<TFile*, std::allocator<TFile*> >"));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const vector<TFile*>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlETFilemUgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const vector<TFile*>*>(nullptr))->GetClass();
      vectorlETFilemUgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlETFilemUgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlETFilemUgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) vector<TFile*> : new vector<TFile*>;
   }
   static void *newArray_vectorlETFilemUgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) vector<TFile*>[nElements] : new vector<TFile*>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlETFilemUgR(void *p) {
      delete (static_cast<vector<TFile*>*>(p));
   }
   static void deleteArray_vectorlETFilemUgR(void *p) {
      delete [] (static_cast<vector<TFile*>*>(p));
   }
   static void destruct_vectorlETFilemUgR(void *p) {
      typedef vector<TFile*> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class vector<TFile*>

namespace ROOT {
   static TClass *vectorlELong64_tgR_Dictionary();
   static void vectorlELong64_tgR_TClassManip(TClass*);
   static void *new_vectorlELong64_tgR(void *p = nullptr);
   static void *newArray_vectorlELong64_tgR(Long_t size, void *p);
   static void delete_vectorlELong64_tgR(void *p);
   static void deleteArray_vectorlELong64_tgR(void *p);
   static void destruct_vectorlELong64_tgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<Long64_t>*)
   {
      vector<Long64_t> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<Long64_t>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<Long64_t>", -2, "vector", 389,
                  typeid(vector<Long64_t>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlELong64_tgR_Dictionary, isa_proxy, 0,
                  sizeof(vector<Long64_t>), alignof(vector<Long64_t>) );
      instance.SetNew(&new_vectorlELong64_tgR);
      instance.SetNewArray(&newArray_vectorlELong64_tgR);
      instance.SetDelete(&delete_vectorlELong64_tgR);
      instance.SetDeleteArray(&deleteArray_vectorlELong64_tgR);
      instance.SetDestructor(&destruct_vectorlELong64_tgR);
      static_assert(alignof(vector<Long64_t>::value_type) <= 4096,
          "Class with alignment strictly greater than 4096 are currently not supported in CollectionProxy. "
          "Please report this case to the developers");
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<Long64_t> >()));

      instance.AdoptAlternate(::ROOT::AddClassAlternate("vector<Long64_t>","std::vector<long long, std::allocator<long long> >"));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const vector<Long64_t>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlELong64_tgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const vector<Long64_t>*>(nullptr))->GetClass();
      vectorlELong64_tgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlELong64_tgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlELong64_tgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) vector<Long64_t> : new vector<Long64_t>;
   }
   static void *newArray_vectorlELong64_tgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) vector<Long64_t>[nElements] : new vector<Long64_t>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlELong64_tgR(void *p) {
      delete (static_cast<vector<Long64_t>*>(p));
   }
   static void deleteArray_vectorlELong64_tgR(void *p) {
      delete [] (static_cast<vector<Long64_t>*>(p));
   }
   static void destruct_vectorlELong64_tgR(void *p) {
      typedef vector<Long64_t> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class vector<Long64_t>

namespace ROOT {
   static TClass *maplETStringcOboolgR_Dictionary();
   static void maplETStringcOboolgR_TClassManip(TClass*);
   static void *new_maplETStringcOboolgR(void *p = nullptr);
   static void *newArray_maplETStringcOboolgR(Long_t size, void *p);
   static void delete_maplETStringcOboolgR(void *p);
   static void deleteArray_maplETStringcOboolgR(void *p);
   static void destruct_maplETStringcOboolgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const map<TString,bool>*)
   {
      map<TString,bool> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(map<TString,bool>));
      static ::ROOT::TGenericClassInfo 
         instance("map<TString,bool>", -2, "map", 100,
                  typeid(map<TString,bool>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &maplETStringcOboolgR_Dictionary, isa_proxy, 0,
                  sizeof(map<TString,bool>), alignof(map<TString,bool>) );
      instance.SetNew(&new_maplETStringcOboolgR);
      instance.SetNewArray(&newArray_maplETStringcOboolgR);
      instance.SetDelete(&delete_maplETStringcOboolgR);
      instance.SetDeleteArray(&deleteArray_maplETStringcOboolgR);
      instance.SetDestructor(&destruct_maplETStringcOboolgR);
      static_assert(alignof(map<TString,bool>::value_type) <= 4096,
          "Class with alignment strictly greater than 4096 are currently not supported in CollectionProxy. "
          "Please report this case to the developers");
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::MapInsert< map<TString,bool> >()));

      instance.AdoptAlternate(::ROOT::AddClassAlternate("map<TString,bool>","std::map<TString, bool, std::less<TString>, std::allocator<std::pair<TString const, bool> > >"));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const map<TString,bool>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *maplETStringcOboolgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const map<TString,bool>*>(nullptr))->GetClass();
      maplETStringcOboolgR_TClassManip(theClass);
   return theClass;
   }

   static void maplETStringcOboolgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_maplETStringcOboolgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) map<TString,bool> : new map<TString,bool>;
   }
   static void *newArray_maplETStringcOboolgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) map<TString,bool>[nElements] : new map<TString,bool>[nElements];
   }
   // Wrapper around operator delete
   static void delete_maplETStringcOboolgR(void *p) {
      delete (static_cast<map<TString,bool>*>(p));
   }
   static void deleteArray_maplETStringcOboolgR(void *p) {
      delete [] (static_cast<map<TString,bool>*>(p));
   }
   static void destruct_maplETStringcOboolgR(void *p) {
      typedef map<TString,bool> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class map<TString,bool>

namespace ROOT {
   // Registration Schema evolution read functions
   int RecordReadRules_TTrainDict() {
      return 0;
   }
   static int _R__UNIQUE_DICT_(ReadRules_TTrainDict) = RecordReadRules_TTrainDict();R__UseDummy(_R__UNIQUE_DICT_(ReadRules_TTrainDict));
} // namespace ROOT
namespace {
  void TriggerDictionaryInitialization_TTrainDict_Impl() {
    static const char* headers[] = {
"TTrain.h",
nullptr
    };
    static const char* includePaths[] = {
"/adaqfs/apps/ROOT/6.40-02/el9/RelWithDebInfo/include",
"/adaqfs/home/hamoller/jonesdc/macros/TTrain/",
nullptr
    };
    static const char* fwdDeclCode = R"DICTFWDDCLS(
#line 1 "TTrainDict dictionary forward declarations' payload"

#pragma diagnostic push
#pragma clang diagnostic ignored "-Wkeyword-compat"
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern int __Cling_AutoLoading_Map;
class  __attribute__((annotate("$clingAutoload$TTrain.h")))  TTrain;
#pragma diagnostic pop
)DICTFWDDCLS";
    static const char* payloadCode = R"DICTPAYLOAD(
#line 1 "TTrainDict dictionary payload"


#define _BACKWARD_BACKWARD_WARNING_H
// Inline headers
#include "TTrain.h"

#undef  _BACKWARD_BACKWARD_WARNING_H
)DICTPAYLOAD";
    static const char* classesHeaders[] = {
"TTrain", payloadCode, "@",
nullptr
};
    static bool isInitialized = false;
    if (!isInitialized) {
      TROOT::RegisterModule("TTrainDict",
        headers, includePaths, payloadCode, fwdDeclCode,
        TriggerDictionaryInitialization_TTrainDict_Impl, {}, classesHeaders, /*hasCxxModule*/false);
      isInitialized = true;
    }
  }
  static struct DictInit {
    DictInit() {
      TriggerDictionaryInitialization_TTrainDict_Impl();
    }
  } __TheDictionaryInitializer;
}
void TriggerDictionaryInitialization_TTrainDict() {
  TriggerDictionaryInitialization_TTrainDict_Impl();
}
