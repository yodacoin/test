#include "testcoinamountfield.h"
#include "qvaluecombobox.h"
#include "testcoinunits.h"

#include "guiconstants.h"
#include "main.h"
#include "util.h"

#include <QLabel>
#include <QLineEdit>
#include <QRegExpValidator>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QApplication>
#include <qmath.h>

testcoinAmountField::testcoinAmountField(QWidget *parent):
        QWidget(parent), amount(0), currentUnit(-1)
{
    amount = new QDoubleSpinBox(this);
    amount->setLocale(QLocale::c());
    amount->setDecimals(8);
    amount->installEventFilter(this);
    amount->setMaximumWidth(170);
    amount->setSingleStep(0.001);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(amount);
    unit = new QValueComboBox(this);
    unit->setModel(new testcoinUnits(this));
    layout->addWidget(unit);
    layout->addStretch(1);
    layout->setContentsMargins(0,0,0,0);

    setLayout(layout);

    setFocusPolicy(Qt::TabFocus);
    setFocusProxy(amount);

    // If one if the widgets changes, the combined content changes as well
    connect(amount, SIGNAL(valueChanged(QString)), this, SIGNAL(textChanged()));
    connect(unit, SIGNAL(currentIndexChanged(int)), this, SLOT(unitChanged(int)));

    // Set default based on configuration
    unitChanged(unit->currentIndex());
}

void testcoinAmountField::setText(const QString &text)
{
    if (text.isEmpty())
        amount->clear();
    else
        amount->setValue(text.toDouble());
}

void testcoinAmountField::clear()
{
    amount->clear();
    unit->setCurrentIndex(0);
}

bool testcoinAmountField::validate()
{
    mpq value = 0; bool valid = false;
    if (testcoinUnits::parse(currentUnit, text(), &value) && MoneyRange(value))
        valid = true;

    setValid(valid);

    return valid;
}

void testcoinAmountField::setValid(bool valid)
{
    if (valid)
        amount->setStyleSheet("");
    else
        amount->setStyleSheet(STYLE_INVALID);
}

QString testcoinAmountField::text() const
{
    if (amount->text().isEmpty())
        return QString();
    else
        return amount->text();
}

bool testcoinAmountField::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::FocusIn)
    {
        // Clear invalid flag on focus
        setValid(true);
    }
    else if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Comma)
        {
            // Translate a comma into a period
            QKeyEvent periodKeyEvent(event->type(), Qt::Key_Period, keyEvent->modifiers(), ".", keyEvent->isAutoRepeat(), keyEvent->count());
            qApp->sendEvent(object, &periodKeyEvent);
            return true;
        }
    }
    return QWidget::eventFilter(object, event);
}

QWidget *testcoinAmountField::setupTabChain(QWidget *prev)
{
    QWidget::setTabOrder(prev, amount);
    return amount;
}

qint64 testcoinAmountField::value(bool *valid_out) const
{
    mpq qValue = valueAsMpq(valid_out);
    mpz zValue = qValue.get_num() / qValue.get_den();
    return mpz_to_i64(zValue);
}

void testcoinAmountField::setValue(qint64 value)
{
    setValue(i64_to_mpq(value));
}

mpq testcoinAmountField::valueAsMpq(bool *valid_out) const
{
    mpq val_out = 0;
    bool valid = testcoinUnits::parse(currentUnit, text(), &val_out);
    if(valid_out)
    {
        *valid_out = valid;
    }
    return val_out;
}

void testcoinAmountField::setValue(const mpq& value)
{
    setText(testcoinUnits::format(currentUnit, RoundAbsolute(value, ROUND_TOWARDS_ZERO)));
}

void testcoinAmountField::unitChanged(int idx)
{
    // Use description tooltip for current unit for the combobox
    unit->setToolTip(unit->itemData(idx, Qt::ToolTipRole).toString());

    // Determine new unit ID
    int newUnit = unit->itemData(idx, testcoinUnits::UnitRole).toInt();

    // Parse current value and convert to new unit
    bool valid = false;
    mpq currentValue = valueAsMpq(&valid);

    currentUnit = newUnit;

    // Set max length after retrieving the value, to prevent truncation
    amount->setDecimals(testcoinUnits::decimals(currentUnit));
    amount->setMaximum(qPow(10, testcoinUnits::amountDigits(currentUnit)) - qPow(10, -amount->decimals()));

    if(testcoinUnits::decimals(currentUnit)<3)
        amount->setSingleStep(0.01);
    else
        amount->setSingleStep(0.001);

    if(valid)
    {
        // If value was valid, re-place it in the widget with the new unit
        setValue(currentValue);
    }
    else
    {
        // If current value is invalid, just clear field
        setText("");
    }
    setValid(true);
}

void testcoinAmountField::setDisplayUnit(int newUnit)
{
    unit->setValue(newUnit);
}
