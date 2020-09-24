#ifndef TWODV_H
#define TWODV_H

#include "includes.h"

#define SIZE_TAB_REG 7

QT_BEGIN_NAMESPACE
namespace Ui {
class TwoDV;
}
class MainWindow; //Définit la "class" en fenêtre principale
class ConnectDialog;
class sceneAsservissement;
class sceneActions;
QT_END_NAMESPACE

class TwoDV : public QMainWindow
{
    Q_OBJECT

public:
    TwoDV(MainWindow *bus, QWidget *parent = nullptr); //Déclare TwoDV
    ~TwoDV(); //Supprimer TwoDV

    void setCanPosRobot(const QCanBusFrame &fram); //Récupère la trame CAN pour la position du robot
    void setCanVentRobot(const QCanBusFrame &fram); //Récupère la trame CAN pour l'état des ventouses
    void setCanServoRobot(const QCanBusFrame &fram); //Récupère la trame CAN pour l'état des servomoteurs
    void setCanColorsRobot(const QCanBusFrame &fram); //Récupère la trame CAN pour l'état des capteurs de couleurs
    void FrameControl(uint ID, int16_t DATA); //Envoi une trame CAN
    bool activeMode();

signals:
    void sendMode(bool state);
    void sendPosXReg(double reglage);
    void sendPosYReg(double reglage);
    void sendAngleReg(double reglage);
    void sendDistanceReg(double reglage);
    void sendRotationReg(double reglage);
    void sendVitesseReg(double reglage);

private slots:
    void on_PB_enregistrer_REG_clicked();
    void setSettings();
    void requestDataCAN();

private:
    Ui::TwoDV *ui; //Créer un User Interface dans TwoDV

    MainWindow *busCan = nullptr; //Créer un busCan dans MainWindow
    sceneAsservissement *asservScene = nullptr;
    sceneActions *actionsScene = nullptr;

    std::unique_ptr<QCanBusDevice> can_device; //Déclaration d'un bus CAN
    QList<QCanBusDeviceInfo> interfaces; //List des données concernant le bus CAN

    QTimer *timer; //Pour faire des demandes sur le bus CAN

    bool Mode = false;

    enum Ligne
    {
        POSX, POSY, ANG, SERVO, VENT, COLSENS
    };

    QString convertID(const QCanBusFrame &fram);

protected:
    virtual void resizeEvent(QResizeEvent *event) override;
};
#endif // TWODV_H
