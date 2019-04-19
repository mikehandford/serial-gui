/*
  ==============================================================================

    seriallib.cpp
    Created:
    Author:

  ==============================================================================
*/

#include "seriallib.h"

#if JUCE_MAC

#include "serial_macos.cpp"

#elif JUCE_WINDOWS

#include "serial_win32.cpp"

#endif


