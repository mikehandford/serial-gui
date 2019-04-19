/*
  ==============================================================================

    LabeledEncoder.cpp
    Created: 9 Dec 2018 8:06:55am
    Author:  Michael Handford

  ==============================================================================
*/

#include "LabeledEncoder.h"

LabeledEncoder::LabeledEncoder (const String& label)
    : _label ("label", label)
    , _slider (Slider::Rotary, Slider::TextBoxBelow)
{
    _label.setJustificationType(Justification::horizontallyCentred);
    
    addAndMakeVisible (_label);
    addAndMakeVisible (_slider);
    
}

LabeledEncoder::~LabeledEncoder ()
{}

void LabeledEncoder::resized ()
{
    auto bounds = getLocalBounds();
    _label.setBounds (bounds.removeFromTop(16));
    _slider.setBounds (bounds);
}

void LabeledEncoder::addListener (Listener* listener)
{
    _slider.addListener (listener);
}

void LabeledEncoder::removeListener (Listener* listener)
{
    _slider.removeListener (listener);
}

