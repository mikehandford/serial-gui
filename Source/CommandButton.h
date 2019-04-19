/*
  ==============================================================================

    CommandButton.h
    Created: 9 Dec 2018 10:16:36am
    Author:  Michael Handford

  ==============================================================================
*/

#pragma once

#include "LabeledButton.h"

class CommandButton : public LabeledButton
{
public:
    CommandButton (const String& cmd, const String& label, const String btnText = "OFF");
    virtual ~CommandButton ();
    
    void onButtonClicked ();
    
    std::function<void(const String& cmd, int value)> _onStateChange;
    
    void serialiseSettings (XmlElement* element);
    void restoreSettings (const XmlElement& element);
    
private:
    
    String _cmd;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CommandButton)
};
