/*
  ==============================================================================

    CommandEncoder.h
    Created: 9 Dec 2018 8:53:38am
    Author:  Michael Handford

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include "LabeledEncoder.h"

class CommandEncoder : public LabeledEncoder
                     , Label::Listener
{
public:
    CommandEncoder (const String& cmd, const String& label, const int maxValue, const int defaultValue);
    virtual ~CommandEncoder ();
    
    void encoderValueChange ();
    
    std::function<void(const String& cmd, int value)> _onValueChange;
    
    virtual void resized() override;
    
    void labelTextChanged (Label* labelThatHasChanged) override;
    
    void serialiseSettings (XmlElement* element);
    void restoreSettings (const XmlElement& element);
    
private:
    
    Label _maxValueTextBox;
    Label _skewValueTextBox;

    String _cmd;
    int _maxValue;

    int _lastValue;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CommandEncoder)
};

