#pragma once

#include <QSpinBox>

// Customized spin box for selecting a voltage in millivolts.
// QSpinBox was not adequate because there was no way to set a step size.
class VoltageSpinBox : public QSpinBox
{
    virtual QValidator::State validate(QString & input, int & pos) const override;
    virtual void fixup(QString & input) const override;
private:
    int numericValue(const QString & input) const;
};
