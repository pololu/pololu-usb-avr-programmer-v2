#include "main_window.h"
#include "main_view.h"
#include "main_controller.h"
#include "voltage_spin_box.h"
#include "frequency_validator.h"

#include "pavrpgm_config.h"
#include "pavr2_protocol.h"

#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcessEnvironment>
#include <QPushButton>
#include <QSpinBox>
#include <QStatusBar>
#include <QTimer>

#include <cassert>

#ifdef QT_STATIC
#include <QtPlugin>
#ifdef _WIN32
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
Q_IMPORT_PLUGIN(QWindowsVistaStylePlugin)
#endif
#endif
#ifdef __linux__
Q_IMPORT_PLUGIN(QLinuxFbIntegrationPlugin)
Q_IMPORT_PLUGIN(QXcbIntegrationPlugin)
#endif
#ifdef __APPLE__
Q_IMPORT_PLUGIN(QCocoaIntegrationPlugin)
#endif
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupWindow();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setView(MainView * view)
{
    this->view = view;
}

void MainWindow::startUpdateTimer(uint32_t intervalMs)
{
    assert(intervalMs <= std::numeric_limits<int>::max());
    updateTimer->start(intervalMs);
}

void MainWindow::showErrorMessage(const std::string & message)
{
    QMessageBox mbox(QMessageBox::Critical, windowTitle(),
        QString(message.c_str()));
    mbox.exec();
}

void MainWindow::showWarningMessage(const std::string & message)
{
    QMessageBox mbox(QMessageBox::Warning, windowTitle(),
        QString(message.c_str()));
    mbox.exec();
}

void MainWindow::showInfoMessage(const std::string & message)
{
    QMessageBox mbox(QMessageBox::Information, windowTitle(),
        QString(message.c_str()));
    mbox.exec();
}

bool MainWindow::confirm(const std::string & message)
{
    QMessageBox mbox(QMessageBox::Question, windowTitle(),
        QString(message.c_str()), QMessageBox::Ok | QMessageBox::Cancel);
    int button = mbox.exec();
    return button == QMessageBox::Ok;
}

void MainWindow::setConnectionStatus(const std::string & status, bool error)
{
    if (error)
    {
        connectionStatusValue->setStyleSheet("QLabel { color: red; }");
    }
    else
    {
        connectionStatusValue->setStyleSheet("");
    }
    connectionStatusValue->setText(QString(status.c_str()));
}

void MainWindow::setMainBoxesEnabled(bool enabled)
{
    deviceInfoBox->setEnabled(enabled);
    programmingResultsBox->setEnabled(enabled);
    currentStatusBox->setEnabled(enabled);
    settingsBox->setEnabled(enabled);
}

void MainWindow::setApplySettingsEnabled(bool enabled)
{
    applySettingsButton->setEnabled(enabled);
    applySettingsAction->setEnabled(enabled);
}

void MainWindow::setConnectEnabled(bool enabled)
{
    connectAction->setEnabled(enabled);
}

void MainWindow::setDisconnectEnabled(bool enabled)
{
    disconnectAction->setEnabled(enabled);
}

void MainWindow::setReloadSettingsEnabled(bool enabled)
{
    reloadSettingsAction->setEnabled(enabled);
}

void MainWindow::setRestoreDefaultsEnabled(bool enabled)
{
    restoreDefaultsAction->setEnabled(enabled);
}

void MainWindow::setDeviceName(const std::string & name, bool linkEnabled)
{
    QString text = name.c_str();
    if (linkEnabled)
    {
        text = "<a href=\"#doc\">" + text + "</a>";
    }

    deviceNameValue->setText(text);
}

void MainWindow::setSerialNumber(const std::string & serialNumber)
{
    serialNumberValue->setText(QString(serialNumber.c_str()));
}

void MainWindow::setFirmwareVersion(const std::string & firmwareVersion)
{
    firmwareVersionValue->setText(QString(firmwareVersion.c_str()));
}

void MainWindow::setProgPort(const std::string & portName)
{
    progPortValue->setText(QString(portName.c_str()));
}

void MainWindow::setTtlPort(const std::string & portName)
{
    ttlPortValue->setText(QString(portName.c_str()));
}

void MainWindow::setLastDeviceReset(const std::string & lastDeviceReset)
{
    lastDeviceResetValue->setText(QString(lastDeviceReset.c_str()));
}

void MainWindow::setProgrammingError(const std::string & shortMessage,
    const std::string & details)
{
    QString text = shortMessage.c_str();
    if (!details.empty())
    {
        text += "\n";
        text += details.c_str();
    }
    programmingErrorValue->setText(text);
}

void MainWindow::setMeasuredVccMin(const std::string & voltage)
{
    measuredVccMinValue->setText(QString(voltage.c_str()));
}

void MainWindow::setMeasuredVccMax(const std::string & voltage)
{
    measuredVccMaxValue->setText(QString(voltage.c_str()));
}

void MainWindow::setMeasuredVddMin(const std::string & voltage)
{
    measuredVddMinValue->setText(QString(voltage.c_str()));
}

void MainWindow::setMeasuredVddMax(const std::string & voltage)
{
    measuredVddMaxValue->setText(QString(voltage.c_str()));
}

void MainWindow::setCurrentVcc(const std::string & voltage)
{
    currentVccValue->setText(QString(voltage.c_str()));
}

void MainWindow::setCurrentVdd(const std::string & voltage)
{
    currentVddValue->setText(QString(voltage.c_str()));
}

