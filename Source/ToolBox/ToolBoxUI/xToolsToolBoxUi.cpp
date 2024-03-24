﻿/***************************************************************************************************
 * Copyright 2023-2024 x-tools-author(x-tools@outlook.com). All rights reserved.
 *
 * The file is encoded using "utf8 with bom", it is a part of xTools project.
 *
 * xTools is licensed according to the terms in the file LICENCE(GPL V3) in the root of the source
 * code directory.
 **************************************************************************************************/
#include "xToolsToolBoxUi.h"
#include "ui_xToolsToolBoxUi.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QMessageBox>
#include <QMetaEnum>
#include <QRegularExpression>

#include "xToolsCommunicationTool.h"
#include "xToolsCrcInterface.h"
#include "xToolsDataStructure.h"
#include "xToolsEmitterToolUi.h"
#include "xToolsInterface.h"
#include "xToolsPrestorerToolUi.h"
#include "xToolsResponserToolUi.h"
#include "xToolsSerialPortToolUi.h"
#include "xToolsSerialPortTransmitterToolUi.h"
#include "xToolsSettings.h"
#include "xToolsSocketClientToolUi.h"
#include "xToolsSocketServerToolUi.h"
#include "xToolsTcpTransmitterToolUi.h"
#include "xToolsToolBoxUiCommunicationMenu.h"
#include "xToolsToolBoxUiInputMenu.h"
#include "xToolsToolBoxUiOutputMenu.h"
#include "xToolsToolFactory.h"
#include "xToolsUdpTransmitterToolUi.h"
#include "xToolsUiInterface.h"
#include "xToolsWebSocketTransmitterToolUi.h"

#ifdef X_TOOLS_IMPORT_MODULE_BLUETOOTH
#include "xToolsBleCentralToolUi.h"
#endif

xToolsToolBoxUi::xToolsToolBoxUi(QWidget* parent)
    : QWidget{parent}
    , ui(new Ui::xToolsToolBoxUi)
{
    ui->setupUi(this);
    m_toolBox = new xToolsToolBox(this);

    m_cycleSendingTimer = new QTimer(this);
    connect(m_cycleSendingTimer, &QTimer::timeout, this, [=]() {
        if (ui->comboBoxInputText->currentText().isEmpty()) {
            setDefaultText();
        }

        try2send();
    });
}

xToolsToolBoxUi::~xToolsToolBoxUi()
{
    delete ui;
}

QList<int> xToolsToolBoxUi::supportedCommunicationTools()
{
    QList<int> list;
    list << xToolsToolFactory::SerialportTool
#ifdef X_TOOLS_IMPORT_MODULE_BLUETOOTH
         << xToolsToolFactory::BleCentralTool
#endif
         << xToolsToolFactory::UdpClientTool << xToolsToolFactory::UdpServerTool
         << xToolsToolFactory::TcpClientTool << xToolsToolFactory::TcpServerTool
         << xToolsToolFactory::WebSocketClientTool << xToolsToolFactory::WebSocketServerTool;
    return list;
}

QString xToolsToolBoxUi::communicationToolName(int type)
{
    if (type == xToolsToolFactory::SerialportTool) {
        return tr("SerialPort");
    } else if (type == xToolsToolFactory::UdpClientTool) {
        return tr("UDP Client");
    } else if (type == xToolsToolFactory::UdpServerTool) {
        return tr("UDP Server");
    } else if (type == xToolsToolFactory::TcpClientTool) {
        return tr("TCP Client");
    } else if (type == xToolsToolFactory::TcpServerTool) {
        return tr("TCP Server");
    } else if (type == xToolsToolFactory::WebSocketClientTool) {
        return tr("WebSocket Client");
    } else if (type == xToolsToolFactory::WebSocketServerTool) {
        return tr("WebSocket Server");
    } else if (type == xToolsToolFactory::BleCentralTool) {
        return tr("BLE Central");
    } else {
        return "Unknow";
    }
}

