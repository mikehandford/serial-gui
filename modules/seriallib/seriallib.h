/*
  ==============================================================================

    seriallib.h
    Created:
    Author:

  ==============================================================================
*/

/*******************************************************************************
The block below describes the properties of this module, and is read by
the Projucer to automatically generate project code that uses it.
For details about the syntax and how to create or use a module, see the
JUCE Module Format.txt file.


BEGIN_JUCE_MODULE_DECLARATION

ID:               seriallib
vendor:
version:          1.0.0
name:             Serial Library
description:      Cross platform serial library
website:
license:

dependencies:     juce_core juce_events
OSXFrameworks:    Foundation IOKit
iOSFrameworks:
windowsLibs:      SetupAPI

END_JUCE_MODULE_DECLARATION

*******************************************************************************/

#ifndef SERIAL_LIB_H_INCLUDED
#define SERIAL_LIB_H_INCLUDED

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

//==============================================================================
// DLL building settings on Windows
#if JUCE_MSVC
    #ifdef SERIALLIB_DLL_BUILD
        #define SERIALLIB_API __declspec (dllexport)
        #pragma warning (disable: 4251)
    #elif defined (SERIALLIB_DLL)
        #define SERIALLIB_API __declspec (dllimport)
        #pragma warning (disable: 4251)
    #endif
    #ifdef __INTEL_COMPILER
        #pragma warning (disable: 1125) // (virtual override warning)
    #endif
#elif defined (SERIALLIB_DLL) || defined (SERIALLIB_DLL_BUILD)
    #define SERIALLIB_API __attribute__ ((visibility("default")))
#endif

//==============================================================================
#ifndef SERIALLIB_API
    #define SERIALLIB_API   /**< This macro is added to all SerialLib public class declarations. */
#endif

#if JUCE_MSVC && SERIALLIB_DLL_BUILD
    #define SERIALLIB_PUBLIC_IN_DLL_BUILD(declaration)  public: declaration; private:
#else
    #define SERIALLIB_PUBLIC_IN_DLL_BUILD(declaration)  declaration;
#endif
//==============================================================================

//#include <juce_core/maths/juce_MathsFunctions.h>

//namespace SerialLib
//{


// Forward declerations


#include <stdio.h>

#if JUCE_MAC

#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/IOBSD.h>
#include <IOKit/storage/IOCDTypes.h>
#include <IOKit/serial/ioss.h>

#elif JUCE_WINDOWS

#include <windows.h>

#define USE_SETUPAPI
#ifdef USE_SETUPAPI
#include <setupapi.h>
#include <devguid.h>
#include <regstr.h>

// The following define is from ntddser.h in the DDK. It is
// needed for serial port enumeration.
#ifndef GUID_CLASS_COMPORT
DEFINE_GUID(GUID_CLASS_COMPORT, 0x86e0d1e0L, 0x8089, 0x11d0, 0x9c, 0xe4, \
			0x08, 0x00, 0x3e, 0x30, 0x1f, 0x73);
#endif // GUID_CLASS_COMPORT
#endif // USE_SETUPAPI


#endif


#include "serial.h"

//}

#endif //SERIAL_LIB_H_INCLUDED
