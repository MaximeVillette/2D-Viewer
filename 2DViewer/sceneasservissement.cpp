#include "sceneasservissement.h"
#include "twodv.h"
#include "tabcommandes.h"

#include "ui_sceneasservissement.h"

sceneAsservissement::sceneAsservissement(QWidget *parent) : QGroupBox(parent),
    asserv_ui(new Ui::sceneAsservissement),
    commandes(new tabCommandes)
{
    asserv_ui->setupUi(this);
    qApp->installEventFilter(this); //Accès aux évènements

    sceneAsservView = new QGraphicsScene(this); //Nouvelle scène -> table de jeu
    asserv_ui->GV_asserv->setScene(sceneAsservView); //Applique la scene dans le graphicsView
    asserv_ui->GV_asserv->viewport()->installEventFilter(this); //Applique les évènements dans le graphicsView

    QPixmap tapis(":/Table/Images/SailTheWorld/TableSailTheWorld.png"); //Chemin d'accès de l'image du tapis
    QPixmap rob(":/Robot/Images/SailTheWorld/Robot.png"); //Chemin d'accès de l'image du robot

    map = sceneAsservView->addPixmap(tapis); //Applique l'image du tapis dans la scene
    map->setOffset(-202,-481); //Décalage pour centrer le tapis dans la scene

    //Place le robot dans la scene d'asservissement
    robot1 = sceneAsservView->addPixmap(rob); //Applique l'image du robot dans la scene
    robot1->setOffset(-robot1->boundingRect().center().x(),-robot1->boundingRect().center().y()); //Applique le centre de rotation du robot
    robot1->setFlags(QGraphicsItem::ItemIsMovable); //Autorise le robot a être déplacé par la souris

    //Créer le style des bordures
    QPen red(Qt::red); //Création d'un pinceau couleur rouge
    red.setStyle(Qt::SolidLine); //Ligne pleine
    red.setWidth(25); //25 pixel de large
    QPen green(Qt::green); //Création d'un pinceau couleur verte
    green.setStyle(Qt::SolidLine);
    green.setWidth(25);

    //Créer les bordures au départ
    bordures[NORD] = sceneAsservView->addLine(ORIGINE_X,ORIGINE_Y,LONGUEUR_TABLE,ORIGINE_Y,red); //Coordonées NORD
        bordures[SUD] = sceneAsservView->addLine(ORIGINE_X,LARGEUR_TABLE,LONGUEUR_TABLE,LARGEUR_TABLE,red); //Coordonées SUD
            bordures[EST] = sceneAsservView->addLine(LONGUEUR_TABLE,ORIGINE_Y,LONGUEUR_TABLE,LARGEUR_TABLE,red); //Coordonées EST
                bordures[OUEST] = sceneAsservView->addLine(ORIGINE_X,ORIGINE_Y,ORIGINE_X,LARGEUR_TABLE,red); //Coordonées OUEST
    obstacles[GAUCHE] = sceneAsservView->addLine(OBSTACLE_1_X,OBSTACLE_1_Y,OBSTACLE_1_X,LARGEUR_TABLE,red); //Coordonées obstacle gauche
        obstacles[MILIEU] = sceneAsservView->addLine(OBSTACLE_2_X,OBSTACLE_2_Y,OBSTACLE_2_X,LARGEUR_TABLE,red); //Coordonées obstacle du milieu
            obstacles[DROITE] = sceneAsservView->addLine(OBSTACLE_3_X,OBSTACLE_3_Y,OBSTACLE_3_X,LARGEUR_TABLE,red); //Coordonées obstacle droit
    //Cache les lignes au départ
    bordures[NORD]->hide();
        bordures[SUD]->hide();
            bordures[EST]->hide();
                bordures[OUEST]->hide();
    obstacles[GAUCHE]->hide();
        obstacles[MILIEU]->hide();
            obstacles[DROITE]->hide();

    mousePosTxt = sceneAsservView->addText(""); //Applique un texte vierge
    mousePosTxt->setPos(2700, -500); //Place la zone de texte en haut à droite du graphicsView
    mousePosTxt->setScale(5); //Applique une échelle sur la zone de texte

    connect(asserv_ui->HS_zoomAsserv, &QSlider::valueChanged, [this]() //Zoom sur la table
    {
        SetView(); //Met à l'échelle de la fenêtre proportionnellement à la taille de celle-ci
        //Met l'échelle en fonction de la valeur du slider (1 à 5)
        asserv_ui->GV_asserv->scale(asserv_ui->HS_zoomAsserv->value(), asserv_ui->HS_zoomAsserv->value());
    });

    QTimer::singleShot(10, this, &sceneAsservissement::SetView); //Timer de 10ms pour mettre la vue à l'échelle
    QTimer::singleShot(50, this, &sceneAsservissement::fTime); //Positionne le robot pour la première fois en fonction des réglages (il faut laisser le temps à TwoDV de lire et envoyer ces réglages)
}

