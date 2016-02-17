#pragma once

#include <stdexcept>
#include <cstdint>

class MainView;
class MainModel;

/** The controller receives events from the view (like user actions or update
 * timer timeouts), calls methods on the model to change the state of hte
 * application, handles errors/exceptions from the model and then tells the view
 * what parts might need to be updated.
 *
 * The model can throw exceptions, and the controller should catch these
 * exceptions and handle them appropriately.  If the controller allows any
 * exceptions to be thrown except under very unusual circumstances, that is
 * considered to be a bug in the controller.  This is important, because you're
 * not supposed to throw exceptions from a Qt slot.
*/
class MainController
{
public:
    void init(MainModel *, MainView *);

    /** This is called when the program starts up. */
    void start();

    /** This is called when the user issues a connect command. */
    void connect();

    /** This is called when the user issues a disconnect command. */
    void disconnect();

    /** This is called when the user issues a command to reload settings from
     * the device. */
    void reloadSettings();

    /** This is called when the user wants to restore the device to its default
     * settings. */
    void restoreDefaultSettings();

    /** This is called when it is time to check if the status of the device has
     * changed. */
    void update();

    /** This is called when the user tries to exit the program.  Returns true if
     * the program is actually allowed to exit. */
    bool exit();

private:
    void reallyConnect();

    /** Returns true for success, false for failure. */
    bool tryUpdateDeviceList();

    void showException(const std::exception & e, const std::string & context);

public:

    /** This is called when the user wants to apply the settings. */
    void applySettings();

    // These are called when the user changes a setting.
    void handleIspFrequencyInput(const std::string & input);
    void handleMaxIspFrequencyInput(const std::string & input);
    void handleRegulatorModeInput(uint8_t regulatorMode);
    void handleVccOutputEnabledInput(bool vccOutputEnabled);
    void handleVccOutputIndicatorInput(uint8_t indicator);
    void handleLineAFunctionInput(uint8_t function);
    void handleLineBFunctionInput(uint8_t function);
    void handleVccVddMaxRangeInput(uint32_t mv);
    void handleVcc3v3MinInput(uint32_t mv);
    void handleVcc3v3MaxInput(uint32_t mv);
    void handleVcc5vMinInput(uint32_t mv);
    void handleVcc5vMaxInput(uint32_t mv);
    void handleStk500VersionInput(uint32_t hardwareVersion,
        uint32_t softwareVersionMajor, uint32_t softwareVersionMinor);

private:

    MainModel * model;
    MainView * view;
};
