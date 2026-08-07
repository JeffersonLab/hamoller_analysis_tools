// Do NOT change. Changes will be lost next time file is generated

#define R__DICTIONARY_FILENAME G__GoodHelicities
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
#include "/adaqfs/home/hamoller/hamoller_analysis_tools/GoodHelicities/GoodHelicities.h"

// Header files passed via #pragma extra_include

// The generated code does not explicitly qualify STL entities
namespace std {} using namespace std;

namespace ROOT {
   static void *new_window_t(void *p = nullptr);
   static void *newArray_window_t(Long_t size, void *p);
   static void delete_window_t(void *p);
   static void deleteArray_window_t(void *p);
   static void destruct_window_t(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::window_t*)
   {
      ::window_t *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::window_t >(nullptr);
      static ::ROOT::TGenericClassInfo 
         instance("window_t", ::window_t::Class_Version(), "GoodHelicities.h", 17,
                  typeid(::window_t), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::window_t::Dictionary, isa_proxy, 4,
                  sizeof(::window_t), alignof(::window_t) );
      instance.SetNew(&new_window_t);
      instance.SetNewArray(&newArray_window_t);
      instance.SetDelete(&delete_window_t);
      instance.SetDeleteArray(&deleteArray_window_t);
      instance.SetDestructor(&destruct_window_t);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::window_t*)
   {
      return GenerateInitInstanceLocal(static_cast<::window_t*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::window_t*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

namespace ROOT {
   static void *new_GoodHelicities(void *p = nullptr);
   static void *newArray_GoodHelicities(Long_t size, void *p);
   static void delete_GoodHelicities(void *p);
   static void deleteArray_GoodHelicities(void *p);
   static void destruct_GoodHelicities(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::GoodHelicities*)
   {
      ::GoodHelicities *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::GoodHelicities >(nullptr);
      static ::ROOT::TGenericClassInfo 
         instance("GoodHelicities", ::GoodHelicities::Class_Version(), "GoodHelicities.h", 43,
                  typeid(::GoodHelicities), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::GoodHelicities::Dictionary, isa_proxy, 4,
                  sizeof(::GoodHelicities), alignof(::GoodHelicities) );
      instance.SetNew(&new_GoodHelicities);
      instance.SetNewArray(&newArray_GoodHelicities);
      instance.SetDelete(&delete_GoodHelicities);
      instance.SetDeleteArray(&deleteArray_GoodHelicities);
      instance.SetDestructor(&destruct_GoodHelicities);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::GoodHelicities*)
   {
      return GenerateInitInstanceLocal(static_cast<::GoodHelicities*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::GoodHelicities*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

//______________________________________________________________________________
atomic_TClass_ptr window_t::fgIsA(nullptr);  // static to hold class pointer

//______________________________________________________________________________
const char *window_t::Class_Name()
{
   return "window_t";
}

//______________________________________________________________________________
const char *window_t::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::window_t*)nullptr)->GetImplFileName();
}

//______________________________________________________________________________
int window_t::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::window_t*)nullptr)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *window_t::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::window_t*)nullptr)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *window_t::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::window_t*)nullptr)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
atomic_TClass_ptr GoodHelicities::fgIsA(nullptr);  // static to hold class pointer

//______________________________________________________________________________
const char *GoodHelicities::Class_Name()
{
   return "GoodHelicities";
}

//______________________________________________________________________________
const char *GoodHelicities::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::GoodHelicities*)nullptr)->GetImplFileName();
}

