#pragma once

#include "programmer.h"

/** The model holds the state of the application and it knows how to perform
 * simple operations that change the state.  The model does not know about
 * error/exception handling, and it does not know how to string together
 * multiple simple operations into a higher-level operation (e.g. it does not
 * know that we should reload the settings after connecting to a device). */
class MainModel
{
public:
    /** Holds a list of the relevant devices that are connected to the computer. */
    std::vector<ProgrammerInstance> deviceList;

    /** Holds an open handle to a device or a null handle if we are not
     * connected. */
    ProgrammerHandle deviceHandle;

    /** True if the last connection or connection attempt resulted in an error.
     * If true, connectionErrorMessage provides some information about the
     * error. */
    bool connectionError = false;
    std::string connectionErrorMessage;

    /** True if we are disconnected now and the last connection was terminated
     * by the user. */
    bool disconnectedByUser = false;

    /** Holds the settings from the device. */
    ProgrammerSettings settings;

    /** True if the settings have been modified by user and could be different
     * from what is on the device. */
    bool settingsModified = false;

    /** Holds the variables/status of the device. */
    ProgrammerVariables variables;

    /** True if the last attempt to update the variables failed (typically due
     * to a USB error). */
    bool variablesUpdateFailed = false;

    /** The firmware version string, including any modification codes
     * (e.g. "1.07nc"). */
    std::string firmwareVersionString;

    /** Returns true if we are currently connected to a device. */
    bool connected() const { return deviceHandle; }

    void updateDeviceList();
    void reloadSettings();
    void applySettings();
    void reloadVariables();
    void reloadFirmwareVersionString();

    void connect(const ProgrammerInstance & instance);

    void disconnectByUser();
    void disconnectByError(std::string errorMessage);

    void setConnectionError(std::string errorMessage);

private:
    void disconnect();
};

