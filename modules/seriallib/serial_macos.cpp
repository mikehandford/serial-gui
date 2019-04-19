//mac_SerialPort.cpp
//Serial Port classes in a Juce stylee, by graffiti
//see SerialPort.h for details
//
// Updated for current Juce API 8/1/13 Marc Lindahl
//



juce::StringPairArray SerialPort::getSerialPortPaths()
{
    juce::StringPairArray SerialPortPaths;
	io_iterator_t matchingServices;
	mach_port_t masterPort;
    CFMutableDictionaryRef classesToMatch;
	io_object_t modemService;
	char deviceFilePath[512];
	char deviceFriendly[1024];
    if (KERN_SUCCESS != IOMasterPort(MACH_PORT_NULL, &masterPort))
    {
        DBG("SerialPort::getSerialPortPaths : IOMasterPort failed");
		return SerialPortPaths;
    }
    classesToMatch = IOServiceMatching(kIOSerialBSDServiceValue);
    if (classesToMatch == NULL)
	{
		DBG("SerialPort::getSerialPortPaths : IOServiceMatching failed");
		return SerialPortPaths;
	}
	CFDictionarySetValue(classesToMatch, CFSTR(kIOSerialBSDTypeKey), CFSTR(kIOSerialBSDRS232Type));    
	if (KERN_SUCCESS != IOServiceGetMatchingServices(masterPort, classesToMatch, &matchingServices))
	{
		DBG("SerialPort::getSerialPortPaths : IOServiceGetMatchingServices failed");
		return SerialPortPaths;
	}
	while ((modemService = IOIteratorNext(matchingServices)))
	{
		CFTypeRef   deviceFilePathAsCFString;
		CFTypeRef   deviceFriendlyAsCFString;
		deviceFilePathAsCFString = IORegistryEntryCreateCFProperty(modemService,CFSTR(kIODialinDeviceKey), kCFAllocatorDefault, 0);
		deviceFriendlyAsCFString = IORegistryEntryCreateCFProperty(modemService,CFSTR(kIOTTYDeviceKey), kCFAllocatorDefault, 0);
		if(deviceFilePathAsCFString)
		{
			if(CFStringGetCString((const __CFString*)deviceFilePathAsCFString, deviceFilePath, 512, kCFStringEncodingASCII)
			&& CFStringGetCString((const __CFString*)deviceFriendlyAsCFString, deviceFriendly, 1024, kCFStringEncodingASCII) )
				SerialPortPaths.set(deviceFriendly, deviceFilePath);
			CFRelease(deviceFilePathAsCFString);
			CFRelease(deviceFriendlyAsCFString);
		}
	}
	IOObjectRelease(modemService);
	return SerialPortPaths;
}

bool SerialPort::exists()
{
	return (-1 != _portDescriptor);
}

void SerialPort::close()
{
	if(-1 != _portDescriptor)
	{
		//wait for garbage to go? nah...
		//tcdrain(portDescriptor);
		::close(_portDescriptor);
		_portDescriptor = -1;
	}
}

