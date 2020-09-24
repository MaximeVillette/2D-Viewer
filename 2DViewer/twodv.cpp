#include "twodv.h"
#include "mainwindow.h"
#include "sceneasservissement.h"
#include "sceneactions.h"

//Librairie du design de la fenêtre
#include "ui_twodv.h"

//-----Variables & Accès-----//

static string Reglages[SIZE_TAB_REG]={"0"}; //Tableau des réglages
string const paramPath("C:/TwoDV/defaultSett.txt"); //Chemin de stockage des paramètres

//-----Définition de la "class" TwoDV-----//

TwoDV::TwoDV(MainWindow *bus, QWidget *parent): QMainWindow(parent),
    ui(new Ui::TwoDV), //Déclare l'utilisation d'un UI dans la class TwoDV
    busCan(bus), //Déclare l'utilisation de 3 nouvelles class (1)
    asservScene(new sceneAsservissement), //(2)
    actionsScene(new sceneActions) //(3)
{
    ui->setupUi(this); //Démarre l'UI à cet endroit
    ui->SW_general->setCurrentIndex(0); //Affiche la première page au départ
    QTimer::singleShot(10, this, SLOT(setSettings())); //Ouvre et applique les derniers réglages

//--Actions barre d'outils--//
    connect(ui->AC_temspReel, &QAction::triggered, [this]() //Active le mode temps réel
        {Mode = true, emit sendMode(Mode);});
    connect(ui->AC_developpement, &QAction::triggered, [this]() //Active le mode développement
        {Mode = false, emit sendMode(Mode);});
    connect(ui->AC_ouvrirPeripherique, &QAction::triggered, [this]() //Ouvre l'interface CAN
    {
        busCan->show(); //Montre la fenêtre
        busCan->activateWindow(); //Met la fenêtre au premier plan
    });
    connect(ui->AC_reglages, &QAction::triggered, [this]() //Ouvre la page de réglages
        {ui->SW_general->setCurrentIndex(1);});
    connect(ui->AC_fermer, &QAction::triggered, [this]() //Ferme l'interface CAN
        {busCan->close();});
    connect(ui->AC_quitter, &QAction::triggered, [this]() //Ferme le logiciel
    {
        TwoDV::close(); //Ferme la fenêtre TwoDV
        busCan->close(); //Ferme l'interface CAN
    });

//--Actions réglages--//
    connect(ui->CB_modeTR_REG, &QCheckBox::clicked, [this]() //Sélectionne le mode temps réel
    {
        //Dans ce cas, les checkbox ne peuvent pas être cochées en même temps. Donc si l'une est cochée, on décoche l'autre :
        ui->CB_modeTR_REG->isChecked() ? ui->CB_modeD_REG->setChecked(0) : ui->CB_modeD_REG->setChecked(1);
        Reglages[6]="0"; //Enregistre dans la RAM
    });
    connect(ui->CB_modeD_REG, &QCheckBox::clicked, [this]() //Séletionne le mode développement
    {
        ui->CB_modeD_REG->isChecked() ? ui->CB_modeTR_REG->setChecked(0) : ui->CB_modeTR_REG->setChecked(1);
        Reglages[6]="1"; //Enregistre dans la RAM
    });
    connect(ui->PB_retour_REG, &QPushButton::clicked, [this]() //Retour page réglage
        {ui->SW_general->setCurrentIndex(0);});

//--Actions page principale--//
    connect(ui->CB_afficherGVasserv, &QCheckBox::clicked, [this]() //Affiche ou non la scène d'asservissement
    {
        //Choisit s'il faut afficher la scène en fonction de l'état de la checkbox :
        ui->CB_afficherGVasserv->isChecked() ? ui->GB_sceneAsserv->show() : ui->GB_sceneAsserv->hide();
        //Timer de 10ms avant d'actualiser la vue (laisse le temps à l'ordinateur d'afficher la scène)
        QTimer::singleShot(10, ui->GB_sceneAsserv, &sceneAsservissement::SetView);
        QTimer::singleShot(10, ui->GB_sceneActions, &sceneActions::SetView);
    });
    connect(ui->CB_afficherGVactions, &QCheckBox::clicked, [this]() //Affiche ou non la scène des autres actions
    {
        ui->CB_afficherGVactions->isChecked() ? ui->GB_sceneActions->show() : ui->GB_sceneActions->hide();
        QTimer::singleShot(10, ui->GB_sceneAsserv, &sceneAsservissement::SetView);
        QTimer::singleShot(10, ui->GB_sceneActions, &sceneActions::SetView);
    });

//--Connecte tous les signaux des 3 class--//
    //Class tabCommandes
    connect(ui->GB_commandes, &tabCommandes::sendRotation, ui->GB_sceneAsserv, &sceneAsservissement::getRotation);
    connect(ui->GB_commandes, &tabCommandes::sendDistance, ui->GB_sceneAsserv, &sceneAsservissement::getDistance);
    connect(ui->GB_commandes, &tabCommandes::sendVitesse, ui->GB_sceneAsserv, &sceneAsservissement::getVitesse);
    connect(ui->GB_commandes, &tabCommandes::sendNewPosX, ui->GB_sceneAsserv, &sceneAsservissement::getNewPosX);
    connect(ui->GB_commandes, &tabCommandes::sendNewPosY, ui->GB_sceneAsserv, &sceneAsservissement::getNewPosY);
    connect(ui->GB_commandes, &tabCommandes::sendNewAngle, ui->GB_sceneAsserv, &sceneAsservissement::getNewAngle);
    connect(ui->GB_commandes, &tabCommandes::sendFrameControl, this, &TwoDV::FrameControl);

    //Class sceneAsservissement
    connect(ui->GB_sceneAsserv, &sceneAsservissement::sendInfosPosRobot, ui->GB_commandes, &tabCommandes::getInfosPosRobot);
    connect(ui->GB_sceneAsserv, &sceneAsservissement::sendNewDefaultPos, ui->GB_commandes, &tabCommandes::getNewDefaultPos);

    //Class TwoDV
    connect(this, &TwoDV::sendMode, ui->GB_commandes, &tabCommandes::getCommandAuthorize);
    connect(this, &TwoDV::sendMode, ui->GB_sceneAsserv, &sceneAsservissement::getMode);
    connect(this, &TwoDV::sendPosXReg, ui->GB_commandes, &tabCommandes::getPosXReg);
    connect(this, &TwoDV::sendPosYReg, ui->GB_commandes, &tabCommandes::getPosYReg);
    connect(this, &TwoDV::sendAngleReg, ui->GB_commandes, &tabCommandes::getAngleReg);
    connect(this, &TwoDV::sendDistanceReg, ui->GB_commandes, &tabCommandes::getDistanceReg);
    connect(this, &TwoDV::sendRotationReg, ui->GB_commandes, &tabCommandes::getRotationReg);
    connect(this, &TwoDV::sendVitesseReg, ui->GB_commandes, &tabCommandes::getVitesseReg);

//--Timer pour faire des demandes sur le bus CAN--//
    timer = new QTimer(this); //Déclare le timer
    connect(timer, SIGNAL(timeout()), this, SLOT(requestDataCAN())); //Connecte le signal à recevoir avec la fonction à exécuter
    timer->start(1000); //Enclenche le timer toutes les secondes
}