QIcon xToolsToolBoxUi::communicationToolIcon(int type)
{
    QString fileName;
    if (type == xToolsToolFactory::SerialportTool) {
        fileName = ":/Resources/Icons/IconSerialPort.svg";
    } else if (type == xToolsToolFactory::UdpClientTool) {
        fileName = ":/Resources/Icons/IconUdpClient.svg";
    } else if (type == xToolsToolFactory::UdpServerTool) {
        fileName = ":/Resources/Icons/IconUdpServer.svg";
    } else if (type == xToolsToolFactory::TcpClientTool) {
        fileName = ":/Resources/Icons/IconTcpClient.svg";
    } else if (type == xToolsToolFactory::TcpServerTool) {
        fileName = ":/Resources/Icons/IconTcpServer.svg";
    } else if (type == xToolsToolFactory::WebSocketClientTool) {
        fileName = ":/Resources/Icons/IconWebSocketClient.svg";
    } else if (type == xToolsToolFactory::WebSocketServerTool) {
        fileName = ":/Resources/Icons/IconWebSocketServer.svg";
    } else if (type == xToolsToolFactory::BleCentralTool) {
        fileName = ":/Resources/Icons/IconBlueTooth.svg";
    }

    QIcon icon(fileName);
    return icon;
}

void xToolsToolBoxUi::initialize(int type)
{
    if (m_communication) {
        m_communication->deleteLater();
        m_communication = nullptr;
    }

    m_toolBox->initialize(type);
    m_communicationType = type;

    QString toolName = communicationToolName(type);
    QIcon icon = communicationToolIcon(type);
    setWindowTitle(toolName);
    setWindowIcon(icon);

    m_communication = m_toolBox->getCommunicationTool();
    if (!m_communication) {
        qWarning() << "initializing failed, "
                                       "tool box is invaliad!";
        return;
    }

    init();
}

xToolsCommunicationToolUi* xToolsToolBoxUi::communicationToolUi(int type)
{
    xToolsCommunicationToolUi* w = nullptr;
    if (type == xToolsToolFactory::SerialportTool) {
        w = new xToolsSerialPortToolUi();
    } else if (type == xToolsToolFactory::UdpClientTool) {
        w = new xToolsSocketClientToolUi();
    } else if (type == xToolsToolFactory::UdpServerTool) {
        w = new xToolsSocketServerToolUi();
    } else if (type == xToolsToolFactory::TcpClientTool) {
        w = new xToolsSocketClientToolUi();
    } else if (type == xToolsToolFactory::TcpServerTool) {
        w = new xToolsSocketServerToolUi();
    } else if (type == xToolsToolFactory::WebSocketClientTool) {
        w = new xToolsSocketClientToolUi();
    } else if (type == xToolsToolFactory::WebSocketServerTool) {
        w = new xToolsSocketServerToolUi();
    }
#ifdef X_TOOLS_IMPORT_MODULE_BLUETOOTH
    else if (type == xToolsToolFactory::BleCentralTool) {
        w = new xToolsBleCentralToolUi();
    }
#endif
    else {
        qWarning() << "unknow type of communication tool ui!";
    }

    return w;
}

void xToolsToolBoxUi::try2send()
{
    auto ctx = m_inputMenu->parameters();
    int prefix = ctx.prefix;
    int suffix = ctx.suffix;
    int esc = ctx.escapeCharacter;

    QByteArray prefixData = xToolsDataStructure::affixesData(prefix);
    QByteArray suffixData = xToolsDataStructure::affixesData(suffix);

    QString input = ui->comboBoxInputText->currentText();
    input = xToolsDataStructure::cookedString(esc, input);
    int format = ui->comboBoxInputFormat->currentData().toInt();
    QByteArray bytes = xToolsInterface::string2array(input, format);
    if (ctx.appendCrc) {
        QByteArray b = calculateCrc(bytes);
        bytes.append(b);
    }

    bytes.prepend(prefixData);
    bytes.append(suffixData);
    m_toolBox->getCommunicationTool()->inputBytes(bytes);
}