sceneAsservissement::~sceneAsservissement() {delete asserv_ui;}

void sceneAsservissement::SetView()
{
    asserv_ui->GV_asserv->fitInView(sceneAsservView->sceneRect(), Qt::KeepAspectRatio); //Le graphicsView est un rectangle qui garde un aspect proportionnel
    qDebug() << sceneAsservView->sceneRect(); //Affiche dans le débogeur la position et la taille du rectangle
}

//Reçoit les consignes de la class tabCommandes
void sceneAsservissement::getMode(bool state) {Mode = state;}
void sceneAsservissement::getRotation(double r) {Rotation = static_cast<int16_t>(r);}
void sceneAsservissement::getDistance(double d) {Distance = static_cast<int16_t>(d);}
void sceneAsservissement::getVitesse(double v) {Vitesse = v;}
void sceneAsservissement::getNewPosX(double x) {PX = x;}
void sceneAsservissement::getNewPosY(double y) {PY = y;}
void sceneAsservissement::getNewAngle(double a) {Rot = a;}

void sceneAsservissement::TableLimit(double Px, double Py) //Vérifie que le robot est bien sur la table virtuelle
{
    //Si le robot dépasse une de ces coordonnées, alors le signaler
    (Px<=ORIGINE_X+(LONGUEUR_ROBOT1/2)+40) ? bordures[OUEST]->show() : bordures[OUEST]->hide(); //OUEST
    (Px>=ORIGINE_X+LONGUEUR_TABLE-(LONGUEUR_ROBOT1/2)+40) ? bordures[EST]->show() : bordures[EST]->hide(); //EST
    (Py>=ORIGINE_Y+LARGEUR_TABLE-(LARGEUR_ROBOT1/2)-40) ? bordures[SUD]->show() : bordures[SUD]->hide(); //SUD
    (Py<=ORIGINE_Y+(LARGEUR_ROBOT1/2)+25) ? bordures[NORD]->show() : bordures[NORD]->hide(); //NORD

    //Si le robot passe sur une de ces coordonnées, alors le signaler
    (((Px>=OBSTACLE_1_X-(LONGUEUR_ROBOT1/2))&&(Px<=OBSTACLE_1_X+(LONGUEUR_ROBOT1/2)))&&(Py>=OBSTACLE_1_Y-(LARGEUR_ROBOT1/2))) ?
        obstacles[GAUCHE]->show() : obstacles[GAUCHE]->hide(); //Obstacle gauche
    (((Px>=OBSTACLE_2_X-(LONGUEUR_ROBOT1/2))&&(Px<=OBSTACLE_2_X+(LONGUEUR_ROBOT1/2)))&&(Py>=OBSTACLE_2_Y-(LARGEUR_ROBOT1/2))) ?
        obstacles[MILIEU]->show() : obstacles[MILIEU]->hide(); //Obstacle du milieu
    (((Px>=OBSTACLE_3_X-(LONGUEUR_ROBOT1/2))&&(Px<=OBSTACLE_3_X+(LONGUEUR_ROBOT1/2)))&&(Py>=OBSTACLE_3_Y-(LARGEUR_ROBOT1/2))) ?
        obstacles[DROITE]->show() : obstacles[DROITE]->hide(); //Obstacle droit

    //Si une ou plusieurs lignes sont visibles, alors forcer le robot à revenir sur la table
    if(bordures[OUEST]->isVisible()) SetPosition(PosX+10.0, PosY, Rotate);
    if(bordures[EST]->isVisible()) SetPosition(PosX-10.0, PosY, Rotate);
    if(bordures[NORD]->isVisible()) SetPosition(PosX, PosY+10.0, Rotate);
    if(bordures[SUD]->isVisible()) SetPosition(PosX, PosY-10.0, Rotate);
}

