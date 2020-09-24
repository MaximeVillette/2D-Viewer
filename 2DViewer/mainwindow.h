#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "includes.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
class ConnectDialog;
class QCanBusFrame;
class QLabel;
class QTimer;
class TwoDV;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

public slots:
    void sendFrame(const QCanBusFrame &frame) const;

private slots:
    void processReceivedFrames();
    void processErrors(QCanBusDevice::CanBusError) const;
    void connectDevice();
    void disconnectDevice();
    void processFramesWritten(qint64);
    void busStatus();
    void on_actionShow2DV_triggered();
    void closeMain();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void initActionsConnections();

    qint64 m_numberFramesWritten = 0;
    Ui::MainWindow *m_ui = nullptr;
    QLabel *m_status = nullptr;
    QLabel *m_written = nullptr;
    ConnectDialog *m_connectDialog = nullptr;
    std::unique_ptr<QCanBusDevice> m_canDevice;
    QTimer *m_busStatusTimer = nullptr;
    TwoDV *twoDV = nullptr;
};

#endif // MAINWINDOW_H
