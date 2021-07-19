﻿/*
 * Copyright 2018-2021 Qter(qsaker@qq.com). All rights reserved.
 *
 * The file is encoded using "utf8 with bom", it is a part
 * of QtSwissArmyKnife project.
 *
 * QtSwissArmyKnife is licensed according to the terms in
 * the file LICENCE in the root of the source code directory.
 */
#ifndef SAKDEBUGGER_HH
#define SAKDEBUGGER_HH

#include <QSize>
#include <QTimer>
#include <QLabel>
#include <QMutex>
#include <QWidget>
#include <QCheckBox>
#include <QGroupBox>
#include <QComboBox>
#include <QDateTime>
#include <QMetaEnum>
#include <QTextEdit>
#include <QSettings>
#include <QListWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QHBoxLayout>
#include <QApplication>
#include <QTextBrowser>
#include <QSqlDatabase>

class SAKDataFactory;
class SAKDebuggerDevice;
class SAKCommonCrcInterface;
class SAKDebugPageController;
class SAKDebuggerInput;
class SAKDebuggerOutput;
class SAKDebuggerPlugins;
#ifdef SAK_IMPORT_MODULE_CHARTS
class SAKDebugPageChartsController;
#endif
class SAKPluginDataForwarding;
class SAKDebuggerStatistics;
class SAKDebugPageCommonDatabaseInterface;

namespace Ui {
    class SAKDebugger;
}

/// @brief Debugging page
class SAKDebugger : public QWidget
{
    Q_OBJECT
public:
    SAKDebugger(int type, QString name, QWidget *parent = Q_NULLPTR);
    ~SAKDebugger();

    friend class SAKDebugPageOtherController;
    friend class SAKDebuggerInput;
    friend class SAKDebugPageChartsController;

    /**
     * @brief write: Write data to device
     * @param data: Data need to be written
     */
    void write(QByteArray data);

    /**
     * @brief writeRawData: Write raw data
     * @param rawData: Input text
     * @param textFormat: input text fromat
     */
    void writeRawData(QString rawData, int textFormat);

    /**
     * @brief outputMessage: Output message to ui
     * @param msg: The message that need to be show
     * @param isInfo: true-text color is green, false-text color is red
     */
    void outputMessage(QString msg, bool isInfo = true);

    /**
     * @brief pageType: The type of debugging page.(SAKDataStruct::SAKEnumDebugPageType)
     * @return Type of debugging page
     */
    quint32 pageType();

    /**
     * @brief settingsInstance: Get the settings instance
     * @return Settings instance
     */
    QSettings *settings();

    /**
     * @brief sqlDatabase: Get the database instance
     * @return Database instance
     */
    QSqlDatabase *sqlDatabase();

    /**
     * @brief settingsGroup: Get the settings group
     * @param pageType: The type of debug page
     * @return Settings group
     */
    QString settingsGroup();

    /**
     * @brief otherController: Get SAKDebugPageOtherController instance pointer
     * @return SAKDebugPageOtherController instance pointer
     */
    SAKDebugPageOtherController *otherController();

    /**
     * @brief inputController: Get SAKDebugPageInputController instance pointer
     * @return SAKDebugPageInputController instance pointer
     */
    SAKDebuggerInput *inputController();

#ifdef SAK_IMPORT_MODULE_CHARTS
    /**
     * @brief chartsController: Get SAKDebugPageChartsController instance pointer
     * @return SAKDebugPageChartsController instance pointer
     */
    SAKDebugPageChartsController *chartsController();
#endif

    /**
     * @brief outputController: Get SAKDebugPageOutputController instance pointer
     * @return SAKDebugPageOutputController instance pointer
     */
    SAKDebuggerOutput *outputController();

    /**
     * @brief statisticsController: Get SAKDebugPageStatisticsController instance pointer
     * @return SAKDebugPageStatisticsController instance pointer
     */
    SAKDebuggerStatistics *statisticsController();

    /**
     * @brief deviceController: Get the device controller instance, the controller will be destroy when the page is closed.
     * @return Device controller instance pointer
     */
    SAKDebugPageController *deviceController();

    /**
     * @brief databaseInterface: Get the data base read-write interface.
     * @return The interface.
     */
    SAKDebugPageCommonDatabaseInterface *databaseInterface();

