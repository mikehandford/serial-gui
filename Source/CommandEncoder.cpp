/*
==============================================================================

CommandEncoder.cpp
Created: 9 Dec 2018 8:53:38am
Author:  Michael Handford

==============================================================================
*/

#include "CommandEncoder.h"

CommandEncoder::CommandEncoder (const String& cmd, const String& label, const int maxValue, const int defaultValue)
    : LabeledEncoder (label)
    , _maxValueTextBox ("MaxValueLabel", String (maxValue))
    , _skewValueTextBox ("SkewValue", "0.5")
    , _cmd (cmd)
    , _maxValue (maxValue)
    , _lastValue (defaultValue)
{
    _slider.setRange (0, maxValue);
    _slider.setNumDecimalPlacesToDisplay (0);

    _slider.setValue (defaultValue); // Not going to call handler below!
    _slider.onValueChange = [this]() {encoderValueChange (); };

    _skewValueTextBox.setColour (Label::ColourIds::textColourId, Colours::orange);
    _skewValueTextBox.setEditable (true);
    _skewValueTextBox.setJustificationType (Justification::centred);
    _skewValueTextBox.addListener (this);

    _maxValueTextBox.setColour (Label::ColourIds::textColourId, Colours::yellow);
    _maxValueTextBox.setEditable (true);
    _maxValueTextBox.setJustificationType (Justification::centred);
    _maxValueTextBox.addListener (this);

    addAndMakeVisible (_skewValueTextBox);
    addAndMakeVisible (_maxValueTextBox);
}

CommandEncoder::~CommandEncoder ()
{
}

void CommandEncoder::encoderValueChange ()
{
    int newValue = (int)_slider.getValue ();

    if (_lastValue != newValue)
    {
        _lastValue = newValue;
        if (_onValueChange != nullptr) {
            _onValueChange (_cmd, (int)_slider.getValue ());
        }
    }
}

void CommandEncoder::resized ()
{
    auto bounds = getLocalBounds ();

    auto midBounds = bounds.withSizeKeepingCentre (50, 40);
    _skewValueTextBox.setBounds (midBounds.removeFromTop (20));
    _maxValueTextBox.setBounds (midBounds);

    LabeledEncoder::resized ();
}

// Max value label handler
void CommandEncoder::labelTextChanged (Label* labelThatHasChanged)
{
    auto value = labelThatHasChanged->getText ();

    if (labelThatHasChanged == &_maxValueTextBox)
    {
        int newMax = value.getIntValue ();

        if (newMax > _maxValue)
            newMax = _maxValue; // can't increase max value

        labelThatHasChanged->setText (String (newMax), NotificationType::dontSendNotification);

        // If value has to change then set new max value
        if (_slider.getValue () > newMax)
            _slider.setValue (newMax);

        _slider.setRange (0, newMax);
        _slider.setNumDecimalPlacesToDisplay (0);
    }
    else
    {
        auto newSkew = value.getFloatValue ();
        labelThatHasChanged->setText (String (newSkew), NotificationType::dontSendNotification);
        _slider.setSkewFactor (newSkew);
        repaint ();
    }
}

void CommandEncoder::serialiseSettings (XmlElement* element)
{
    element->setAttribute (_cmd, (int)_slider.getValue ());
}

void CommandEncoder::restoreSettings (const XmlElement& element)
{
    if (element.hasAttribute (_cmd)) {
        auto value = element.getIntAttribute (_cmd, -1);
        _slider.setValue (value);
    }
}
