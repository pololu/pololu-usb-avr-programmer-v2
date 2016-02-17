#include "main_model.h"

#include <cassert>

void MainModel::updateDeviceList()
{
    deviceList = programmerGetList();
}

void MainModel::connect(const ProgrammerInstance & instance)
{
    // Close the old handle in case one is already open.
    deviceHandle.close();

    connectionError = false;
    disconnectedByUser = false;

    try
    {
        // Open a handle to the specified programmer.
        deviceHandle = ProgrammerHandle(instance);
    }
    catch (...)
    {
        setConnectionError("Failed to connect to device.");
        throw;
    }
}

void MainModel::reloadSettings()
{
    assert(connected());

    try
    {
        settings = deviceHandle.getSettings();
        settingsModified = false;
    }
    catch (...)
    {
        settingsModified = true;
        throw;
    }
}

void MainModel::applySettings()
{
    assert(connected());
    deviceHandle.applySettings(settings);
    settingsModified = false;  // this must be last in case exceptions are thrown
}

void MainModel::reloadVariables()
{
    assert(connected());

    try
    {
        variables = deviceHandle.getVariables();
        variablesUpdateFailed = false;
    }
    catch (...)
    {
        variablesUpdateFailed = true;
        throw;
    }
}

void MainModel::reloadFirmwareVersionString()
{
    try
    {
        firmwareVersionString = deviceHandle.getFirmwareVersionString();
    }
    catch (...)
    {
        firmwareVersionString = "?";
        throw;
    }
}

void MainModel::disconnectByUser()
{
    disconnect();
    disconnectedByUser = true;

    connectionError = false;
}

void MainModel::disconnectByError(std::string errorMessage)
{
    disconnect();
    setConnectionError(errorMessage);

    disconnectedByUser = false;
}

void MainModel::setConnectionError(std::string errorMessage)
{
    connectionError = true;
    connectionErrorMessage = errorMessage;
}

void MainModel::disconnect()
{
    deviceHandle.close();
    settingsModified = false;
}

