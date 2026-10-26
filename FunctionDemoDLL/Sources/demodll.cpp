/*--------------------------------------------------------------------------
// File:    DemoDll.cpp
// Project: CPL function DLL
//--------------------------------------------------------------------------
// Description:
//   The file contains a demonstration how to implement DLL functions
//   for CPL script language.
//
//
//---------------------------------------------------------------------------
// Copyright (c) by Vector Informatik GmbH.  All rights reserved.
----------------------------------------------------------------------------*/

#include <windows.h>
#include <string.h>
#include <float.h>

#include "functiondll.h"

#if defined(_DEBUG)
  #define new DEBUG_NEW
  #undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif

//-----------------------------------------------------------------------------
// Define exported functions
//-----------------------------------------------------------------------------
// As type of parameter are allowed:
//   VALUE       Single value          "Call by Value"
//   REFERENCE   Single value or array "Call by Reference"
//-----------------------------------------------------------------------------
const static VDllInterfaceTable sDllInterfaceDescriptorTable[]=
{
  { "ArithMinV",          "VALUE arg1, VALUE arg2" },
  { "ArithMaxV",          "VALUE arg1, VALUE arg2" },
  { "ArithMinB",          "REFERENCE arg" },
  { "ArithMaxB",          "REFERENCE arg" },
  { "ArithMiddleB",       "REFERENCE arg" },
  { "ChangeBuffer1D",     "REFERENCE arg1, REFERENCE arg2" },
  { "ChangeBuffer2D",     "REFERENCE arg1, REFERENCE arg2" },
  { "RotateString",       "REFERENCE arg1, REFERENCE arg2" },
  { "ArithMiddleB_Big",   "REFERENCE arg, VALUE dimension" },

  // The table must contains following line - table termination
  { NULL, NULL }
};

//-----------------------------------------------------------------------------
// Define Info Table for exported functions
//-----------------------------------------------------------------------------
// In this table you can specify function comment (occurring in Functions-Editor)
// and one or combination of following flags:
//
//   DF_FLG_NONE                      - no flag
//   DF_FLG_OVERRIDE_SHORT_DIMENSION  - allow to pass to function arrays with more than 64K elements
//   DF_FLG_SLOW_FUNCTION             - prevent function from blocking main application process during its execution.
//
//-----------------------------------------------------------------------------
const static VDllInterfaceInfoTable sDllInterfaceInfoTable[] =
{

  { "ArithMiddleB",      DF_FLG_NONE,                                            "Determine Arithmetic middle from array (max 65535 elements) "       , NULL },
  { "ArithMiddleB_Big",  DF_FLG_OVERRIDE_SHORT_DIMENSION | DF_FLG_SLOW_FUNCTION, "Determine Arithmetic middle from big array (max 1048576 elements)"  , NULL },

  /* The table must contains following line - table termination*/
  {NULL, 0, NULL, NULL}
};

//----------------------------------------------------------------------------
// Implement obligatory interface functions
//  - funDllVersionControl
//  - funDllGetInterface
//  - funDllGetInterfaceInfoTable
//----------------------------------------------------------------------------
MYAPI(long) funDllVersionControl(VFunDllVersion * version) {

  version->mainVersion= FUN_DLL_MAIN_VERSION;
  version->subVersion=  FUN_DLL_SUB_VERSION;
  version->release=     FUN_DLL_RELEASE;
  version->osVersion=   FUN_DLL_OS_VERSION;
  version->osRelease=   FUN_DLL_OS_RELEASE;

  return 1;
}

MYAPI(const VDllInterfaceTable*) funDllGetInterface()
{
  return sDllInterfaceDescriptorTable;
}

MYAPI(const VDllInterfaceInfoTable *)  funDllGetInterfaceInfoTable(void)
{
  return sDllInterfaceInfoTable;
}

