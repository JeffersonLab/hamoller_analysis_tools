// Do NOT change. Changes will be lost next time file is generated

#define R__DICTIONARY_FILENAME G__HelPat_t
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
#include "/adaqfs/home/hamoller/hamoller_analysis_tools/HelPat_t/HelPat_t.h"

// Header files passed via #pragma extra_include

// The generated code does not explicitly qualify STL entities
namespace std {} using namespace std;

namespace ROOT {
   static void *new_HelPat_t(void *p = nullptr);
   static void *newArray_HelPat_t(Long_t size, void *p);
   static void delete_HelPat_t(void *p);
   static void deleteArray_HelPat_t(void *p);
   static void destruct_HelPat_t(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::HelPat_t*)
   {
      ::HelPat_t *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::HelPat_t >(nullptr);
      static ::ROOT::TGenericClassInfo 
         instance("HelPat_t", ::HelPat_t::Class_Version(), "HelPat_t.h", 7,
                  typeid(::HelPat_t), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::HelPat_t::Dictionary, isa_proxy, 4,
                  sizeof(::HelPat_t), alignof(::HelPat_t) );
      instance.SetNew(&new_HelPat_t);
      instance.SetNewArray(&newArray_HelPat_t);
      instance.SetDelete(&delete_HelPat_t);
      instance.SetDeleteArray(&deleteArray_HelPat_t);
      instance.SetDestructor(&destruct_HelPat_t);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::HelPat_t*)
   {
      return GenerateInitInstanceLocal(static_cast<::HelPat_t*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::HelPat_t*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

//______________________________________________________________________________
atomic_TClass_ptr HelPat_t::fgIsA(nullptr);  // static to hold class pointer

//______________________________________________________________________________
const char *HelPat_t::Class_Name()
{
   return "HelPat_t";
}

//______________________________________________________________________________
const char *HelPat_t::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::HelPat_t*)nullptr)->GetImplFileName();
}

