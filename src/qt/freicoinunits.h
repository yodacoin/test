#ifndef testCOINUNITS_H
#define testCOINUNITS_H

#include "bignum.h" // for mpq

#include <QString>
#include <QAbstractListModel>

/** testcoin unit definitions. Encapsulates parsing and formatting
   and serves as list model for dropdown selection boxes.
*/
class testcoinUnits: public QAbstractListModel
{
public:
    explicit testcoinUnits(QObject *parent);

    /** testcoin units.
      @note Source: https://en.bitcoin.it/wiki/Units . Please add only sensible ones
     */
    enum Unit
    {
        FRC,
        mFRC,
        uFRC
    };

    //! @name Static API
    //! Unit conversion and formatting
    ///@{

    //! Get list of units, for drop-down box
    static QList<Unit> availableUnits();
    //! Is unit ID valid?
    static bool valid(int unit);
    //! Short name
    static QString name(int unit);
    //! Longer description
    static QString description(int unit);
    //! Number of Satoshis (1e-8) per unit
    static mpq factor(int unit);
    //! Number of amount digits (to represent max number of coins)
    static int amountDigits(int unit);
    //! Number of decimals left
    static int decimals(int unit);
    //! Format as string
    static QString format(int unit, const mpq& amount, bool plussign=false);
    //! Format as string (with unit)
    static QString formatWithUnit(int unit, const mpq& amount, bool plussign=false);
    //! Parse string to coin amount
    static bool parse(int unit, const QString &value, mpq *val_out);
    ///@}

    //! @name AbstractListModel implementation
    //! List model for unit drop-down selection box.
    ///@{
    enum RoleIndex {
        /** Unit identifier */
        UnitRole = Qt::UserRole
    };
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    ///@}
private:
    QList<testcoinUnits::Unit> unitlist;
};
typedef testcoinUnits::Unit testcoinUnit;

#endif // testCOINUNITS_H
