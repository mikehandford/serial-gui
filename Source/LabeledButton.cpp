/*
  ==============================================================================

    LabeledButton.cpp
    Created: 9 Dec 2018 10:16:27am
    Author:  Michael Handford

  ==============================================================================
*/

#include "LabeledButton.h"

LabeledButton::LabeledButton (const String& label, const String& btnText)
    : _label ("label", label)
    , _button ("LabeledButton")
{
    _label.setJustificationType(Justification::horizontallyCentred);
    
    _button.setButtonText(btnText);
    
    addAndMakeVisible (_label);
    addAndMakeVisible (_button);
}

LabeledButton::~LabeledButton ()
{}

void LabeledButton::resized ()
{
    static int buttonSize = 48;
    auto bounds = getLocalBounds();
    _label.setBounds (bounds.removeFromTop(16));
    _button.setBounds (bounds.withSizeKeepingCentre(buttonSize, buttonSize));
}

void LabeledButton::addListener (Listener* listener)
{
    _button.addListener (listener);
}

void LabeledButton::removeListener (Listener* listener)
{
    _button.removeListener (listener);
}
