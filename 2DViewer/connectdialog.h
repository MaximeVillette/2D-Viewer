#ifndef CONNECTDIALOG_H
#define CONNECTDIALOG_H

#include "includes.h"

#define SIZE_TAB 6

static string defSet[SIZE_TAB] = {"0","btcan","COM12","1000000","0","1"};
string const paramCANPath("C:/TwoDV/defaultSettCAN.txt");

QT_BEGIN_NAMESPACE
namespace Ui {
class ConnectDialog;
}
QT_END_NAMESPACE

class ConnectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectDialog(QWidget *parent = nullptr);
    ~ConnectDialog();

    typedef QPair<QCanBusDevice::ConfigurationKey, QVariant> ConfigurationItem;

    struct Settings {
        QString pluginName;
        QString deviceInterfaceName;
        QList<ConfigurationItem> configurations;
        bool useConfigurationEnabled = false;
    };

    Settings settings() const;

private slots:
    void pluginChanged(const QString &plugin);
    void interfaceChanged(const QString &inter);
    void ok();
    void cancel();

private:
    QString configurationValue(QCanBusDevice::ConfigurationKey key);
    void revertSettings();
    void updateSettings();

    Ui::ConnectDialog *m_ui = nullptr;
    Settings m_currentSettings;
    QList<QCanBusDeviceInfo> m_interfaces;
};

#endif // CONNECTDIALOG_H
