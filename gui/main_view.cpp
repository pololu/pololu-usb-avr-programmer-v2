#include "main_view.h"
#include "main_controller.h"
#include "main_model.h"

void MainView::init(const MainModel * model, MainController * controller)
{
    this->model = model;
    this->controller = controller;
    this->window.setView(this);
}

void MainView::showWindow()
{
    window.show();
}

void MainView::startUpdateTimer(uint32_t intervalMs)
{
    window.startUpdateTimer(intervalMs);
}

void MainView::showErrorMessage(const std::string & message)
{
    window.showErrorMessage(message);
}

void MainView::showWarningMessage(const std::string & message)
{
    window.showWarningMessage(message);
}

void MainView::showInfoMessage(const std::string & message)
{
    window.showInfoMessage(message);
}

bool MainView::confirm(const std::string & question)
{
    return window.confirm(question);
}

void MainView::handleModelChanged()
{
    handleDeviceChanged();
    handleVariablesChanged();
    handleSettingsChanged();
}

void MainView::handleDeviceChanged()
{
    bool connected = model->connected();

    if (connected)
    {
        const ProgrammerInstance & device = model->deviceHandle.getInstance();
        window.setDeviceName(device.getName(), true);
        window.setSerialNumber(device.getSerialNumber());
        window.setFirmwareVersion(model->firmwareVersionString);
        window.setProgPort(device.tryGetProgrammingPortName());
        window.setTtlPort(device.tryGetTtlPortName());
    }
    else
    {
        std::string value = "N/A";
        window.setDeviceName(value, false);
        window.setSerialNumber(value);
        window.setFirmwareVersion(value);
        window.setProgPort(value);
        window.setTtlPort(value);
    }

    window.setConnectEnabled(!connected);
    window.setDisconnectEnabled(connected);
    window.setReloadSettingsEnabled(connected);
    window.setRestoreDefaultsEnabled(connected);
    window.setMainBoxesEnabled(connected);

    if (connected)
    {
        window.setConnectionStatus("Connected.", false);
    }
    else if (model->connectionError)
    {
        window.setConnectionStatus(model->connectionErrorMessage, true);
    }
    else if (model->disconnectedByUser)
    {
        window.setConnectionStatus("Not connected.", false);
    }
    else
    {
        // This is a subtle way of saying that we are not connected but we will
        // auto-connect when we see a device available.
        window.setConnectionStatus("Not connected yet...", false);
    }

    if (connected)
    {
        window.configureIspFrequencyControls(
            programmerAllowedFrequencyTable,
            programmerSuggestedFrequencyTable,
            programmerDefaultFrequency,
            programmerAllowedMaxFrequencyTable,
            programmerSuggestedMaxFrequencyTable,
            programmerDefaultMaxFrequency);
    }
}

/** Takes a number of millivolts and returns a string like "123 mV". */
static std::string convertMvToString(uint32_t mv)
{
    return std::to_string(mv) + " mV";
}

void MainView::handleVariablesChanged()
{
    const ProgrammerVariables & vars = model->variables;

    window.setLastDeviceReset(Programmer::convertDeviceResetToString(vars.lastDeviceReset));

    if (vars.programmingError == 0)
    {
        window.setProgrammingError("No error.", "");
    }
    else
    {
        window.setProgrammingError(std::string("Error: ") +
            Programmer::convertProgrammingErrorToShortString(vars.programmingError),
            Programmer::convertProgrammingErrorToLongString(vars.programmingError));
    }

    // Uncomment this code to test that the GUI can show the longest error
    // message:
    // window.setProgrammingError(
    //     Programmer::convertProgrammingErrorToShortString(PGM04A_PROGRAMMING_ERROR_SYNCH),
    //     Programmer::convertProgrammingErrorToLongString(PGM04A_PROGRAMMING_ERROR_SYNCH));

    if (vars.hasResultsFromLastProgramming)
    {
        window.setMeasuredVccMin(convertMvToString(vars.targetVccMeasuredMinMv));
        window.setMeasuredVccMax(convertMvToString(vars.targetVccMeasuredMaxMv));
        window.setMeasuredVddMin(convertMvToString(vars.programmerVddMeasuredMinMv));
        window.setMeasuredVddMax(convertMvToString(vars.programmerVddMeasuredMaxMv));
    }
    else
    {
        std::string value = "N/A";
        window.setMeasuredVccMin(value);
        window.setMeasuredVccMax(value);
        window.setMeasuredVddMin(value);
        window.setMeasuredVddMax(value);
    }

    window.setCurrentVcc(convertMvToString(vars.targetVccMv));
    window.setCurrentVdd(convertMvToString(vars.programmerVddMv));
    window.setRegulatorLevel(
        Programmer::convertRegulatorLevelToString(vars.regulatorLevel));

    // Note: It would be nice to display some error indication if
    // model->variablesUpdateFailed is true.
}

void MainView::handleSettingsChanged()
{
    // [all-settings]
    window.setIspFrequency(Programmer::getFrequencyName(
            model->settings.sckDuration, model->settings.ispFastestPeriod));
    window.setMaxIspFrequency(Programmer::getMaxFrequencyName(
            model->settings.ispFastestPeriod));
    window.setRegulatorMode(model->settings.regulatorMode);
    window.setVccOutputEnabled(model->settings.vccOutputEnabled);
    window.setVccOutputIndicator(model->settings.vccOutputIndicator);
    window.setLineAFunction(model->settings.lineAFunction);
    window.setLineBFunction(model->settings.lineBFunction);
    window.setVccVddMaxRange(model->settings.vccVddMaxRange);
    window.setVcc3v3Min(model->settings.vcc3v3Min);
    window.setVcc3v3Max(model->settings.vcc3v3Max);
    window.setVcc5vMin(model->settings.vcc5vMin);
    window.setVcc5vMax(model->settings.vcc5vMax);
    window.setStk500Versions(
        model->settings.hardwareVersion,
        model->settings.softwareVersionMajor,
        model->settings.softwareVersionMinor);

    window.setApplySettingsEnabled(model->connected() && model->settingsModified);
}
