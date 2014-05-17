     /*********************************************************************
     
     omega.h
     
     Copyright (c) Creative Technology Ltd. 1994-1995. All Rights Reserved.
     
     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
     KIND,  EITHER EXPRESSED OR IMPLIED,  INCLUDING BUT NOT LIMITED TO THE 
     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR 
     PURPOSE.
     
     *********************************************************************/
//*****************************************************************************
//
//                          Copyright (c) 1994
//               E-mu Systems Proprietary All rights Reserved.
//                             
//*****************************************************************************

#ifndef __OMEGA_H
#define __OMEGA_H

//*******************************************************************
// @(#)omega.h	1.1 12:06:31 3/15/95 12:06:36
//
//  File:           OMEGA.H
//
//  Description: 
//
//  The error class to be included in other classes to keep track of
//  their instance error-states.
//
//*******************************************************************


#include "DATATYPE.H"

#define SUCCESS 0

///////////////////////////
//   Class Declarations  //
///////////////////////////

typedef CHAR *ErrVec;

class OmegaClass
{
  public:
    OmegaClass(ErrVec *EVec); 
    OmegaClass(void); 

    ~OmegaClass(void) {}

    void     SetError(SHORT shError)  { shOmega = shError;            }
    SHORT    GetError(void) const     { return (shOmega);             }
    CHAR*   GetErrorStr(void)         { return ErrorArray[shOmega];   } 
    BOOL    IsBad(void)               { return (shOmega != SUCCESS);  }
    BOOL    IsOK(void)                { return (shOmega == SUCCESS);  }
    void    SetErrors(ErrVec *EVec)   { ErrorArray = EVec;            }
    void    ReportError(void);
  private:

    SHORT    shOmega;   // this will be updated to a DWORD for v2.0 of omega
    ErrVec  *ErrorArray;
};


#endif // __OMEGA_H
////////////////////////// End of OMEGA.H ///////////////////////////
