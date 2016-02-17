#include "frequency_validator.h"

void FrequencyValidator::setAllowedFrequencies(
    const std::vector<ProgrammerFrequency> & allowedFrequencies)
{
    this->allowedFrequencies = allowedFrequencies;
}

void FrequencyValidator::setDefaultFrequency(
    const ProgrammerFrequency & frequency
)
{
    this->defaultFrequency = frequency;
}

QValidator::State FrequencyValidator::validate(QString & input, int & pos) const
{
    Q_UNUSED(pos);

    // If the frequency is on the list, return Acceptable.
    for (const ProgrammerFrequency & freq : allowedFrequencies)
    {
        if (input == QString(freq.name) + " kHz")
        {
            return Acceptable;
        }
    }

    // If the input contains invalid characters, return Invalid.
    std::string inputStdString = input.toStdString();
    for (size_t i = 0; i < inputStdString.size(); i++)
    {
        char c = inputStdString[i];
        if (!((c >= '0' && c <= '9') || c == '.' || c == ' '
                || c == 'k' || c == 'K'
                || c == 'm' || c == 'M'
                || c == 'h' || c == 'H'
                || c == 'z' || c == 'z'))
        {
            return Invalid;
        }
    }

    return Intermediate;
}

/** Converts a suffix the user typed to either " kHz" or " MHz", with " kHz"
 * being the default if we can't figure out what the user meant. */
static std::string normalize_suffix(const std::string & originalSuffix)
{
    std::string suffix;

    // Convert to lower-case and remove all spaces.
    for (size_t i = 0; i < originalSuffix.size(); i++)
    {
        char c = originalSuffix[i];
        if (std::isspace(c)) { continue; }
        suffix += std::tolower(c);
    }

    // If it's clear that the user means MHz, return that.
    if (suffix == "mhz" || suffix == "mh" || suffix == "m" || suffix == "mz")
    {
        return " MHz";
    }

    // Otherwise, we default to kHz.
    return " kHz";
}

void FrequencyValidator::fixup(QString & input) const
{
    const std::string inputStdString = input.toStdString();

    // Attempt to parse the beginning of the string as a number.
    bool ok = false;
    double value = 0;
    size_t suffixIndex = 0;
    try
    {
        value = std::stod(inputStdString, &suffixIndex);
        ok = true;
    }
    catch (const std::invalid_argument & e)
    {
        // It doesn't look the user even gave us a number.
    }
    catch (const std::out_of_range & e)
    {
        // The user gave a number that is out of range.
    }
    if (!ok)
    {
        // We don't have a numeric value, so just revert to the default.
        input = QString(defaultFrequency.name) + " kHz";
        return;
    }

    // Extract the numeric part of the string.
    std::string valueStr = inputStdString.substr(0, suffixIndex);

    // Extract the part after the number; this is our suffix.
    std::string suffix = inputStdString.substr(
        suffixIndex, inputStdString.size() - suffixIndex);

    // Normalize the suffix to " kHz" or " MHz".
    suffix = normalize_suffix(suffix);

    // Normalize the floating-point value so it is in units of kHz.
    if (suffix == " MHz")
    {
        value *= 1000;
    }

    if (suffix == " kHz")
    {
        // If the suffix is kHz, we try to find an exact match to one of the
        // frequencies.  This is important because it allows "57.4" to be
        // auto-corrected to "57.4 kHz" and instead of going down to 52.6 kHz.

        for (const ProgrammerFrequency & freq : allowedFrequencies)
        {
            if (valueStr == freq.name)
            {
                // Found an exact match.
                input = QString(freq.name) + " kHz";
                return;
            }
        }
    }

    // Convert the frequency to a period in the native units of the programmer
    // (one twelfth microseconds).
    double period_approximation = 12000 / value;

    // Find the first frequency that is lower than this.
    for (const ProgrammerFrequency & freq : allowedFrequencies)
    {
        if (freq.period >= period_approximation - 0.00001)
        {
            input = QString(freq.name) + " kHz";
            return;
        }
    }

    // Nothing we have is low enough; just use the lowest frequency.
    const ProgrammerFrequency & lowestFrequency = allowedFrequencies.back();
    input = QString(lowestFrequency.name) + " kHz";
}
