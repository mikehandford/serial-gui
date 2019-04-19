/*
==============================================================================

This file was auto-generated!

==============================================================================
*/

#include "MainComponent.h"

#include <sstream>
#include <iomanip>

//==============================================================================
MainComponent::MainComponent ()
    : _serialPortCombo ("SerialPortComboBox")
    , _savePresetButton ("Save Soundset", "SAVE")
    , _openPresetButton ("Open Soundset", "OPEN")
    , _htButton ("HV", "High Voltage!")
{
    //get a list of serial ports installed on the system, as a StringPairArray containing a friendly name and the port path
    StringPairArray portlist = SerialPort::getSerialPortPaths ();

    int portIndexToUse = 0;

    // Populate comboBox with available COM ports
    Logger::outputDebugString (portlist.getDescription ());
    for (auto portPath : portlist.getAllKeys ())
    {
        _serialPortCombo.addItem (portPath, ++portIndexToUse); // Index must start from 1
    }

    portIndexToUse = 0;
    for (auto portPath : portlist.getAllValues ())
    {
        if (portPath.contains ("ftdi"))
        {
            Logger::outputDebugString ("Found an FTDI device Using port [" + String (portIndexToUse) + "] " + portPath);

            _serialPortCombo.setSelectedId (portIndexToUse + 1, NotificationType::sendNotification); // This calls the combo change handler to open the port
            break;
        }
        ++portIndexToUse;
    }

    // Handler for combobox changes
    _serialPortCombo.onChange = [this] {
        auto portId = _serialPortCombo.getSelectedId () - 1;

        StringPairArray portlist = SerialPort::getSerialPortPaths ();
        _serialPort = new SerialPort (portlist.getAllValues ()[portId], SerialPortConfig (115200, 8, SerialPortConfig::SERIALPORT_PARITY_EVEN, SerialPortConfig::STOPBITS_2, SerialPortConfig::FLOWCONTROL_NONE));

        //create streams for reading/writing
        _pOutputStream = new SerialPortOutputStream (_serialPort);
        _pInputStream = new SerialPortInputStream (_serialPort);

    };

    // Handler for encoder values changes
    auto valueChangeHandler = [this](const String& cmd, int value) {
        std::stringstream stream;
        stream << std::hex << std::setfill ('0') << std::setw (3) << std::hex << value;
        std::string value3char (stream.str ());

        String cmdAndValue (cmd);
        cmdAndValue.append (value3char, 3);
        cmdAndValue.append (String ("\r"), 1);

        //Logger::outputDebugString("Last Command: " + cmdAndValue);
        _previousCommands.setText ("Last Command: " + cmdAndValue, NotificationType::dontSendNotification);

        if (_pOutputStream != nullptr) {
            _pOutputStream->write (cmdAndValue.toRawUTF8 (), cmdAndValue.getNumBytesAsUTF8 ());
        }
        Time::waitForMillisecondCounter (Time::getMillisecondCounter () + 3);
    };

    auto buttonStateChangeHandler = [this](const String& cmd, int value) {
        String cmdAndValue (cmd);
        cmdAndValue.append (value ? "ON\r" : "OFF\r", value ? 3 : 4);

        //Logger::outputDebugString (cmdAndValue);
        _previousCommands.setText ("Last Command: " + cmdAndValue, NotificationType::dontSendNotification);

        if (_pOutputStream != nullptr) {
            _pOutputStream->write (cmdAndValue.toRawUTF8 (), cmdAndValue.getNumBytesAsUTF8 ());
        }
        Time::waitForMillisecondCounter (Time::getMillisecondCounter () + 3);
    };

    auto savePresetHandler = [this]()
    {
        WildcardFileFilter wildcardFilter ("*.ss", String (), "Soundset Files");
        FileBrowserComponent browser ((FileBrowserComponent::canSelectFiles |
            FileBrowserComponent::saveMode |
            FileBrowserComponent::warnAboutOverwriting),
            File (),
            &wildcardFilter,
            nullptr);

        FileChooserDialogBox dialogBox ("Save Soundset file",
            "Please choose a filename to save as",
            browser,
            false,
            Colours::lightgrey);
        if (dialogBox.show ())
        {
            File selectedFile = browser.getSelectedFile (0);
            Logger::outputDebugString ("Saving Soundset to file: " + selectedFile.getFullPathName ());

            ScopedPointer<XmlElement> presetState (new XmlElement (XML_TAG_PRESET));

            //_htButton.serialiseSettings (presetState);

            for (auto enc : _encoders)
            {
                enc->serialiseSettings (presetState);
            }

            if (!selectedFile.hasFileExtension (".ss"))
            {
                selectedFile = selectedFile.withFileExtension (".ss");
            }

            if (selectedFile.exists ()) {
                selectedFile.replaceWithText ("");
            }
            else {
                selectedFile.create ();
            }

            presetState->writeToFile (selectedFile, "");
        }
    };

    auto openPresetHandler = [this]()
    {
        WildcardFileFilter wildcardFilter ("*.ss", String (), "Soundset Files");
        FileBrowserComponent browser ((FileBrowserComponent::canSelectFiles |
            FileBrowserComponent::openMode),
            File (),
            &wildcardFilter,
            nullptr);

        FileChooserDialogBox dialogBox ("Open Soundset file",
            "Please choose a Soundset file to open",
            browser,
            false,
            Colours::lightgrey);
        if (dialogBox.show ())
        {
            File selectedFile = browser.getSelectedFile (0);
            Logger::outputDebugString ("Opening preset file: " + selectedFile.getFullPathName ());

            if (selectedFile.exists ())
            {
                if (ScopedPointer<XmlElement> root = XmlDocument::parse (selectedFile))
                {
                    if (root->hasTagName (XML_TAG_PRESET))
                    {
                        //_htButton.restoreSettings (*root);

                        for (auto enc : _encoders)
                        {
                            enc->restoreSettings (*root);
                        }
                    }
                }
            }
        }
    };

    // GUI

    addAndMakeVisible (_serialPortCombo);

    addAndMakeVisible (_savePresetButton);
    _savePresetButton._button.onClick = savePresetHandler;

    addAndMakeVisible (_openPresetButton);
    _openPresetButton._button.onClick = openPresetHandler;

    addAndMakeVisible (_htButton);
    _htButton._onStateChange = buttonStateChangeHandler;


    // Create encoders
    _encoders.add (new CommandEncoder ("HV", "Voltage", 0xfff, 0x7ff));
    _encoders.add (new CommandEncoder ("HTR", "Heater Voltage", 0xfff, 0xfff));
    _encoders.add (new CommandEncoder ("P", "Pulse Time", 0x3, 0x3));
    _encoders.add (new CommandEncoder ("F", "Frequency", 0x3, 0x2));

    _encoders.add (new CommandEncoder ("V1", "Volume", 150, 0xf));
    _encoders.add (new CommandEncoder ("B1", "Bass", 150, 0xf));
    _encoders.add (new CommandEncoder ("M1", "Middle", 150, 0xf));
    _encoders.add (new CommandEncoder ("T1", "Trebble", 150, 0xf));
    _encoders.add (new CommandEncoder ("A1", "Anode 1", 0xf, 0xf));
    _encoders.add (new CommandEncoder ("G1", "Grid 1", 0xfff, 0x1ff));
    _encoders.add (new CommandEncoder ("A2", "Anode 2", 0xf, 0xf));
    _encoders.add (new CommandEncoder ("G2", "Grid 2", 0xfff, 0x1ff));

    _encoders.add (new CommandEncoder ("V2", "Volume", 150, 0xf));
    _encoders.add (new CommandEncoder ("B2", "Bass", 150, 0xf));
    _encoders.add (new CommandEncoder ("M2", "Middle", 150, 0xf));
    _encoders.add (new CommandEncoder ("T2", "Trebble", 150, 0xf));
    _encoders.add (new CommandEncoder ("A3", "Anode 3", 0xf, 0xf));
    _encoders.add (new CommandEncoder ("G3", "Grid 3", 0xfff, 0x1ff));
    _encoders.add (new CommandEncoder ("A4", "Anode 4", 0xf, 0xf));
    _encoders.add (new CommandEncoder ("G4", "Grid 4", 0xfff, 0x1ff));

    _encoders.add (new CommandEncoder ("V3", "Volume", 150, 0xf));
    _encoders.add (new CommandEncoder ("B3", "Bass", 150, 0xf));
    _encoders.add (new CommandEncoder ("M3", "Middle", 150, 0xf));
    _encoders.add (new CommandEncoder ("T3", "Trebble", 150, 0xf));
    _encoders.add (new CommandEncoder ("A5", "Anode 5", 0xf, 0xf));
    _encoders.add (new CommandEncoder ("G5", "Grid 5", 0xfff, 0x1ff));
    _encoders.add (new CommandEncoder ("A6", "Anode 6", 0xf, 0xf));
    _encoders.add (new CommandEncoder ("G6", "Grid 6", 0xfff, 0x1ff));

    // Add encoders and set handlers
    for (auto enc : _encoders)
    {
        addAndMakeVisible (enc);
        enc->_onValueChange = valueChangeHandler;
    }

    addAndMakeVisible (_previousCommands);
    setSize (1200, 700);
}