//---------------------------------------------------------------------------------------
// Implementation of Helper functions
//
//  void SetString(VDllFunArgVariant* arg, const char * str);
//  int  GetString(VDllFunArgVariant* arg, char * str, unsigned int size);
//  int  GetValue (VDllFunArgVariant* arg, double* value, int row, int col);
//  void SetValue (VDllFunArgVariant* arg, double value, int row, int col);
//
//-----------------------------------------------------------------------------
// SetString(VDllFunArgVariant * arg, const char * str)
//
// Implementation of help function to fill out the argument with string
//-----------------------------------------------------------------------------
void SetString(VDllFunArgVariant * arg, const char * str)
{
  if ((arg->type == kDtypByte) ||
      (arg->type == kDtypUByte)) {

    int col= 0;
    int l=   (int) strlen(str);

    if (l > (arg->xDim - 1)) {
      l= (arg->xDim - 1);
    }

    while (col < l) {
      arg->data[col].l= str[col];
      col++;
    }

    //null terminated
    arg->data[col].l= '\0';
  }
  else {

    if (arg->type == kDclassString) {
      strncpy((char *) arg->data->s.pc, str, arg->data->s.l - 1);
    }
  }
}

//-----------------------------------------------------------------------------
// GetString(VDllFunArgVariant * arg, char * str, lon size)
//
// Implementation of help function to fill out the argument with string
//-----------------------------------------------------------------------------
int GetString(VDllFunArgVariant * arg, char * str, unsigned int size)
{
  long i;
  int result= 1;

  memset(str, '\0', size);

  if ((arg->type == kDtypByte) ||
      (arg->type == kDtypUByte)) {

    if (size > (unsigned int) arg->xDim) {

      for (i= 0; i < arg->xDim; i++) {

        if (arg->data[i].l) {
          str[i]= (char) arg->data[i].l;
        }
        else {
          break;
        }
      }
    }
    else {
      result= 0;
    }
  }
  else {

    if (arg->type == kDclassString) {

      if (size > arg->data->s.l) {

        strncpy(str, (char *) arg->data->s.pc, arg->data->s.l);
      }
      else {
        result= 0;
      }
    }
  }

  return result;
}

//-----------------------------------------------------------------------------
// GetValue(VDllFunArgVariant* arg, double* value, int row, int col)
//
// Implementation of help function to get the argument value
//
// Warning: The function doesn't support array with more than 64K elements
//          see DF_FLG_OVERRIDE_SHORT_DIMENSION flag description
//-----------------------------------------------------------------------------
int GetValue(VDllFunArgVariant* arg, double* value, int row, int col)
{
  if ( (arg->xDim<=col) || (arg->yDim<=row) ) return 0;

  switch(arg->type) {
    case kDtypUByte:
    case kDtypUWord:
    case kDtypULong:
       *value = (double)arg->data[(row*arg->xDim)+col].ul;
       break;
    case kDtypWord:
    case kDtypLong:
    case kDtypByte:
       *value = (double)arg->data[(row*arg->xDim)+col].l;
       break;
    case kDtypDouble:
       *value = (double)arg->data[(row*arg->xDim)+col].d;
       break;
    case kDtypFloat:
       *value = (double)arg->data[(row*arg->xDim)+col].f;
       break;
    case kDclassString:
       return 0;
    default: // handle not-predefined integer types (bit-fields) and unsupported types
      if      (arg->type & kDclassInt)  { *value = (double) arg->data[(row*arg->xDim)+col].l;  }
      else if (arg->type & kDclassUint) { *value = (double) arg->data[(row*arg->xDim)+col].ul; }
      else                              { return 0; /*unsupported type*/                       }
      break;
  }
  return 1;
}

//-----------------------------------------------------------------------------
// SetValue(VDllFunArgVariant* arg, double value, int row, int col)
//
// Implementation of help function to set the argument value
//
// Warning: The function doesn't support array with more than 64K elements
//          see DF_FLG_OVERRIDE_SHORT_DIMENSION flag description
//
//-----------------------------------------------------------------------------
void SetValue(VDllFunArgVariant* arg, double value, int row, int col)
{
  if ( (arg->xDim<=col) || (arg->yDim<=row) ) return;

  switch(arg->type) {
    case kDtypUByte:
    case kDtypUWord:
    case kDtypULong:
       arg->data[(row*arg->xDim)+col].ul = (unsigned long) value;
       break;
    case kDtypWord:
    case kDtypLong:
    case kDtypByte:
       arg->data[(row*arg->xDim)+col].l = (signed long) value;
       break;
    case kDtypDouble:
       arg->data[(row*arg->xDim)+col].d = (double) value;
       break;
    case kDtypFloat:
       arg->data[(row*arg->xDim)+col].f = (float) value;
       break;
    default: // handle not-predefined integer types (bit-fields) and unsupported types
      if      (arg->type & kDclassInt)  { arg->data[(row*arg->xDim)+col].l  = (signed long)   value; }
      else if (arg->type & kDclassUint) { arg->data[(row*arg->xDim)+col].ul = (unsigned long) value; }
      break;
  }
}
//---------------------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Declaration of exported functions
//----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif

