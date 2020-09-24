#include "mainwindow.h"
#include "sendframebox.h"
#include "connectdialog.h"
#include "twodv.h"

#include "ui_mainwindow.h"
#include "ui_sendframebox.h"
#include "ui_connectdialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), m_ui(new Ui::MainWindow),
    m_busStatusTimer(new QTimer(this))
{

    m_ui->setupUi(this);

    ifstream flux(paramCANPath);
    if(flux) for(int i=0; i<SIZE_TAB; i++) flux >> defSet[i];
    else QMessageBox::warning(this,"Erreur","Impossible d'ouvrir les réglages CAN !");

    m_connectDialog = new ConnectDialog; //Créer une nouvelle fenêtre de dialog (ici pour connecter un périphérique au busCAN)
    m_status = new QLabel; //Nouveau lable pour écrire le status
    m_ui->statusBar->addPermanentWidget(m_status); //Ajout d'une fenêtre permanente dans la bar d'état du parent (ConnectDialog)
    m_written = new QLabel; //Nouveau label pour écrire le nombre de trame émise
    m_ui->statusBar->addWidget(m_written); //Ajout d'une fenêtre dans la barre d'état du parent (ConnectDialog)

    initActionsConnections(); //Initialise les actions (voir ligne XX pour plus d'informations)

    QTimer::singleShot(50, m_connectDialog, &ConnectDialog::show); //Timer de 50ms qui montre le parent (ConnectDialog)

    twoDV = new TwoDV(this); //Nouvelle fenêtre
    twoDV->show(); //Montrer la fenêtre

    connect(m_busStatusTimer, &QTimer::timeout, this, &MainWindow::busStatus); //A chaque coup d'horloge, affiche le status
    if(defSet[5]=="1") QTimer::singleShot(500, this, SLOT(closeMain())); //Ferme la fenêtre (si le paramètre l'autorise)
}

MainWindow::~MainWindow() //Supprime
{
    delete m_connectDialog;
    delete m_ui;
}

void MainWindow::initActionsConnections()
{
    //Etat des objets au début
    m_ui->actionDisconnect->setEnabled(false);
    m_ui->sendFrameBox->setEnabled(false);

    //Actions de la barre d'outils
    connect(m_ui->sendFrameBox, &SendFrameBox::sendFrame, this, &MainWindow::sendFrame);
    connect(m_ui->actionConnect, &QAction::triggered, [this]()
    {
        m_canDevice.release()->deleteLater();
        m_connectDialog->show();
    });
    connect(m_connectDialog, &QDialog::accepted, this, &MainWindow::connectDevice);
    connect(m_ui->actionDisconnect, &QAction::triggered, this, &MainWindow::disconnectDevice);
    connect(m_ui->actionResetController, &QAction::triggered, this, [this]()
        {m_canDevice->resetController();});
    connect(m_ui->actionQuit, &QAction::triggered, this, &QWidget::close);
    connect(m_ui->actionAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt);
    connect(m_ui->actionClearLog, &QAction::triggered, m_ui->receivedMessagesEdit, &QTextEdit::clear);
    connect(m_ui->actionPluginDocumentation, &QAction::triggered, this, []()
        {QDesktopServices::openUrl(QUrl("http://doc.qt.io/qt-5/qtcanbus-backends.html#can-bus-plugins"));});
}

void MainWindow::processErrors(QCanBusDevice::CanBusError error) const //Détecteur d'erreur sur le bus CAN
{
    switch (error)
    {
    case QCanBusDevice::ReadError:
    case QCanBusDevice::WriteError:
    case QCanBusDevice::ConnectionError:
    case QCanBusDevice::ConfigurationError:
    case QCanBusDevice::UnknownError:
        m_status->setText(m_canDevice->errorString());
        break;
    default:
        break;
    }
}

