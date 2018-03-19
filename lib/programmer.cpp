#include <cassert>
#include <thread>
#include <chrono>

#include <programmer.h>
#include <pavrpgm_config.h>

// A setup packet bRequest value from USB 2.0 Table 9-4
#define USB_REQUEST_GET_DESCRIPTOR 6

// A descriptor type from USB 2.0 Table 9-5
#define USB_DESCRIPTOR_TYPE_STRING 3

// pgm04a uses voltage units of 32 mV, so the maximum representable voltage
// is 8160 mV.
static const uint32_t maxRepresentableVoltage = 255 * PAVR2_VOLTAGE_UNITS;
#if PAVR2_VOLTAGE_UNITS == 32
#define MAX_REPRESENTABLE_VOLTAGE_STR "8160 mV"
#endif

// Searches for a frequency with the given name inside a vector of frequencies
// and returns its index.  Returns -1 if it is not found.
static int32_t frequencyFindByName(
    const std::vector<ProgrammerFrequency> & table,
    int32_t start, int32_t end,
    std::string name)
{
    for(int32_t i = start; i <= end; i++)
    {
        if (table[i].name == name)
        {
            return i;
        }
    }
    return -1;
}

// Look up a frequency in the list of options that are a user can select when
// they are setting the "Max ISP Frequency" setting.
static int32_t allowedMaxFrequencyFind(std::string name)
{
    return frequencyFindByName(programmerAllowedMaxFrequencyTable,
        0, programmerAllowedMaxFrequencyTable.size() - 1, name);
}

static int32_t stk500FrequencyFind(std::string name)
{
    return frequencyFindByName(programmerStk500FrequencyTable, 0, 255, name);
}

std::string Programmer::getMaxFrequencyName(uint32_t ispFastestPeriod)
{
    if (ispFastestPeriod <= 255)
    {
        return programmerFullMaxFrequencyTable[ispFastestPeriod].name;
    }
    else
    {
        throw std::runtime_error("The period for the maximum ISP frequency must be 255 or less.");
    }
}

void Programmer::setMaxFrequency(ProgrammerSettings & settings,
    std::string maxFrequencyName)
{
    int32_t index = allowedMaxFrequencyFind(maxFrequencyName);

    if (index < 0)
    {
        throw std::runtime_error(
            std::string("Invalid maximum frequency name: '") +
            maxFrequencyName + "'.");
    }

    settings.ispFastestPeriod = programmerAllowedMaxFrequencyTable[index].period;
}

std::string Programmer::getFrequencyName(uint32_t sckDuration,
    uint32_t ispFastestPeriod)
{
    if (sckDuration == 0)
    {
        // When SCK_DURATION is 0, that means that the frequency is controlled
        // by the ISP_FASTEST_PERIOD setting.
        return getMaxFrequencyName(ispFastestPeriod);
    }
    else if (sckDuration <= 255)
    {
        return programmerStk500FrequencyTable[sckDuration].name;
    }
    else
    {
        throw std::runtime_error("SCK duration must be 255 or less.");
    }
}

void Programmer::setFrequency(ProgrammerSettings & settings, std::string frequencyName)
{
    int32_t index = stk500FrequencyFind(frequencyName);
    if (index > 0)
    {
        // The comparison above is "> 0" on purpose.  If index == 0, it means that
        // we that we found the first element, which is really just a dummy element.
        // Setting SCK_DURATION to 0 does not use mean that element will be used
        // (see below).

        // This is a frequency we can achieve by just setting the SCK_DURATION
        // parameter, so do that.
        settings.sckDuration = index;
    }
    else
    {
        index = allowedMaxFrequencyFind(frequencyName);

        if (index >= 0)
        {
            // This is a frequency we can achieve by setting SCK_DURATION to 0,
            // which means to use ISP_FASTEST_PERIOD, and then setting the
            // ISP_FASTEST_PERIOD.
            settings.sckDuration = 0;
            settings.ispFastestPeriod = programmerAllowedMaxFrequencyTable[index].period;
        }
        else
        {
            throw std::runtime_error(
                std::string("Invalid frequency name: '") + frequencyName + "'.");
        }
    }
}

