// Do NOT change. Changes will be lost next time file is generated

#define R__DICTIONARY_FILENAME G__Pedestal
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
#include "/adaqfs/home/hamoller/hamoller_analysis_tools/Pedestal/Pedestal.h"

// Header files passed via #pragma extra_include

// The generated code does not explicitly qualify STL entities
namespace std {} using namespace std;

namespace ROOT {
   static void *new_Pedestal(void *p = nullptr);
   static void *newArray_Pedestal(Long_t size, void *p);
   static void delete_Pedestal(void *p);
   static void deleteArray_Pedestal(void *p);
   static void destruct_Pedestal(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::Pedestal*)
   {
      ::Pedestal *ptr = nullptr;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::Pedestal >(nullptr);
      static ::ROOT::TGenericClassInfo 
         instance("Pedestal", ::Pedestal::Class_Version(), "Pedestal.h", 8,
                  typeid(::Pedestal), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::Pedestal::Dictionary, isa_proxy, 4,
                  sizeof(::Pedestal), alignof(::Pedestal) );
      instance.SetNew(&new_Pedestal);
      instance.SetNewArray(&newArray_Pedestal);
      instance.SetDelete(&delete_Pedestal);
      instance.SetDeleteArray(&deleteArray_Pedestal);
      instance.SetDestructor(&destruct_Pedestal);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::Pedestal*)
   {
      return GenerateInitInstanceLocal(static_cast<::Pedestal*>(nullptr));
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal(static_cast<const ::Pedestal*>(nullptr)); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

//______________________________________________________________________________
atomic_TClass_ptr Pedestal::fgIsA(nullptr);  // static to hold class pointer

//______________________________________________________________________________
const char *Pedestal::Class_Name()
{
   return "Pedestal";
}

//______________________________________________________________________________
const char *Pedestal::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::Pedestal*)nullptr)->GetImplFileName();
}

//______________________________________________________________________________
int Pedestal::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::Pedestal*)nullptr)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *Pedestal::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::Pedestal*)nullptr)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *Pedestal::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::Pedestal*)nullptr)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
void Pedestal::Streamer(TBuffer &R__b)
{
   // Stream an object of class Pedestal.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(Pedestal::Class(),this);
   } else {
      R__b.WriteClassBuffer(Pedestal::Class(),this);
   }
}

namespace ROOT {
   // Wrappers around operator new
   static void *new_Pedestal(void *p) {
      return  p ? new(p) ::Pedestal : new ::Pedestal;
   }
   static void *newArray_Pedestal(Long_t nElements, void *p) {
      return p ? new(p) ::Pedestal[nElements] : new ::Pedestal[nElements];
   }
   // Wrapper around operator delete
   static void delete_Pedestal(void *p) {
      delete (static_cast<::Pedestal*>(p));
   }
   static void deleteArray_Pedestal(void *p) {
      delete [] (static_cast<::Pedestal*>(p));
   }
   static void destruct_Pedestal(void *p) {
      typedef ::Pedestal current_t;
      (static_cast<current_t*>(p))->~current_t();
   }
} // end of namespace ROOT for class ::Pedestal

namespace ROOT {
   // Registration Schema evolution read functions
   int RecordReadRules_libPedestal() {
      return 0;
   }
   static int _R__UNIQUE_DICT_(ReadRules_libPedestal) = RecordReadRules_libPedestal();R__UseDummy(_R__UNIQUE_DICT_(ReadRules_libPedestal));
} // namespace ROOT
namespace {
  void TriggerDictionaryInitialization_libPedestal_Impl() {
    static const char* headers[] = {
"/adaqfs/home/hamoller/hamoller_analysis_tools/Pedestal/Pedestal.h",
nullptr
    };
    static const char* includePaths[] = {
"/adaqfs/apps/ROOT/6.40-02/el9/RelWithDebInfo/include",
"/adaqfs/home/hamoller/hamoller_analysis_tools/Pedestal/include",
"/adaqfs/home/hamoller/hamoller_analysis_tools/Pedestal",
"/adaqfs/apps/ROOT/6.40-02/el9/RelWithDebInfo/include",
"/adaqfs/home/hamoller/hamoller_analysis_tools/Pedestal/build/",
nullptr
    };
    static const char* fwdDeclCode = R"DICTFWDDCLS(
#line 1 "libPedestal dictionary forward declarations' payload"

#pragma diagnostic push
#pragma clang diagnostic ignored "-Wkeyword-compat"
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern int __Cling_AutoLoading_Map;
class  __attribute__((annotate("$clingAutoload$/adaqfs/home/hamoller/hamoller_analysis_tools/Pedestal/Pedestal.h")))  Pedestal;
#pragma diagnostic pop
)DICTFWDDCLS";
    static const char* payloadCode = R"DICTPAYLOAD(
#line 1 "libPedestal dictionary payload"

#ifndef R__DUMMY_CXX_STANDARD_20
  #define R__DUMMY_CXX_STANDARD_20 1
#endif

#define _BACKWARD_BACKWARD_WARNING_H
// Inline headers
#include "/adaqfs/home/hamoller/hamoller_analysis_tools/Pedestal/Pedestal.h"

#undef  _BACKWARD_BACKWARD_WARNING_H
)DICTPAYLOAD";
    static const char* classesHeaders[] = {
"Pedestal", payloadCode, "@",
nullptr
};
    static bool isInitialized = false;
    if (!isInitialized) {
      TROOT::RegisterModule("libPedestal",
        headers, includePaths, payloadCode, fwdDeclCode,
        TriggerDictionaryInitialization_libPedestal_Impl, {}, classesHeaders, /*hasCxxModule*/false);
      isInitialized = true;
    }
  }
  static struct DictInit {
    DictInit() {
      TriggerDictionaryInitialization_libPedestal_Impl();
    }
  } __TheDictionaryInitializer;
}
void TriggerDictionaryInitialization_libPedestal() {
  TriggerDictionaryInitialization_libPedestal_Impl();
}