void MainWindow::connectDevice() //Connecter le périphérique
{
    const ConnectDialog::Settings p = m_connectDialog->settings(); //Récupère les paramètres définit par l'utilisateur

    QString errorString;
    m_canDevice.reset(QCanBus::instance()->createDevice(p.pluginName, p.deviceInterfaceName, &errorString));
    if (!m_canDevice) //S'il n'y a pas de périphérique
    {
        m_status->setText(tr("Error creating device '%1', reason: '%2'").arg(p.pluginName).arg(errorString));
        return;
    }
    m_numberFramesWritten = 0;

    //Reçoit des infos du bus CAN
    connect(m_canDevice.get(), &QCanBusDevice::errorOccurred,this, &MainWindow::processErrors); //Erreur
    connect(m_canDevice.get(), &QCanBusDevice::framesReceived,this, &MainWindow::processReceivedFrames); //Trame reçu
    connect(m_canDevice.get(), &QCanBusDevice::framesWritten,this, &MainWindow::processFramesWritten); //trame envoyée

    if (p.useConfigurationEnabled) //Si on utilise les paramètres avancés
    {
        for (const ConnectDialog::ConfigurationItem &item : p.configurations) //Pour chaque item
            m_canDevice->setConfigurationParameter(item.first, item.second); //Applique la configuration
    }

    if (!m_canDevice->connectDevice()) //Si le périphérique n'est pas connecté
    {
        m_status->setText(tr("Connection error: %1").arg(m_canDevice->errorString())); //Type d'erreur
        m_canDevice.reset(); //Reset la connexion
    }
    else
    {
        //Les objets changent d'état
        m_ui->actionConnect->setEnabled(false);
        m_ui->actionDisconnect->setEnabled(true);
        m_ui->sendFrameBox->setEnabled(true);

        const QVariant bitRate = m_canDevice->configurationParameter(QCanBusDevice::BitRateKey);
        if (bitRate.isValid()) //Si le débit est correct
        {
            //Interprète le paramètre FD (Flexible Data)
            const bool isCanFd = m_canDevice->configurationParameter(QCanBusDevice::CanFdKey).toBool(); //1 s'il est utilisé, 0 sinon
            const QVariant dataBitRate = m_canDevice->configurationParameter(QCanBusDevice::DataBitRateKey); //Récupère le débit de donnée
            if (isCanFd && dataBitRate.isValid()) //Si le FD est utilisé
            {
                m_status->setText(tr("Plugin: %1, connected to %2 at %3 / %4 kBit/s").arg(p.pluginName).arg(p.deviceInterfaceName)
                .arg(bitRate.toInt() / 1000).arg(dataBitRate.toInt() / 1000));
            }
            else //Sinon
            {
                m_status->setText(tr("Plugin: %1, connected to %2 at %3 kBit/s").arg(p.pluginName).arg(p.deviceInterfaceName)
                .arg(bitRate.toInt() / 1000));
            }
            //bitrate divisé par 1000 pour l'afficher en kBits/s
        }
        else
        {
            m_status->setText(tr("Plugin: %1, connected to %2").arg(p.pluginName).arg(p.deviceInterfaceName));
        }
        if (m_canDevice->hasBusStatus()) m_busStatusTimer->start(2000); //Enclenche un timer toute les 2s pour mettre à jour le status du bus
        else m_ui->busStatus->setText(tr("Non disponible"));
    }
}

void MainWindow::busStatus() //Etat du bus CAN
{
    if (!m_canDevice || !m_canDevice->hasBusStatus())
    {
        m_ui->busStatus->setText(tr("No CAN bus status available."));
        m_busStatusTimer->stop();
        return;
    }

    switch (m_canDevice->busStatus()) //En fonction de l'état
    {
    case QCanBusDevice::CanBusStatus::Good: m_ui->busStatus->setText("Status du bus : Bon.");
        break;
    case QCanBusDevice::CanBusStatus::Warning: m_ui->busStatus->setText("Statusdu bus : Attention.");
        break;
    case QCanBusDevice::CanBusStatus::Error: m_ui->busStatus->setText("Status du bus : Erreur.");
        break;
    case QCanBusDevice::CanBusStatus::BusOff: m_ui->busStatus->setText("Status du bus : Absent.");
        break;
    default: m_ui->busStatus->setText("Status du bus : Inconnu.");
        break;
    }
}

