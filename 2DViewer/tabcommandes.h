#ifndef TABCOMMANDES_H
#define TABCOMMANDES_H

#include "includes.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class tabCommandes;
}
QT_END_NAMESPACE

class tabCommandes : public QGroupBox
{
    Q_OBJECT

public:
    explicit tabCommandes(QWidget *parent = nullptr);
    ~tabCommandes();

    void getInfosPosRobot(double x, double y, double r);
    void getNewDefaultPos(double x, double y, double r);
    void getCommandAuthorize(bool mode);
    void getPosXReg(double reglage);
    void getPosYReg(double reglage);
    void getAngleReg(double reglage);
    void getDistanceReg(double reglage);
    void getRotationReg(double reglage);
    void getVitesseReg(double reglage);

signals:
    void sendVitesse(double v);
    void sendDistance(double d);
    void sendRotation(double r);
    void sendNewPosX(double x);
    void sendNewPosY(double y);
    void sendNewAngle(double a);
    void sendFrameControl(uint ID, int16_t DATA);

private slots:
    void on_DSB_vitesse_valueChanged(double arg1);
    void on_DSB_distance_valueChanged(double arg1);
    void on_DSB_rotation_valueChanged(double arg1);
    void on_DSB_posX_valueChanged(double arg1);
    void on_DSB_posY_valueChanged(double arg1);
    void on_DSB_angle_valueChanged(double arg1);

private:
    Ui::tabCommandes *command_ui;

    int16_t dataServos;
};

#endif // TABCOMMANDES_H
