//win32_SerialPort.cpp
//Serial Port classes in a Juce stylee, by graffiti
//see SerialPort.h for details
//
// Updated for current Juce API 8/1/13 Marc Lindahl
//

#if WIN32

juce::StringPairArray SerialPort::getSerialPortPaths()
{
    juce::StringPairArray SerialPortPaths;
#ifdef USE_SETUPAPI
	HDEVINFO hDevInfo;
    SP_DEVINFO_DATA DeviceInfoData;
	DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
	DWORD dwDetDataSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA) + 256;
	SP_DEVICE_INTERFACE_DETAIL_DATA *pDetData = (SP_DEVICE_INTERFACE_DETAIL_DATA*) new char[dwDetDataSize];
	pDetData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
	SP_DEVICE_INTERFACE_DATA ifcData;
	ifcData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
    DWORD i;
	GUID *guidDev = (GUID*) &GUID_CLASS_COMPORT;
    if(INVALID_HANDLE_VALUE == (hDevInfo = SetupDiGetClassDevs(guidDev, 0, 0, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE )))
    {
		DBG("SerialPort::getSerialPortPaths :: SetupDiGetClassDevs failed");
		return SerialPortPaths;
    }
    for (i=0;SetupDiEnumDeviceInfo(hDevInfo,i,&DeviceInfoData);i++)
    {
        DWORD DataT;
        char buffer[1024];
        DWORD buffersize = 1024;
		DWORD buffersizerequired=0;
		SetupDiEnumDeviceInterfaces(hDevInfo, NULL, guidDev, i, &ifcData);
		SetupDiGetDeviceInterfaceDetail(hDevInfo, &ifcData, pDetData, dwDetDataSize, NULL, &DeviceInfoData);
		SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData, SPDRP_FRIENDLYNAME, &DataT, (PBYTE)buffer, buffersize, &buffersizerequired);
		SerialPortPaths.set(buffer, pDetData->DevicePath);
    }
    SetupDiDestroyDeviceInfoList(hDevInfo);
	delete[] pDetData;
#endif// USE_SETUPAPI

	return SerialPortPaths;
}

void SerialPort::close()
{
	if(_portHandle)
	{
		CloseHandle(_portHandle);
		_portHandle = 0;
	}
}

bool SerialPort::exists()
{
	return _portHandle ? true : false;
}