TwoDV::~TwoDV() {delete ui;} //Supprime l'UI

void TwoDV::resizeEvent(QResizeEvent* event) //Adapte la taille de la fenêtre
{
   QMainWindow::resizeEvent(event); //Détecte de l'évènement
   QTimer::singleShot(10, ui->GB_sceneAsserv, &sceneAsservissement::SetView);
   QTimer::singleShot(10, ui->GB_sceneActions, &sceneActions::SetView);
}

void TwoDV::requestDataCAN() //Demande des infos sur le CAN (fonction exécutée toute les secondes via le timer)
{
    FrameControl(0x4B0, 0); //Capteurs couleurs
    FrameControl(0x27A, 0); //Ventouses
}

QString TwoDV::convertID(const QCanBusFrame &fram)
{
    const char * const idFormat = fram.hasExtendedFrameFormat() ? "%08X" : "%03X";
    uint fid = static_cast<uint>(fram.frameId()); //Convertit un 'quint32' en 'uint'
    QString frID = QString::asprintf(idFormat, fid); //Convertit en 'QString' l'ID en fonction de 'idFormat' (le format) et 'fid' (l'ID)
    return frID;
}

void TwoDV::FrameControl(uint ID, int16_t DATA) //Envoi d'une consigne avec l'ID et les données
{
    QByteArray data(8, 0); //Donnée sur 8 octets
    data[0] = static_cast<char>(DATA & 0xFF); //Fait un masque sur le premier octet
    data[1] = static_cast<char>((DATA >> 8) & 0xFF); //Fait un masque sur le deuxième octet
    QCanBusFrame frame(ID, data); //Définit une trame avec un ID (identifiant) et une donnée (data)
    busCan->sendFrame(frame); //Envoi la trame au robot
}

