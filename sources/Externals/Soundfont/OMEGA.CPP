     /*********************************************************************
     
     omega.cpp
     
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


//*****************************************************************************
//  @(#)omega.cpp	1.1 12:06:44 3/15/95 12:06:47
// 
// File      : Omega.c
// 
// The module provides the implementation of the class Omega. Omega is a 
// set of routines which set and report error conditions. It is widely
// used within the codebase.
//
//  Name                   Date              Reason
//  -----------------      --------------    ----------------------
//  RSCrawford             7/7/94            Initial creation
//
//*****************************************************************************

#include "OMEGA.H"
#ifdef DEBUG
#include <iostream>
#endif


///////////////////////////
//    Class Methods      //
///////////////////////////

OmegaClass::OmegaClass(void)
{
 shOmega      = SUCCESS; 
 ErrorArray  = 0;
}

 OmegaClass::OmegaClass(ErrVec *EVec)
{
  shOmega     = SUCCESS; 
  ErrorArray = EVec;
} 




void OmegaClass::ReportError(void)
//*****************************************************************************
// 
// Implementation Notes: 
//
//    This function should be reimplmented in any new class derived 
// from this, that needs a different method for outputing the string values of 
// the error to whatever device is needed. (Windows, to a file, across the 
// net, to the moon, whatever...)
//
// History: 
//
//   Person               Date         Reason
// --------------------   -----------  ---------------------------------------
//
//****************************************************************************
{
#ifdef DEBUG
    std::cout << GetErrorStr() << std::endl;
#endif
}