void MainWindow::setRegulatorLevel(const std::string & level)
{
    currentRegulatorLevelValue->setText(QString(level.c_str()));
}

void MainWindow::configureIspFrequencyControls(
    const std::vector<ProgrammerFrequency> & allowedFrequencyTable,
    const std::vector<ProgrammerFrequency> & suggestedFrequencyTable,
    const ProgrammerFrequency & defaultFrequency,
    const std::vector<ProgrammerFrequency> & allowedMaxFrequencyTable,
    const std::vector<ProgrammerFrequency> & suggestedMaxFrequencyTable,
    const ProgrammerFrequency & defaultMaxFrequency)
{
    QString suffix = " kHz";

    ispFrequencyValidator->setAllowedFrequencies(allowedFrequencyTable);
    ispFrequencyValidator->setDefaultFrequency(defaultFrequency);

    maxIspFrequencyValidator->setAllowedFrequencies(allowedMaxFrequencyTable);
    maxIspFrequencyValidator->setDefaultFrequency(defaultMaxFrequency);

    // Set the items in the ispFrequencyValue combobox to be equal to the
    // frequency names in suggestedFrequencyTable, without modifying anything
    // that is already correct.
    for (size_t i = 0; i < suggestedFrequencyTable.size(); i++)
    {
        QString desiredText = QString(suggestedFrequencyTable[i].name) + suffix;
        QString currentText = ispFrequencyValue->itemText(i);
        if (currentText.size() == 0)
        {
            ispFrequencyValue->addItem(desiredText);
        }
        else if (currentText != desiredText)
        {
            ispFrequencyValue->setItemText(i, desiredText);
        }
    }
    while ((size_t)ispFrequencyValue->count() > suggestedFrequencyTable.size())
    {
        ispFrequencyValue->removeItem(ispFrequencyValue->count() - 1);
    }

    // Set the items in the maxFrequencyValue combobox to be the items that are
    // in both the suggestedFrequencyTable and maxFrequencyTable, without
    // modifying anything that is already correct.
    for (size_t i = 0; i < suggestedMaxFrequencyTable.size(); i++)
    {
        QString desiredText = QString(suggestedMaxFrequencyTable[i].name) + suffix;
        QString currentText = maxIspFrequencyValue->itemText(i);
        if (currentText.size() == 0)
        {
            maxIspFrequencyValue->addItem(desiredText);
        }
        else if (currentText != desiredText)
        {
            maxIspFrequencyValue->setItemText(i, desiredText);
        }
    }
    while ((size_t)maxIspFrequencyValue->count() > suggestedMaxFrequencyTable.size())
    {
        maxIspFrequencyValue->removeItem(maxIspFrequencyValue->count() - 1);
    }

    // In some cases, Qt automatically sets the text of a combo box to be equal
    // to the first item.  It would be bad/confusing if we had a bug in our
    // code and the user saw this invalid value.  So we will set the text to be
    // empty, and users of this class should know that they have to set the text
    // separately after calling this function.
    ispFrequencyValue->lineEdit()->setText("");
    maxIspFrequencyValue->lineEdit()->setText("");
}

void MainWindow::setIspFrequency(const std::string & frequency)
{
    cachedIspFrequency = frequency + " kHz";
    ispFrequencyValue->setCurrentText(QString(cachedIspFrequency.c_str()));
}

void MainWindow::setMaxIspFrequency(const std::string & frequency)
{
    cachedMaxIspFrequency = frequency + " kHz";
    maxIspFrequencyValue->setCurrentText(cachedMaxIspFrequency.c_str());
}

void MainWindow::setRegulatorMode(uint8_t regulatorMode)
{
    setU8ComboBox(regulatorModeValue, regulatorMode);
}

void MainWindow::setVccOutputEnabled(bool enabled)
{
    setU8ComboBox(vccOutputEnabledValue, enabled);
}

void MainWindow::setVccOutputIndicator(bool steady)
{
    setU8ComboBox(vccOutputIndicatorValue, steady);
}

void MainWindow::setLineAFunction(uint8_t function)
{
    setU8ComboBox(lineAFunctionValue, function);
}

void MainWindow::setLineBFunction(uint8_t function)
{
    setU8ComboBox(lineBFunctionValue, function);
}

void MainWindow::setVccVddMaxRange(uint32_t mv)
{
    setVoltageSetting(vccVddMaxRangeValue, mv);
}

void MainWindow::setVcc3v3Min(uint32_t mv)
{
    setVoltageSetting(vcc3v3MinValue, mv);
}

void MainWindow::setVcc3v3Max(uint32_t mv)
{
    setVoltageSetting(vcc3v3MaxValue, mv);
}

void MainWindow::setVcc5vMin(uint32_t mv)
{
    setVoltageSetting(vcc5vMinValue, mv);
}

void MainWindow::setVcc5vMax(uint32_t mv)
{
    setVoltageSetting(vcc5vMaxValue, mv);
}

void MainWindow::setStk500Versions(uint8_t hardwareVersion,
        uint8_t softwareVersionMajor, uint8_t softwareVersionMinor)
{
    suppressEvents = true;
    stk500HardwareVersionValue->setValue(hardwareVersion);
    stk500SoftwareVersionMajorValue->setValue(softwareVersionMajor);
    stk500SoftwareVersionMinorValue->setValue(softwareVersionMinor);
    suppressEvents = false;
}

void MainWindow::setU8ComboBox(QComboBox * combo, uint8_t value)
{
    int index = 0;
    for (int i = 0; i < combo->count(); i++)
    {
        if (value == combo->itemData(i).toUInt())
        {
            index = i;
            break;
        }
    }
    suppressEvents = true;
    combo->setCurrentIndex(index);
    suppressEvents = false;
}