QString xToolsToolBoxUi::dateTimeFormat()
{
    static QString dateFormat = QLocale::system().dateFormat();

    QString dateTimeFormat = "";
    bool d = ui->checkBoxOutputDate->isChecked();
    bool t = ui->checkBoxOutputTime->isChecked();
    bool m = ui->checkBoxOutputMs->isChecked();

    if ((!d) && (!t) && (!m)) {
        return QString("");
    }

    if (d && t && m) {
        dateTimeFormat = dateFormat + " hh:mm:ss.zzz";
    } else if (d && t) {
        dateTimeFormat = dateFormat + " hh:mm:ss";
    } else if (t && m) {
        dateTimeFormat = " hh:mm:ss.zzz";
    } else if (d && m) {
        dateTimeFormat = dateFormat + " zzz";
    } else if (d) {
        dateTimeFormat = dateFormat;
    } else if (t) {
        dateTimeFormat = "hh:mm:ss";
    } else if (m) {
        dateTimeFormat = "zzz";
    }

    return QDateTime::currentDateTime().toString(dateTimeFormat);
}

void xToolsToolBoxUi::output2ui(const QByteArray& bytes, const QString& flag, bool isRx)
{
    int format = ui->comboBoxOutputFormat->currentData().toInt();
    QString str = xToolsInterface::arrayToString(bytes, format);

    if (!str.contains(m_outputMenu->filter())) {
        return;
    }

    QString dt = dateTimeFormat();
    QString flags = isRx ? "Rx" : "Tx";
    QString color = isRx ? "red" : "blue";

    flags = QString("<font color=%1>%2</font>").arg(color, flag);
    QString info;
    if (dt.isEmpty()) {
        info = QString("[%1]").arg(flags);
    } else {
        dt = QString("<font color=silver>%1</font>").arg(dt);
        info = QString("[%1 %2]").arg(dt, flags);
    }

    info = QString("<font color=silver>%1</font>").arg(info);

    ui->textBrowserOutput->append(info + " " + str);
}

QString xToolsToolBoxUi::settingsGroup()
{
    if (m_communicationType == xToolsToolFactory::SerialportTool) {
        return "SerialportToolBox";
    } else if (m_communicationType == xToolsToolFactory::UdpClientTool) {
        return "UdpClientToolBox";
    } else if (m_communicationType == xToolsToolFactory::UdpServerTool) {
        return "UdpServerTool";
    } else if (m_communicationType == xToolsToolFactory::TcpClientTool) {
        return "TcpClientTool";
    } else if (m_communicationType == xToolsToolFactory::TcpServerTool) {
        return "TcpServerToolBox";
    } else if (m_communicationType == xToolsToolFactory::WebSocketClientTool) {
        return "WebSocketClientToolBox";
    } else if (m_communicationType == xToolsToolFactory::WebSocketServerTool) {
        return "WebSocketServerToolBox";
    } else if (m_communicationType == xToolsToolFactory::BleCentralTool) {
        return "BleCentralToolBox";
    } else {
        qWarning() << "unknow type of communication tool ui:" << m_communicationType;
        return "UnknowToolBox";
    }
}

QByteArray xToolsToolBoxUi::calculateCrc(const QByteArray& bytes, bool fixedOriginOrder)
{
    auto ctx = m_inputMenu->parameters();
    QByteArray inputBytes = bytes;
    if (inputBytes.isEmpty()) {
        int format = ui->comboBoxInputFormat->currentData().toInt();
        QString input = ui->comboBoxInputText->currentText();
        int esc = ctx.escapeCharacter;

        input = xToolsDataStructure::cookedString(esc, input);
        inputBytes = xToolsInterface::string2array(input, format);
    }

    int algorithm = ctx.algorithm;
    int startIndex = ctx.startIndex;
    int endIndex = ctx.endIndex;
    bool bigEndian = fixedOriginOrder ? true : ctx.bigEndian;
    xToolsCrcInterface crci;
    QByteArray crc = crci.calculateBytes(inputBytes, algorithm, startIndex, endIndex, bigEndian);
    return crc;
}

void xToolsToolBoxUi::setDefaultText()
{
    QByteArray ba("(null)");
    int format = ui->comboBoxInputFormat->currentData().toInt();
    QString str = xToolsInterface::arrayToString(ba, format);
    ui->comboBoxInputText->setCurrentText(str);
}