void TwoDV::setCanPosRobot(const QCanBusFrame &fram) //Décompose la trame pour placer le robot
{
    double px, py, theta; //Variables pour positionner le robot
    QString id;

    //Utilisation du poids fort/faible de la donnée (payload)
    uint16_t x_faible = static_cast<uint8_t>(fram.payload()[0]);
    uint16_t x_fort = static_cast<uint8_t>(fram.payload()[1]) << 8u; //Dans '8u', 'u' signifie 'uint'
    px = static_cast<int16_t>(x_faible | x_fort); //Retourne la position X

    uint16_t y_faible = static_cast<uint8_t>(fram.payload()[2]);
    uint16_t y_fort = static_cast<uint8_t>(fram.payload()[3]) << 8u;
    py = static_cast<int16_t>(y_faible | y_fort); //Retourne la position Y

    uint16_t th_faible = static_cast<uint8_t>(fram.payload()[4]);
    uint16_t th_fort = static_cast<uint8_t>(fram.payload()[5]) << 8u;
    theta = static_cast<int16_t>(th_faible | th_fort);
    theta *= 0.1; //Retourne l'angle theta avec un facteur 0.1

    //Modifie les variables dans la class sceneActions
    id = convertID(fram); //Extraction de l'ID de la trame pour l'afficher dans le tableau de actionsScene
    actionsScene->setVariables(POSX, "Position X", id, QString("%0").arg(px), "Description vide");
    actionsScene->setVariables(POSY, "Position Y", id, QString("%0").arg(py), "Description vide");
    actionsScene->setVariables(ANG, "Angle", id, QString("%0").arg(theta), "Description vide");

    //Positionne le robot (250 et 1750 sont des offsets (taille du robot) pour régler le placement)
    asservScene->SetPosition(px+250, 1750-py, 90-theta);

}
void TwoDV::setCanServoRobot(const QCanBusFrame &fram) //Décompose la trame pour les servomoteurs (marche, mais à revoir)
{
    QString etat, id;

    if(fram.payload().toHex()=="9900") etat = "Bras sorties";
    else if(fram.payload().toHex()=="1601") etat = "Bras rentrées";
    else etat = "Aucun mouvements";

    //Modifie la variable dans la class sceneActions
    id = convertID(fram);
    actionsScene->setVariables(SERVO, "Servo 1", id, fram.payload().toHex(), etat);
}
void TwoDV::setCanVentRobot(const QCanBusFrame &fram) //Décompose la trame pour les ventouses (à finir)
{
    QString id;
    qDebug() << "Ventouses :" << fram.payload().toHex();

    //Modifie la variable dans la class sceneActions
    id = convertID(fram);
    actionsScene->setVariables(VENT, "Ventouse 1", id, fram.payload().toHex(), "Description vide");
}
void TwoDV::setCanColorsRobot(const QCanBusFrame &fram) //Décompose la trame pour les capteurs de couleurs (à finir)
{
    QString id;
    qDebug() << "Capteurs coulerus :" << fram.payload().toHex();

    //Modifie la variable dans la class sceneActions
    id = convertID(fram);
    actionsScene->setVariables(COLSENS, "Color 1", id, fram.payload().toHex(), "Description vide");
}