void MainWindow::setVoltageSetting(QSpinBox * box, uint32_t mv)
{
    suppressEvents = true;
    box->setValue(mv);
    suppressEvents = false;
}

void MainWindow::centerAtStartupIfNeeded()
{
  // Center the window.  This fixes a strange bug on the Raspbian Jessie that we
  // saw while developing the Tic software where the window would appear in the
  // upper left with its title bar off the screen.  On other platforms, the
  // default window position did not make much sense, so it is nice to center
  // it.
  //
  // In case this causes problems, you can set the PAVR2GUI_CENTER environment
  // variable to "N".
  //
  // NOTE: This position issue on Raspbian is a bug in Qt that should be fixed.
  // Another workaround for it was to uncomment the lines in retranslate() that
  // set up errors_stopping_header_label, error_rows[*].name_label, and
  // manual_target_velocity_mode_radio, but then the Window would strangely
  // start in the lower right.
  auto env = QProcessEnvironment::systemEnvironment();
  if (env.value("PAVR2GUI_CENTER") != "N")
  {
    setGeometry(
      QStyle::alignedRect(
        Qt::LeftToRight,
        Qt::AlignCenter,
        size(),
        qApp->desktop()->availableGeometry()
        )
      );
  }
}

void MainWindow::showEvent(QShowEvent * event)
{
    Q_UNUSED(event);
    if (!startEventReported)
    {
        startEventReported = true;
        centerAtStartupIfNeeded();
        view->userInputReceiver()->start();
    }
}

void MainWindow::closeEvent(QCloseEvent * event)
{
    bool allowExit = view->userInputReceiver()->exit();
    if (!allowExit)
    {
        event->ignore();
    }
}

void MainWindow::on_connectAction_triggered()
{
    view->userInputReceiver()->connect();
}

void MainWindow::on_disconnectAction_triggered()
{
    view->userInputReceiver()->disconnect();
}

void MainWindow::on_reloadSettingsAction_triggered()
{
    view->userInputReceiver()->reloadSettings();
}

void MainWindow::on_restoreDefaultsAction_triggered()
{
    view->userInputReceiver()->restoreDefaultSettings();
}

void MainWindow::on_updateTimer_timeout()
{
    view->userInputReceiver()->update();
}

void MainWindow::on_deviceNameValue_linkActivated()
{
    on_documentationAction_triggered();
}

void MainWindow::on_documentationAction_triggered()
{
    QDesktopServices::openUrl(QUrl(DOCUMENTATION_URL));
}

void MainWindow::on_aboutAction_triggered()
{
    QMessageBox::about(this, tr("About") + " " + windowTitle(),
        QString("<h2>") + windowTitle() + "</h2>" +
        tr("<h4>Version %1</h4>"
            "<h4>Copyright &copy; %2 Pololu Corporation</h4>"
            "<p>See LICENSE.html for copyright and license information.</p>"
            "<p><a href=\"%3\">Online documentation</a></p>")
        .arg(SOFTWARE_VERSION_STRING, SOFTWARE_YEAR, DOCUMENTATION_URL));
}

void MainWindow::on_applySettingsAction_triggered()
{
    view->userInputReceiver()->applySettings();
}

static std::string removeFrequencySuffix(const std::string input)
{
    // The input should have already been fixed up by the FrequencyValidator for
    // this control.  We just need to remove the " kHz" suffix (because that is
    // a detailed handled by MainWindow) and pass the value up to the view.

    assert(input.size() > 4 && input.substr(input.size() - 4, 4) == " kHz");
    return input.substr(0, input.size() - 4);
}

void MainWindow::on_ispFrequencyValueLineEdit_editingFinished()
{
    std::string input = ispFrequencyValue->currentText().toStdString();
    if (input != cachedIspFrequency)
    {
        cachedIspFrequency = input;
        view->userInputReceiver()->
            handleIspFrequencyInput(removeFrequencySuffix(input));
    }
}

void MainWindow::on_ispFrequencyValue_activated(int index)
{
    Q_UNUSED(index);
    on_ispFrequencyValueLineEdit_editingFinished();
}

void MainWindow::on_maxIspFrequencyValueLineEdit_editingFinished()
{
    std::string input = maxIspFrequencyValue->currentText().toStdString();
    if (input != cachedMaxIspFrequency)
    {
        cachedMaxIspFrequency = input;
        view->userInputReceiver()->
            handleMaxIspFrequencyInput(removeFrequencySuffix(input));
    }
}

void MainWindow::on_maxIspFrequencyValue_activated(int index)
{
    Q_UNUSED(index);
    on_maxIspFrequencyValueLineEdit_editingFinished();
}

void MainWindow::on_regulatorModeValue_currentIndexChanged(int index)
{
    if (suppressEvents) { return; }
    uint8_t mode = regulatorModeValue->itemData(index).toUInt();
    view->userInputReceiver()->handleRegulatorModeInput(mode);
}

void MainWindow::on_vccOutputEnabledValue_currentIndexChanged(int index)
{
    if (suppressEvents) { return; }
    view->userInputReceiver()->handleVccOutputEnabledInput(
        vccOutputEnabledValue->itemData(index).toBool());
}

void MainWindow::on_vccOutputIndicatorValue_currentIndexChanged(int index)
{
    if (suppressEvents) { return; }
    view->userInputReceiver()->handleVccOutputIndicatorInput(
        vccOutputIndicatorValue->itemData(index).toUInt());
}