void xToolsToolBoxUi::onIsWorkingChanged()
{
    bool isWorking = m_toolBox->isWorking();
    ui->pushButtonInputSend->setEnabled(isWorking);
    ui->comboBoxInputIntervel->setEnabled(isWorking);
    QTimer::singleShot(1000, this, [=]() { ui->pushButtonCommunicationOpen->setEnabled(true); });

    if (isWorking) {
        ui->pushButtonCommunicationOpen->setText(tr("Close"));
    } else {
        ui->pushButtonCommunicationOpen->setText(tr("Open"));
    }

    if (!isWorking) {
        m_cycleSendingTimer->stop();
    }
}

void xToolsToolBoxUi::onBytesWritten(const QByteArray& bytes, const QString& from)
{
    if (!ui->checkBoxOutputTx->isChecked()) {
        return;
    }

    output2ui(bytes, from, false);
}

void xToolsToolBoxUi::onBytesRead(const QByteArray& bytes, const QString& to)
{
    if (!ui->checkBoxOutputRx->isChecked()) {
        return;
    }

    output2ui(bytes, to, true);
}

void xToolsToolBoxUi::onInputTextChanged()
{
    QByteArray b = calculateCrc(QByteArray(), true);
    QString crc = QString::fromLatin1(b.toHex());
    crc = "0x" + crc.toUpper();
    ui->labelCrc->setText(crc);
    ui->labelAlgorithm->setText(m_inputMenu->parameters().algorithmName);
}

void xToolsToolBoxUi::init()
{
    m_settingsKey.tabIndex = settingsGroup() + "/tabIndex";
    m_settingsKey.items = settingsGroup() + "/items";
    m_settingsKey.transmitterTabIndex = settingsGroup() + "/transmitterTabIndex";

    initUi();
    initSettings();
    initSignals();
    initTools();

    onIsWorkingChanged();
    onInputTextChanged();
    onComboBoxInputFormatActivated();
}

void xToolsToolBoxUi::initUi()
{
    initUiCommunication();
    initUiInput();
    initUiOutput();

    QSettings* settings = xToolsSettings::instance();
    int index = settings->value(m_settingsKey.tabIndex).toInt();
    if (index >= 0 && index < ui->tabWidget->count()) {
        ui->tabWidget->setCurrentIndex(index);
    }
}

void xToolsToolBoxUi::initUiCommunication()
{
    // Setup communication tool.
    m_communicationUi = communicationToolUi(m_communicationType);
    if (!m_communicationUi) {
        qWarning() << "mCommunicationToolUi is nullptr";
        return;
    }

    // Clear widget.
    auto l = ui->widgetCommunicationToolUi->layout();
    if (l) {
        while (!l->isEmpty()) {
            auto item = l->itemAt(0);
            l->removeItem(item);
        }
    } else {
        ui->widgetCommunicationToolUi->setLayout(new QHBoxLayout());
        auto lt = ui->widgetCommunicationToolUi->layout();
        lt->setContentsMargins(0, 0, 0, 0);
    }

    ui->widgetCommunicationToolUi->layout()->addWidget(m_communicationUi);
}

void xToolsToolBoxUi::initUiInput()
{
    ui->comboBoxInputIntervel->addItem(tr("Disable"), -1);
    for (int i = 10; i <= 100; i += 10) {
        ui->comboBoxInputIntervel->addItem(QString::number(i), i);
    }
    for (int i = 200; i <= 1000; i += 100) {
        ui->comboBoxInputIntervel->addItem(QString::number(i), i);
    }
    for (int i = 2000; i <= 5000; i += 1000) {
        ui->comboBoxInputIntervel->addItem(QString::number(i), i);
    }

    QString hex = xToolsSettings::instance()->value(m_settingsKey.items).toString();
    if (!hex.isEmpty()) {
        QString json = QString::fromUtf8(QByteArray::fromHex(hex.toLatin1()));
        QJsonDocument jsonDoc = QJsonDocument::fromJson(json.toUtf8());
        QJsonArray arr = jsonDoc.array();
        for (int i = 0; i < arr.count(); i++) {
            QJsonObject obj = arr.at(i).toObject();
            QString text = obj.value("text").toString();
            int format = obj.value("data").toInt();
            ui->comboBoxInputText->addItem(text, format);
        }
    }

    m_inputMenu = new xToolsToolBoxUiInputMenu(settingsGroup(), this);
    connect(m_inputMenu,
            &xToolsToolBoxUiInputMenu::parametersChanged,
            this,
            &xToolsToolBoxUi::onInputTextChanged);
    ui->pushButtonInputSettings->setMenu(m_inputMenu);
}

