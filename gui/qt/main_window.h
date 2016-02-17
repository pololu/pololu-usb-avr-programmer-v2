#pragma once

#include <programmer_frequency_tables.h>

#include <QMainWindow>

class FrequencyValidator;
class QCheckBox;
class QComboBox;
class QGridLayout;
class QGroupBox;
class QLabel;
class QMainWindow;
class QPushButton;
class QSpinBox;
class MainView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    /** Stores a pointer to the MainView object so that we can report events. **/
    void setView(MainView * view);

    /** This causes the window to call the view's handleUpdateEvent() function
     * periodically, on the same thread as everything else.
     *
     * intervalMs is the amount of time between updates, in milliseconds.
     */
    void startUpdateTimer(uint32_t intervalMs);

    void showErrorMessage(const std::string & message);
    void showWarningMessage(const std::string & message);
    void showInfoMessage(const std::string & message);

    /** Show an OK/Cancel dialog, return true if the user selects OK. */
    bool confirm(const std::string & question);

    /** Sets the label that shows the connection status/error. */
    void setConnectionStatus(const std::string & status, bool error);

    /** Controls whether the main controls of the application are enabled or
     * disabled. **/
    void setMainBoxesEnabled(bool enabled);

    /** Controls whether the Apply Settings action/button is enabled or
     * disabled. */
    void setApplySettingsEnabled(bool enabled);

    /** Controls whether the connect action is enabled or disabled. */
    void setConnectEnabled(bool enabled);

    /** Controls whether the disconnect action is enabled or disabled. */
    void setDisconnectEnabled(bool enabled);

    /** Controls whether the reload settings from device action is enabled. */
    void setReloadSettingsEnabled(bool enabled);

    /** Controls whether the restore defaults option is enabled. */
    void setRestoreDefaultsEnabled(bool enabled);

    void setDeviceName(const std::string & name, bool linkEnabled);
    void setSerialNumber(const std::string & serialNumber);
    void setFirmwareVersion(const std::string & firmwareVersion);
    void setProgPort(const std::string & portName);
    void setTtlPort(const std::string & portName);
    void setLastDeviceReset(const std::string & lastDeviceReset);
    void setProgrammingError(const std::string & shortMessage, const std::string & details);
    void setMeasuredVccMin(const std::string & voltage);
    void setMeasuredVccMax(const std::string & voltage);
    void setMeasuredVddMin(const std::string & voltage);
    void setMeasuredVddMax(const std::string & voltage);
    void setCurrentVcc(const std::string & voltage);
    void setCurrentVdd(const std::string & voltage);
    void setRegulatorLevel(const std::string & level);

    void configureIspFrequencyControls(
        const std::vector<ProgrammerFrequency> & allowedFrequencyTable,
        const std::vector<ProgrammerFrequency> & suggestedFrequencyTable,
        const ProgrammerFrequency & defaultFrequency,
        const std::vector<ProgrammerFrequency> & allowedMaxFrequencyTable,
        const std::vector<ProgrammerFrequency> & suggestedMaxFrequencyTable,
        const ProgrammerFrequency & defaultMaxFrequency);

    void setIspFrequency(const std::string & frequency);
    void setMaxIspFrequency(const std::string & frequency);
    void setRegulatorMode(uint8_t regulatorMode);
    void setVccOutputEnabled(bool enabled);
    void setVccOutputIndicator(bool steady);
    void setLineAFunction(uint8_t function);
    void setLineBFunction(uint8_t function);
    void setVccVddMaxRange(uint32_t mv);
    void setVcc3v3Min(uint32_t mv);
    void setVcc3v3Max(uint32_t mv);
    void setVcc5vMin(uint32_t mv);
    void setVcc5vMax(uint32_t mv);
    void setStk500Versions(uint8_t hardwareVersion,
        uint8_t softwareVersionMajor, uint8_t softwareVersionMinor);