void MainWindow::on_lineAFunctionValue_currentIndexChanged(int index)
{
    if (suppressEvents) { return; }
    uint8_t function = lineAFunctionValue->itemData(index).toUInt();
    view->userInputReceiver()->handleLineAFunctionInput(function);
}

void MainWindow::on_lineBFunctionValue_currentIndexChanged(int index)
{
    if (suppressEvents) { return; }
    uint8_t function = lineBFunctionValue->itemData(index).toUInt();
    view->userInputReceiver()->handleLineBFunctionInput(function);
}

void MainWindow::on_vccVddMaxRangeValue_valueChanged(int value)
{
    if (suppressEvents) { return; }
    view->userInputReceiver()->handleVccVddMaxRangeInput(value);
}

void MainWindow::on_vcc3v3MinValue_valueChanged(int value)
{
    if (suppressEvents) { return; }
    view->userInputReceiver()->handleVcc3v3MinInput(value);
}

void MainWindow::on_vcc3v3MaxValue_valueChanged(int value)
{
    if (suppressEvents) { return; }
    view->userInputReceiver()->handleVcc3v3MaxInput(value);
}

void MainWindow::on_vcc5vMinValue_valueChanged(int value)
{
    if (suppressEvents) { return; }
    view->userInputReceiver()->handleVcc5vMinInput(value);
}

void MainWindow::on_vcc5vMaxValue_valueChanged(int value)
{
    if (suppressEvents) { return; }
    view->userInputReceiver()->handleVcc5vMaxInput(value);
}

void MainWindow::on_stk500HardwareVersionValue_valueChanged(int value)
{
    Q_UNUSED(value);
    if (suppressEvents) { return; }
    view->userInputReceiver()->handleStk500VersionInput(
        stk500HardwareVersionValue->value(),
        stk500SoftwareVersionMajorValue->value(),
        stk500SoftwareVersionMinorValue->value());
}

void MainWindow::on_stk500SoftwareVersionMinorValue_valueChanged(int value)
{
    Q_UNUSED(value);
    on_stk500HardwareVersionValue_valueChanged(0);
}

void MainWindow::on_stk500SoftwareVersionMajorValue_valueChanged(int value)
{
    Q_UNUSED(value);
    on_stk500HardwareVersionValue_valueChanged(0);
}

// On Mac OS X, field labels are usually right-aligned.
#ifdef __APPLE__
#define FIELD_LABEL_SUFFIX ":"
#define FIELD_LABEL_ALIGNMENT Qt::AlignRight
#else
#define FIELD_LABEL_SUFFIX ":"
#define FIELD_LABEL_ALIGNMENT Qt::AlignLeft
#endif

static void setupReadOnlyTextField(QGridLayout * layout, int row,
    QLabel ** label, QLabel ** value)
{
    QLabel * newValue = new QLabel();
    newValue->setTextInteractionFlags(Qt::TextSelectableByMouse);

    QLabel * newLabel = new QLabel();
    newLabel->setBuddy(newValue);

    layout->addWidget(newLabel, row, 0, FIELD_LABEL_ALIGNMENT);
    layout->addWidget(newValue, row, 1, 0);

    if (label) { *label = newLabel; }
    if (value) { *value = newValue; }
}

static void setupVoltageInput(QGridLayout * layout, int row,
    QLabel ** label, QSpinBox ** value)
{
    VoltageSpinBox * newValue = new VoltageSpinBox();
    newValue->setRange(0, PAVR2_VOLTAGE_UNITS * 255);
    newValue->setSuffix(" mV");
    newValue->setSingleStep(PAVR2_VOLTAGE_UNITS);

    QLabel * newLabel = new QLabel();
    newLabel->setBuddy(newValue);

    layout->addWidget(newLabel, row, 0, FIELD_LABEL_ALIGNMENT);
    layout->addWidget(newValue, row, 1, Qt::AlignLeft);

    if (label) { *label = newLabel; }
    if (value) { *value = newValue; }
}

static QSpinBox * newHexByteInput()
{
    // NOTE: It would be kind of nice to make a subclass of QSpinBox that
    // displays the hex values in upper-case and use it here.
    QSpinBox * input = new QSpinBox();
    input->setDisplayIntegerBase(0x10);
    input->setRange(0x00, 0xFF);
    return input;
}

void MainWindow::setupWindow()
{
    setStyleSheet("QPushButton { padding: 0.3em 1em; }");

    setupMenuBar();

    centralWidget = new QWidget();
    QGridLayout * layout = centralWidgetLayout = new QGridLayout();

    int row = 0;
    layout->addWidget(setupDeviceInfoBox(), row++, 0, 0);
    layout->addWidget(setupProgrammingResultsBox(), row++, 0, 0);
    layout->addWidget(setupCurrentStatusBox(), row++, 0, 0);
    layout->addWidget(setupSettingsWidget(), 0, 1, row, 1, 0);
    layout->addWidget(setupFooter(), row++, 0, 1, 2, 0);

    layout->setRowStretch(row, 1);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    retranslate();

    adjustSizes();

    // Make the window non-resizable.
    setFixedSize(sizeHint());

    programIcon = QIcon(":app_icon");
    setWindowIcon(programIcon);

    updateTimer = new QTimer(this);
    updateTimer->setObjectName("updateTimer");

    QMetaObject::connectSlotsByName(this);
}