// Declare Exported Functions
MYAPI(double) ArithMinV(VDllFunArgVariant * arg, int argCnt);
MYAPI(double) ArithMaxV(VDllFunArgVariant * arg, int argCnt);
MYAPI(double) ArithMinB(VDllFunArgVariant * arg, int argCnt);
MYAPI(double) ArithMaxB(VDllFunArgVariant * arg, int argCnt);
MYAPI(double) ArithMiddleB(VDllFunArgVariant * arg, int argCnt);
MYAPI(double) ArithMiddleB_Big(VDllFunArgVariant * arg, int argCnt);
MYAPI(double) ChangeBuffer1D(VDllFunArgVariant * arg, int argCnt);
MYAPI(double) ChangeBuffer2D(VDllFunArgVariant * arg, int argCnt);
MYAPI(double) RotateString(VDllFunArgVariant * arg, int argCnt);

#ifdef __cplusplus
}
#endif



//-----------------------------------------------------------------------------
// USER DEFINED FUNCTIONS
//
// Warning: All exported function must have the same interface as
//          predefined function funDllCall() (see functiondll.h)
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ArithMinV
//-----------------------------------------------------------------------------
// Example function to show how to work with values
//-----------------------------------------------------------------------------
// Definition: { "ArithMinV", "VALUE arg1, VALUE arg2" },
//-----------------------------------------------------------------------------
// Script:
//   double arg1, arg2, minimum;
//   arg1=    3.14;
//   arg2=    3.1415;
//   minimum= ArithMinV(arg1, arg2);
//   write("Minimum from %f and %f is %f", arg1, arg2, minimum);
//-----------------------------------------------------------------------------
MYAPI(double) ArithMinV(VDllFunArgVariant * arg, int argCnt)
{
  double arg1= DBL_MAX, arg2= DBL_MAX;

  // Check arguments
  if (argCnt      != 2)              return DBL_MAX;
  if (arg[0].type == kDclassUnknown) return DBL_MAX;
  if (arg[0].type == kDclassString)  return DBL_MAX;
  if (arg[1].type == kDclassUnknown) return DBL_MAX;
  if (arg[1].type == kDclassString)  return DBL_MAX;


  // Get/convert argument 1 to double
  switch(arg[0].type)
  {
  case kDtypUByte:
  case kDtypUWord:
  case kDtypULong:  arg1 = (double) arg[0].data->ul; break;
  case kDtypByte:
  case kDtypWord:
  case kDtypLong:   arg1 = (double) arg[0].data->l;  break;
  case kDtypFloat:  arg1 = (double) arg[0].data->f;  break;
  case kDtypDouble: arg1 =          arg[0].data->d;  break;
  default: // handle not-predefined integer types (bit-fields) and unsupported types
    if      (arg[0].type & kDclassInt)  { arg1 = (double) arg[0].data->l;         }
    else if (arg[0].type & kDclassUint) { arg1 = (double) arg[0].data->ul;        }
    else                                { /* unsupported type */                  }
    break;
  }

  // Get/convert argument 2 to double
  switch(arg[1].type)
  {
  case kDtypUByte:
  case kDtypUWord:
  case kDtypULong:  arg2 = (double) arg[1].data->ul; break;
  case kDtypByte:
  case kDtypWord:
  case kDtypLong:   arg2 = (double) arg[1].data->l;  break;
  case kDtypFloat:  arg2 = (double) arg[1].data->f;  break;
  case kDtypDouble: arg2 =          arg[1].data->d;  break;
  default: // handle not-predefined integer types (bit-fields) and unsupported types
    if      (arg[1].type & kDclassInt)  { arg2 = (double) arg[1].data->l;         }
    else if (arg[1].type & kDclassUint) { arg2 = (double) arg[1].data->ul;        }
    else                                { /* unsupported type */                  }
    break;
  }

  // Return the minimum
  return (arg1 < arg2) ? arg1 : arg2;
}

