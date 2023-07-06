// Copyright (c) 2011-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_SENDMPDIALOG_H
#define BITCOIN_QT_SENDMPDIALOG_H

#include <qt/walletmodel.h>

#include <QDialog>
#include <QString>

class ClientModel;

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

namespace Ui {
    class SendMPDialog;
}

/** Dialog for sending Master Protocol tokens */
class SendMPDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SendMPDialog(const PlatformStyle *platformStyle, QWidget *parent = 0);
    ~SendMPDialog();

    void setClientModel(ClientModel *model);
    void setWalletModel(WalletModel *model);

    void clearFields();
    void sendMPTransaction();
    void updateProperty();
    void updatePropSelector();
    bool handlePaymentRequest(const SendCoinsRecipient& recipient);

public Q_SLOTS:
    void propertyComboBoxChanged(int idx);
    void sendFromComboBoxChanged(int idx);
    void clearButtonClicked();
    void sendButtonClicked();
    void balancesUpdated();
    void updateFrom();

private:
    Ui::SendMPDialog *ui;
    ClientModel *clientModel;
    WalletModel *walletModel;
    const PlatformStyle *platformStyle;

Q_SIGNALS:
    // Fired when a message should be reported to the user
    void message(const QString &title, const QString &message, unsigned int style);
};

class QOmniAddressValidator : public QObject
{
    Q_OBJECT

public:
    explicit QOmniAddressValidator(QObject *parent) : QObject(parent) {}
protected:
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // BITCOIN_QT_SENDMPDIALOG_H