std::string Programmer::convertProgrammingErrorToShortString(uint8_t programmingError)
{
    switch (programmingError)
    {
    case 0:
        return "No error.";

    case PAVR2_PROGRAMMING_ERROR_TARGET_POWER_BAD:
        return "Target power error.";

    case PAVR2_PROGRAMMING_ERROR_SYNCH:
        return "Initial SPI command failed.";

    case PAVR2_PROGRAMMING_ERROR_IDLE_FOR_TOO_LONG:
        return "Idle error.";

    case PAVR2_PROGRAMMING_ERROR_USB_NOT_CONFIGURED:
        return "USB not configured.";

    case PAVR2_PROGRAMMING_ERROR_USB_SUSPEND:
        return "USB suspended.";

    case PAVR2_PROGRAMMING_ERROR_PROGRAMMER_POWER_BAD:
        return "Programmer power error.";

    default:
        return std::string("Unknown code ") + std::to_string(programmingError) + ".";
    }
}

std::string Programmer::convertProgrammingErrorToLongString(uint8_t programmingError)
{
    switch (programmingError)
    {
    case 0:
        return "";

    case PAVR2_PROGRAMMING_ERROR_TARGET_POWER_BAD:
        return "Target VCC went outside of the allowed range, "
            "so programming was aborted.  "
            "Make sure that the target is powered on and its batteries "
            "are not too low (if applicable).";

    case PAVR2_PROGRAMMING_ERROR_SYNCH:
        return "The SPI command for entering programming mode was sent, "
            "but the expected response from the target was not received.  "
            "Make sure that the ISP frequency is less than one sixth "
            "of the target's clock frequency.";

    case PAVR2_PROGRAMMING_ERROR_IDLE_FOR_TOO_LONG:
        return "The programmer received no programming commands from the "
            "computer for a time longer than the timeout period, "
            "so programming was aborted.";

    case PAVR2_PROGRAMMING_ERROR_USB_NOT_CONFIGURED:
        return "The computer's USB controller deconfigured the programmer, "
            "so programming was aborted.";

    case PAVR2_PROGRAMMING_ERROR_USB_SUSPEND:
        return "The computer's USB controller put the programmer into suspend mode, "
            "so programming was aborted.";

    case PAVR2_PROGRAMMING_ERROR_PROGRAMMER_POWER_BAD:
        return "The programmer's VDD either went too low or had too much range, "
            "so programming was aborted.";

    default:
        return "";
    }
}

std::string Programmer::convertDeviceResetToString(uint8_t deviceReset)
{
    switch(deviceReset)
    {
    case PAVR2_RESET_POWER_UP:
        return "Power-on reset";

    case PAVR2_RESET_BROWNOUT:
        return "Brown-out reset";

    case PAVR2_RESET_RESET_LINE:
        return "Reset pin driven low";

    case PAVR2_RESET_WATCHDOG:
        return "Watchdog reset";

    case PAVR2_RESET_SOFTWARE:
        return "Software reset (bootloader)";

    case PAVR2_RESET_STACK_OVERFLOW:
        return "Stack overflow";

    case PAVR2_RESET_STACK_UNDERFLOW:
        return "Stack underflow";

    default:
        return std::string("Unknown code ") + std::to_string(deviceReset) + ".";
    }
}

std::string Programmer::convertRegulatorModeToString(uint8_t regulatorMode)
{
    switch (regulatorMode)
    {
    case PAVR2_REGULATOR_MODE_3V3:  return "3.3 V";
    case PAVR2_REGULATOR_MODE_5V:   return "5 V";
    default: return "auto";
    }
}

std::string Programmer::convertRegulatorLevelToString(uint8_t regulatorLevel)
{
    // The levels are a subset of the modes.
    return convertRegulatorModeToString(regulatorLevel);
}

