#include "main_controller.h"
#include "main_view.h"
#include "main_model.h"

#include <cassert>

/** This is how often we fetch the variables from the device. */
static const uint32_t UPDATE_INTERVAL_MS = 1000;

void MainController::init(MainModel * model, MainView * view)
{
    this->model = model;
    this->view = view;
}

void MainController::start()
{
    assert(!model->connected());

    // Start the update timer so that update() will be called regularly.
    view->startUpdateTimer(UPDATE_INTERVAL_MS);

    // The program has just started, so try to connect to a device.

    bool successfullyUpdatedList = tryUpdateDeviceList();
    if (!successfullyUpdatedList)
    {
        view->handleModelChanged();
        return;
    }

    if (model->deviceList.size() > 0)
    {
        reallyConnect();
    }
    else
    {
        // No device was found.  Let's look for older versions of this
        // product and warn the user that this software does not work with
        // them.
        bool pgm03aFound = false;
        try
        {
            pgm03aFound = pgm03aPresent();
        }
        catch (const std::exception & e)
        {
            showException(e, "There was an error while checking for older programmers.");
        }
        if (pgm03aFound)
        {
            view->showWarningMessage(pgm03aMessage);
        }
        view->handleModelChanged();
    }
}

void MainController::connect()
{
    // The user wants to connect to a device.

    if (model->connected())
    {
        // Since we don't allow them to pick what device to connect to, and
        // we are already connected, just return.
        return;
    }

    bool successfullyUpdatedList = tryUpdateDeviceList();
    if (!successfullyUpdatedList)
    {
        return;
    }

    if (model->deviceList.size() > 0)
    {
        reallyConnect();
    }
    else
    {
        view->showErrorMessage(
            "No programmer was found.  "
            "Please verify that the programmer is connected to the computer via USB."
            );
    }
}

void MainController::disconnect()
{
    if (!model->connected()) { return; }
    model->disconnectByUser();
    view->handleModelChanged();
}

void MainController::reloadSettings()
{
    if (!model->connected()) { return; }
    try
    {
        model->reloadSettings();
    }
    catch (const std::exception & e)
    {
        showException(e, "There was an error loading the settings from the device.");
    }
    view->handleSettingsChanged();
}

void MainController::restoreDefaultSettings()
{
    if (!model->connected()) { return; }

    std::string question = "This will reset all of your device's settings "
        "back to their default values.  "
        "You will lose your custom settings.  "
        "Are you sure you want to continue?";
    if (!view->confirm(question))
    {
        return;
    }

    bool restoreSuccess = false;
    try
    {
        model->deviceHandle.restoreDefaults();
        restoreSuccess = true;
    }
    catch (const std::exception & e)
    {
        showException(e, "There was an error resetting to the default settings.");
    }

    // This takes care of reloading the settings and telling the view to update.
    reloadSettings();

    if (restoreSuccess)
    {
        view->showInfoMessage(
            "Your device's settings have been reset to their default values.");
    }
}

/** Returns true if the device list includes the specified device. */
static bool deviceListIncludes(
    const std::vector<ProgrammerInstance> & deviceList,
    const ProgrammerInstance & device)
{
    std::string id = device.getOsId();
    for (const ProgrammerInstance & candidate : deviceList)
    {
        if (candidate.getOsId() == id)
        {
            return true;
        }
    }
    return false;
}

void MainController::update()
{
    // This is called regularly by the view when it is time to check for updates
    // to the state of the USB devices.  This runs on the same thread as
    // everything else, so we should be careful not to do anything too slow
    // here.  If the user tries to use the UI at all while this function is
    // running, the UI cannot respond until this function returns.

    if (model->connected())
    {
        // First, see if the programmer we are connected to is still available.
        // Note: It would be nice if the libusbp::generic_handle class had a
        // function that tests if the actual handle we are using is still valid.
        // This would be better for tricky cases like if someone unplugs and
        // plugs the same programmer in very fast.

        bool successfullyUpdatedList = tryUpdateDeviceList();
        if (!successfullyUpdatedList)
        {
            // Ignore this unexpected error.  We are already successfully
            // connected to the device, so it will still be on our list, and we
            // will try to keep using it if we can.
        }

        bool deviceStillPresent = deviceListIncludes(
            model->deviceList, model->deviceHandle.getInstance());

        if (deviceStillPresent)
        {
            // Reload the variables from the device.
            try
            {
                model->reloadVariables();
            }
            catch (const std::exception & e)
            {
                // Ignore the exception.  The model provides other ways to tell that
                // the variable update failed, and the exact message is probably
                // not that useful since it is probably just a generic problem with
                // the USB connection.
            }
            view->handleVariablesChanged();
        }
        else
        {
            // The device is gone.
            model->disconnectByError("The connection to the device was lost.");
            view->handleModelChanged();
        }
    }
    else
    {
        // We are not connected, so consider auto-connecting to a device.

        if (model->connectionError)
        {
            // There is an error related to a previous connection or connection
            // attempt, so don't automatically reconnect.  That would be
            // confusing, because the user might be looking away and not notice
            // that the connection was lost and then regained, or they could be
            // trying to read the error message.
        }
        else if (model->disconnectedByUser)
        {
            // The user explicitly disconnected the last connection, so don't
            // automatically reconnect.
        }
        else
        {
            bool successfullyUpdatedList = tryUpdateDeviceList();
            if (!successfullyUpdatedList)
            {
                // Since this is a background update, don't report
                // this error to the user.
            }

            if (successfullyUpdatedList && model->deviceList.size() > 0)
            {
                reallyConnect();
            }
        }
    }
}