MainComponent::~MainComponent ()
{
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel ().findColour (ResizableWindow::backgroundColourId));
}

void MainComponent::resized ()
{
    static int borderPixels = 10;
    static int rowHeight = 128;
    static int controlWidth = 128;

    auto bounds = getLocalBounds ();
    bounds.reduce (borderPixels, borderPixels);

    auto boundsTopRow = bounds.removeFromTop (rowHeight);
    auto comPanelBounds = boundsTopRow.removeFromLeft (controlWidth * 2);
    comPanelBounds.removeFromTop (rowHeight / 4);
    _serialPortCombo.setBounds (comPanelBounds.removeFromTop (rowHeight / 4));
    comPanelBounds.removeFromTop (10);
    _previousCommands.setBounds (comPanelBounds.removeFromTop (rowHeight / 4));

    _savePresetButton.setBounds (boundsTopRow.removeFromLeft (controlWidth));
    _openPresetButton.setBounds (boundsTopRow.removeFromLeft (controlWidth));
    _htButton.setBounds (boundsTopRow.removeFromLeft (controlWidth));

    int i = 0;

    auto nextRow = bounds.removeFromTop (rowHeight);
    _encoders[i++]->setBounds (nextRow.removeFromLeft (controlWidth));
    _encoders[i++]->setBounds (nextRow.removeFromLeft (controlWidth));
    _encoders[i++]->setBounds (nextRow.removeFromLeft (controlWidth));
    _encoders[i++]->setBounds (nextRow.removeFromLeft (controlWidth));
    bounds.removeFromTop (borderPixels);

    nextRow = bounds.removeFromTop (rowHeight);
    _encoders[i++]->setBounds (nextRow.removeFromLeft (controlWidth));
    _encoders[i++]->setBounds (nextRow.removeFromLeft (controlWidth));
    _encoders[i++]->setBounds (nextRow.removeFromLeft (controlWidth));
    _encoders[i++]->setBounds (nextRow.removeFromLeft (controlWidth));
    _encoders[i++]->setBounds (nextRow.removeFromLeft (controlWidth));
    _encoders[i++]->setBounds (nextRow.removeFromLeft (controlWidth));
    _encoders[i++]->setBounds (nextRow.removeFromLeft (controlWidth));
    _encoders[i++]->setBounds (nextRow.removeFromLeft (controlWidth));
    bounds.removeFromTop (borderPixels);

    nextRow = bounds.removeFromTop (rowHeight);
    _encoders[i++]->setBounds (nextRow.removeFromLeft (controlWidth));
    _encoders[i++]->setBounds (nextRow.removeFromLeft (controlWidth));
    _encoders[i++]->setBounds (nextRow.removeFromLeft (controlWidth));
    _encoders[i++]->setBounds (nextRow.removeFromLeft (controlWidth));
    _encoders[i++]->setBounds (nextRow.removeFromLeft (controlWidth));
    _encoders[i++]->setBounds (nextRow.removeFromLeft (controlWidth));
    _encoders[i++]->setBounds (nextRow.removeFromLeft (controlWidth));
    _encoders[i++]->setBounds (nextRow.removeFromLeft (controlWidth));
    bounds.removeFromTop (borderPixels);

    nextRow = bounds.removeFromTop (rowHeight);
    _encoders[i++]->setBounds (nextRow.removeFromLeft (controlWidth));
    _encoders[i++]->setBounds (nextRow.removeFromLeft (controlWidth));
    _encoders[i++]->setBounds (nextRow.removeFromLeft (controlWidth));
    _encoders[i++]->setBounds (nextRow.removeFromLeft (controlWidth));
    _encoders[i++]->setBounds (nextRow.removeFromLeft (controlWidth));
    _encoders[i++]->setBounds (nextRow.removeFromLeft (controlWidth));
    _encoders[i++]->setBounds (nextRow.removeFromLeft (controlWidth));
    _encoders[i++]->setBounds (nextRow.removeFromLeft (controlWidth));
    bounds.removeFromTop (borderPixels);
}

//////////////////////////////////////////////////////////////////////////////////