void MainWindow::setupMenuBar()
{
    menuBar = new QMenuBar();

    fileMenu = menuBar->addMenu("");

    exitAction = new QAction(this);
    exitAction->setObjectName("exitAction");
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));
    fileMenu->addAction(exitAction);

    deviceMenu = menuBar->addMenu("");

    connectAction = new QAction(this);
    connectAction->setObjectName("connectAction");
    connectAction->setShortcut(Qt::CTRL + Qt::Key_N);
    deviceMenu->addAction(connectAction);

    disconnectAction = new QAction(this);
    disconnectAction->setObjectName("disconnectAction");
    disconnectAction->setShortcut(Qt::CTRL + Qt::Key_D);
    deviceMenu->addAction(disconnectAction);

    deviceMenu->addSeparator();

    reloadSettingsAction = new QAction(this);
    reloadSettingsAction->setObjectName("reloadSettingsAction");
    deviceMenu->addAction(reloadSettingsAction);

    restoreDefaultsAction = new QAction(this);
    restoreDefaultsAction->setObjectName("restoreDefaultsAction");
    deviceMenu->addAction(restoreDefaultsAction);

    applySettingsAction = new QAction(this);
    applySettingsAction->setObjectName("applySettingsAction");
    applySettingsAction->setShortcut(Qt::CTRL + Qt::Key_P);
    deviceMenu->addAction(applySettingsAction);

    helpMenu = menuBar->addMenu("");

    documentationAction = new QAction(this);
    documentationAction->setObjectName("documentationAction");
    documentationAction->setShortcut(QKeySequence::HelpContents);
    helpMenu->addAction(documentationAction);

    aboutAction = new QAction(this);
    aboutAction->setObjectName("aboutAction");
    aboutAction->setShortcut(QKeySequence::WhatsThis);
    helpMenu->addAction(aboutAction);

    setMenuBar(menuBar);
}

void MainWindow::adjustSizes()
{
    // Here is some code we used to use to make the values on the left half of
    // the window line up with eachother even though they are in different
    // boxes:
    // int leftLabelWidth = measuredVddMaxLabel->sizeHint().width();
    // programmingResultsBoxLayout->setColumnMinimumWidth(0, leftLabelWidth);
    // currentStatusBoxLayout->setColumnMinimumWidth(0, leftLabelWidth);
    // deviceInfoBoxLayout->setColumnMinimumWidth(0, leftLabelWidth);

    // Make the inputs have nice widths.
    QComboBox tmpBox;
    tmpBox.addItem("9.999 kHz");
    int typicalInputWidth = tmpBox.sizeHint().width() * 110 / 100;
    maxIspFrequencyValue->setMinimumWidth(typicalInputWidth);
    ispFrequencyValue->setMinimumWidth(typicalInputWidth);
    vccOutputEnabledValue->setMinimumWidth(typicalInputWidth);
    vccOutputIndicatorValue->setMinimumWidth(typicalInputWidth);
    vccVddMaxRangeValue->setMinimumWidth(typicalInputWidth);
    vcc3v3MinValue->setMinimumWidth(typicalInputWidth);
    vcc3v3MaxValue->setMinimumWidth(typicalInputWidth);
    vcc5vMinValue->setMinimumWidth(typicalInputWidth);
    vcc5vMaxValue->setMinimumWidth(typicalInputWidth);
}

QWidget * MainWindow::setupDeviceInfoBox()
{
    deviceInfoBox = new QGroupBox();
    QGridLayout * layout = deviceInfoBoxLayout = new QGridLayout();
    layout->setColumnStretch(1, 1);

    int row = 0;

    setupReadOnlyTextField(layout, row++,
        &deviceNameLabel, &deviceNameValue);
    deviceNameValue->setObjectName("deviceNameValue");
    deviceNameValue->setTextInteractionFlags(Qt::TextBrowserInteraction);

    setupReadOnlyTextField(layout, row++,
        &serialNumberLabel, &serialNumberValue);
    setupReadOnlyTextField(layout, row++,
        &firmwareVersionLabel, &firmwareVersionValue);
    setupReadOnlyTextField(layout, row++,
        &progPortLabel, &progPortValue);
    setupReadOnlyTextField(layout, row++,
        &ttlPortLabel, &ttlPortValue);

    // Make the right column wide enough to display the name of the programmer,
    // which should be the widest thing that needs to fit in that column.
    // This is important for making sure that the sizeHint of the overall main
    // window has a good width before we set the window to be a fixed size.
    {
        QLabel tmpLabel;
        tmpLabel.setText("Pololu USB AVR Programmer v99.9");
        layout->setColumnMinimumWidth(1, tmpLabel.sizeHint().width());
    }

    layout->setRowStretch(row, 1);
    deviceInfoBox->setLayout(layout);
    return deviceInfoBox;
}

QWidget * MainWindow::setupProgrammingResultsBox()
{
    programmingResultsBox = new QGroupBox();
    QGridLayout * layout = programmingResultsBoxLayout = new QGridLayout();
    layout->setColumnStretch(1, 1);
    int row = 0;
    setupReadOnlyTextField(layout, row++,
        &measuredVccMinLabel, &measuredVccMinValue);
    setupReadOnlyTextField(layout, row++,
        &measuredVccMaxLabel, &measuredVccMaxValue);
    setupReadOnlyTextField(layout, row++,
        &measuredVddMinLabel, &measuredVddMinValue);
    setupReadOnlyTextField(layout, row++,
        &measuredVddMaxLabel, &measuredVddMaxValue);

    // Make programmingErrorvalue be a multi-line label with selectable text.
    const int lineCount = 6;
    programmingErrorValue = new QLabel();
    programmingErrorValue->setWordWrap(true);
    programmingErrorValue->setMinimumHeight(
        programmingErrorValue->sizeHint().height() * lineCount);
    programmingErrorValue->setAlignment(Qt::AlignTop);
    programmingErrorValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(programmingErrorValue, row++, 0, 1, 2, 0);

    layout->setRowStretch(row, 1);
    programmingResultsBox->setLayout(layout);
    return programmingResultsBox;
}