void sceneAsservissement::ReadPosition() //Lit la position du robot
{
    PosX = robot1->pos().rx();
    PosY = robot1->pos().ry();
    Rotate = robot1->rotation();
}
void sceneAsservissement::SetPosition(double x, double y, double r) //Place le robot (mode développement uniquement)
{
    robot1->setX(x);
    robot1->setY(y);
    robot1->setRotation(r);
    emit sendInfosPosRobot(PosX, PosY, Rotate);
}

//Les 2 fonctions suivantes, calcul la trajectoire virtuelle du robot (sans avoir besoin du CAN)
void sceneAsservissement::SelectionQuadrantSensN(double teta) //Calcul de la trajectoire dans le sens NEGATIF
{
    if((teta >= 0)&&(teta <= 90)) //Si l'angle est dans le quadrant 1
    {
        PosY -= (-0.0111 * Vitesse * Rotate)+(1 * Vitesse);
        PosX += (0.0111 * Vitesse * Rotate);
    }
    if((teta > 90)&&(teta <= 180)) //Si l'angle est dans le quadrant 2
    {
        PosY += (0.0111 * Vitesse * Rotate)-(1 * Vitesse);
        PosX += (-0.0111 * Vitesse * Rotate)+(2 * Vitesse);
    }
    if((teta > 180)&&(teta <= 270)) //Si l'angle est dans ce quadrant 3
    {
        PosY += (-0.0111 * Vitesse * Rotate)+(3 * Vitesse);
        PosX -= (0.0111 * Vitesse * Rotate)-(2 * Vitesse);
    }
    if((teta > 270)&&(teta <= 360)) //Si l'angle est dans ce quadrant 4
    {
        PosY -= (0.0111 * Vitesse * Rotate)-(3 * Vitesse);
        PosX -= (-0.0111 * Vitesse * Rotate)+(4 * Vitesse);
    }
}
void sceneAsservissement::SelectionQuadrantSensP(double teta) //Calcul de la trajectoire dans le sens POSITIF
{
    if((teta >= 0)&&(teta <= 90)) //Si l'angle est dans le quadrant 1
    {
        PosY += (-0.0111 * Vitesse * Rotate)+(1 * Vitesse);
        PosX -= (0.0111 * Vitesse * Rotate);
    }
    if((teta > 90)&&(teta <= 180)) //Si l'angle est dans le quadrant 2
    {
        PosY -= (0.0111 * Vitesse * Rotate)-(1 * Vitesse);
        PosX -= (-0.0111 * Vitesse * Rotate)+(2 * Vitesse);
    }
    if((teta > 180)&&(teta <= 270)) //Si l'angle est dans ce quadrant 3
    {
        PosY -= (-0.0111 * Vitesse * Rotate)+(3 * Vitesse);
        PosX += (0.0111 * Vitesse * Rotate)-(2 * Vitesse);
    }
    if((teta > 270)&&(teta <= 360)) //Si l'angle est dans ce quadrant 4
    {
        PosY += (0.0111 * Vitesse * Rotate)-(3 * Vitesse);
        PosX += (-0.0111 * Vitesse * Rotate)+(4 * Vitesse);
    }
}