private:
    /** Helper method for setting the index of a combo box, given the desired
     * uint8_t item value.  Defaults to using the first entry in the combo box if
     * the specified value is not found. */
    void setU8ComboBox(QComboBox * combo, uint8_t value);

    void setVoltageSetting(QSpinBox * box, uint32_t mv);

protected:
    /** This is called by Qt just before the window is shown for the first time,
     * and is also called whenever the window becomes unminimized. */
    void showEvent(QShowEvent *) override;

    /** This is called by Qt when the "close" slot is triggered, meaning that
     * the user wants to close the window. */
    void closeEvent(QCloseEvent *) override;

private slots:
    void on_connectAction_triggered();
    void on_disconnectAction_triggered();
    void on_reloadSettingsAction_triggered();
    void on_restoreDefaultsAction_triggered();
    void on_updateTimer_timeout();
    void on_deviceNameValue_linkActivated();
    void on_documentationAction_triggered();
    void on_aboutAction_triggered();

    /** This is called by Qt when the user wants to apply settings. */
    void on_applySettingsAction_triggered();

    /** This slot is called directly by Qt when the user presses enter in the
     * ISP frequency input or when the editing box loses focus (which can
     * actually happen while the user is just moving the window).  That's not
     * quite good enough for our purposes, so we also call this slot from
     * on_ispFrequencyValue_activated, which gets called whenever the user
     * selects an item in the combo box. */
    void on_ispFrequencyValueLineEdit_editingFinished();

    void on_ispFrequencyValue_activated(int index);
    void on_maxIspFrequencyValueLineEdit_editingFinished();
    void on_maxIspFrequencyValue_activated(int index);
    void on_regulatorModeValue_currentIndexChanged(int index);
    void on_vccOutputEnabledValue_currentIndexChanged(int index);
    void on_vccOutputIndicatorValue_currentIndexChanged(int index);
    void on_lineAFunctionValue_currentIndexChanged(int index);
    void on_lineBFunctionValue_currentIndexChanged(int index);
    void on_vccVddMaxRangeValue_valueChanged(int value);
    void on_vcc3v3MinValue_valueChanged(int value);
    void on_vcc3v3MaxValue_valueChanged(int value);
    void on_vcc5vMinValue_valueChanged(int value);
    void on_vcc5vMaxValue_valueChanged(int value);
    void on_stk500HardwareVersionValue_valueChanged(int value);
    void on_stk500SoftwareVersionMinorValue_valueChanged(int value);
    void on_stk500SoftwareVersionMajorValue_valueChanged(int value);

private:
    /** We use these variables to hold the current values of the two frequency
     * settings (with the " kHz" suffix) and make sure we don't send spurious
     * user input events when the user didn't actually change anything.  These
     * are only necessary because the signals provided by Qt are sometimes sent
     * too often, so these variables and functionality they allow belong
     * here. */
    std::string cachedIspFrequency;
    std::string cachedMaxIspFrequency;