QWidget * MainWindow::setupCurrentStatusBox()
{
    currentStatusBox = new QGroupBox();
    QGridLayout * layout = currentStatusBoxLayout = new QGridLayout();
    layout->setColumnStretch(1, 1);
    int row = 0;
    setupReadOnlyTextField(layout, row++,
        &currentVccLabel, &currentVccValue);
    setupReadOnlyTextField(layout, row++,
        &currentVddLabel, &currentVddValue);
    setupReadOnlyTextField(layout, row++,
        &currentRegulatorLevelLabel, &currentRegulatorLevelValue);
    setupReadOnlyTextField(layout, row++,
        &lastDeviceResetLabel, &lastDeviceResetValue);

    // Make the right column wide enough to display all the resets.
    {
        QLabel tmpLabel;
        tmpLabel.setText("Software reset (bootloader)  ");
        layout->setColumnMinimumWidth(1, tmpLabel.sizeHint().width());
    }

    layout->setRowStretch(row, 1);
    currentStatusBox->setLayout(layout);
    return currentStatusBox;
}

QWidget * MainWindow::setupSettingsWidget()
{
#ifdef __APPLE__
    // There is something odd about Qt 5.5.1 on Mac OS X which makes the
    // QGridLayout add extra padding that we don't want (about 10px at the top
    // and about 1px at the bottom).  Since the QGridLayout only has one
    // element, we don't really need it, and we can use this simple workaround
    // for now.
    return setupSettingsBox();
#endif

    settingsWidget = new QWidget();
    QGridLayout * layout = settingsWidgetLayout = new QGridLayout();
    layout->setContentsMargins(0, 0, 0, 0);

    int row = 0;
    layout->addWidget(setupSettingsBox(), row++, 0, 0);

    settingsWidget->setLayout(layout);
    return settingsWidget;
}