std::string Programmer::convertLineFunctionToString(uint8_t lineFunction)
{
    switch (lineFunction)
    {
    case PAVR2_LINE_IS_DTR: return "DTR";
    case PAVR2_LINE_IS_RTS: return "RTS";
    case PAVR2_LINE_IS_CD: return "CD";
    case PAVR2_LINE_IS_DSR: return "DSR";
    case PAVR2_LINE_IS_CLOCK: return "Clock";
    case PAVR2_LINE_IS_DTR_RESET: return "DTR reset";
    default: return "None";
    }
}

ProgrammerInstance::ProgrammerInstance()
{
}

ProgrammerInstance::ProgrammerInstance(
    libusbp::device usbDevice,
    libusbp::generic_interface usbInterface,
    uint16_t productId,
    std::string serialNumber,
    uint16_t firmwareVersion)
    : usbDevice(usbDevice), usbInterface(usbInterface), productId(productId),
      serialNumber(serialNumber), firmwareVersion(firmwareVersion)
{
}

ProgrammerInstance::operator bool() const
{
    return usbInterface;
}

std::string ProgrammerInstance::getName() const
{
    if (productId == PAVR2_USB_PRODUCT_ID_V2)
    {
        return "Pololu USB AVR Programmer v2";
    }
    else if (productId == PAVR2_USB_PRODUCT_ID_V2_1)
    {
        return "Pololu USB AVR Programmer v2.1";
    }
    else
    {
        // Should not happen.
        return "Pololu USB AVR Programmer v2.x?";
    }
}

std::string ProgrammerInstance::getOsId() const
{
    return usbDevice.get_os_id();
}

std::string ProgrammerInstance::getSerialNumber() const
{
    return serialNumber;
}

uint16_t ProgrammerInstance::getFirmwareVersion() const
{
    return firmwareVersion;
}

static uint8_t bcdToDecimal(uint8_t bcd)
{
    return (bcd & 0xF) + 10 * (bcd >> 4);
}

std::string ProgrammerInstance::getFirmwareVersionString() const
{
    char buffer[6];
    snprintf(buffer, sizeof(buffer), "%d.%02d",
        getFirmwareVersionMajor(), getFirmwareVersionMinor());
    return std::string(buffer);
}

uint8_t ProgrammerInstance::getFirmwareVersionMajor() const
{
    return bcdToDecimal(firmwareVersion >> 8);
}

uint8_t ProgrammerInstance::getFirmwareVersionMinor() const
{
    return bcdToDecimal(firmwareVersion & 0xFF);
}

std::string ProgrammerInstance::getProgrammingPortName() const
{
    libusbp::serial_port port(usbDevice, 1, true);
    return port.get_name();
}

std::string ProgrammerInstance::getTtlPortName() const
{
    libusbp::serial_port port(usbDevice, 3, true);
    return port.get_name();
}

std::string ProgrammerInstance::tryGetProgrammingPortName() const
{
    try
    {
        return getProgrammingPortName();
    }
    catch (const libusbp::error & exception)
    {
        return "(unknown)";
    }
}

std::string ProgrammerInstance::tryGetTtlPortName() const
{
    try
    {
        return getTtlPortName();
    }
    catch (const libusbp::error & exception)
    {
        return "(unknown)";
    }
}

std::vector<ProgrammerInstance> programmerGetList()
{
    std::vector<ProgrammerInstance> list;
    for (const libusbp::device & device : libusbp::list_connected_devices())
    {
        if (device.get_vendor_id() != PAVR2_USB_VENDOR_ID) { continue; }

        uint16_t productId = device.get_product_id();

        bool isProgrammer =
            productId == PAVR2_USB_PRODUCT_ID_V2 ||
            productId == PAVR2_USB_PRODUCT_ID_V2_1;

        if (!isProgrammer) { continue; }

        libusbp::generic_interface usbInterface;
        try
        {
            uint8_t interfaceNumber = 0;
            bool composite = true;
            usbInterface = libusbp::generic_interface(device, interfaceNumber, composite);
        }
        catch(const libusbp::error & error)
        {
            if (error.has_code(LIBUSBP_ERROR_NOT_READY))
            {
                // An error occurred that is normal if the interface is simply
                // not ready to use yet.  Silently ignore it.
                continue;
            }
            throw;
        }
        ProgrammerInstance instance(device, usbInterface, productId,
            device.get_serial_number(), device.get_revision());
        list.push_back(instance);
    }
    return list;
}

