#include "sceneactions.h"

#include "ui_sceneactions.h"

sceneActions::sceneActions(QWidget *parent) : QGroupBox(parent),
    actions_ui(new Ui::sceneActions)
{
    actions_ui->setupUi(this);

    sceneActionsView = new QGraphicsScene(this);
    actions_ui->GV_actions->setScene(sceneActionsView);

    QPixmap rob(":/Robot/Images/SailTheWorld/Robot.png");
    QPixmap sfl(":/Robot/Images/SailTheWorld/AirPump.png");

    robot = sceneActionsView->addPixmap(rob);
    robot->setOffset(-robot->boundingRect().center().x(),-robot->boundingRect().center().y());
    robot->setPos(115, 118); //Met le robot au centre du graphicsView, ici le robot ne bougera plus
    robot->setRotation(90); //Permet de voir l'avant à droite de l'écran

    QPen blue(Qt::blue); //Création d'un pinceau de couleur bleue
    blue.setStyle(Qt::SolidLine);
    blue.setWidth(2); //2 pixel de large

    infoActionTxt = sceneActionsView->addText("Avant >>>"); //Applique un texte
    infoActionTxt->setPos(240, -10); //Place la zone de texte en haut à droite du graphicsView

    //Place des objets représentatifs sur le robot
    for(int i=0; i<2; i++)
    {
        for(int j=0; j<3; j++)
        {
            //Capteurs de couleurs
            i==0 ? colors[i][j] = sceneActionsView->addEllipse((i*180)+15, 25+(j*80), 30, 30, blue, Qt::transparent) :
                    colors[i][j] = sceneActionsView->addEllipse(i*180, 25+(j*80), 30, 30, blue, Qt::transparent);
            colors[i][j]->setFlags(QGraphicsItem::ItemIsSelectable); //L'item peut être sélectionné (mais ne peut pas bouger)

            //Ventouses
            airPump[i][j] = sceneActionsView->addPixmap(sfl);
            airPump[i][j]->setOffset(-airPump[i][j]->boundingRect().center().x(),-airPump[i][j]->boundingRect().center().y());
            airPump[i][j]->setScale(0.2); //Met à l'échelle pour avoir une taille résonnable
            if(i) airPump[i][j]->setRotation(180); //Retourne dans l'autre sens en fonction du coté
            i==0 ? airPump[i][j]->setPos((i*280)-30, 35+(j*80)) : airPump[i][j]->setPos((i*250)+10, 35+(j*80));
            airPump[i][j]->setFlags(QGraphicsPixmapItem::ItemIsSelectable); //L'item peut être sélectionné (mais ne peut pas bouger)
        }
    }

    QStringList titres; //Titres pour le tableau
    titres << "Nom" << "CAN ID" << "Valeur" << "Description"; //Définit chaque titre dans l'ordre
    actions_ui->TW_variables->setColumnCount(4); //Définit le nombre de colonne du tableau
    actions_ui->TW_variables->setHorizontalHeaderLabels(titres); //Applique les titres

    //Exemples de déclaration d'informations à afficher
    addVariables("Position X", "", "", "");
    addVariables("Position Y", "", "", "");
    addVariables("Angle", "", "", "");
    addVariables("Servomoteurs", "", "", "");
    addVariables("Ventouses", "", "", "");
    addVariables("Capteurs de couleurs", "", "", "");

    QTimer::singleShot(10, this, &sceneActions::SetView);
}

sceneActions::~sceneActions() {delete actions_ui;}

void sceneActions::SetView()
{
    actions_ui->GV_actions->fitInView(sceneActionsView->sceneRect(), Qt::KeepAspectRatio);
    qDebug() << sceneActionsView->sceneRect();
}

void sceneActions::addVariables(QString nom, QString canID, QString valeur, QString infos) //Ajouter une nouvelle ligne
{
    actions_ui->TW_variables->insertRow(actions_ui->TW_variables->rowCount()); //Insert une nouvelle ligne après la précédente
    //Dans cette nouvelle ligne on affiche les informations selon leurs titres :
    actions_ui->TW_variables->setItem(actions_ui->TW_variables->rowCount()-1, NOM, new QTableWidgetItem(nom)); //Nom de la variable
    actions_ui->TW_variables->setItem(actions_ui->TW_variables->rowCount()-1, CAN_ID, new QTableWidgetItem(canID)); //ID sur le CAN
    actions_ui->TW_variables->setItem(actions_ui->TW_variables->rowCount()-1, VALEUR, new QTableWidgetItem(valeur)); //Valeur de la variable
    actions_ui->TW_variables->setItem(actions_ui->TW_variables->rowCount()-1, INFOS, new QTableWidgetItem(infos)); //Description sur la variable
}
void sceneActions::setVariables(int var, QString nom, QString canID, QString valeur, QString infos) //Modifier une ligne
{
    //var est le numéro de la ligne à modifier
    actions_ui->TW_variables->setItem(var, NOM, new QTableWidgetItem(nom));
    actions_ui->TW_variables->setItem(var, CAN_ID, new QTableWidgetItem(canID));
    actions_ui->TW_variables->setItem(var, VALEUR, new QTableWidgetItem(valeur));
    actions_ui->TW_variables->setItem(var, INFOS, new QTableWidgetItem(infos));
}
