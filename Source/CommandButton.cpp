/*
  ==============================================================================

    CommandButton.cpp
    Created: 9 Dec 2018 10:16:36am
    Author:  Michael Handford

  ==============================================================================
*/

#include "CommandButton.h"

CommandButton::CommandButton (const String& cmd, const String& label, const String btnText)
    : LabeledButton (label, btnText)
    , _cmd (cmd)
{
    _button.onClick = [this]() {onButtonClicked ();};
}

CommandButton::~CommandButton ()
{
}

void CommandButton::onButtonClicked ()
{
    if ( _onStateChange != nullptr)
    {
        int state = (int)_button.getToggleState();
        state = !state; // toggle
        _button.setToggleState(state, dontSendNotification);
        _button.setButtonText(state ? "ON" : "OFF");
        _onStateChange (_cmd, state);
    }
}

void CommandButton::serialiseSettings (XmlElement* element)
{
    element->setAttribute (_cmd, (int)_button.getToggleState ());
}

void CommandButton::restoreSettings (const XmlElement& element)
{
    if (element.hasAttribute(_cmd)) {
        auto value = element.getIntAttribute (_cmd, -1);
        
        bool newState = value > 0 ? 1 : 0;
        
        if (newState != _button.getToggleState ())
        {
            _button.setToggleState (newState, NotificationType::dontSendNotification);
            _button.setButtonText (newState ? "ON" : "OFF");
            _onStateChange (_cmd, newState);
        }
    }
}