//-----------------------------------------------------------------------------
// ArithMaxV
//-----------------------------------------------------------------------------
// Example function to show how to work with values
//-----------------------------------------------------------------------------
// Definition: { "ArithMaxV", "VALUE arg1, VALUE arg2" },
//-----------------------------------------------------------------------------
// Script:
//   double arg1, arg2, maximum;
//   arg1=    3.14;
//   arg2=    3.1415;
//   maximum= ArithMaxV(arg1, arg2);
//   write("Maximum from %f and %f is %f", arg1, arg2, maximum);
//-----------------------------------------------------------------------------
MYAPI(double) ArithMaxV(VDllFunArgVariant * arg, int argCnt)
{
  double arg1= DBL_MIN, arg2= DBL_MIN;

  // Check arguments
  if (argCnt      != 2)              return DBL_MIN;
  if (arg[0].type == kDclassUnknown) return DBL_MIN;
  if (arg[0].type == kDclassString)  return DBL_MIN;
  if (arg[1].type == kDclassUnknown) return DBL_MIN;
  if (arg[1].type == kDclassString)  return DBL_MIN;

  // Get/convert argument 1 to double
  switch(arg[0].type)
  {
  case kDtypUByte:
  case kDtypUWord:
  case kDtypULong:  arg1 = (double) arg[0].data->ul; break;
  case kDtypByte:
  case kDtypWord:
  case kDtypLong:   arg1 = (double) arg[0].data->l;  break;
  case kDtypFloat:  arg1 = (double) arg[0].data->f;  break;
  case kDtypDouble: arg1 =          arg[0].data->d;  break;
  default: // handle not-predefined integer types (bit-fields) and unsupported types
    if      (arg[0].type & kDclassInt)  { arg1 = (double) arg[0].data->l;         }
    else if (arg[0].type & kDclassUint) { arg1 = (double) arg[0].data->ul;        }
    else                                { /* unsupported type */                  }
    break;
  }

  // Get/convert argument 2 to double
  switch(arg[1].type)
  {
  case kDtypUByte:
  case kDtypUWord:
  case kDtypULong:  arg2 = (double) arg[1].data->ul; break;
  case kDtypByte:
  case kDtypWord:
  case kDtypLong:   arg2 = (double) arg[1].data->l;  break;
  case kDtypFloat:  arg2 = (double) arg[1].data->f;  break;
  case kDtypDouble: arg2 =          arg[1].data->d;  break;
  default: // handle not-predefined integer types (bit-fields) and unsupported types
    if      (arg[1].type & kDclassInt)  { arg2 = (double) arg[1].data->l;         }
    else if (arg[1].type & kDclassUint) { arg2 = (double) arg[1].data->ul;        }
    else                                { /* unsupported type */                  }
    break;
  }
  // Return the minimum
  return (arg1 > arg2) ? arg1 : arg2;
}

//-----------------------------------------------------------------------------
// ArithMinB
//-----------------------------------------------------------------------------
// Example function to show how to work with 1 buffer
//-----------------------------------------------------------------------------
// Definition: { "ArithMinB", "REFERENCE arg"},
//-----------------------------------------------------------------------------
// Script:
//   double arg[128], min;
//   for (i= 0; i < 128; i++) arg[i]= i;
//   minimum= ArithMinB(arg);
//   write("Minimum from array is %f", minimum);
//-----------------------------------------------------------------------------
MYAPI(double) ArithMinB(VDllFunArgVariant * arg, int argCnt)
{
  double help, min= DBL_MAX;
  long   i;

  // Check arguments
  if (argCnt      != 1)              return DBL_MAX;
  if (arg[0].type == kDclassUnknown) return DBL_MAX;
  if (arg[0].type == kDclassString)  return DBL_MAX;

  for (i= 0; i < arg[0].xDim; i++) {

    help= DBL_MAX;

    switch(arg[0].type)
    {
    case kDtypUByte:
    case kDtypUWord:
    case kDtypULong:  help = (double) arg[0].data[i].ul; break;
    case kDtypByte:
    case kDtypWord:
    case kDtypLong:   help = (double) arg[0].data[i].l;  break;
    case kDtypFloat:  help = (double) arg[0].data[i].f;  break;
    case kDtypDouble: help =          arg[0].data[i].d;  break;
    default: // handle not-predefined integer types (bit-fields) and unsupported types
      if      (arg[0].type & kDclassInt)  { help = (double) arg[0].data[i].l;         }
      else if (arg[0].type & kDclassUint) { help = (double) arg[0].data[i].ul;        }
      else                                { /* unsupported type */                    }
      break;
    }

    if (help < min) min= help;
  }

  return min;
}