bool MainController::exit()
{
    if (model->connected() && model->settingsModified)
    {
        std::string question =
            "The settings you changed have not been applied to the device.  "
            "If you exit now, those changes will be lost.  "
            "Are you sure you want to exit?";
        return view->confirm(question);
    }
    else
    {
        return true;
    }
}

void MainController::reallyConnect()
{
    assert(model->deviceList.size() > 0);

    try
    {
        model->connect(model->deviceList.at(0));
    }
    catch (const std::exception & e)
    {
        showException(e, "There was an error connecting to the device.");
        view->handleModelChanged();
        return;
    }

    try
    {
        model->reloadFirmwareVersionString();
    }
    catch (const std::exception & e)
    {
        showException(e, "There was an error getting the firmware version.");
    }

    try
    {
        model->reloadSettings();
    }
    catch (const std::exception & e)
    {
        showException(e, "There was an error loading settings from the device.");
    }

    try
    {
        model->reloadVariables();
    }
    catch (const std::exception & e)
    {
        showException(e, "There was an error getting the status of the device.");
    }

    view->handleModelChanged();
}

bool MainController::tryUpdateDeviceList()
{
    try
    {
        model->updateDeviceList();
        return true;
    }
    catch (const std::exception & e)
    {
        model->setConnectionError("Failed to get the list of devices.");
        showException(e, "There was an error getting the list of devices.");
        return false;
    }
}

void MainController::showException(const std::exception & e,
    const std::string & context = "")
{
    std::string message;
    if (context.size() > 0)
    {
        message += context;
        message += "  ";
    }
    message += e.what();
    view->showErrorMessage(message);
}

void MainController::applySettings()
{
    if (!model->connected()) { return; }

    try
    {
        model->applySettings();
    }
    catch (const std::exception & e)
    {
        showException(e, "There was an error applying settings.");
    }

    view->handleSettingsChanged();
}

void MainController::handleIspFrequencyInput(const std::string & input)
{
    // If someone disconnects the device while this input box is selected, we
    // will disable the input box and that could cause a spurious user input
    // event to be sent from the view to here, which is not great.
    if (!model->connected()) { return; }

    try
    {
        Programmer::setFrequency(model->settings, input);
        model->settingsModified = true;
    }
    catch (const std::exception & e)
    {
        // This should not happen because the view already fixed up the input to
        // be valid.
        assert(0);
        showException(e, "There was an error setting the ISP frequency.");
    }

    // This change might affect the max ISP frequency
    // (and also enable the Apply Settings button).
    view->handleSettingsChanged();
}

void MainController::handleMaxIspFrequencyInput(const std::string & input)
{
    if (!model->connected()) { return; }

    try
    {
        Programmer::setMaxFrequency(model->settings, input);
        model->settingsModified = true;
    }
    catch (const std::exception & e)
    {
        // This should not happen because the view already fixed up the input to
        // be valid.
        assert(0);
        showException(e, "There was an error setting the Max ISP frequency.");
    }

    // This change might affect the displayed ISP frequency.
    // (and also enable the Apply Settings button).
    view->handleSettingsChanged();
}

void MainController::handleRegulatorModeInput(uint8_t regulatorMode)
{
    if (!model->connected()) { return; }
    model->settings.regulatorMode = regulatorMode;
    model->settingsModified = true;
    view->handleSettingsChanged();
}

void MainController::handleVccOutputEnabledInput(bool vccOutputEnabled)
{
    if (!model->connected()) { return; }
    model->settings.vccOutputEnabled = vccOutputEnabled;
    model->settingsModified = true;
    view->handleSettingsChanged();
}

void MainController::handleVccOutputIndicatorInput(uint8_t indicator)
{
    if (!model->connected()) { return; }
    model->settings.vccOutputIndicator = indicator;
    model->settingsModified = true;
    view->handleSettingsChanged();
}

void MainController::handleLineAFunctionInput(uint8_t function)
{
    if (!model->connected()) { return; }
    model->settings.lineAFunction = function;
    model->settingsModified = true;
    view->handleSettingsChanged();
}

void MainController::handleLineBFunctionInput(uint8_t function)
{
    if (!model->connected()) { return; }
    model->settings.lineBFunction = function;
    model->settingsModified = true;
    view->handleSettingsChanged();
}

void MainController::handleVccVddMaxRangeInput(uint32_t mv)
{
    if (!model->connected()) { return; }
    model->settings.vccVddMaxRange = mv;
    model->settingsModified = true;
    view->handleSettingsChanged();
}

void MainController::handleVcc3v3MinInput(uint32_t mv)
{
    if (!model->connected()) { return; }
    model->settings.vcc3v3Min = mv;
    model->settingsModified = true;
    view->handleSettingsChanged();
}

void MainController::handleVcc3v3MaxInput(uint32_t mv)
{
    if (!model->connected()) { return; }
    model->settings.vcc3v3Max = mv;
    model->settingsModified = true;
    view->handleSettingsChanged();
}

void MainController::handleVcc5vMinInput(uint32_t mv)
{
    if (!model->connected()) { return; }
    model->settings.vcc5vMin = mv;
    model->settingsModified = true;
    view->handleSettingsChanged();
}

void MainController::handleVcc5vMaxInput(uint32_t mv)
{
    if (!model->connected()) { return; }
    model->settings.vcc5vMax = mv;
    model->settingsModified = true;
    view->handleSettingsChanged();
}

void MainController::handleStk500VersionInput(uint32_t hardwareVersion,
        uint32_t softwareVersionMajor, uint32_t softwareVersionMinor)
{
    if (!model->connected()) { return; }
    model->settings.hardwareVersion = hardwareVersion;
    model->settings.softwareVersionMajor = softwareVersionMajor;
    model->settings.softwareVersionMinor = softwareVersionMinor;
    model->settingsModified = true;
    view->handleSettingsChanged();
}
