//---------------------------------------------------------------------------
// File: functiondll.h
// Project: CPL function DLL
//
// Description:
//   The header file containing definition of the interface of the
//   CPL function DLL.
//
//---------------------------------------------------------------------------
// Copyright (c) by Vector Informatik GmbH.  All rights reserved.
//---------------------------------------------------------------------------
#pragma once

  #ifndef IMPORT_FUNCTIONS_DLL
    #define MYAPI(ret) __declspec(dllexport) ret
  #endif

  //-------------------------------------------------------------------------
  // Version control definition
  //-------------------------------------------------------------------------
  #define FUN_DLL_MAIN_VERSION    2
  #define FUN_DLL_SUB_VERSION     2
  #define FUN_DLL_RELEASE         0
  #define FUN_DLL_OS_VERSION      "Windows9x/WindowsNT/WindowsXP/Windows7"
  #define FUN_DLL_OS_RELEASE      1

  typedef struct {
    long   mainVersion;
    long   subVersion;
    long   release;
    char * osVersion;
    long   osRelease;
  } VFunDllVersion;

  //-------------------------------------------------------------------------
  // DLL Functions interface descriptor
  //-------------------------------------------------------------------------
  typedef struct {
    const char * function_name;
    const char * argstring;
  } VDllInterfaceTable;


  //-------------------------------------------------------------------------
  // Define flags for DLL Functions
  // (added in interface version 2.1)
  //-------------------------------------------------------------------------

  #define DF_FLG_NONE                     (0x00)
  #define DF_FLG_SLOW_FUNCTION            (0x01) // DLL-Function marked with this flag will be called from calling
                                                 // script synchronous (script waits until function finishes), but in own thread.
                                                 // This flag is intended to be used e.g. by implementation of function containing calculation which
                                                 // takes longer time (seconds or more).
                                                 // Notice: Without specifying this flag, the DLL function is called directly - blocking the application process
                                                 //         during whole execution time.
                                                 //
  #define DF_FLG_OVERRIDE_SHORT_DIMENSION (0x02) // DLL-Function marked with this flag accepts array parameters with more than 64K elements.
                                                 // If this flag is specified, the VDllFunArgVariant attributes xDim, yDim doesn't provide valid information and
                                                 // user must define additional parameters to provide valid array dimensions.
                                                 // Size of array passed to function flagged with DF_FLG_OVERRIDE_SHORT_DIMENSION is limited by maximal 1048576 (0x100000) elements.
                                                 // (added in interface version 2.2)

  typedef struct {
    const char *  function_name;      // Function Name - must match the name specified in VDllInterfaceTable
    unsigned long function_flags;     // Function Flags
    const char*   function_comment;   // Comment
    void*         reserved;           // Reserved
  } VDllInterfaceInfoTable;

  //-------------------------------------------------------------------------
  #ifndef IMPORT_FUNCTIONS_DLL
  //-------------------------------------------------------------------------

  //-------------------------------------------------------------------------
  // Data Type Definition
  // INT-types:
  // - Bit 0x80 set
  // - Bit 0x40 set if signed
  // - the rest is the bit-count - 1
  //
  // FLOAT-types / TRANSID-type:
  // - Bit 0x80 is not set
  //-------------------------------------------------------------------------
  typedef unsigned short VDatatypEnum;

  /* Datatype Classes */
  #define kDclassUnknown     0
  #define kDclassFloat       1
  #define kDclassDouble      2
  #define kDclassString      3
  #define kDclassInt         0xC0
  #define kDclassUint        0x80

  /* Specific predefined types */
  #define kDtypUnknown       0
  #define kDtypBit           0x80
  #define kDtypUByte         0x87
  #define kDtypUWord         0x8F
  #define kDtypULong         0x9F
  #define kDtypByte          0xC7
  #define kDtypWord          0xCF
  #define kDtypLong          0xDF
  #define kDtypFloat         1
  #define kDtypDouble        2

  //-------------------------------------------------------------------------
  #endif //IMPORT_FUNCTIONS_DLL
  //-------------------------------------------------------------------------


  //--------------------------------------
  // Attributes value structure
  //--------------------------------------
  typedef union {
    unsigned __int64 ull; // Not supported by scripting
    unsigned long    ul;
    long             l;
    float            f;
    double           d;
    struct s {
      unsigned char * pc;
      unsigned int    l;
    } s;
  } TDllFunValue;


  //--------------------------------------
  // Attributes  definition - Version 1.0
  //--------------------------------------
  typedef struct structVDllFunArgVariant {
    VDatatypEnum   type;   // Data type
    unsigned short yDim;   // Y-dimension (rows count)
    unsigned short xDim;   // X-dimension (columns count)
    TDllFunValue * data;   // Pointer to array of TDllFunValue objects

  } VDllFunArgVariant;



  #ifdef __cplusplus
    extern "C" {
  #endif

  MYAPI(const VDllInterfaceTable *)      funDllGetInterface(void);
  MYAPI(const VDllInterfaceInfoTable *)  funDllGetInterfaceInfoTable(void);
  MYAPI(long)                            funDllVersionControl(VFunDllVersion * version);
  MYAPI(double)                          funDllCall(VDllFunArgVariant * arg, int argCnt);

  #ifdef __cplusplus
    }
  #endif