bool sceneAsservissement::eventFilter(QObject *obj, QEvent *event) //Détecte un évènement
{
    if(obj == asserv_ui->GV_asserv->viewport()) //Si cela se passe dans le graphicsView
    {
        if(event->type()==QEvent::MouseMove) //Quel type d'évènement (souris)
        {
            QMouseEvent *mouseEvent = (QMouseEvent*)event; //Créer un évènement de souris
            varMousePos = {mouseEvent->x(), mouseEvent->y()}; //Récupère la position x et y
            mousePosTxt->setPlainText(QString("X : %0\nY : %1").arg(varMousePos.x()).arg(varMousePos.y())); //Affiche la position dans la zone de texte
        }
    }
    return false;
}
void sceneAsservissement::keyPressEvent(QKeyEvent *keyevent) //Détecte un évènement clavier
{
    ReadPosition(); //Lit la position du robot
    if(Mode) //Si c'est le mode temps réel
    {
        //faire un emit pour FrameControl
        if(keyevent->key() == Qt::Key_Z)
           emit sendFrameControl(0x24, Distance); //En avant
        if(keyevent->key() == Qt::Key_S)
           emit sendFrameControl(0x24, Distance*-1); //En arrière
        if(keyevent->key() == Qt::Key_D)
           emit sendFrameControl(0x23, Rotation*-10); //A droite
        if(keyevent->key() == Qt::Key_Q)
           emit sendFrameControl(0x23, Rotation*10); //A gauche
    }
    else //C'est le mode développement
    {
        //Les fonctions "SelectionQuadrant SensP/SensN" : Calcul les coordonées d'arriver du robot en fonction de son angle dans les deux sens
        if(keyevent->key() == Qt::Key_Z) SelectionQuadrantSensP(Rotate); //En avant
        if(keyevent->key() == Qt::Key_S) SelectionQuadrantSensN(Rotate); //En arrière

        if(keyevent->key() == Qt::Key_D) //A droite
        {
            Rotate += Vitesse; //Incrémente
            if(Rotate > 359) Rotate = 0; //Si l'angle est supérieur à 359°, il faut repartir à 0°
        }
        if(keyevent->key() == Qt::Key_Q) //A gauche
        {
            Rotate -= Vitesse; //Décrémente
            if(Rotate < 1) Rotate = 360; //Si l'angle est inférieur à 1°, il faut repartir à 360°
        }

        if((keyevent->key()== Qt::Key_I)&&(keyevent->modifiers()==Qt::ControlModifier)) //Applique la position par défaut
        {
            PosX = PX;
            PosY = PY;
            Rotate = Rot;
            emit sendNewDefaultPos(PosX, PosY, Rotate); //Envoi la nouvelle position par défaut
        }
    }
    if((keyevent->key()== Qt::Key_P)&&(keyevent->modifiers()==Qt::ControlModifier)) //Enregistre une position par défaut
    {
        emit sendNewDefaultPos(PosX, PosY, Rotate); //Envoi la nouvelle position par défaut
    }
    SetPosition(PosX, PosY, Rotate); //Positionne le robot
    TableLimit(PosX, PosY); //Vérifie que le robot est bien sur la table virtuelle
}

void sceneAsservissement::fTime() //Positionne le robot pour la première fois à l'ouverture de l'appli
{
    if (firstTime)
    {
        PosX = PX;
        PosY = PY;
        Rotate = Rot;
        firstTime = false;
        emit sendNewDefaultPos(PosX, PosY, Rotate); //Envoi la nouvelle position par défaut
    }
    SetPosition(PosX, PosY, Rotate); //Positionne le robot
    TableLimit(PosX, PosY); //Vérifie que le robot est bien sur la table virtuelle*/
}
