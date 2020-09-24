#include "connectdialog.h"

#include "ui_connectdialog.h"

ConnectDialog::ConnectDialog(QWidget *parent) :QDialog(parent),
    m_ui(new Ui::ConnectDialog)
{
    m_ui->setupUi(this);

    ifstream flux(paramCANPath);
    if(flux) for(int i=0; i<SIZE_TAB; i++) flux >> defSet[i];
    else QMessageBox::warning(this,"Erreur","Impossible d'ouvrir les réglages CAN !");
    connect(m_ui->CB_defSet, &QCheckBox::clicked, [this]()
    {
        ofstream flux(paramCANPath);
        defSet[0] = m_ui->CB_defSet->isChecked() ? "1" : "0";
        if(flux) for(int i=0; i<SIZE_TAB; i++) flux << defSet[i] << endl;
        else QMessageBox::warning(this,"Erreur","Impossible d'enregistrer les réglages CAN !");
    });
    defSet[0]=="1" ? m_ui->CB_defSet->setChecked(1) : m_ui->CB_defSet->setChecked(0);

    m_ui->errorFilterEdit->setValidator(new QIntValidator(0, 0x1FFFFFFFU, this));
    m_ui->loopbackBox->addItem(tr("Non spécifié"), QVariant());
    m_ui->loopbackBox->addItem(tr("Non"), QVariant(false));
    m_ui->loopbackBox->addItem(tr("Oui"), QVariant(true));
    m_ui->receiveOwnBox->addItem(tr("Non spécifié"), QVariant());
    m_ui->receiveOwnBox->addItem(tr("Non"), QVariant(false));
    m_ui->receiveOwnBox->addItem(tr("Oui"), QVariant(true));
    m_ui->canFdBox->addItem(tr("Non"), QVariant(false));
    m_ui->canFdBox->addItem(tr("Oui"), QVariant(true));
    m_ui->dataBitrateBox->setFlexibleDateRateEnabled(true);

    connect(m_ui->okButton, &QPushButton::clicked, this, &ConnectDialog::ok);
    connect(m_ui->cancelButton, &QPushButton::clicked, this, &ConnectDialog::cancel);
    connect(m_ui->useConfigurationBox, &QCheckBox::clicked,m_ui->configurationBox, &QGroupBox::setEnabled);
    connect(m_ui->pluginListBox, &QComboBox::currentTextChanged,this, &ConnectDialog::pluginChanged);
    connect(m_ui->interfaceListBox, &QComboBox::currentTextChanged,this, &ConnectDialog::interfaceChanged);

    m_ui->rawFilterEdit->hide();
    m_ui->rawFilterLabel->hide();
    m_ui->pluginListBox->addItems(QCanBus::instance()->plugins());
    updateSettings();

    if(defSet[0]=="1")
    {
        m_ui->pluginListBox->setCurrentText(QString::fromStdString(defSet[1]));
        m_ui->interfaceListBox->setCurrentText(QString::fromStdString(defSet[2]));
    }
    defSet[0]=="1" ? m_ui->useConfigurationBox->setChecked(1) : m_ui->useConfigurationBox->setChecked(0);
    defSet[0]=="1" ? m_ui->configurationBox->setEnabled(1) :m_ui->configurationBox->setEnabled(0);
    if(defSet[0]=="1") m_ui->bitrateBox->setCurrentText(QString::fromStdString(defSet[3]));
    if(defSet[4]=="1") QTimer::singleShot(1000, this, SLOT(ok()));
    if(defSet[5]=="1") QTimer::singleShot(500, this, SLOT(cancel()));
}

ConnectDialog::~ConnectDialog()
{
    delete m_ui;
}

ConnectDialog::Settings ConnectDialog::settings() const
{
    return m_currentSettings;
}

void ConnectDialog::pluginChanged(const QString &plugin)
{
    m_ui->interfaceListBox->clear();
    m_interfaces = QCanBus::instance()->availableDevices(plugin);
    for (const QCanBusDeviceInfo &info : qAsConst(m_interfaces)) m_ui->interfaceListBox->addItem(info.name());
}

void ConnectDialog::interfaceChanged(const QString &inter)
{
    m_ui->isVirtual->setChecked(false);
    m_ui->isFlexibleDataRateCapable->setChecked(false);

    for (const QCanBusDeviceInfo &info : qAsConst(m_interfaces))
    {
        if (info.name() == inter)
        {
            m_ui->descriptionLabel->setText(info.description());
            QString serialNumber = info.serialNumber();
            if (serialNumber.isEmpty()) serialNumber = tr("n/a");
            m_ui->serialNumberLabel->setText(tr("Serial: %1").arg(serialNumber));
            m_ui->channelLabel->setText(tr("Channel: %1").arg(info.channel()));
            m_ui->isVirtual->setChecked(info.isVirtual());
            m_ui->isFlexibleDataRateCapable->setChecked(info.hasFlexibleDataRate());
            break;
        }
    }
}

