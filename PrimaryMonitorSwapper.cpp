#include <iostream>
#include <string>
#include <Windows.h>

#define ERR_UNKNOWN -100
#define ERR_BAD_ARGS -110
#define ERR_NO_ARGS -111
#define ERR_BAD_MONITOR_ID -121
#define ERR_FAIL_GET_MONITOR_SETTINGS -122

#define SUCCESS 0
#define REQUIRES_RESTART 1

//Also returns the error for convenience, except in the case that _err is unknown, when ERR_UNKNOWN is returned instead
static LONG PrintChangeDisplaySettingsExWError(LONG _err)
{
    if (_err < DISP_CHANGE_SUCCESSFUL)
    {
        switch (_err)
        {
        case (DISP_CHANGE_BADDUALVIEW):
            std::wcerr << "Monitor change unsuccessful because the system is DualView capable." << std::endl;
            return _err;
        case (DISP_CHANGE_BADFLAGS):
            std::wcerr << "Monitor change unsuccessful because an invalid set of flags was passed in." << std::endl;
            return _err;
        case (DISP_CHANGE_BADMODE):
            std::wcerr << "Monitor change unsuccessful because the graphics mode is not supported." << std::endl;
            return _err;
        case (DISP_CHANGE_BADPARAM):
            std::wcerr << "Monitor change unsuccessful because an invalid parameter was passed in." << std::endl;
            return _err;
        case (DISP_CHANGE_FAILED):
            std::wcerr << "Monitor change unsuccessful because the display driver failed the specified graphics mode." << std::endl;
            return _err;
        case (DISP_CHANGE_NOTUPDATED):
            std::wcerr << "Monitor change unsuccessful because it was unable to write settings to the registry." << std::endl;
            return _err;
        default:
            std::wcerr << "UNKNOWN ERROR [" << _err << "]!" << std::endl;
            return ERR_UNKNOWN;
        }
    }
}

//Thanks to:
//https://foxlearn.com/csharp/how-to-define-a-monitor-as-the-primary-display-in-csharp-8551.html
//https://blog.lohr.dev/primary-display-windows
//And the MS Documentation
static int SetPrimaryMonitor(DWORD _monitorId)
{
    //Initialize memory
    DISPLAY_DEVICEW _device;
    memset(&_device, 0, sizeof(_device));
    _device.cb = sizeof(_device);

    DEVMODEW _deviceMode;
    memset(&_deviceMode, 0, sizeof(_deviceMode));
    _deviceMode.dmSize = sizeof(_deviceMode);

    //Grab display device info
    if (!EnumDisplayDevicesW(nullptr, _monitorId, &_device, 0))
    {
        std::wcerr << "Bad Monitor ID [" << _monitorId << "]!" << std::endl;
        return ERR_BAD_MONITOR_ID;
    }

    //Grab display settings
    if (!EnumDisplaySettingsW(_device.DeviceName, -1, &_deviceMode))
    {
        std::wcerr << "Failed to get new primary monitor [" << _monitorId << "] settings." << std::endl;
        return ERR_FAIL_GET_MONITOR_SETTINGS;
    }

    //Save offset of new primary monitor from (0, 0)
    LONG _offsetX = _deviceMode.dmPosition.x;
    LONG _offsetY = _deviceMode.dmPosition.y;
    //Move new primary monitor to (0, 0)
    _deviceMode.dmPosition.x = 0; //Could also -= _offsetX, but that would get the same result
    _deviceMode.dmPosition.y = 0; //Could also -= _offsetY, but that would get the same result

    //Make the target monitor the primary monitor
    LONG _err = ChangeDisplaySettingsExW(_device.DeviceName, &_deviceMode, nullptr, CDS_SET_PRIMARY | CDS_UPDATEREGISTRY | CDS_NORESET, nullptr);
    BOOL _requiresRestart = false;
    if (_err < DISP_CHANGE_SUCCESSFUL)
    {
        return PrintChangeDisplaySettingsExWError(_err);
    }
    else if (_err == DISP_CHANGE_RESTART)
    {
        _requiresRestart = true;
        std::wcout << "MONITOR CHANGES REQUIRE RESTART TO TAKE EFFECT!" << std::endl;
    }

    //Clear _err
    _err = 0;

    //Update other monitors, both to make them no longer primary and to update their positions to be relative to the new primary
    memset(&_device, 0, sizeof(_device));
    _device.cb = sizeof(_device);
    for (LONG _otherMonitorId = 0; EnumDisplayDevicesW(nullptr, _otherMonitorId, &_device, 0); _otherMonitorId++)
    {
        //Ensure the device is attached and is not our primary monitor
        if (_device.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP && _otherMonitorId != _monitorId)
        {
            memset(&_deviceMode, 0, sizeof(_deviceMode));
            if (!EnumDisplaySettingsW(_device.DeviceName, -1, &_deviceMode))
            {
                std::wcerr << "Failed to get monitor [" << _otherMonitorId << "] settings." << std::endl;
                return ERR_FAIL_GET_MONITOR_SETTINGS;
            }

            _deviceMode.dmPosition.x -= _offsetX;
            _deviceMode.dmPosition.y -= _offsetY;

            _err = ChangeDisplaySettingsExW(_device.DeviceName, &_deviceMode, nullptr, CDS_UPDATEREGISTRY | CDS_NORESET, nullptr);
            if (_err < DISP_CHANGE_SUCCESSFUL)
            {
                return PrintChangeDisplaySettingsExWError(_err);
            }
            else if (_err == DISP_CHANGE_RESTART)
            {
                _requiresRestart = true;
                std::wcout << "MONITOR CHANGES REQUIRE RESTART TO TAKE EFFECT!" << std::endl;
            }
        }
        _device.cb = sizeof(_device);
    }

    //Commit new display settings
    _err = ChangeDisplaySettingsExW(nullptr, nullptr, nullptr, 0, nullptr);
    if (_err < DISP_CHANGE_SUCCESSFUL)
    {
        std::wcerr << "ERROR COMMITING CHANGES:" << std::endl;
        return PrintChangeDisplaySettingsExWError(_err);
    }
    else if (_err == DISP_CHANGE_RESTART)
    {
        _requiresRestart = true;
        std::wcout << "MONITOR CHANGES REQUIRE RESTART TO TAKE EFFECT!" << std::endl;
    }

    //If a restart is required, notify the user and return an actionable value
    if (_requiresRestart)
    {
        std::wcout << "MONITOR CHANGES REQUIRE RESTART TO TAKE EFFECT!" << std::endl;
        return REQUIRES_RESTART;
    }

    return SUCCESS;
}