//______________________________________________________________________________
int HelPat_t::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::HelPat_t*)nullptr)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *HelPat_t::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::HelPat_t*)nullptr)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *HelPat_t::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::HelPat_t*)nullptr)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
void HelPat_t::Streamer(TBuffer &R__b)
{
   // Stream an object of class HelPat_t.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(HelPat_t::Class(),this);
   } else {
      R__b.WriteClassBuffer(HelPat_t::Class(),this);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_HelPat_t(void *p) {
      return  p ? new(p) ::HelPat_t : new ::HelPat_t;
   }
   static void *newArray_HelPat_t(Long_t nElements, void *p) {
      return p ? new(p) ::HelPat_t[nElements] : new ::HelPat_t[nElements];
   }
   // Wrapper around operator delete
   static void delete_HelPat_t(void *p) {
      delete (static_cast<::HelPat_t*>(p));
   }
   static void deleteArray_HelPat_t(void *p) {
      delete [] (static_cast<::HelPat_t*>(p));
   }
   static void destruct_HelPat_t(void *p) {
      typedef ::HelPat_t current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::HelPat_t

namespace ROOT {
   static TClass *vectorlEboolgR_Dictionary();
   static void vectorlEboolgR_TClassManip(TClass*);
   static void *new_vectorlEboolgR(void *p = nullptr);
   static void *newArray_vectorlEboolgR(Long_t size, void *p);
   static void delete_vectorlEboolgR(void *p);
   static void deleteArray_vectorlEboolgR(void *p);
   static void destruct_vectorlEboolgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<bool>*)
   {
      vector<bool> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<bool>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<bool>", -2, "vector", 596,
                  typeid(vector<bool>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlEboolgR_Dictionary, isa_proxy, 0,
                  sizeof(vector<bool>), alignof(vector<bool>) );
      instance.SetNew(&new_vectorlEboolgR);
      instance.SetNewArray(&newArray_vectorlEboolgR);
      instance.SetDelete(&delete_vectorlEboolgR);
      instance.SetDeleteArray(&deleteArray_vectorlEboolgR);
      instance.SetDestructor(&destruct_vectorlEboolgR);
      static_assert(alignof(vector<bool>::value_type) <= 4096,
          "Class with alignment strictly greater than 4096 are currently not supported in CollectionProxy. "
          "Please report this case to the developers");
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<bool> >()));

      instance.AdoptAlternate(::ROOT::AddClassAlternate("vector<bool>","std::vector<bool, std::allocator<bool> >"));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const vector<bool>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlEboolgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const vector<bool>*>(nullptr))->GetClass();
      vectorlEboolgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlEboolgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlEboolgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) vector<bool> : new vector<bool>;
   }
   static void *newArray_vectorlEboolgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) vector<bool>[nElements] : new vector<bool>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlEboolgR(void *p) {
      delete (static_cast<vector<bool>*>(p));
   }
   static void deleteArray_vectorlEboolgR(void *p) {
      delete [] (static_cast<vector<bool>*>(p));
   }
   static void destruct_vectorlEboolgR(void *p) {
      typedef vector<bool> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class vector<bool>

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
   // Registration Schema evolution read functions
   int RecordReadRules_libHelPat_t() {
      return 0;
   }
   static int _R__UNIQUE_DICT_(ReadRules_libHelPat_t) = RecordReadRules_libHelPat_t();R__UseDummy(_R__UNIQUE_DICT_(ReadRules_libHelPat_t));
} // namespace ROOT
namespace {
  void TriggerDictionaryInitialization_libHelPat_t_Impl() {
    static const char* headers[] = {
"/adaqfs/home/hamoller/hamoller_analysis_tools/HelPat_t/HelPat_t.h",
nullptr
    };
    static const char* includePaths[] = {
"/adaqfs/apps/ROOT/6.40-02/el9/RelWithDebInfo/include",
"/adaqfs/home/hamoller/hamoller_analysis_tools/HelPat_t",
"/adaqfs/apps/ROOT/6.40-02/el9/RelWithDebInfo/include",
"/adaqfs/home/hamoller/hamoller_analysis_tools/HelPat_t/build/",
nullptr
    };
    static const char* fwdDeclCode = R"DICTFWDDCLS(
#line 1 "libHelPat_t dictionary forward declarations' payload"

#pragma diagnostic push
#pragma clang diagnostic ignored "-Wkeyword-compat"
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern int __Cling_AutoLoading_Map;
struct  __attribute__((annotate("$clingAutoload$/adaqfs/home/hamoller/hamoller_analysis_tools/HelPat_t/HelPat_t.h")))  HelPat_t;
#pragma diagnostic pop
)DICTFWDDCLS";
    static const char* payloadCode = R"DICTPAYLOAD(
#line 1 "libHelPat_t dictionary payload"

#ifndef R__DUMMY_CXX_STANDARD_20
  #define R__DUMMY_CXX_STANDARD_20 1
#endif

#define _BACKWARD_BACKWARD_WARNING_H
// Inline headers
#include "/adaqfs/home/hamoller/hamoller_analysis_tools/HelPat_t/HelPat_t.h"

#undef  _BACKWARD_BACKWARD_WARNING_H
)DICTPAYLOAD";
    static const char* classesHeaders[] = {
"HelPat_t", payloadCode, "@",
nullptr
};
    static bool isInitialized = false;
    if (!isInitialized) {
      TROOT::RegisterModule("libHelPat_t",
        headers, includePaths, payloadCode, fwdDeclCode,
        TriggerDictionaryInitialization_libHelPat_t_Impl, {}, classesHeaders, /*hasCxxModule*/false);
      isInitialized = true;
    }
  }
  static struct DictInit {
    DictInit() {
      TriggerDictionaryInitialization_libHelPat_t_Impl();
    }
  } __TheDictionaryInitializer;
}
void TriggerDictionaryInitialization_libHelPat_t() {
  TriggerDictionaryInitialization_libHelPat_t_Impl();
}