// [all-settings]
QWidget * MainWindow::setupSettingsBox()
{
    settingsBox = new QGroupBox();
    QGridLayout * layout = settingsBoxLayout = new QGridLayout();
    layout->setColumnStretch(1, 1);
    int row = 0;

    {
        ispFrequencyValue = new QComboBox();
        ispFrequencyValue->setObjectName("ispFrequencyValue");
        ispFrequencyValue->setEditable(true);
        ispFrequencyValue->lineEdit()->setObjectName("ispFrequencyValueLineEdit");
        ispFrequencyValue->setInsertPolicy(QComboBox::NoInsert);
        ispFrequencyValidator = new FrequencyValidator();
        ispFrequencyValue->setValidator(ispFrequencyValidator);
        ispFrequencyLabel = new QLabel();
        ispFrequencyLabel->setBuddy(ispFrequencyValue);
        layout->addWidget(ispFrequencyLabel, row, 0, FIELD_LABEL_ALIGNMENT);
        layout->addWidget(ispFrequencyValue, row, 1, Qt::AlignLeft);
        row++;
    }

    {
        maxIspFrequencyValue = new QComboBox();
        maxIspFrequencyValue->setObjectName("maxIspFrequencyValue");
        maxIspFrequencyValue->setEditable(true);
        maxIspFrequencyValue->lineEdit()->setObjectName("maxIspFrequencyValueLineEdit");
        maxIspFrequencyValue->setInsertPolicy(QComboBox::NoInsert);
        maxIspFrequencyValidator = new FrequencyValidator();
        maxIspFrequencyValue->setValidator(maxIspFrequencyValidator);
        maxIspFrequencyLabel = new QLabel();
        maxIspFrequencyLabel->setBuddy(maxIspFrequencyValue);
        layout->addWidget(maxIspFrequencyLabel, row, 0, FIELD_LABEL_ALIGNMENT);
        layout->addWidget(maxIspFrequencyValue, row, 1, Qt::AlignLeft);
        row++;
    }

    {
        regulatorModeValue = new QComboBox();
        regulatorModeValue->setObjectName("regulatorModeValue");
        regulatorModeValue->addItem("Auto", PAVR2_REGULATOR_MODE_AUTO);
        regulatorModeValue->addItem("5 V", PAVR2_REGULATOR_MODE_5V);
        regulatorModeValue->addItem("3.3 V", PAVR2_REGULATOR_MODE_3V3);
        regulatorModeLabel = new QLabel();
        regulatorModeLabel->setBuddy(regulatorModeValue);
        layout->addWidget(regulatorModeLabel, row, 0, FIELD_LABEL_ALIGNMENT);
        layout->addWidget(regulatorModeValue, row, 1, Qt::AlignLeft);
        row++;
    }

    {
        vccOutputEnabledValue = new QComboBox();
        vccOutputEnabledValue->setObjectName("vccOutputEnabledValue");
        vccOutputEnabledValue->addItem("Disabled", false);
        vccOutputEnabledValue->addItem("Enabled", true);
        vccOutputEnabledLabel = new QLabel();
        vccOutputEnabledLabel->setBuddy(vccOutputEnabledValue);
        layout->addWidget(vccOutputEnabledLabel, row, 0, FIELD_LABEL_ALIGNMENT);
        layout->addWidget(vccOutputEnabledValue, row, 1, Qt::AlignLeft);
        row++;
    }

    {
        vccOutputIndicatorValue = new QComboBox();
        vccOutputIndicatorValue->setObjectName("vccOutputIndicatorValue");
        vccOutputIndicatorValue->addItem("Blinking", PAVR2_VCC_OUTPUT_INDICATOR_BLINKING);
        vccOutputIndicatorValue->addItem("Steady", PAVR2_VCC_OUTPUT_INDICATOR_STEADY);
        vccOutputIndicatorLabel = new QLabel();
        vccOutputIndicatorLabel->setObjectName("vccOutputIndicatorLabel");
        layout->addWidget(vccOutputIndicatorLabel, row, 0, FIELD_LABEL_ALIGNMENT);
        layout->addWidget(vccOutputIndicatorValue, row, 1, Qt::AlignLeft);
        row++;
    }

    {
        lineAFunctionValue = new QComboBox();
        lineAFunctionValue->setObjectName("lineAFunctionValue");
        lineAFunctionValue->addItem("(none)", PAVR2_LINE_IS_NOTHING);
        lineAFunctionValue->addItem("CD (input)", PAVR2_LINE_IS_CD);
        lineAFunctionValue->addItem("DSR (input)", PAVR2_LINE_IS_DSR);
        lineAFunctionValue->addItem("DTR (output)", PAVR2_LINE_IS_DTR);
        lineAFunctionValue->addItem("RTS (output)", PAVR2_LINE_IS_RTS);
        lineAFunctionValue->addItem("DTR reset (output)", PAVR2_LINE_IS_DTR_RESET);
        lineAFunctionLabel = new QLabel();
        lineAFunctionLabel->setBuddy(lineAFunctionValue);
        layout->addWidget(lineAFunctionLabel, row, 0, FIELD_LABEL_ALIGNMENT);
        layout->addWidget(lineAFunctionValue, row, 1, Qt::AlignLeft);
        row++;
    }

    {
        lineBFunctionValue = new QComboBox();
        lineBFunctionValue->setObjectName("lineBFunctionValue");
        lineBFunctionValue->addItem("(none)", PAVR2_LINE_IS_NOTHING);
        lineBFunctionValue->addItem("CD (input)", PAVR2_LINE_IS_CD);
        lineBFunctionValue->addItem("DSR (input)", PAVR2_LINE_IS_DSR);
        lineBFunctionValue->addItem("DTR (output)", PAVR2_LINE_IS_DTR);
        lineBFunctionValue->addItem("RTS (output)", PAVR2_LINE_IS_RTS);
        lineBFunctionValue->addItem("Clock (output)", PAVR2_LINE_IS_CLOCK);
        lineBFunctionValue->addItem("DTR reset (output)", PAVR2_LINE_IS_DTR_RESET);
        lineBFunctionLabel = new QLabel();
        lineBFunctionLabel->setBuddy(lineBFunctionValue);
        layout->addWidget(lineBFunctionLabel, row, 0, FIELD_LABEL_ALIGNMENT);
        layout->addWidget(lineBFunctionValue, row, 1, Qt::AlignLeft);
        row++;
    }

    setupVoltageInput(layout, row++, &vccVddMaxRangeLabel, &vccVddMaxRangeValue);
    vccVddMaxRangeValue->setObjectName("vccVddMaxRangeValue");
    setupVoltageInput(layout, row++, &vcc3v3MinLabel, &vcc3v3MinValue);
    vcc3v3MinValue->setObjectName("vcc3v3MinValue");
    setupVoltageInput(layout, row++, &vcc3v3MaxLabel, &vcc3v3MaxValue);
    vcc3v3MaxValue->setObjectName("vcc3v3MaxValue");
    setupVoltageInput(layout, row++, &vcc5vMinLabel, &vcc5vMinValue);
    vcc5vMinValue->setObjectName("vcc5vMinValue");
    setupVoltageInput(layout, row++, &vcc5vMaxLabel, &vcc5vMaxValue);
    vcc5vMaxValue->setObjectName("vcc5vMaxValue");

    {
        stk500HardwareVersionValue = newHexByteInput();
        stk500HardwareVersionValue->setObjectName("stk500HardwareVersionValue");
        stk500HardwareVersionLabel = new QLabel();
        stk500HardwareVersionLabel->setBuddy(stk500HardwareVersionValue);
        layout->addWidget(stk500HardwareVersionLabel, row, 0, FIELD_LABEL_ALIGNMENT);
        layout->addWidget(stk500HardwareVersionValue, row, 1, Qt::AlignLeft);
        row++;
    }

    {
        stk500SoftwareVersionMinorValue = newHexByteInput();
        stk500SoftwareVersionMinorValue->setObjectName("stk500SoftwareVersionMinorValue");
        stk500SoftwareVersionMajorValue = newHexByteInput();
        stk500SoftwareVersionMajorValue->setObjectName("stk500SoftwareVersionMajorValue");
        stk500SoftwareVersionLabel = new QLabel();
        stk500SoftwareVersionLabel->setBuddy(stk500SoftwareVersionLabel);
        layout->addWidget(stk500SoftwareVersionLabel, row, 0, FIELD_LABEL_ALIGNMENT);

        QWidget * stk500SoftwareVersionWidget = new QWidget();
        QGridLayout * subLayout = new QGridLayout();
        subLayout->setContentsMargins(0, 0, 0, 0);
        subLayout->setColumnStretch(2, 1);
        subLayout->addWidget(stk500SoftwareVersionMajorValue, 0, 0, Qt::AlignLeft);
        subLayout->addWidget(stk500SoftwareVersionMinorValue, 0, 1, Qt::AlignLeft);
        stk500SoftwareVersionWidget->setLayout(subLayout);
        layout->addWidget(stk500SoftwareVersionWidget);
        row++;
    }

    layout->setRowStretch(row, 1);

    settingsBox->setLayout(layout);
    return settingsBox;
}