static void PrintHelp()
{
    std::wcout << "PrimaryMonitorSwapper Usage:" << std::endl;
    std::wcout << "-h | --help          :    Displays the HELP (that you're seeing now)." << std::endl;
    std::wcout << "-n | --number        :    Prints and returns how many monitors are detected and valid." << std::endl;
    std::wcout << "-N | --all-number    :    Prints and returns how many monitors are detected (not necessarily valid)." << std::endl;
    std::wcout << "-l | --list-devices  :    Prints friendly names and ids of all valid connected devices." << std::endl;
    std::wcout << "<monitorId>          :    Sets the monitor with id monitorId to be the primary monitor." << std::endl;
}

static LONG GetNumDevices()
{
    //Initialize memory
    DISPLAY_DEVICEW _device;
    memset(&_device, 0, sizeof(_device));
    _device.cb = sizeof(_device);

    LONG _monitorCount = 0;
    while (EnumDisplayDevicesW(nullptr, _monitorCount, &_device, 0))
    {
        _monitorCount++;
        _device.cb = sizeof(_device);
    }

    return _monitorCount;
}

static LONG GetNumValidDevices()
{
    //Initialize memory
    DISPLAY_DEVICEW _device;
    memset(&_device, 0, sizeof(_device));
    _device.cb = sizeof(_device);

    LONG _monitorCount = 0;
    LONG _validMonitorCount = 0;
    while (EnumDisplayDevicesW(nullptr, _monitorCount, &_device, 0))
    {
        //Ensure the device is attached
        if (_device.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)
        {
            _validMonitorCount++;
        }
        _monitorCount++;
        _device.cb = sizeof(_device);
    }

    return _validMonitorCount;
}

static void ListDevices()
{
    //Initialize memory
    DISPLAY_DEVICEW _device;
    memset(&_device, 0, sizeof(_device));
    _device.cb = sizeof(_device);

    DISPLAY_DEVICEW _friendlyDevice;
    memset(&_friendlyDevice, 0, sizeof(_friendlyDevice));
    _friendlyDevice.cb = sizeof(_friendlyDevice);

    LONG _monitorId = 0;
    while (EnumDisplayDevicesW(nullptr, _monitorId, &_device, 0))
    {
        if (_device.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)
        {
            _device.cb = sizeof(_device);

            if (EnumDisplayDevicesW(_device.DeviceName, 0, &_friendlyDevice, 0))
            {
                std::wcout << _friendlyDevice.DeviceString << " | ID: " << _monitorId << std::endl;
            }
            else
            {
                std::wcerr << "Failed to retrieve friendly name for device id [" << _monitorId << "]." << std::endl;
            }
        }

        _monitorId++;
        _device.cb = sizeof(_device);
        _friendlyDevice.cb = sizeof(_friendlyDevice);
    }
}

int main(int _argc, char* _argv[])
{
    if (_argc < 2)
    {
        PrintHelp();
        return ERR_NO_ARGS;
    }

    if (strncmp(_argv[1], "-h", strlen("-h")) == 0 || strncmp(_argv[1], "--help", strlen("--help")) == 0)
    {
        PrintHelp();
        return SUCCESS;
    }
    else if(strncmp(_argv[1], "-n", strlen("-n")) == 0 || strncmp(_argv[1], "--number", strlen("--number")) == 0)
    {
        LONG _numValidDevices = GetNumValidDevices();
        std::cout << _numValidDevices << std::endl;
        return _numValidDevices;
    }
    else if (strncmp(_argv[1], "-N", strlen("-N")) == 0 || strncmp(_argv[1], "--all-number", strlen("--all-number")) == 0)
    {
        LONG _numDevices = GetNumDevices();
        std::cout << _numDevices << std::endl;
        return _numDevices;
    }
    else if (strncmp(_argv[1], "-l", strlen("-l")) == 0 || strncmp(_argv[1], "--list-devices", strlen("--list-devices")) == 0)
    {
        ListDevices();
        return SUCCESS;
    }
    else
    {
        LONG _monitorId;
        try
        {
            _monitorId = std::stol(_argv[1]);
        }
        catch (...)
        {
            std::wcerr << "Bad Monitor Id [" << _argv[1] << "]." << std::endl;
            return ERR_BAD_ARGS;
        }

        if (_monitorId < 0)
        {
            std::wcerr << "Bad Monitor Id [" << _monitorId << "]." << std::endl;
            return ERR_BAD_ARGS;
        }

        return SetPrimaryMonitor(_monitorId);
    }
}
