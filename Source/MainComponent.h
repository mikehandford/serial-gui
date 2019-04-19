/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include "CommandButton.h"
#include "CommandEncoder.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent   : public Component
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    
private:
    
    static constexpr const char* XML_TAG_PRESET = "preset";
    
    ScopedPointer<SerialPort> _serialPort;

    ScopedPointer<SerialPortOutputStream> _pOutputStream;
    ScopedPointer<SerialPortInputStream> _pInputStream;

    // GUI
    ComboBox      _serialPortCombo;
    LabeledButton _savePresetButton;
    LabeledButton _openPresetButton;
    CommandButton _htButton;
    
    OwnedArray<CommandEncoder> _encoders;

    Label _previousCommands;
    
    // Model State
    bool _htState = 0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