QWidget * MainWindow::setupFooter()
{
    footerWidget = new QWidget();
    QGridLayout * layout = footerWidgetLayout = new QGridLayout();
    layout->setContentsMargins(0, 0, 0, 0);

    int col = 0;
    layout->addWidget(setupConnectionStatus(), 0, col++, Qt::AlignLeft);
    layout->addWidget(setupApplyButton(), 0, col++, Qt::AlignRight);

    layout->setColumnStretch(0, 1);

    footerWidget->setLayout(layout);
    return footerWidget;
}

QWidget * MainWindow::setupConnectionStatus()
{
    connectionStatusValue = new QLabel();
    return connectionStatusValue;
}

QWidget * MainWindow::setupApplyButton()
{
    applySettingsButton = new QPushButton();

    // Make the button a little bit bigger so it's easier to click.
    connect(applySettingsButton, SIGNAL(clicked()),
        this, SLOT(on_applySettingsAction_triggered()));
    return applySettingsButton;
}

void MainWindow::retranslate()
{
    setWindowTitle(tr("Pololu USB AVR Programmer v2 Configuration Utility"));

    fileMenu->setTitle(tr("&File"));
    exitAction->setText(tr("E&xit"));
    deviceMenu->setTitle(tr("&Device"));
    connectAction->setText(tr("&Connect"));
    disconnectAction->setText(tr("&Disconnect"));
    reloadSettingsAction->setText(tr("Re&load Settings from Device"));
    restoreDefaultsAction->setText(tr("&Restore Default Settings"));
    applySettingsAction->setText(tr("&Apply Settings"));
    helpMenu->setTitle(tr("&Help"));
    documentationAction->setText(tr("&Online Documentation..."));
    aboutAction->setText(tr("&About..."));

    deviceInfoBox->setTitle(tr("Device info"));
    deviceNameLabel->setText(tr("Name") + FIELD_LABEL_SUFFIX);
    serialNumberLabel->setText(tr("Serial number") + FIELD_LABEL_SUFFIX);
    firmwareVersionLabel->setText(tr("Firmware version") + FIELD_LABEL_SUFFIX);
    progPortLabel->setText(tr("Programming port") + FIELD_LABEL_SUFFIX);
    ttlPortLabel->setText(tr("TTL port") + FIELD_LABEL_SUFFIX);
    lastDeviceResetLabel->setText(tr("Last device reset") + FIELD_LABEL_SUFFIX);

    programmingResultsBox->setTitle(tr("Results from last programming"));
    measuredVccMinLabel->setText(tr("Target VCC measured minimum") + FIELD_LABEL_SUFFIX);
    measuredVccMaxLabel->setText(tr("Target VCC measured maximum") + FIELD_LABEL_SUFFIX);
    measuredVddMinLabel->setText(tr("Programmer VDD measured minimum") + FIELD_LABEL_SUFFIX);
    measuredVddMaxLabel->setText(tr("Programmer VDD measured maximum") + FIELD_LABEL_SUFFIX);

    currentStatusBox->setTitle(tr("Current status"));
    currentVccLabel->setText(tr("Target VCC") + FIELD_LABEL_SUFFIX);
    currentVddLabel->setText(tr("Programmer VDD") + FIELD_LABEL_SUFFIX);
    currentRegulatorLevelLabel->setText(tr("VDD regulator set point") + FIELD_LABEL_SUFFIX);

    // [all-settings]
    settingsBox->setTitle(tr("Settings"));
    ispFrequencyLabel->setText(tr("ISP Frequency") + FIELD_LABEL_SUFFIX);
    maxIspFrequencyLabel->setText(tr("Max ISP Frequency") + FIELD_LABEL_SUFFIX);
    regulatorModeLabel->setText(tr("Regulator mode") + FIELD_LABEL_SUFFIX);
    vccOutputEnabledLabel->setText(tr("VCC output") + FIELD_LABEL_SUFFIX);
    vccOutputIndicatorLabel->setText(tr("VCC output indicator") + FIELD_LABEL_SUFFIX);
    lineAFunctionLabel->setText(tr("Line A function") + FIELD_LABEL_SUFFIX);
    lineBFunctionLabel->setText(tr("Line B function") + FIELD_LABEL_SUFFIX);
    vccVddMaxRangeLabel->setText(tr("VCC/VDD maximum range") + FIELD_LABEL_SUFFIX);
    vcc3v3MinLabel->setText(tr("VCC 3.3 V minimum") + FIELD_LABEL_SUFFIX);
    vcc3v3MaxLabel->setText(tr("VCC 3.3 V maximum") + FIELD_LABEL_SUFFIX);
    vcc5vMinLabel->setText(tr("VCC 5 V minimum") + FIELD_LABEL_SUFFIX);
    vcc5vMaxLabel->setText(tr("VCC 5 V maximum") + FIELD_LABEL_SUFFIX);
    stk500HardwareVersionLabel->setText(tr("STK500 hardware version") + FIELD_LABEL_SUFFIX);
    stk500SoftwareVersionLabel->setText(tr("STK500 software version") + FIELD_LABEL_SUFFIX);

    applySettingsButton->setText(applySettingsAction->text());
}