ProgrammerHandle::ProgrammerHandle()
{
}

ProgrammerHandle::ProgrammerHandle(ProgrammerInstance instance)
{
    assert(instance);

    if (instance.getFirmwareVersionMajor() > PAVR2_FIRMWARE_VERSION_MAJOR_MAX)
    {
        throw std::runtime_error(
            "The device has new firmware that is not supported by this software.  "
            "Try using the latest version of this software from " DOCUMENTATION_URL);
    }

    this->instance = instance;
    handle = libusbp::generic_handle(instance.usbInterface);

    // Set a timeout for all control transfers to prevent the CLI from hanging
    // indefinitely if something goes wrong with the USB communication.
    handle.set_timeout(0, 300);
}

void ProgrammerHandle::close()
{
    handle.close();
    instance = ProgrammerInstance();
}

const ProgrammerInstance & ProgrammerHandle::getInstance() const
{
    return instance;
}

uint8_t ProgrammerHandle::getRawSetting(uint8_t id)
{
    uint8_t value;
    size_t transferred;
    try
    {
        handle.control_transfer(0xC0, PAVR2_REQUEST_GET_SETTING,
            0, id, &value, 1, &transferred);
    }
    catch (const libusbp::error & error)
    {
        throw std::runtime_error(std::string("Failed to read a setting.  ")
            + error.message());
    }

    if (transferred != 1)
    {
        throw std::runtime_error(std::string("Failed to read a setting.  ") +
            "Expected 1 byte, got " + std::to_string(transferred));
    }

    return value;
}

void ProgrammerHandle::setRawSetting(uint8_t id, uint8_t value)
{
    try
    {
        handle.control_transfer(0x40, PAVR2_REQUEST_SET_SETTING, value, id);
    }
    catch(const libusbp::error & error)
    {
        throw std::runtime_error(std::string("Failed to set a setting.  ") +
            error.message());
    }
}

uint8_t ProgrammerHandle::getRawVariable(uint8_t id)
{
    uint8_t value;
    size_t transferred;
    try
    {
        handle.control_transfer(0xC0, PAVR2_REQUEST_GET_VARIABLE,
            0, id, &value, 1, &transferred);
    }
    catch (const libusbp::error & error)
    {
        throw std::runtime_error(std::string("Failed to get a variable.  ")
            + error.message());
    }

    if (transferred != 1)
    {
        throw std::runtime_error(std::string("Failed to get a variable.  ") +
            "Expected 1 byte, got " + std::to_string(transferred));
    }

    return value;
}

std::string ProgrammerHandle::getFirmwareVersionString()
{
    if (cachedFirmwareVersion.size() > 0)
    {
        return cachedFirmwareVersion;
    }

    std::string version = instance.getFirmwareVersionString();

    // Get the firmware modification string.
    const uint8_t stringIndex = 6;
    size_t transferred = 0;
    uint8_t buffer[64];
    try
    {
        handle.control_transfer(0x80, USB_REQUEST_GET_DESCRIPTOR,
            (USB_DESCRIPTOR_TYPE_STRING << 8) | stringIndex,
            0, buffer, sizeof(buffer), &transferred);
    }
    catch (const libusbp::error & error)
    {
        // Let's make this be a non-fatal error because it's not so important.
        // Just add a question mark so we can tell if something is wrong.
        version += "?";

        // Uncomment this line to debug the error:
        // throw std::runtime_error(std::string("Failed to get firmware modification string."));
    }

    // Add the modification string to the firmware version string, assuming it
    // is ASCII.
    std::string mod;
    for (size_t i = 2; i < transferred; i += 2)
    {
        mod += buffer[i];
    }
    if (mod != "-") { version += mod; }

    cachedFirmwareVersion = version;
    return version;
}