//-----------------------------------------------------------------------------
// ArithMaxB
//-----------------------------------------------------------------------------
// Example function to show how to work with 1 buffer
//-----------------------------------------------------------------------------
// Definition: { "ArithMaxB", "REFERENCE arg" },
//-----------------------------------------------------------------------------
// Script:
//   double arg[128], maximum;
//   for (i= 0; i < 128; i++) arg[i]= i;
//   maximum= ArithMaxB(arg);
//   write("Maximum from array is %f", maximum);
//-----------------------------------------------------------------------------
MYAPI(double) ArithMaxB(VDllFunArgVariant * arg, int argCnt)
{
  double help, max= DBL_MIN;
  long   i;

  // Check arguments
  if (argCnt      != 1)              return DBL_MIN;
  if (arg[0].type == kDclassUnknown) return DBL_MIN;
  if (arg[0].type == kDclassString)  return DBL_MIN;

  for (i= 0; i < arg[0].xDim; i++) {

    help= DBL_MIN;

    switch(arg[0].type)
    {
    case kDtypUByte:
    case kDtypUWord:
    case kDtypULong:  help = (double) arg[0].data[i].ul; break;
    case kDtypByte:
    case kDtypWord:
    case kDtypLong:   help = (double) arg[0].data[i].l;  break;
    case kDtypFloat:  help = (double) arg[0].data[i].f;  break;
    case kDtypDouble: help =          arg[0].data[i].d;  break;
    default: // handle not-predefined integer types (bit-fields) and unsupported types
      if      (arg[0].type & kDclassInt)  { help = (double) arg[0].data[i].l;         }
      else if (arg[0].type & kDclassUint) { help = (double) arg[0].data[i].ul;        }
      else                                { /* unsupported type */                    }
      break;
    }


    if (help > max) max= help;
  }

  return max;
}

//-----------------------------------------------------------------------------
// ArithMiddleB
//-----------------------------------------------------------------------------
// Example function to show how to work with 1 buffer
//-----------------------------------------------------------------------------
// Definition: { "ArithMiddleB", "REFERENCE arg" },
//-----------------------------------------------------------------------------
// Script:
//   double arg[128], middle;
//   for (i= 0; i < 128; i++) arg[i]= i;
//   middle= ArithMiddleB(arg);
//   write("Arithmetic middle from array is %f", middle);
//-----------------------------------------------------------------------------
MYAPI(double) ArithMiddleB(VDllFunArgVariant * arg, int argCnt)
{
  double sum= 0, help;
  long   i;

  // Check arguments
  if (argCnt      != 1)              return DBL_MIN;
  if (arg[0].type == kDclassUnknown) return DBL_MIN;
  if (arg[0].type == kDclassString)  return DBL_MIN;

  for (i= 0; i < arg[0].xDim; i++) {

    help= DBL_MIN;

    switch(arg[0].type)
    {
    case kDtypUByte:
    case kDtypUWord:
    case kDtypULong:  help = (double) arg[0].data[i].ul; break;
    case kDtypByte:
    case kDtypWord:
    case kDtypLong:   help = (double) arg[0].data[i].l;  break;
    case kDtypFloat:  help = (double) arg[0].data[i].f;  break;
    case kDtypDouble: help =          arg[0].data[i].d;  break;
    default: // handle not-predefined integer types (bit-fields) and unsupported types
      if      (arg[0].type & kDclassInt)  { help = (double) arg[0].data[i].l;         }
      else if (arg[0].type & kDclassUint) { help = (double) arg[0].data[i].ul;        }
      else                                { /* unsupported type */                    }
      break;
    }

    sum += help;
  }

  return sum / arg[0].xDim;
}

