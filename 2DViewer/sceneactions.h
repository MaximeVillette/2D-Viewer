#ifndef SCENEACTIONS_H
#define SCENEACTIONS_H

#include "includes.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class sceneActions;
}
QT_END_NAMESPACE

class sceneActions : public QGroupBox
{
    Q_OBJECT

public:
    explicit sceneActions(QWidget *parent = nullptr);
    ~sceneActions();

    void SetView();
    void setVariables(int var, QString nom, QString canID, QString valeur, QString infos);

private:
    Ui::sceneActions *actions_ui;

    QGraphicsScene *sceneActionsView;
    QGraphicsPixmapItem *robot;
    QGraphicsTextItem *infoActionTxt;

    QGraphicsEllipseItem *colors[2][3]; //Cercles pour simuler les capteurs de couleurs
    QGraphicsPixmapItem *airPump[2][3]; //Image d'air pour les ventouses

    enum Ligne
    {
        POSX, POSY, ANG, SERVO, VENT, COLSENS
    };
    enum Colonne
    {
        NOM , CAN_ID, VALEUR , INFOS
    };

    void addVariables(QString nom, QString canID, QString valeur, QString infos);
};

#endif // SCENEACTIONS_H