void xToolsToolBoxUi::initUiOutput()
{
    ui->checkBoxOutputRx->setChecked(true);
    ui->checkBoxOutputTx->setChecked(true);
    ui->textBrowserOutput->document()->setMaximumBlockCount(2000);

    m_outputMenu = new xToolsToolBoxUiOutputMenu(settingsGroup(),
                                              ui->textBrowserOutput->document(),
                                              this);
    ui->pushButtonOutputSettings->setMenu(m_outputMenu);
}

void xToolsToolBoxUi::initSettings()
{
    initSettingsCommunication();
    initSettingsInput();
    initSettingsOutput();
}

void xToolsToolBoxUi::initSettingsCommunication()
{
    const QString key = settingsGroup() + "/communication";
    m_communicationUi->initialize(m_communication, key);
}

void xToolsToolBoxUi::initSettingsInput()
{
    QString group = settingsGroup();
    ui->comboBoxInputFormat->setGroupKey(group + "/input", "fromat");
    ui->comboBoxInputText->setGroupKey(group + "/input", "text", false);
}

void xToolsToolBoxUi::initSettingsOutput()
{
    QString group = settingsGroup();
    ui->comboBoxOutputFormat->setGroupKey(group + "/output", "outputFormat");
    ui->checkBoxOutputRx->setGroupKey(group + "/output", "outputRx");
    ui->checkBoxOutputTx->setGroupKey(group + "/output", "outputTx");
    ui->checkBoxOutputDate->setGroupKey(group + "/output", "outputDate");
    ui->checkBoxOutputTime->setGroupKey(group + "/output", "outputTime");
    ui->checkBoxOutputMs->setGroupKey(group + "/output", "outputMs");
    ui->checkBoxOutputWrap->setGroupKey(group + "/output", "outputWrap");

#if 1
    ui->checkBoxOutputRx->setChecked(true);
    ui->checkBoxOutputTx->setChecked(true);
    ui->checkBoxOutputWrap->setChecked(true);
#endif
}

void xToolsToolBoxUi::initSignals()
{
    initSignalsCommunication();
    initSignalsInput();
    initSignalsOutput();

    connect(ui->tabWidget,
            &QTabWidget::currentChanged,
            this,
            &xToolsToolBoxUi::onTabWidgetCurrentChanged);
}

void xToolsToolBoxUi::initSignalsCommunication()
{
    connect(ui->pushButtonCommunicationOpen,
            &QPushButton::clicked,
            this,
            &xToolsToolBoxUi::onPushButtonCommunicationOpenClicked);
}

void xToolsToolBoxUi::initSignalsInput()
{
    connect(ui->comboBoxInputIntervel,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
            this,
            &xToolsToolBoxUi::onComboBoxInputIntervalActivated);
    connect(ui->pushButtonInputSend,
            &QPushButton::clicked,
            this,
            &xToolsToolBoxUi::onPushButtonInputSendClicked);
    connect(ui->comboBoxInputFormat,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
            this,
            &xToolsToolBoxUi::onComboBoxInputFormatActivated);
    connect(ui->comboBoxInputText,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
            this,
            &xToolsToolBoxUi::onComboBoxInputTextActivated);
    connect(ui->comboBoxInputText,
            &QComboBox::currentTextChanged,
            this,
            &xToolsToolBoxUi::onInputTextChanged);
}

void xToolsToolBoxUi::initSignalsOutput()
{
    connect(ui->checkBoxOutputWrap,
            &QCheckBox::clicked,
            this,
            &xToolsToolBoxUi::onCheckBoxOutputWrapClicked);
}