void ConnectDialog::ok()
{
    updateSettings();
    accept();
}

void ConnectDialog::cancel()
{
    revertSettings();
    reject();
}

QString ConnectDialog::configurationValue(QCanBusDevice::ConfigurationKey key)
{
    QVariant result;
    for (const ConfigurationItem &item : qAsConst(m_currentSettings.configurations))
    {
        if (item.first == key)
        {
            result = item.second;
            break;
        }
    }

    if (result.isNull() && (key == QCanBusDevice::LoopbackKey ||key == QCanBusDevice::ReceiveOwnKey))
    {
        return tr("Non spécifié");
    }
    return result.toString();
}

void ConnectDialog::revertSettings()
{
    m_ui->pluginListBox->setCurrentText(m_currentSettings.pluginName);
    m_ui->interfaceListBox->setCurrentText(m_currentSettings.deviceInterfaceName);
    m_ui->useConfigurationBox->setChecked(m_currentSettings.useConfigurationEnabled);

    QString value = configurationValue(QCanBusDevice::LoopbackKey);
    m_ui->loopbackBox->setCurrentText(value);

    value = configurationValue(QCanBusDevice::ReceiveOwnKey);
    m_ui->receiveOwnBox->setCurrentText(value);

    value = configurationValue(QCanBusDevice::ErrorFilterKey);
    m_ui->errorFilterEdit->setText(value);

    value = configurationValue(QCanBusDevice::BitRateKey);
    m_ui->bitrateBox->setCurrentText(value);

    value = configurationValue(QCanBusDevice::CanFdKey);
    m_ui->canFdBox->setCurrentText(value);

    value = configurationValue(QCanBusDevice::DataBitRateKey);
    m_ui->dataBitrateBox->setCurrentText(value);
}

void ConnectDialog::updateSettings()
{
    m_currentSettings.pluginName = m_ui->pluginListBox->currentText();
    m_currentSettings.deviceInterfaceName = m_ui->interfaceListBox->currentText();
    m_currentSettings.useConfigurationEnabled = m_ui->useConfigurationBox->isChecked();

    if (m_currentSettings.useConfigurationEnabled)
    {
        m_currentSettings.configurations.clear();
        //Process LoopBack
        if (m_ui->loopbackBox->currentIndex() != 0)
        {
            ConfigurationItem item;
            item.first = QCanBusDevice::LoopbackKey;
            item.second = m_ui->loopbackBox->currentData();
            m_currentSettings.configurations.append(item);
        }
        //Process ReceiveOwnKey
        if (m_ui->receiveOwnBox->currentIndex() != 0)
        {
            ConfigurationItem item;
            item.first = QCanBusDevice::ReceiveOwnKey;
            item.second = m_ui->receiveOwnBox->currentData();
            m_currentSettings.configurations.append(item);
        }
        //Process error filter
        if (!m_ui->errorFilterEdit->text().isEmpty())
        {
            QString value = m_ui->errorFilterEdit->text();
            bool ok = false;
            int dec = value.toInt(&ok);
            if (ok)
            {
                ConfigurationItem item;
                item.first = QCanBusDevice::ErrorFilterKey;
                item.second = QVariant::fromValue(QCanBusFrame::FrameErrors(dec));
                m_currentSettings.configurations.append(item);
            }
        }
        //Process raw filter list
        if (!m_ui->rawFilterEdit->text().isEmpty())
        {
            //TODO current ui not sfficient to reflect this param
        }
        //Process bitrate
        const int bitrate = m_ui->bitrateBox->bitRate();
        if (bitrate > 0)
        {
            const ConfigurationItem item(QCanBusDevice::BitRateKey, QVariant(bitrate));
            m_currentSettings.configurations.append(item);
        }
        //Process CAN FD setting
        ConfigurationItem fdItem;
        fdItem.first = QCanBusDevice::CanFdKey;
        fdItem.second = m_ui->canFdBox->currentData();
        m_currentSettings.configurations.append(fdItem);
        //Process data bitrate
        const int dataBitrate = m_ui->dataBitrateBox->bitRate();
        if (dataBitrate > 0)
        {
            const ConfigurationItem item(QCanBusDevice::DataBitRateKey, QVariant(dataBitrate));
            m_currentSettings.configurations.append(item);
        }
    }
}
