/*
  ==============================================================================

    LabeledEncoder.h
    Created: 9 Dec 2018 8:06:55am
    Author:  Michael Handford

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class LabeledEncoder : public Component
{
public:
    LabeledEncoder (const String& label);
    virtual ~LabeledEncoder ();
    
    virtual void resized() override;
    
    Label _label;
    Slider _slider;
    
    typedef Slider::Listener Listener;
    void addListener (Listener* listener);
    void removeListener (Listener* listener);
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LabeledEncoder)
};