void xToolsToolBoxUi::initTools()
{
    m_communicationMenu = new xToolsToolBoxUiCommunicationMenu(this);
    m_communicationMenu->initialize(m_toolBox, settingsGroup());
    ui->pushButtonCommunicationSettings->setMenu(m_communicationMenu);

    auto rxVelometer = m_toolBox->getRxVelometerTool();
    auto txVelometer = m_toolBox->getTxVelometerTool();
    auto rxS = m_toolBox->getRxStatisticianTool();
    auto txS = m_toolBox->getTxStatisticianTool();
    const QString rxVelometerGroup = settingsGroup() + "/rxVelometer";
    const QString txVelometerGroup = settingsGroup() + "/txVelometer";
    const QString rxSGroup = settingsGroup() + "/rxStatistician";
    const QString txSGroup = settingsGroup() + "/txStatistician";
    ui->widgetRxVelometer->initialize(rxVelometer, rxVelometerGroup);
    ui->widgetTxVelometer->initialize(txVelometer, txVelometerGroup);
    ui->widgetRxStatistician->initialize(rxS, rxSGroup);
    ui->widgetTxStatistician->initialize(txS, txSGroup);

    m_emitterUi = new xToolsEmitterToolUi();
    ui->tabEmiter->setLayout(new QVBoxLayout());
    ui->tabEmiter->layout()->addWidget(m_emitterUi);
    m_emitterUi->initialize(m_toolBox->getEmitterTool(), settingsGroup() + "/emitter");

    m_responserUi = new xToolsResponserToolUi();
    ui->tabResponser->setLayout(new QVBoxLayout());
    ui->tabResponser->layout()->addWidget(m_responserUi);
    m_responserUi->initialize(m_toolBox->getResponserTool(), settingsGroup() + "/responser");

    m_prestorerUi = new xToolsPrestorerToolUi();
    ui->tabPrestorer->setLayout(new QVBoxLayout());
    ui->tabPrestorer->layout()->addWidget(m_prestorerUi);
    m_prestorerUi->initialize(m_toolBox->getPrestorerTool(), settingsGroup() + "/prestorer");

    m_tcpTransmitterUi = new xToolsTcpTransmitterToolUi(this);
    m_tcpTransmitterUi->initialize(m_toolBox->getTcpTransmitterTool(),
                                   settingsGroup() + "/tcpTransmitter");
    m_udpTransmitterUi = new xToolsUdpTransmitterToolUi(this);
    m_udpTransmitterUi->initialize(m_toolBox->getUdpTransmitterTool(),
                                   settingsGroup() + "/udpTransmitter");
    m_webSocketTransmitterUi = new xToolsWebSocketTransmitterToolUi(this);
    m_webSocketTransmitterUi->initialize(m_toolBox->getWebSocketTransmitterTool(),
                                         settingsGroup() + "/webSocketTransmitter");
    m_serialPortTransmitterUi = new xToolsSerialPortTransmitterToolUi(this);
    m_serialPortTransmitterUi->initialize(m_toolBox->getSerialPortTransmitterTool(),
                                          settingsGroup() + "/serialPortTransmitter");

    m_tcpTransmitterUi->layout()->setContentsMargins(9, 9, 9, 9);
    m_udpTransmitterUi->layout()->setContentsMargins(9, 9, 9, 9);
    m_webSocketTransmitterUi->layout()->setContentsMargins(9, 9, 9, 9);
    m_serialPortTransmitterUi->layout()->setContentsMargins(9, 9, 9, 9);

    ui->tabWidgetTransmitter->clear();
    ui->tabWidgetTransmitter->addTab(m_serialPortTransmitterUi,
                                     tr("SerialPort", "Transmitter", __LINE__));
    ui->tabWidgetTransmitter->addTab(m_udpTransmitterUi, tr("UDP"));
    ui->tabWidgetTransmitter->addTab(m_tcpTransmitterUi, tr("TCP"));
    ui->tabWidgetTransmitter->addTab(m_webSocketTransmitterUi, tr("WebSocket"));

    const QString key = m_settingsKey.transmitterTabIndex;
    int tabIndex = xToolsSettings::instance()->value(key).toInt();
    if ((tabIndex >= 0) && (tabIndex < ui->tabWidgetTransmitter->count())) {
        ui->tabWidgetTransmitter->setCurrentIndex(tabIndex);
    }

    connect(ui->tabWidgetTransmitter, &QTabWidget::currentChanged, this, [=](int index) {
        xToolsSettings::instance()->setValue(key, index);
    });

    connect(m_toolBox, &xToolsToolBox::isWorkingChanged, this, &xToolsToolBoxUi::onIsWorkingChanged);
    connect(m_toolBox, &xToolsToolBox::errorOccurred, this, [=](const QString& err) {
        QMessageBox::warning(this, tr("Error Occured"), err);
    });

    connect(m_communication,
            &xToolsCommunicationTool::bytesWritten,
            this,
            &xToolsToolBoxUi::onBytesWritten);
    connect(m_communication, &xToolsCommunicationTool::bytesRead, this, &xToolsToolBoxUi::onBytesRead);

    ui->pushButtonPrestorer->setMenu(m_prestorerUi->menu());
}

