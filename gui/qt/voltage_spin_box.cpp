#include "voltage_spin_box.h"
#include "pgm04a_native_usb_protocol.h"
#include <cassert>

int VoltageSpinBox::numericValue(const QString & input) const
{
    assert(prefix().isEmpty());
    assert(input.endsWith(suffix()));   // the input should end with " mV"
    QString numericInput = input.mid(0, input.size() - suffix().size());
    bool ok;
    int num = numericInput.toInt(&ok);
    assert(ok);
    return num;
}

/** input is the current value of the input, e.g. "123 mV"
 * pos is the position of the user's cursor, between 0 and input.size().
 */
QValidator::State VoltageSpinBox::validate(QString & input, int & pos) const
{
    QValidator::State result = QSpinBox::validate(input, pos);
    if (result == QValidator::Acceptable)
    {
        // Our parent class thinks this input is acceptable.  But if it is not
        // a multiple of 32, we want to consider it an intermediate value
        // instead.
        if ((numericValue(input) % PGM04A_VOLTAGE_UNITS) != 0)
        {
            result = QValidator::Intermediate;
        }
    }
    return result;
}

void VoltageSpinBox::fixup(QString & input) const
{
    QSpinBox::fixup(input);

    // We don't have to worry about totally invalid inputs here;
    // QSpinBox::validate fixes them.

    int num = numericValue(input);
    if ((num % PGM04A_VOLTAGE_UNITS) != 0)
    {
        int fixedNum = (num + (PGM04A_VOLTAGE_UNITS / 2))
            / PGM04A_VOLTAGE_UNITS * PGM04A_VOLTAGE_UNITS;
        input = QString::number(fixedNum);
    }
}
