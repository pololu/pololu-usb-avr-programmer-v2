#pragma once

#include <QValidator>
#include <vector>
#include <string>
#include <programmer.h>

/** This class is used on our ISP Frequency and Max ISP Frequency boxes to help
 * correct the user's input so it matches one of the allowed input values.
 *
 * You must call setAllowedFrequencies with a sorted (descending) list of
 * frequencies before the other methods will do anything useful.
 *
 * This class assumes that we want to have the suffix " kHz" after the
 * frequency. */
class FrequencyValidator : public QValidator
{
public:
    virtual State validate(QString & input, int & pos) const override;
    virtual void fixup(QString & input) const override;

    /** Note: For the purposes of fixup(), the table provided to this function
     * must be sorted by the frequency in kHz, descending. */
    void setAllowedFrequencies(const std::vector<ProgrammerFrequency> &);

    void setDefaultFrequency(const ProgrammerFrequency &);
private:
    std::vector<ProgrammerFrequency> allowedFrequencies;
    ProgrammerFrequency defaultFrequency;
};