bool SerialPort::open(const juce::String& portPath)
{
	this->_portPath = portPath;
	_portHandle = CreateFile( (LPCSTR)portPath.toUTF8(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if(_portHandle == INVALID_HANDLE_VALUE)
	{
		//DBG_PRINTF((T("(SerialPort::open) CreateFile failed with error %d.\n"), GetLastError()));
		_portHandle = 0;
		return false;
	}
	COMMTIMEOUTS commTimeout;
	if(GetCommTimeouts(_portHandle, &commTimeout))
	{
		commTimeout.ReadIntervalTimeout = MAXDWORD;
		commTimeout.ReadTotalTimeoutConstant = 0;
		commTimeout.ReadTotalTimeoutMultiplier = 0;
		commTimeout.WriteTotalTimeoutConstant = 0;
		commTimeout.WriteTotalTimeoutMultiplier = 0;
	}
	else
		DBG("GetCommTimeouts error");
	if(!SetCommTimeouts(_portHandle, &commTimeout))
		DBG("SetCommTimeouts error");

	if(!SetCommMask(_portHandle,EV_RXCHAR))
		DBG("SetCommMask error");

	return true;
}

bool SerialPort::setConfig(const SerialPortConfig& config)
{
	if(!_portHandle) return false;

	DCB dcb;
	memset(&dcb, 0, sizeof(DCB));
	dcb.DCBlength = sizeof(DCB);
	dcb.fBinary = 1;
	dcb.XonLim = 2048;
	dcb.XoffLim = 512;
	dcb.BaudRate = config._bps;
	dcb.ByteSize = (BYTE) config._databits;
	dcb.fParity = true;
	switch(config._parity)
	{
	case SerialPortConfig::SERIALPORT_PARITY_ODD:
		dcb.Parity = ODDPARITY;
		break;
	case SerialPortConfig::SERIALPORT_PARITY_EVEN:
		dcb.Parity = EVENPARITY;
		break;
	case SerialPortConfig::SERIALPORT_PARITY_MARK:
		dcb.Parity = MARKPARITY;
		break;
	case SerialPortConfig::SERIALPORT_PARITY_SPACE:
		dcb.Parity = SPACEPARITY;
		break;
	case SerialPortConfig::SERIALPORT_PARITY_NONE:
	default:
		dcb.Parity = NOPARITY;
		dcb.fParity = false;
		break;
	}
	switch(config._stopbits)
	{
	case SerialPortConfig::STOPBITS_1:
	default:
		dcb.StopBits = ONESTOPBIT;
		break;
	case SerialPortConfig::STOPBITS_1ANDHALF:
		dcb.StopBits = ONE5STOPBITS;
		break;
	case SerialPortConfig::STOPBITS_2:
		dcb.StopBits = TWOSTOPBITS;
		break;
	}
	switch(config._flowcontrol)
	{
	case SerialPortConfig::FLOWCONTROL_XONXOFF:
		dcb.fOutxCtsFlow = 0;
		dcb.fOutxDsrFlow = 0;
		dcb.fDtrControl = DTR_CONTROL_ENABLE;
		dcb.fOutX = 1;
		dcb.fInX = 1;
		dcb.fRtsControl = RTS_CONTROL_ENABLE;
		break;
	case SerialPortConfig::FLOWCONTROL_HARDWARE:
		dcb.fOutxCtsFlow = 1;
		dcb.fOutxDsrFlow = 1;
		dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;
		dcb.fOutX = 0;
		dcb.fInX = 0;
		dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
		break;
	case SerialPortConfig::FLOWCONTROL_NONE:
	default:
		dcb.fOutxCtsFlow = 0;
		dcb.fOutxDsrFlow = 0;
		dcb.fDtrControl = DTR_CONTROL_ENABLE;
		dcb.fOutX = 0;
		dcb.fInX = 0;
		dcb.fRtsControl = RTS_CONTROL_ENABLE;
		break;
	}
	return (SetCommState(_portHandle, &dcb) ? true : false);
}

bool SerialPort::getConfig(SerialPortConfig& config)
{
	if(!_portHandle) return false;

	DCB dcb;
	if(!GetCommState(_portHandle, &dcb))
		return false;

	config._bps = dcb.BaudRate;
	config._databits = dcb.ByteSize;

	switch(dcb.Parity)
	{
	case ODDPARITY:
		config._parity = SerialPortConfig::SERIALPORT_PARITY_ODD;
		break;
	case EVENPARITY:
		config._parity = SerialPortConfig::SERIALPORT_PARITY_EVEN;
		break;
	case MARKPARITY:
		config._parity = SerialPortConfig::SERIALPORT_PARITY_MARK;
		break;
	case SPACEPARITY:
		config._parity = SerialPortConfig::SERIALPORT_PARITY_SPACE;
		break;
	case NOPARITY:
	default:
		config._parity = SerialPortConfig::SERIALPORT_PARITY_NONE;
		break;
	}
	switch(dcb.StopBits)
	{
	case ONESTOPBIT:
	default:
		config._stopbits = SerialPortConfig::STOPBITS_1;
		break;
	case ONE5STOPBITS:
		config._stopbits = SerialPortConfig::STOPBITS_1ANDHALF;
		break;
	case TWOSTOPBITS:
		config._stopbits = SerialPortConfig::STOPBITS_2;
		break;
	}
	if(dcb.fOutX && dcb.fInX)
		config._flowcontrol=SerialPortConfig::FLOWCONTROL_XONXOFF;
	else if((dcb.fDtrControl == DTR_CONTROL_HANDSHAKE) && (dcb.fRtsControl == RTS_CONTROL_HANDSHAKE))
		config._flowcontrol=SerialPortConfig::FLOWCONTROL_HARDWARE;
	else
		config._flowcontrol=SerialPortConfig::FLOWCONTROL_NONE;

	return true;
}

/////////////////////////////////
// SerialPortInputStream
/////////////////////////////////
void SerialPortInputStream::run()
{
	DWORD dwEventMask=0;
	//overlapped structure for the wait
	OVERLAPPED ov;
	memset(&ov, 0, sizeof(ov));
	ov.hEvent = CreateEvent(0, true, 0, 0);
	//overlapped structure for the read
	OVERLAPPED ovRead;
	memset(&ovRead, 0, sizeof(ovRead));
	ovRead.hEvent = CreateEvent(0, true, 0, 0);
	while(_port && _port->_portHandle && !threadShouldExit())
	{
		unsigned char c;
		DWORD bytesread=0;
		WaitCommEvent(_port->_portHandle, &dwEventMask, &ov);
		if(WAIT_OBJECT_0 == WaitForSingleObject(ov.hEvent, 100))
		{
			DWORD dwMask;
			if (GetCommMask(_port->_portHandle, &dwMask) )
			{
				if ( dwMask & EV_RXCHAR )
				{
					do
					{
						ResetEvent( ovRead.hEvent  );
						ReadFile(_port->_portHandle, &c, 1, &bytesread, &ov);
						if(bytesread == 1)
						{
							const juce::ScopedLock l(_bufferCriticalSection);
							_buffer.ensureSize(_bufferedbytes+1);
							_buffer[_bufferedbytes]=c;
							_bufferedbytes++;
							if(_notify == NOTIFY_ALWAYS||((_notify == NOTIFY_ON_CHAR) && (c == _notifyChar)))
								sendChangeMessage();
						}
					}while(bytesread);
				}
			}
			ResetEvent ( ov.hEvent );
		}
	}
	CloseHandle(ovRead.hEvent);
	CloseHandle(ov.hEvent);
}

int SerialPortInputStream::read(void *destBuffer, int maxBytesToRead)
{
	const juce::ScopedLock l(_bufferCriticalSection);
	if(maxBytesToRead>_bufferedbytes)maxBytesToRead = _bufferedbytes;
	memcpy(destBuffer, _buffer.getData(), maxBytesToRead);
	_buffer.removeSection(0, maxBytesToRead);
	_bufferedbytes -= maxBytesToRead;
	return maxBytesToRead;
}

/////////////////////////////////
// SerialPortInputStream
/////////////////////////////////
void SerialPortOutputStream::run()
{
	unsigned char tempbuffer[_writeBufferSize];
	OVERLAPPED ov;
	memset(&ov, 0, sizeof(ov));
	ov.hEvent = CreateEvent( 0,true,0,0);
	while(_port && _port->_portHandle && !threadShouldExit())
	{
		if(!_bufferedbytes)
			_triggerWrite.wait(100);
		if(_bufferedbytes)
		{
			DWORD byteswritten=0;
			_bufferCriticalSection.enter();
			DWORD bytestowrite = _bufferedbytes>_writeBufferSize ? _writeBufferSize : _bufferedbytes;
			memcpy(tempbuffer, _buffer.getData(), bytestowrite);
			_bufferCriticalSection.exit();
			ResetEvent ( ov.hEvent );
			int iRet = WriteFile(_port->_portHandle, tempbuffer, bytestowrite, &byteswritten, &ov);
			if ( iRet == 0 )
			{
				WaitForSingleObject(ov.hEvent, INFINITE);
			}
			GetOverlappedResult(_port->_portHandle, &ov, &byteswritten, TRUE);
			if(byteswritten)
			{
				const juce::ScopedLock l(_bufferCriticalSection);
				_buffer.removeSection(0, byteswritten);
				_bufferedbytes -= byteswritten;
			}
		}
	}
	CloseHandle(ov.hEvent);
}

bool SerialPortOutputStream::write(const void *dataToWrite, size_t howManyBytes)
{
	_bufferCriticalSection.enter();
	_buffer.append(dataToWrite, howManyBytes);
	_bufferedbytes += (int) howManyBytes;
	_bufferCriticalSection.exit();
	_triggerWrite.signal();
	return true;
}

#endif // JUCE_WIN