// [all-settings]
ProgrammerSettings ProgrammerHandle::getSettings()
{
    ProgrammerSettings settings;
    settings.sckDuration = getRawSetting(PAVR2_SETTING_SCK_DURATION);
    settings.ispFastestPeriod = getRawSetting(PAVR2_SETTING_ISP_FASTEST_PERIOD);
    settings.regulatorMode = getRawSetting(PAVR2_SETTING_REGULATOR_MODE);
    settings.vccOutputEnabled = getRawSetting(PAVR2_SETTING_VCC_OUTPUT_ENABLED) ? 1 : 0;
    settings.vccOutputIndicator = getRawSetting(PAVR2_SETTING_VCC_OUTPUT_INDICATOR) ? 1 : 0;
    settings.lineAFunction = getRawSetting(PAVR2_SETTING_LINE_A_FUNCTION);
    settings.lineBFunction = getRawSetting(PAVR2_SETTING_LINE_B_FUNCTION);
    settings.softwareVersionMajor = getRawSetting(PAVR2_SETTING_SOFTWARE_VERSION_MAJOR);
    settings.softwareVersionMinor = getRawSetting(PAVR2_SETTING_SOFTWARE_VERSION_MINOR);
    settings.hardwareVersion = getRawSetting(PAVR2_SETTING_HARDWARE_VERSION);
    settings.vccVddMaxRange = getRawSetting(PAVR2_SETTING_VCC_VDD_MAX_RANGE)
        * PAVR2_VOLTAGE_UNITS;
    settings.vcc3v3Min = getRawSetting(PAVR2_SETTING_VCC_3V3_MIN) * PAVR2_VOLTAGE_UNITS;
    settings.vcc3v3Max = getRawSetting(PAVR2_SETTING_VCC_3V3_MAX) * PAVR2_VOLTAGE_UNITS;
    settings.vcc5vMin = getRawSetting(PAVR2_SETTING_VCC_5V_MIN) * PAVR2_VOLTAGE_UNITS;
    settings.vcc5vMax = getRawSetting(PAVR2_SETTING_VCC_5V_MAX) * PAVR2_VOLTAGE_UNITS;

    // We don't read the reset polarity here because that gets set by programming
    // software before each session; it is not really a persistent setting
    // and it might be confusing to present it that way.

    return settings;
}

// [all-settings]
void ProgrammerHandle::validateSettings(const ProgrammerSettings & settings)
{
    if (settings.sckDuration > 255)
    {
        throw std::runtime_error("The SCK duration should be at most 255.");
    }

    if (settings.ispFastestPeriod < PAVR2_ISP_FASTEST_PERIOD_MIN
        || settings.ispFastestPeriod > PAVR2_ISP_FASTEST_PERIOD_MAX)
    {
        throw std::runtime_error("The ISP fastest period is not valid.");
    }

    if (settings.regulatorMode > PAVR2_REGULATOR_MODE_5V)
    {
        throw std::runtime_error("Invalid regulator mode.");
    }

    if (settings.regulatorMode == PAVR2_REGULATOR_MODE_AUTO &&
        settings.vccOutputEnabled)
    {
        throw std::runtime_error("VCC cannot be an output if the regulator mode is auto.");
    }

    if (settings.lineAFunction > PAVR2_LINE_IS_DTR_RESET)
    {
        throw std::runtime_error("Invalid line A function.");
    }

    if (settings.lineAFunction == PAVR2_LINE_IS_CLOCK)
    {
        throw std::runtime_error("Line A cannot be a clock output.");
    }

    if (settings.lineBFunction > PAVR2_LINE_IS_DTR_RESET)
    {
        throw std::runtime_error("Invalid line B function.");
    }

    if (settings.softwareVersionMajor > 255)
    {
        throw std::runtime_error("Invalid software major version.");
    }

    if (settings.softwareVersionMinor > 255)
    {
        throw std::runtime_error("Invalid software minor version.");
    }

    if (settings.hardwareVersion > 255)
    {
        throw std::runtime_error("Invalid hardware version.");
    }

    if (settings.vccVddMaxRange > maxRepresentableVoltage)
    {
        throw std::runtime_error(
            "The VCC/VDD maximum range cannot be larger than "
            MAX_REPRESENTABLE_VOLTAGE_STR);
    }

    if (settings.vcc3v3Max > maxRepresentableVoltage)
    {
        throw std::runtime_error(
            "The VCC 3.3 V maximum cannot be larger than "
            MAX_REPRESENTABLE_VOLTAGE_STR);
    }

    if (settings.vcc5vMax > maxRepresentableVoltage)
    {
        throw std::runtime_error(
            "The VCC 5 V maximum cannot be larger than "
            MAX_REPRESENTABLE_VOLTAGE_STR);
    }

    if (settings.vcc3v3Min > settings.vcc3v3Max)
    {
        throw std::runtime_error(
            "The VCC 3.3 V minimum cannot be greater than the maximum.");
    }

    if (settings.vcc5vMin > settings.vcc5vMax)
    {
        throw std::runtime_error(
            "The VCC 5 V minimum cannot be greater than the maximum.");
    }
}