//-----------------------------------------------------------------------------
// ArithMiddleB_Big
//-----------------------------------------------------------------------------
// This example function show how to pass array with more than 64K elements
// to function DLL.
//-----------------------------------------------------------------------------
// Definition: { "ArithMiddleB_Big",   "REFERENCE arg, VALUE dimension" },
//-----------------------------------------------------------------------------
// Script:
//
//   double big_arg[1000000], middle;
//   long   arraySize, i;
//
//   arraySize = xdimension(big_arg);
//
//   for (i= 0; i < arraySize; i++) {
//     big_arg[i]= i;
//   }
//
//   middle = ArithMiddleB_Big(big_arg, arraySize);
//
//   print("Arithmetic middle from array is %f", middle);
//
//-----------------------------------------------------------------------------
MYAPI(double) ArithMiddleB_Big(VDllFunArgVariant * arg, int argCnt)
{
  double sum= 0, help;
  long   i;

  // Check arguments
  if (argCnt      != 2)              return DBL_MIN;
  if (arg[0].type == kDclassUnknown) return DBL_MIN;
  if (arg[0].type == kDclassString)  return DBL_MIN;


  // Get array size from second parameter 'arg[1]':
  // Because array size information transported in 'arg[0].xDim' is limited by its 'short' data type,
  // we must use extra argument to get real size of first argument.
  // Remember that function ArithMiddleB must be flagged with DF_FLG_OVERRIDE_SHORT_DIMENSION flag
  // to allow passing array with more than 64K to DLL function (see sDllInterfaceInfoTable)
  double arraySize;
  if ( GetValue(&arg[1], &arraySize, 0, 0) == 0 )  return DBL_MIN;

  for (i= 0; i < (long)arraySize; i++) {

    help= DBL_MIN;

    switch(arg[0].type)
    {
    case kDtypUByte:
    case kDtypUWord:
    case kDtypULong:  help = (double) arg[0].data[i].ul; break;
    case kDtypByte:
    case kDtypWord:
    case kDtypLong:   help = (double) arg[0].data[i].l;  break;
    case kDtypFloat:  help = (double) arg[0].data[i].f;  break;
    case kDtypDouble: help =          arg[0].data[i].d;  break;
    default: // handle not-predefined integer types (bit-fields) and unsupported types
      if      (arg[0].type & kDclassInt)  { help = (double) arg[0].data[i].l;         }
      else if (arg[0].type & kDclassUint) { help = (double) arg[0].data[i].ul;        }
      else                                { /* unsupported type */                    }
      break;
    }

    sum += help;
  }

  return sum / arraySize;
}
//-----------------------------------------------------------------------------
// ChangeBuffer1D
//-----------------------------------------------------------------------------
// Example function to show how to work with 2 buffers
//-----------------------------------------------------------------------------
// Definition: { "ChangeBuffer1D", "REFERENCE arg1, REFERENCE arg2" },
//-----------------------------------------------------------------------------
// Script:
//   double arg1[128], arg2[128];
//   for (i= 0; i < 128; i++) arg1[i]= i;
//   ChangeBuffer1D(arg1, arg2);
//-----------------------------------------------------------------------------
MYAPI(double) ChangeBuffer1D(VDllFunArgVariant * arg, int argCnt)
{
  long i;

  // Check arguments
  if (argCnt      != 2)              return  0;
  if (arg[0].type == kDclassUnknown) return -1;
  if (arg[0].type == kDclassString)  return -2;
  if (arg[1].type == kDclassUnknown) return -3;
  if (arg[1].type == kDclassString)  return -4;
  if (arg[0].type != arg[1].type)    return -5;
  if (arg[0].xDim != arg[1].xDim)    return -6;

  // Copy buffer 1 to buffer2
  for (i= 0; i < arg[0].xDim; i++) {

    switch(arg[0].type)
    {
    case kDtypUByte:
    case kDtypUWord:
    case kDtypULong:  arg[1].data[i].ul = arg[0].data[i].ul; break;
    case kDtypByte:
    case kDtypWord:
    case kDtypLong:   arg[1].data[i].l  = arg[0].data[i].l;  break;
    case kDtypFloat:  arg[1].data[i].f  = arg[0].data[i].f;  break;
    case kDtypDouble: arg[1].data[i].d  = arg[0].data[i].d;  break;
    default: // handle not-predefined integer types (bit-fields) and unsupported types
      if      (arg[0].type & kDclassInt)  { arg[1].data[i].l  = arg[0].data[i].l;     }
      else if (arg[0].type & kDclassUint) { arg[1].data[i].ul = arg[0].data[i].ul;    }
      else                                { /* unsupported type */                    }
      break;
    }
  }


  return 1;
}