bool SerialPort::open(const juce::String& portPath)
{
	_portPath = portPath;
    struct termios options;
	_portDescriptor = ::open(_portPath.getCharPointer(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (_portDescriptor == -1)
    {
        DBG("SerialPort::open : open() failed");
        return false;
    }
    // don't allow multiple opens
    if (ioctl(_portDescriptor, TIOCEXCL) == -1)
    {
        DBG("SerialPort::open : ioctl error, non critical");
    }
    // we want blocking io actually
	if (fcntl(_portDescriptor, F_SETFL, 0) == -1)
    {
        DBG("SerialPort::open : fcntl error");
		close();
        return false;
    }
	// Get the current options
    if (tcgetattr(_portDescriptor, &options) == -1)
    {
        DBG("SerialPort::open : can't get port settings to set timeouts");
		close();
        return false;
    }
	//non canocal, 0.5 second timeout, read returns as soon as any data is recieved
	cfmakeraw(&options);
    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = 5;
	if (tcsetattr(_portDescriptor, TCSANOW, &options) == -1)
    {
        DBG("SerialPort::open : can't set port settings (timeouts)");
		close();
        return false;
    }
	return true;
}

bool SerialPort::setConfig(const SerialPortConfig& config)
{
	if(-1 == _portDescriptor) return false;
	struct termios options;
	memset(&options, 0, sizeof(struct termios));
	//non canocal, 0.5 second timeout, read returns as soon as any data is recieved
	cfmakeraw(&options);
    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = 5;
	options.c_cflag |= CREAD; //enable reciever (daft)
	options.c_cflag |= CLOCAL;//don't monitor modem control lines
	//baud and bits
	cfsetspeed(&options, config._bps);
	switch(config._databits)
	{
		case 5: options.c_cflag |= CS5; break;
		case 6: options.c_cflag |= CS6; break;
		case 7: options.c_cflag |= CS7; break;
		case 8: options.c_cflag |= CS8; break;
	}
	//parity
	switch(config._parity)
	{
	case SerialPortConfig::SERIALPORT_PARITY_ODD:
		options.c_cflag |= PARENB;
		options.c_cflag |= PARODD;
		break;
	case SerialPortConfig::SERIALPORT_PARITY_EVEN:
		options.c_cflag |= PARENB;
		break;
	case SerialPortConfig::SERIALPORT_PARITY_MARK:
	case SerialPortConfig::SERIALPORT_PARITY_SPACE:
		DBG("SerialPort::setConfig : SERIALPORT_PARITY_MARK and SERIALPORT_PARITY_SPACE not supported on Mac");
		return false;//not supported
		break;
	case SerialPortConfig::SERIALPORT_PARITY_NONE:
	default:
		break;
	}
	//stopbits
	if(config._stopbits == SerialPortConfig::STOPBITS_1ANDHALF)
	{
		DBG("SerialPort::setConfig : STOPBITS_1ANDHALF not supported on Mac");
		return false;//not supported
	}
	if(config._stopbits == SerialPortConfig::STOPBITS_2)
		options.c_cflag |= CSTOPB;
	//flow control
	switch(config._flowcontrol)
	{
	case SerialPortConfig::FLOWCONTROL_XONXOFF:
		options.c_iflag |= IXON;
		options.c_iflag |= IXOFF;
		break;
	case SerialPortConfig::FLOWCONTROL_HARDWARE:
		options.c_cflag |= CCTS_OFLOW;
		options.c_cflag |= CRTS_IFLOW;
		break;
	case SerialPortConfig::FLOWCONTROL_NONE:
	default:
		break;
	}
	if (tcsetattr(_portDescriptor, TCSANOW, &options) == -1)
    {
        DBG("SerialPort::setConfig : can't set port settings");
        return false;
    }
	return true;
}
bool SerialPort::getConfig(SerialPortConfig & config)
{
	struct termios options;
	if(-1 == _portDescriptor)return false;
	if (tcgetattr(_portDescriptor, &options) == -1)
    {
        DBG("SerialPort::getConfig : cannot get port settings");
        return false;
    }
	config._bps = ((int)cfgetispeed(&options))>((int)cfgetospeed(&options))?(int)cfgetispeed(&options):(int)cfgetospeed(&options);
	switch(options.c_cflag & CSIZE)
	{
	case CS5: config._databits=5; break;
	case CS6: config._databits=6; break;
	case CS7: config._databits=7; break;
	case CS8: config._databits=8; break;
	}
	config._parity = SerialPortConfig::SERIALPORT_PARITY_NONE;
	if(options.c_cflag & PARENB)
	{ 
		if(options.c_cflag & PARODD)config._parity = SerialPortConfig::SERIALPORT_PARITY_ODD;
		else config._parity = SerialPortConfig::SERIALPORT_PARITY_EVEN;
	}
	//stopbits
	config._stopbits = SerialPortConfig::STOPBITS_1;
	if(options.c_cflag & CSTOPB)config._stopbits = SerialPortConfig::STOPBITS_2;
	//flow control
	config._flowcontrol=SerialPortConfig::FLOWCONTROL_NONE;
	if((options.c_iflag & IXON) || (options.c_iflag & IXOFF))
		config._flowcontrol=SerialPortConfig::FLOWCONTROL_XONXOFF;
	else if((options.c_cflag & CCTS_OFLOW) || (options.c_cflag & CRTS_IFLOW))
		config._flowcontrol=SerialPortConfig::FLOWCONTROL_HARDWARE;
	
	return true;
}
/////////////////////////////////
// SerialPortInputStream
/////////////////////////////////
void SerialPortInputStream::run()
{
	while(_port && (_port->_portDescriptor != -1) && !threadShouldExit())
	{
		unsigned char c;
		int bytesread = 0;
		bytesread = ::read(_port->_portDescriptor, &c, 1);
		if(bytesread == 1)
		{
			const juce::ScopedLock l(_bufferCriticalSection);
			_buffer.ensureSize(_bufferedbytes+1);
			_buffer[_bufferedbytes]=c;
			_bufferedbytes++;
			if(_notify==NOTIFY_ALWAYS||((_notify==NOTIFY_ON_CHAR) && (c == _notifyChar)))
					sendChangeMessage();
		}
	}
}

int SerialPortInputStream::read(void *destBuffer, int maxBytesToRead)
{
	const juce::ScopedLock l(_bufferCriticalSection);
	if(maxBytesToRead > _bufferedbytes) maxBytesToRead = _bufferedbytes;
	memcpy(destBuffer, _buffer.getData(), maxBytesToRead);
	_buffer.removeSection(0, maxBytesToRead);
	_bufferedbytes -= maxBytesToRead;
	return maxBytesToRead;
}

/////////////////////////////////
// SerialPortOutputStream
/////////////////////////////////
void SerialPortOutputStream::run()
{
	unsigned char tempbuffer[_writeBufferSize];
	while(_port && (_port->_portDescriptor != -1) && !threadShouldExit())
	{
		if(!_bufferedbytes)
			_triggerWrite.wait(100);
		if(_bufferedbytes)
		{
			int byteswritten = 0;
			_bufferCriticalSection.enter();
			int bytestowrite = _bufferedbytes > _writeBufferSize ? _writeBufferSize : _bufferedbytes;
			memcpy(tempbuffer, _buffer.getData(), bytestowrite);
			_bufferCriticalSection.exit();
			byteswritten = ::write(_port->_portDescriptor, tempbuffer, bytestowrite);
			if(byteswritten>0)
			{
				const juce::ScopedLock l(_bufferCriticalSection);
				_buffer.removeSection(0, byteswritten);
				_bufferedbytes -= byteswritten;
			}
		}
	}
}
bool SerialPortOutputStream::write(const void *dataToWrite, size_t howManyBytes)
{
	_bufferCriticalSection.enter();
    _buffer.append(dataToWrite, howManyBytes);
	_bufferedbytes+=howManyBytes;
	_bufferCriticalSection.exit();
	_triggerWrite.signal();
	return true;
}