// Divide by PAVR2_VOLTAGE_UNITs, rounding to the nearest integer.
static uint8_t convertMvToRawUnits(uint32_t mv)
{
    uint32_t x = (mv + PAVR2_VOLTAGE_UNITS / 2 - 1) / PAVR2_VOLTAGE_UNITS;
    if (x > 0xFF)
    {
        x = 0xFF;

        // Bounds are checked before we get to this point,
        // so this block should never run.
        assert(0);
    }
    return x;
}

// [all-settings]
void ProgrammerHandle::applySettings(const ProgrammerSettings & settings)
{
    validateSettings(settings);

    // We set vccOutputEnabled in a special way to ensure that if it is changing,
    // we won't accidentally output the wrong voltage on VCC for some time.
    if (!settings.vccOutputEnabled)
    {
        setRawSetting(PAVR2_SETTING_VCC_OUTPUT_ENABLED, 0);
    }

    setRawSetting(PAVR2_SETTING_SCK_DURATION, settings.sckDuration);
    setRawSetting(PAVR2_SETTING_ISP_FASTEST_PERIOD, settings.ispFastestPeriod);
    setRawSetting(PAVR2_SETTING_REGULATOR_MODE, settings.regulatorMode);
    setRawSetting(PAVR2_SETTING_VCC_OUTPUT_INDICATOR, settings.vccOutputIndicator);
    setRawSetting(PAVR2_SETTING_LINE_A_FUNCTION, settings.lineAFunction);
    setRawSetting(PAVR2_SETTING_LINE_B_FUNCTION, settings.lineBFunction);
    setRawSetting(PAVR2_SETTING_SOFTWARE_VERSION_MAJOR, settings.softwareVersionMajor);
    setRawSetting(PAVR2_SETTING_SOFTWARE_VERSION_MINOR, settings.softwareVersionMinor);
    setRawSetting(PAVR2_SETTING_HARDWARE_VERSION, settings.hardwareVersion);
    setRawSetting(PAVR2_SETTING_VCC_VDD_MAX_RANGE,
        convertMvToRawUnits(settings.vccVddMaxRange));
    setRawSetting(PAVR2_SETTING_VCC_3V3_MIN,
        convertMvToRawUnits(settings.vcc3v3Min));
    setRawSetting(PAVR2_SETTING_VCC_3V3_MAX,
        convertMvToRawUnits(settings.vcc3v3Max));
    setRawSetting(PAVR2_SETTING_VCC_5V_MIN,
        convertMvToRawUnits(settings.vcc5vMin));
    setRawSetting(PAVR2_SETTING_VCC_5V_MAX,
        convertMvToRawUnits(settings.vcc5vMax));

    if (settings.vccOutputEnabled)
    {
        setRawSetting(PAVR2_SETTING_VCC_OUTPUT_ENABLED, 1);
    }
}

void ProgrammerHandle::restoreDefaults()
{
    setRawSetting(PAVR2_SETTING_NOT_INITIALIZED, 0xFF);

    // The request above returns before the settings are actually initialized.
    // Wait until the programmer succeeds in reinitializing its settings.
    uint32_t timeMs = 0;
    while (1)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        timeMs += 10;

        uint8_t notInitialized = getRawSetting(PAVR2_SETTING_NOT_INITIALIZED);
        if (!notInitialized)
        {
            break;
        }

        if (timeMs > 300)
        {
            throw std::runtime_error(
                "A timeout occurred while resetting to default settings.");
        }
    }
}

