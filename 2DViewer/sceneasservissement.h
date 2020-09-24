#ifndef SCENEASSERVISSEMENT_H
#define SCENEASSERVISSEMENT_H

#include "includes.h"

#define LONGUEUR_TABLE 2907
#define LARGEUR_TABLE 2000
#define LONGUEUR_ROBOT1 240
#define LARGEUR_ROBOT1 240

#define TAILLE_GOB_X 70
#define TAILLE_GOB_Y 70

#define ORIGINE_X -70
#define ORIGINE_Y 10

#define OBSTACLE_1_X 840
#define OBSTACLE_1_Y 1850

#define OBSTACLE_2_X 1420
#define OBSTACLE_2_Y 1700

#define OBSTACLE_3_X 2010
#define OBSTACLE_3_Y 1850

QT_BEGIN_NAMESPACE
namespace Ui {
class sceneAsservissement;
}
class TwoDV;
class tabCommandes;
QT_END_NAMESPACE

class sceneAsservissement : public QGroupBox
{
    Q_OBJECT

public:
    explicit sceneAsservissement(QWidget *parent = nullptr);
    ~sceneAsservissement();

    double PosX = 0, PosY = 0, Rotate = 0, Vitesse = 1.0;
    double PX = 0, PY = 0, Rot = 0;
    int16_t Distance = 100, Rotation = 15; //Consignes du tableau de commandesRotation = 0, Vitesse = 0;
    bool Mode = false;

    void TableLimit(double Px, double Py);
    void ReadPosition();
    void SetPosition(double x, double y, double r);
    void SelectionQuadrantSensP(double teta); //Calcul le sens de déplacement avec un angle positif
    void SelectionQuadrantSensN(double teta); //Calcul le sens de déplacement avec un angle négatifdqz

signals:
    void sendInfosPosRobot(double x, double y, double r);
    void sendNewDefaultPos(double x, double y, double r);
    void sendFrameControl(uint ID, int16_t DATA);

public slots:
    void SetView(); //Actualise la vue
    void getMode(bool state);
    void getRotation(double r);
    void getDistance(double d);
    void getVitesse(double v);
    void getNewPosX(double x);
    void getNewPosY(double y);
    void getNewAngle(double a);

private:
    Ui::sceneAsservissement *asserv_ui;

    TwoDV *infosTwoDV;
    tabCommandes *commandes;

    QGraphicsScene *sceneAsservView; //Scene pour voir le robot se déplacer
    QGraphicsPixmapItem *map; //Image de la table
    QGraphicsPixmapItem *robot1; //Image de la table
    QGraphicsLineItem *bordures[4];  //Bordures de la table
    QGraphicsLineItem *obstacles[3];  //Obstacles de la table
    QGraphicsTextItem *mousePosTxt; //Pour avoir la position de la souris dans sceneAsserv

    QPoint varMousePos = {0, 0};
    bool firstTime = true;

    enum nomBordures
    {
        NORD, SUD, OUEST, EST
    };
    enum nomObstacles
    {
        GAUCHE, MILIEU, DROITE
    };

    void fTime();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void keyPressEvent(QKeyEvent *keyevent) override;
};

#endif // SCENEASSERVISSEMENT_H