void xToolsToolBoxUi::onTabWidgetCurrentChanged(int index)
{
    if (m_settingsKey.tabIndex.isEmpty()) {
        return;
    }

    xToolsSettings::instance()->setValue(m_settingsKey.tabIndex, index);
}

void xToolsToolBoxUi::onPushButtonCommunicationOpenClicked()
{
    ui->pushButtonCommunicationOpen->setEnabled(false);
    if (m_toolBox->isWorking()) {
        m_toolBox->close();
    } else {
        m_toolBox->open();
    }
}

void xToolsToolBoxUi::onCheckBoxOutputWrapClicked()
{
    if (ui->checkBoxOutputWrap->isChecked()) {
        ui->textBrowserOutput->setWordWrapMode(QTextOption::WrapAnywhere);
    } else {
        ui->textBrowserOutput->setWordWrapMode(QTextOption::NoWrap);
    }
}

void xToolsToolBoxUi::onPushButtonInputSendClicked()
{
    if (ui->comboBoxInputText->currentText().isEmpty()) {
        qInfo() << "input text is empty,"
                                    "the text will be set as (null)";
        QApplication::beep();
        ui->comboBoxInputText->setFocus();

        setDefaultText();
    }

    QString text = ui->comboBoxInputText->currentText();
    int format = ui->comboBoxInputFormat->currentData().toInt();
    bool isExist = false;
    for (int i = 0; i < ui->comboBoxInputText->count(); i++) {
        QString t = ui->comboBoxInputText->itemText(i);
        int f = ui->comboBoxInputText->itemData(i).toInt();
        if ((text == t) && (format == f)) {
            isExist = true;
            break;
        }
    }

    if (!isExist) {
        ui->comboBoxInputText->addItem(text, format);
        while (ui->comboBoxInputText->count() > 10) {
            ui->comboBoxInputText->removeItem(0);
        }

        QJsonArray arr;
        QJsonObject obj;
        for (int i = 0; i < ui->comboBoxInputText->count(); i++) {
            QString t = ui->comboBoxInputText->itemText(i);
            int f = ui->comboBoxInputText->itemData(i).toInt();
            obj.insert("text", t);
            obj.insert("data", f);
            arr.append(obj);
        }

        QJsonDocument jsonDoc;
        jsonDoc.setArray(arr);
        QByteArray json = jsonDoc.toJson();
        QString hex = QString::fromLatin1(json.toHex());
        xToolsSettings::instance()->setValue(m_settingsKey.items, hex);
    }

    try2send();
}

void xToolsToolBoxUi::onComboBoxInputIntervalActivated()
{
    int interval = ui->comboBoxInputIntervel->currentText().toInt();
    interval = interval < 10 ? 10 : interval;
    qInfo() << "start sending automatically, the interval is:" << interval;

    if (ui->comboBoxInputIntervel->currentIndex() == 0) {
        m_cycleSendingTimer->stop();
    } else {
        m_cycleSendingTimer->start(interval);
    }
}

void xToolsToolBoxUi::onComboBoxInputFormatActivated()
{
    int format = ui->comboBoxInputFormat->currentData().toInt();
    auto lineEdit = ui->comboBoxInputText->lineEdit();
    ui->comboBoxInputText->clear();
    xToolsUiInterface::setValidator(lineEdit, format);
}

void xToolsToolBoxUi::onComboBoxInputTextActivated()
{
    int format = ui->comboBoxInputText->currentData().toInt();
    ui->comboBoxInputFormat->setCurrentIndexFromData(format);
    onComboBoxInputFormatActivated();
}