//-----------------------------------------------------------------------------
// ChangeBuffer2D
//-----------------------------------------------------------------------------
// Example function to show how to work with 2 2D buffers
//-----------------------------------------------------------------------------
// Definition: { "ChangeBuffer2D", "REFERENCE arg1, REFERENCE arg2" },
//-----------------------------------------------------------------------------
// Script:
//   double arg1[128][128], arg2[128][128];
//   for (i= 0; i < 128; i++) {
//     arg1[i][0]= i;
//     arg1[0][i]= i;
//   }
//   ChangeBuffer2D(arg1, arg2);
//-----------------------------------------------------------------------------
MYAPI(double) ChangeBuffer2D(VDllFunArgVariant * arg, int argCnt)
{
  long row, col, pos;

  // Check arguments
  if (argCnt      != 2)              return  0;
  if (arg[0].type == kDclassUnknown) return -1;
  if (arg[0].type == kDclassString)  return -2;
  if (arg[1].type == kDclassUnknown) return -3;
  if (arg[1].type == kDclassString)  return -4;
  if (arg[0].type != arg[1].type)    return -5;
  if (arg[0].xDim != arg[1].xDim)    return -6;
  if (arg[0].yDim != arg[1].yDim)    return -7;

  // Go over all rows
  for (row= 0; row < arg[0].yDim; row++) {

    // Go over all cols
    for (col= 0; col < arg[0].xDim; col++) {

      // Calculate position in array
      pos= row * arg[0].xDim + col;

      // Copy the byte
      switch(arg[0].type)
      {
      case kDtypUByte:
      case kDtypUWord:
      case kDtypULong:  arg[1].data[pos].ul = arg[0].data[pos].ul; break;
      case kDtypByte:
      case kDtypWord:
      case kDtypLong:   arg[1].data[pos].l  = arg[0].data[pos].l;  break;
      case kDtypFloat:  arg[1].data[pos].f  = arg[0].data[pos].f;  break;
      case kDtypDouble: arg[1].data[pos].d  = arg[0].data[pos].d;  break;
      default: // handle not-predefined integer types (bit-fields) and unsupported types
        if      (arg[0].type & kDclassInt)  { arg[1].data[pos].l  = arg[0].data[pos].l;     }
        else if (arg[0].type & kDclassUint) { arg[1].data[pos].ul = arg[0].data[pos].ul;    }
        else                                { /* unsupported type */                        }
        break;
      }

    }
  }

  return 1;
};

//-----------------------------------------------------------------------------
// RotateString
//-----------------------------------------------------------------------------
// Example function to show how to work with strings
//-----------------------------------------------------------------------------
// Definition: { "RotateString", "REFERENCE arg1, REFERENCE arg2" },
//-----------------------------------------------------------------------------
// Script:
//   char buf[]= "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
//   char helpBuf[25];
//   if (RotateString(buf, helpBuf) > 0) {
//     write("String before rotating: %s", buf);
//     write("String after  rotating: %s", helpBuf);
//   }
//-----------------------------------------------------------------------------

MYAPI(double) RotateString(VDllFunArgVariant * arg, int argCnt)
{
  char buf[256], help[256];
  int  i, result= 1;

  // Check arguments
  if (argCnt      != 2)              return  0;
  if (arg[0].type == kDclassUnknown) return -1;
  if (arg[0].type == kDclassFloat)   return -2;
  if (arg[0].type == kDclassDouble)  return -3;
  if (arg[1].type == kDclassUnknown) return -4;
  if (arg[1].type == kDclassFloat)   return -5;
  if (arg[1].type == kDclassDouble)  return -6;

  // Get the string from the argument
  if (GetString(&arg[0], buf, sizeof(buf))) {

    // Rotate the string
    for (i= 0; i < (int) strlen(buf); i++) {
      help[i]= buf[strlen(buf) - i - 1];
    }

    help[i]= '\0';

    // Set the string in the argument
    SetString(&arg[1], help);
  }
  else {
    result= -7;
  }

  return result;
}