private:
    MainView * view;
    bool startEventReported = false;

    /* We set this to true temporarily when programmatically setting the value
     * of an input in order to suppress sending a spurious user-input event to
     * the rest of the program. */
    bool suppressEvents = false;

    QTimer * updateTimer;

    // These are low-level functions called in the constructor that set up the
    // GUI elements.
    void setupWindow();
    void setupMenuBar();
    void adjustSizes();
    QWidget * setupDeviceInfoBox();
    QWidget * setupProgrammingResultsBox();
    QWidget * setupCurrentStatusBox();
    QWidget * setupSettingsWidget();
    QWidget * setupSettingsBox();
    QWidget * setupFooter();
    QWidget * setupConnectionStatus();
    QWidget * setupCancelChangesButton();
    QWidget * setupDefaultsButton();
    QWidget * setupApplyButton();
    void retranslate();

    QIcon programIcon;

    QMenuBar * menuBar;
    QMenu * fileMenu;
    QAction * exitAction;
    QMenu * deviceMenu;
    QAction * connectAction;
    QAction * disconnectAction;
    QAction * reloadSettingsAction;
    QAction * restoreDefaultsAction;
    QAction * applySettingsAction;
    QMenu * helpMenu;
    QAction * documentationAction;
    QAction * aboutAction;

    QWidget * centralWidget;
    QGridLayout * centralWidgetLayout;

    QGroupBox * deviceInfoBox;
    QGridLayout * deviceInfoBoxLayout;
    QLabel * deviceNameLabel;
    QLabel * deviceNameValue;
    QLabel * serialNumberLabel;
    QLabel * serialNumberValue;
    QLabel * firmwareVersionLabel;
    QLabel * firmwareVersionValue;
    QLabel * progPortLabel;
    QLabel * progPortValue;
    QLabel * ttlPortLabel;
    QLabel * ttlPortValue;
    QLabel * lastDeviceResetLabel;
    QLabel * lastDeviceResetValue;

    QGroupBox * programmingResultsBox;
    QGridLayout * programmingResultsBoxLayout;
    QLabel * programmingErrorValue;
    QLabel * measuredVccMinLabel;
    QLabel * measuredVccMinValue;
    QLabel * measuredVccMaxLabel;
    QLabel * measuredVccMaxValue;
    QLabel * measuredVddMinLabel;
    QLabel * measuredVddMinValue;
    QLabel * measuredVddMaxLabel;
    QLabel * measuredVddMaxValue;

    QGroupBox * currentStatusBox;
    QGridLayout * currentStatusBoxLayout;
    QLabel * currentVccLabel;
    QLabel * currentVccValue;
    QLabel * currentVddLabel;
    QLabel * currentVddValue;
    QLabel * currentRegulatorLevelLabel;
    QLabel * currentRegulatorLevelValue;

    QWidget * settingsWidget;
    QGridLayout * settingsWidgetLayout;

    // [all-settings]
    QGroupBox * settingsBox;
    QGridLayout * settingsBoxLayout;
    QLabel * ispFrequencyLabel;
    QComboBox * ispFrequencyValue;
    FrequencyValidator * ispFrequencyValidator;
    QLabel * maxIspFrequencyLabel;
    QComboBox * maxIspFrequencyValue;
    FrequencyValidator * maxIspFrequencyValidator;
    QLabel * regulatorModeLabel;
    QComboBox * regulatorModeValue;
    QLabel * vccOutputEnabledLabel;
    QComboBox * vccOutputEnabledValue;
    QLabel * vccOutputIndicatorLabel;
    QComboBox * vccOutputIndicatorValue;
    QLabel * lineAFunctionLabel;
    QComboBox * lineAFunctionValue;
    QLabel * lineBFunctionLabel;
    QComboBox * lineBFunctionValue;
    QLabel * vccVddMaxRangeLabel;
    QSpinBox * vccVddMaxRangeValue;
    QLabel * vcc3v3MinLabel;
    QSpinBox * vcc3v3MinValue;
    QLabel * vcc3v3MaxLabel;
    QSpinBox * vcc3v3MaxValue;
    QLabel * vcc5vMinLabel;
    QSpinBox * vcc5vMinValue;
    QLabel * vcc5vMaxLabel;
    QSpinBox * vcc5vMaxValue;
    QLabel * stk500HardwareVersionLabel;
    QSpinBox * stk500HardwareVersionValue;
    QLabel * stk500SoftwareVersionLabel;
    QSpinBox * stk500SoftwareVersionMajorValue;
    QSpinBox * stk500SoftwareVersionMinorValue;

    QWidget * footerWidget;
    QGridLayout * footerWidgetLayout;
    QLabel * connectionStatusValue;
    QPushButton * defaultsButton;
    QPushButton * cancelChangesButton;
    QPushButton * applySettingsButton;
};