//______________________________________________________________________________
int GoodHelicities::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::GoodHelicities*)nullptr)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *GoodHelicities::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::GoodHelicities*)nullptr)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *GoodHelicities::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::GoodHelicities*)nullptr)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
void window_t::Streamer(TBuffer &R__b)
{
   // Stream an object of class window_t.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(window_t::Class(),this);
   } else {
      R__b.WriteClassBuffer(window_t::Class(),this);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_window_t(void *p) {
      return  p ? new(p) ::window_t : new ::window_t;
   }
   static void *newArray_window_t(Long_t nElements, void *p) {
      return p ? new(p) ::window_t[nElements] : new ::window_t[nElements];
   }
   // Wrapper around operator delete
   static void delete_window_t(void *p) {
      delete (static_cast<::window_t*>(p));
   }
   static void deleteArray_window_t(void *p) {
      delete [] (static_cast<::window_t*>(p));
   }
   static void destruct_window_t(void *p) {
      typedef ::window_t current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::window_t

//______________________________________________________________________________
void GoodHelicities::Streamer(TBuffer &R__b)
{
   // Stream an object of class GoodHelicities.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(GoodHelicities::Class(),this);
   } else {
      R__b.WriteClassBuffer(GoodHelicities::Class(),this);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_GoodHelicities(void *p) {
      return  p ? new(p) ::GoodHelicities : new ::GoodHelicities;
   }
   static void *newArray_GoodHelicities(Long_t nElements, void *p) {
      return p ? new(p) ::GoodHelicities[nElements] : new ::GoodHelicities[nElements];
   }
   // Wrapper around operator delete
   static void delete_GoodHelicities(void *p) {
      delete (static_cast<::GoodHelicities*>(p));
   }
   static void deleteArray_GoodHelicities(void *p) {
      delete [] (static_cast<::GoodHelicities*>(p));
   }
   static void destruct_GoodHelicities(void *p) {
      typedef ::GoodHelicities current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::GoodHelicities

namespace ROOT {
   static TClass *vectorlEHelPat_tgR_Dictionary();
   static void vectorlEHelPat_tgR_TClassManip(TClass*);
   static void *new_vectorlEHelPat_tgR(void *p = nullptr);
   static void *newArray_vectorlEHelPat_tgR(Long_t size, void *p);
   static void delete_vectorlEHelPat_tgR(void *p);
   static void deleteArray_vectorlEHelPat_tgR(void *p);
   static void destruct_vectorlEHelPat_tgR(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const vector<HelPat_t>*)
   {
      vector<HelPat_t> *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TIsAProxy(typeid(vector<HelPat_t>));
      static ::ROOT::TGenericClassInfo 
         instance("vector<HelPat_t>", -2, "vector", 389,
                  typeid(vector<HelPat_t>), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &vectorlEHelPat_tgR_Dictionary, isa_proxy, 0,
                  sizeof(vector<HelPat_t>), alignof(vector<HelPat_t>) );
      instance.SetNew(&new_vectorlEHelPat_tgR);
      instance.SetNewArray(&newArray_vectorlEHelPat_tgR);
      instance.SetDelete(&delete_vectorlEHelPat_tgR);
      instance.SetDeleteArray(&deleteArray_vectorlEHelPat_tgR);
      instance.SetDestructor(&destruct_vectorlEHelPat_tgR);
      static_assert(alignof(vector<HelPat_t>::value_type) <= 4096,
          "Class with alignment strictly greater than 4096 are currently not supported in CollectionProxy. "
          "Please report this case to the developers");
      instance.AdoptCollectionProxyInfo(TCollectionProxyInfo::Generate(TCollectionProxyInfo::Pushback< vector<HelPat_t> >()));

      instance.AdoptAlternate(::ROOT::AddClassAlternate("vector<HelPat_t>","std::vector<HelPat_t, std::allocator<HelPat_t> >"));
      return &instance;
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const vector<HelPat_t>*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));

   // Dictionary for non-ClassDef classes
   static TClass *vectorlEHelPat_tgR_Dictionary() {
      TClass* theClass =::ROOT::GenerateInitInstanceLocal(static_cast<const vector<HelPat_t>*>(nullptr))->GetClass();
      vectorlEHelPat_tgR_TClassManip(theClass);
   return theClass;
   }

   static void vectorlEHelPat_tgR_TClassManip(TClass* ){
   }

} // end of namespace ROOT

namespace ROOT {
   // Wrappers around operator new
   static void *new_vectorlEHelPat_tgR(void *p) {
      return  p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) vector<HelPat_t> : new vector<HelPat_t>;
   }
   static void *newArray_vectorlEHelPat_tgR(Long_t nElements, void *p) {
      return p ? ::new(static_cast<::ROOT::Internal::TOperatorNewHelper*>(p)) vector<HelPat_t>[nElements] : new vector<HelPat_t>[nElements];
   }
   // Wrapper around operator delete
   static void delete_vectorlEHelPat_tgR(void *p) {
      delete (static_cast<vector<HelPat_t>*>(p));
   }
   static void deleteArray_vectorlEHelPat_tgR(void *p) {
      delete [] (static_cast<vector<HelPat_t>*>(p));
   }
   static void destruct_vectorlEHelPat_tgR(void *p) {
      typedef vector<HelPat_t> current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class vector<HelPat_t>

namespace ROOT {
   // Registration Schema evolution read functions
   int RecordReadRules_libGoodHelicities() {
      return 0;
   }
   static int _R__UNIQUE_DICT_(ReadRules_libGoodHelicities) = RecordReadRules_libGoodHelicities();R__UseDummy(_R__UNIQUE_DICT_(ReadRules_libGoodHelicities));
} // namespace ROOT
namespace {
  void TriggerDictionaryInitialization_libGoodHelicities_Impl() {
    static const char* headers[] = {
"/adaqfs/home/hamoller/hamoller_analysis_tools/GoodHelicities/GoodHelicities.h",
nullptr
    };
    static const char* includePaths[] = {
"/adaqfs/apps/ROOT/6.40-02/el9/RelWithDebInfo/include",
"/adaqfs/home/hamoller/hamoller_analysis_tools/TTrain",
"/adaqfs/home/hamoller/hamoller_analysis_tools/HelPat_t",
"/adaqfs/home/hamoller/hamoller_analysis_tools/GoodHelicities",
"/adaqfs/apps/ROOT/6.40-02/el9/RelWithDebInfo/include",
"/adaqfs/home/hamoller/hamoller_analysis_tools/GoodHelicities/build/",
nullptr
    };
    static const char* fwdDeclCode = R"DICTFWDDCLS(
#line 1 "libGoodHelicities dictionary forward declarations' payload"

#pragma diagnostic push
#pragma clang diagnostic ignored "-Wkeyword-compat"
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern int __Cling_AutoLoading_Map;
struct  __attribute__((annotate("$clingAutoload$/adaqfs/home/hamoller/hamoller_analysis_tools/GoodHelicities/GoodHelicities.h")))  window_t;
class  __attribute__((annotate("$clingAutoload$/adaqfs/home/hamoller/hamoller_analysis_tools/GoodHelicities/GoodHelicities.h")))  GoodHelicities;
#pragma diagnostic pop
)DICTFWDDCLS";
    static const char* payloadCode = R"DICTPAYLOAD(
#line 1 "libGoodHelicities dictionary payload"

#ifndef R__DUMMY_CXX_STANDARD_20
  #define R__DUMMY_CXX_STANDARD_20 1
#endif

#define _BACKWARD_BACKWARD_WARNING_H
// Inline headers
#include "/adaqfs/home/hamoller/hamoller_analysis_tools/GoodHelicities/GoodHelicities.h"

#undef  _BACKWARD_BACKWARD_WARNING_H
)DICTPAYLOAD";
    static const char* classesHeaders[] = {
"GoodHelicities", payloadCode, "@",
"window_t", payloadCode, "@",
nullptr
};
    static bool isInitialized = false;
    if (!isInitialized) {
      TROOT::RegisterModule("libGoodHelicities",
        headers, includePaths, payloadCode, fwdDeclCode,
        TriggerDictionaryInitialization_libGoodHelicities_Impl, {}, classesHeaders, /*hasCxxModule*/false);
      isInitialized = true;
    }
  }
  static struct DictInit {
    DictInit() {
      TriggerDictionaryInitialization_libGoodHelicities_Impl();
    }
  } __TheDictionaryInitializer;
}
void TriggerDictionaryInitialization_libGoodHelicities() {
  TriggerDictionaryInitialization_libGoodHelicities_Impl();
}