bool TwoDV::activeMode() //Active ou non la réception des trames CAN
{
    return Mode; //0 pour le mode développement, 1 pour le mode temps réel.
}

void TwoDV::on_PB_enregistrer_REG_clicked() //Enregistre les réglages
{
    ofstream flux(paramPath); //Créer un flux de sortie
    //Chaque objet retourne son réglage
    Reglages[0] = QString("%0").arg(ui->DSB_posX_REG->value()).toStdString(); //Convertit 'QString' en 'string'
    Reglages[1] = QString("%0").arg(ui->DSB_posY_REG->value()).toStdString();
    Reglages[2] = QString("%0").arg(ui->DSB_angle_REG->value()).toStdString();
    Reglages[3] = QString("%0").arg(ui->DSB_distance_REG->value()).toStdString();
    Reglages[4] = QString("%0").arg(ui->DSB_rotation_REG->value()).toStdString();
    Reglages[5] = QString("%0").arg(ui->DSB_vitesse_REG->value()).toStdString();
    Reglages[6] = ui->CB_modeTR_REG->isChecked() ? "1" : "0";
    if(flux) //Si le flux est bien présent
    {
        for(int i=0; i<SIZE_TAB_REG; i++) flux << Reglages[i] << endl; //Enregistre chaque réglage dans la ROM
        setSettings();//Applique les réglages
    }
    else QMessageBox::warning(this,"Erreur","Impossible d'enregistrer les réglages !");
    ui->SW_general->setCurrentIndex(0); //Affiche la première page
}

void TwoDV::setSettings() //Applique les réglages
{
    ifstream flux(paramPath); //Créer un flux en entrée
    if(flux) //Si le flux est bien présent
    {
        for(int i=0; i<SIZE_TAB_REG; i++) flux >> Reglages[i]; //Charge chaque réglage
        //Applique chaque réglage à son objet
        ui->DSB_posX_REG->setValue(QString::fromStdString(Reglages[0]).toDouble()); //Convertit un 'QString' en 'string' puis en 'double'
        ui->DSB_posY_REG->setValue(QString::fromStdString(Reglages[1]).toDouble());
        ui->DSB_angle_REG->setValue(QString::fromStdString(Reglages[2]).toDouble());
        ui->DSB_distance_REG->setValue(QString::fromStdString(Reglages[3]).toDouble());
        ui->DSB_rotation_REG->setValue(QString::fromStdString(Reglages[4]).toDouble());
        ui->DSB_vitesse_REG->setValue(QString::fromStdString(Reglages[5]).toDouble());
        if(Reglages[6] == "1") //Différencie le mode temps réeel et développement
        {
            ui->CB_modeTR_REG->setChecked(1);
            ui->CB_modeD_REG->setChecked(0);
            Mode = true;
        }
        else
        {
            ui->CB_modeTR_REG->setChecked(0);
            ui->CB_modeD_REG->setChecked(1);
            Mode = false;
        }
        emit sendMode(Mode); //Envoi l'état du mode à la class sceneAsservissement
        //Envoi un signal pour chaque réglage à la class tabCommandes
        emit sendPosXReg(QString::fromStdString(Reglages[0]).toDouble());
        emit sendPosYReg(QString::fromStdString(Reglages[1]).toDouble());
        emit sendAngleReg(QString::fromStdString(Reglages[2]).toDouble());
        emit sendDistanceReg(QString::fromStdString(Reglages[3]).toDouble());
        emit sendRotationReg(QString::fromStdString(Reglages[4]).toDouble());
        emit sendVitesseReg(QString::fromStdString(Reglages[5]).toDouble());

        //Applique la position de départ
        asservScene->SetPosition(QString::fromStdString(Reglages[0]).toDouble(),
                QString::fromStdString(Reglages[1]).toDouble(), QString::fromStdString(Reglages[2]).toDouble());
    }
    else QMessageBox::warning(this,"Erreur","Impossible d'ouvrir les réglages!");
}