void MainWindow::disconnectDevice() //Déconnecter le périphérique (proprement)
{
    if (!m_canDevice) return;
    m_busStatusTimer->stop();
    m_canDevice->disconnectDevice();
    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setEnabled(false);
    m_ui->sendFrameBox->setEnabled(false);
    m_status->setText(tr("Déconnecté"));
}

void MainWindow::processFramesWritten(qint64 count) //Compteur de trame émise
{
    m_numberFramesWritten += count;
    m_written->setText(tr("%1 trames écrites").arg(m_numberFramesWritten));
}

void MainWindow::closeEvent(QCloseEvent *event) //Action pour fermer la fenêtre
{
    m_connectDialog->close();
    event->accept();
}

static QString frameFlags(const QCanBusFrame &frame) //Récupère le drapeau
{
    QString result = QLatin1String(" --- "); //Format du résuslat
    if (frame.hasBitrateSwitch()) result[1] = QLatin1Char('B'); //B pour bitrate
    if (frame.hasErrorStateIndicator()) result[2] = QLatin1Char('E'); //E pour Error
    if (frame.hasLocalEcho()) result[3] = QLatin1Char('L'); //L pour Local echo
    return result;
}

void MainWindow::processReceivedFrames() //Trame reçu
{
    if (!m_canDevice) return; //S'il n'y a pas de périphérique
    while (m_canDevice->framesAvailable()) //Tant qu'il y aura des trames disponibles sur le bus CAN
    {
        const QCanBusFrame frame = m_canDevice->readFrame(); //Lit
        //Extrait l'ID de la trame
        const char * const idFormat = frame.hasExtendedFrameFormat() ? "%08X" : "%03X";
        uint fid = static_cast<uint>(frame.frameId());
        QString frID = QString::asprintf(idFormat, fid);

        QString view;
        if (frame.frameType() == QCanBusFrame::ErrorFrame) view = m_canDevice->interpretErrorFrame(frame); //Si erreur
        else view = frame.toString(); //Sinon, on prépare la trame pour l'afficher
        const QString time = QString::fromLatin1("%1.%2  ").arg(frame.timeStamp().seconds(), 10, 10, QLatin1Char(' '))
        .arg(frame.timeStamp().microSeconds() / 100, 4, 10, QLatin1Char('0')); //Prépare l'heure de réception  pour l'afficher
        const QString flags = frameFlags(frame); //Prépare le drapeau pour l'afficher
        m_ui->receivedMessagesEdit->append(QString("%0           %1 %2").arg(time).arg(flags).arg(view)); //Affiche chaque élément vu ci-dessus

        if(twoDV->activeMode()) //Si mode temps réel
        {
            //Analyse chaque ID dans twoDV
            if(frID == "028") twoDV->setCanPosRobot(frame);
            if(frID == "27A") twoDV->setCanVentRobot(frame);
            if(frID == "106") twoDV->setCanServoRobot(frame);
            if(frID == "4B0") twoDV->setCanColorsRobot(frame);
        }
    }
}

void MainWindow::sendFrame(const QCanBusFrame &frame) const //Envoi une trame
{
    if (!m_canDevice) return;
    m_canDevice->writeFrame(frame);
}

void MainWindow::on_actionShow2DV_triggered() //Afficher la fenêtre twoDV
{
    twoDV->show(); //Montre
    twoDV->activateWindow(); //Met au premier plan
}

void MainWindow::closeMain() {MainWindow::close();} //Ferme la fenêtre mainwindow