ProgrammerVariables ProgrammerHandle::getVariables()
{
    ProgrammerVariables vars;

    vars.lastDeviceReset = getRawVariable(PAVR2_VARIABLE_LAST_DEVICE_RESET);

    vars.programmingError = getRawVariable(PAVR2_VARIABLE_PROGRAMMING_ERROR);

    vars.targetVccMeasuredMinMv =
        getRawVariable(PAVR2_VARIABLE_TARGET_VCC_MEASURED_MIN) * PAVR2_VOLTAGE_UNITS;

    vars.targetVccMeasuredMaxMv =
        getRawVariable(PAVR2_VARIABLE_TARGET_VCC_MEASURED_MAX) * PAVR2_VOLTAGE_UNITS;

    vars.programmerVddMeasuredMinMv =
        getRawVariable(PAVR2_VARIABLE_PROGRAMMER_VDD_MEASURED_MIN) * PAVR2_VOLTAGE_UNITS;

    vars.programmerVddMeasuredMaxMv =
        getRawVariable(PAVR2_VARIABLE_PROGRAMMER_VDD_MEASURED_MAX) * PAVR2_VOLTAGE_UNITS;

    vars.hasResultsFromLastProgramming =
        (vars.programmingError != 0) ||
        (vars.targetVccMeasuredMinMv != 255 * PAVR2_VOLTAGE_UNITS) ||
        (vars.targetVccMeasuredMaxMv != 0) ||
        (vars.programmerVddMeasuredMinMv != 255 * PAVR2_VOLTAGE_UNITS) ||
        (vars.programmerVddMeasuredMaxMv != 0);

    vars.targetVccMv =
        getRawVariable(PAVR2_VARIABLE_TARGET_VCC) * PAVR2_VOLTAGE_UNITS;

    vars.programmerVddMv =
        getRawVariable(PAVR2_VARIABLE_PROGRAMMER_VDD) * PAVR2_VOLTAGE_UNITS;

    vars.regulatorLevel = getRawVariable(PAVR2_VARIABLE_REGULATOR_LEVEL);

    vars.inProgrammingMode = getRawVariable(PAVR2_VARIABLE_IN_PROGRAMMING_MODE) ? 1 : 0;

    return vars;
}

ProgrammerDigitalReadings ProgrammerHandle::digitalRead()
{
    uint8_t buffer[3];
    size_t transferred;
    try
    {
        handle.control_transfer(0xC0, PAVR2_REQUEST_DIGITAL_READ,
            0, 0, &buffer, 3, &transferred);
    }
    catch (const libusbp::error & error)
    {
        throw std::runtime_error(std::string("Failed to get a variable.  ")
            + error.message());
    }

    if (transferred != 3)
    {
        throw std::runtime_error(std::string("Failed to get a variable.  ") +
            "Expected 3 bytes, got " + std::to_string(transferred));
    }

    ProgrammerDigitalReadings readings;
    readings.portA = buffer[0];
    readings.portB = buffer[1];
    readings.portC = buffer[2];
    return readings;
}

bool pgm03aPresent()
{
    for (const libusbp::device & device : libusbp::list_connected_devices())
    {
        bool isPgm03a = device.get_vendor_id() == 0x1FFB &&
            device.get_product_id() == 0x0081;
        if (isPgm03a) { return true; }
    }
    return false;
}


/** This message doesn't look great when printed in a CLI because it has lines
 * longer than 80 characters.  But if we add wrapping to it then it will look
 * bad when it is printed in a messagebox in a typical GUI. **/
const char * pgm03aMessage =
    "This utility only supports the Pololu USB AVR Programmer v2 and v2.1 "
    "(blue-colored, labeled \"pgm04a\" or \"pgm04b\").\n"
    "\n"
    "It looks you have an older programmer, the Pololu USB AVR Programmer (pgm03a).  "
    "You can find documentation and software for the older programmer here:\n"
    "\n"
    "https://www.pololu.com/docs/0J36";
