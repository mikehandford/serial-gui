/*
  ==============================================================================

    LabeledButton.h
    Created: 9 Dec 2018 10:16:27am
    Author:  Michael Handford

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class LabeledButton : public Component
{
public:
    LabeledButton (const String& label, const String& btnText = "OFF");
    virtual ~LabeledButton ();
    
    void resized() override;
    
    Label _label;
    TextButton _button;
    
    typedef TextButton::Listener Listener;
    void addListener (Listener* listener);
    void removeListener (Listener* listener);
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LabeledButton)
};
