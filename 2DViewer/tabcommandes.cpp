#include "tabcommandes.h"

#include "ui_tabcommandes.h"

tabCommandes::tabCommandes(QWidget *parent) :
    QGroupBox(parent),
    command_ui(new Ui::tabCommandes)
{
    command_ui->setupUi(this);

    //Connecte l'état des boutons (page servomoteurs) à sa trame avec ID et donnée
    connect(command_ui->PB_servoOrdreAttraper_1, &QPushButton::clicked, [this]()
        {emit sendFrameControl(0x260, command_ui->SB_servoOrdreBras1_1->value());});
    connect(command_ui->PB_servoOrdreRelacher_1, &QPushButton::clicked, [this]()
        {emit sendFrameControl(0x261, command_ui->SB_servoOrdreBras1_2->value());});
    connect(command_ui->PB_servoOrdreAttraper_2, &QPushButton::clicked, [this]()
        {emit sendFrameControl(0x262, command_ui->SB_servoOrdreBras2_1->value());});
    connect(command_ui->PB_servoOrdreRelacher_2, &QPushButton::clicked, [this]()
        {emit sendFrameControl(0x263, command_ui->SB_servoOrdreBras2_2->value());});
    connect(command_ui->PB_servoOrdreAttraper_3, &QPushButton::clicked, [this]()
        {emit sendFrameControl(0x264, dataServos);});
    connect(command_ui->PB_servoOrdreRelacher_3, &QPushButton::clicked, [this]()
        {emit sendFrameControl(0x265, dataServos);});
    connect(command_ui->PB_drapeaux, &QPushButton::clicked, [this]()
        {emit sendFrameControl(0x254, 01);});
    connect(command_ui->PB_positionInitiale, &QPushButton::clicked, [this]()
        {emit sendFrameControl(0x26A, 0);});
    connect(command_ui->CB_servoOrdreAvant, &QCheckBox::clicked, [this]()
    {
        if((command_ui->CB_servoOrdreAvant->isChecked()==0)&&(command_ui->CB_servoOrdreArriere->isChecked()==0))
        {
            command_ui->PB_servoOrdreAttraper_3->setEnabled(0);
            command_ui->PB_servoOrdreRelacher_3->setEnabled(0);
        }
        else if(command_ui->CB_servoOrdreAvant->isChecked())
        {
            command_ui->CB_servoOrdreArriere->setChecked(0);
            command_ui->PB_servoOrdreAttraper_3->setEnabled(1);
            command_ui->PB_servoOrdreRelacher_3->setEnabled(1);
            dataServos = 0;
        }
    });
    connect(command_ui->CB_servoOrdreArriere, &QCheckBox::clicked, [this]()
    {
        if((command_ui->CB_servoOrdreAvant->isChecked()==0)&&(command_ui->CB_servoOrdreArriere->isChecked()==0))
        {
            command_ui->PB_servoOrdreAttraper_3->setEnabled(0);
            command_ui->PB_servoOrdreRelacher_3->setEnabled(0);
        }
        else if(command_ui->CB_servoOrdreArriere->isChecked())
        {
            command_ui->CB_servoOrdreAvant->setChecked(0);
            command_ui->PB_servoOrdreAttraper_3->setEnabled(1);
            command_ui->PB_servoOrdreRelacher_3->setEnabled(1);
            dataServos = 1;
        }
    });

    //Connecte l'état des boutons (page ventouses) à sa trame avec ID et donnée
    connect(command_ui->PB_ventouseOrdreAttraper, &QPushButton::clicked, [this]()
        {emit sendFrameControl(0x266, command_ui->SB_ventOrdre_1->value());});
    connect(command_ui->PB_ventouseOrdreRelacher, &QPushButton::clicked, [this]()
        {emit sendFrameControl(0x267, command_ui->SB_ventOrdre_2->value());});
    connect(command_ui->PB_toutActiver, &QPushButton::clicked, [this]()
        {emit sendFrameControl(0x268, 0);});
    connect(command_ui->PB_toutDesactiver, &QPushButton::clicked, [this]()
        {emit sendFrameControl(0x269, 0);});

    //Affiche la positon de départ
    command_ui->L_posXYR->setText(QString("%0 X ; %1 Y ; %2 R")
                           .arg(command_ui->DSB_posX->value()).arg(command_ui->DSB_posY->value()).arg(command_ui->DSB_angle->value())); //Affiche la position par défault
}

tabCommandes::~tabCommandes() {delete command_ui;}

//Envoi les consignes en fonction de l'état des DoubleSpinBox
void tabCommandes::on_DSB_rotation_valueChanged(double arg1) {emit sendRotation(arg1);}
void tabCommandes::on_DSB_distance_valueChanged(double arg1) {emit sendDistance(arg1);}
void tabCommandes::on_DSB_vitesse_valueChanged(double arg1) {emit sendVitesse(arg1);}
void tabCommandes::on_DSB_posX_valueChanged(double arg1) {emit sendNewPosX(arg1);}
void tabCommandes::on_DSB_posY_valueChanged(double arg1) {emit sendNewPosY(arg1);}
void tabCommandes::on_DSB_angle_valueChanged(double arg1) {emit sendNewAngle(arg1);}

//Applique les réglages reçu à l'ouverture
void tabCommandes::getPosXReg(double reglage)
{
    command_ui->DSB_posX->setValue(reglage);
}
void tabCommandes::getPosYReg(double reglage)
{
    command_ui->DSB_posY->setValue(reglage);
}
void tabCommandes::getAngleReg(double reglage)
{
    command_ui->DSB_angle->setValue(reglage);
}
void tabCommandes::getDistanceReg(double reglage) {command_ui->DSB_distance->setValue(reglage);}
void tabCommandes::getRotationReg(double reglage) {command_ui->DSB_rotation->setValue(reglage);}
void tabCommandes::getVitesseReg(double reglage) {command_ui->DSB_vitesse->setValue(reglage);}

void tabCommandes::getInfosPosRobot(double x, double y, double r) //Reçoit la position du robot
{
    //Affiche chaque donnée
    command_ui->L_posX_actuelle->setText(QString("Position X : %0").arg(x));
    command_ui->L_posY_actuelle->setText(QString("Position Y : %0").arg(y));
    command_ui->L_rotation_actuelle->setText(QString("Position R : %0").arg(r));
}
void tabCommandes::getNewDefaultPos(double x, double y, double r) //Reçoit la nouvelle position par défaut
{
    //Applique chaque donnée
    command_ui->DSB_posX->setValue(x);
    command_ui->DSB_posY->setValue(y);
    command_ui->DSB_angle->setValue(r);
    //Affiche la nouvelle position par défaut
    command_ui->L_posXYR->setText(QString("%0 X ; %1 Y ; %2 R").arg(x).arg(y).arg(r)); //Affiche la position par défault
}
void tabCommandes::getCommandAuthorize(bool mode) //Autorise certaines commandes en fonction du mode
{
    bool modeTR, modeD;
    if(mode) modeTR = true, modeD = false; //Si on veut le mode temps réel
    else modeTR = false, modeD = true; //Sinon c'est le mode développement

    //Consignes pour le mode temps réel
    command_ui->DSB_distance->setEnabled(modeTR);
    command_ui->DSB_rotation->setEnabled(modeTR);
    command_ui->L_distance->setEnabled(modeTR);
    command_ui->L_rotation->setEnabled(modeTR);

    //Consignes pour le mode développement
    command_ui->DSB_vitesse->setEnabled(modeD);
    command_ui->L_vitesse->setEnabled(modeD);
}