    // Table name
    QString tableNameAutoResponseTable();
    QString tableNamePresettingDataTable();
    QString tableNameTimingSendingTable();
protected:
    /**
     * @brief createDevice: Get the device instance, the device will be destroy when it is closed.
     * @return Device instance pointer
     */
    virtual SAKDebuggerDevice* device() = 0;

    /**
     * @brief controller: Get the device controller of the debugger
     * @return Device controller
     */
    virtual SAKDebugPageController *controller() = 0;

    /**
     * @brief initializePage: Initializing, the function must be called in the constructor of subclass.
     */
    void initializePage();
private:
    bool mIsInitializing;
    int mDebugPageType;
    QTimer mClearInfoTimer;
    QMutex mReadWriteParametersMutex;
    QString mSettingGroup;
    QAction *mRefreshAction;
    SAKDebugPageCommonDatabaseInterface *mDatabaseInterface;

    // Debug page modules
    SAKDebugPageOtherController *mOtherController;
    SAKDebuggerInput *mInputController;
#ifdef SAK_IMPORT_MODULE_CHARTS
    SAKDebugPageChartsController *mChartsController;
#endif
    SAKDebuggerOutput *mOutputController;
    SAKDebuggerStatistics *mStatistics;
    SAKDebuggerDevice *mDevice;
    SAKDebugPageController *mDeviceController;
    SAKDebuggerPlugins *mPlugins;
private:
    void cleanInfo();
    void changedDeviceState(bool opened);
    void openDevice();
    void closeDevice();
    void refreshDevice();
signals:
    // Emit the read data
    void bytesRead(QByteArray data);
    // Emit the written data
    void bytesWritten(QByteArray data);
    void requestWriteData(QByteArray data);
    void requestWriteRawData(QString data, int textFormat);
    // ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
    // ui component
private:
    Ui::SAKDebugger *mUi;
private:
    // All variable about ui will be initialize in the function
    void initializingVariables();
    /*************************************************************************/
    // Device control module
protected:
    QPushButton *mSwitchPushButton;
    QPushButton *mDeviceMorePushButton;
    QWidget *mControllerWidget;
private slots:
    void on_switchPushButton_clicked();
    /*************************************************************************/
    // Data input settings module
protected:
    QComboBox *mCyclingTimeComboBox;
    QComboBox *mInputFormatComboBox;
    QPushButton *mMoreInputSettingsPushButton;
    QPushButton *mSendPushButton;
    QTextEdit *mInputTextEdit;
    QLabel *mCrcLabel;
    QPushButton *mAddInputItemPushButton;
    QPushButton *mDeleteInputItemPushButton;
    QListWidget *mInputDataItemListWidget;
    /*************************************************************************/
    // Message output module
protected:
    QLabel *mInfoLabel;
    /*************************************************************************/
    // Data output module
protected:
    QLabel *mRxLabel;
    QLabel *mTxLabel;
    QComboBox *mOutputTextFormatComboBox;
    QCheckBox *mShowDateCheckBox;
    QCheckBox *mAutoWrapCheckBox;
    QCheckBox *mShowTimeCheckBox;
    QCheckBox *mShowMsCheckBox;
    QCheckBox *mShowRxDataCheckBox;
    QCheckBox *mShowTxDataCheckBox;
    QCheckBox *mSaveOutputToFileCheckBox;
    QCheckBox *mRawDataCheckBox;
    QPushButton *mMoreOutputSettingsPushButton;
    QPushButton *mClearOutputPushButton;
    QTextBrowser *mOutputTextBroswer;
    /*************************************************************************/
    // Data statistics module
protected:
    bool mReceivedFlag;
    bool mSendFlag;

    QLabel *mRxSpeedLabel;
    QLabel *mTxSpeedLabel;
    QLabel *mRxFramesLabel;
    QLabel *mTxFramesLabel;
    QLabel *mRxBytesLabel;
    QLabel *mTxBytesLabel;
    QPushButton *mResetTxCountPushButton;
    QPushButton *mResetRxCountPushButton;
    /*************************************************************************/
    // Other settings module
protected:
    QPushButton *mTransmissionSettingPushButton;
    QPushButton *mAutoResponseSettingPushButton;
    QPushButton *mTimingSendingPushButton;
    QPushButton *mMoreSettingsPushButton;
    QPushButton *mAnalyzerPushButton;
};

#endif  // SAKTABPAGE_HH
