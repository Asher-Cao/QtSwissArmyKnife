﻿/****************************************************************************************
 * Copyright 2021 Qter(qsaker@qq.com). All rights reserved.
 *
 * The file is encoded using "utf8 with bom", it is a part
 * of QtSwissArmyKnife project.
 *
 * QtSwissArmyKnife is licensed according to the terms in
 * the file LICENCE in the root of the source code directory.
 ***************************************************************************************/
#include <QDir>
#include <QMenu>
#include <QDebug>
#include <QJsonObject>
#include <QCoreApplication>

#include "SAKMainWindow.hh"
#include "SAKDebuggerPlugins.hh"
#ifdef SAK_IMPORT_MODULE_PLUGIN
#include "SAKDebuggerPlugin.hh"
#endif

SAKDebuggerPlugins::SAKDebuggerPlugins(QPushButton *managerBt,
                                       QPushButton *menuBt,
                                       QSettings *settings,
                                       const QString &settingsGroup,
                                       QLabel *titleLabel,
                                       QSqlDatabase *sqlDatabase,
                                       QWidget *panelWidget,
                                       QObject *parent)
    :QObject(parent)
    ,mTransponders(Q_NULLPTR)
    ,mAutoResponse(Q_NULLPTR)
    ,mTimedSending(Q_NULLPTR)
    ,mTitleLabel(titleLabel)
    ,mPanelWidget(panelWidget)
    ,mPluginDialog(Q_NULLPTR)
    ,mActiveWidgetInPanel(Q_NULLPTR)
    ,mActiveWidgetInDialog(Q_NULLPTR)
{
    QMenu *menu = new QMenu(menuBt);
    menuBt->setMenu(menu);

    mTitleLabel->setHidden(true);
    mPanelWidget->setHidden(true);

    // Go to a web page.
    if (managerBt) {
        mManager = new SAKDebuggerPluginsManager();
        //mManager->setWindowModality(Qt::ApplicationModal);
        connect(managerBt, &QPushButton::clicked, this, [=](){
            if (mManager->isHidden()) {
                mManager->show();
            } else {
                mManager->activateWindow();
            }
        });
    }


    // Instance plugins.
    mTransponders = new SAKDebuggerPluginTransponders(sqlDatabase,
                                                      settings,
                                                      settingsGroup,
                                                      "Transponders");
    connect(this, &SAKDebuggerPlugins::bytesRead,
            mTransponders, &SAKDebuggerPluginTransponders::bytesRead);
    connect(mTransponders, &SAKDebuggerPluginTransponders::invokeWriteCookedBytes,
            this, &SAKDebuggerPlugins::invokeWriteCookedBytes);
    mTimedSending = new SAKDebuggerPluginTimedSending(sqlDatabase,
                                                      settings,
                                                      settingsGroup,
                                                      "TimedSending");
    connect(mTimedSending, &SAKDebuggerPluginTimedSending::invokeWriteCookedBytes,
            this, &SAKDebuggerPlugins::invokeWriteCookedBytes);
    mAutoResponse = new SAKDebuggerPluginAutoResponse(settings,
                                                      settingsGroup,
                                                      sqlDatabase,
                                                      "AutoResponse");
    connect(this, &SAKDebuggerPlugins::bytesRead,
            mAutoResponse, &SAKDebuggerPluginAutoResponse::onBytesRead);
    connect(mAutoResponse, &SAKDebuggerPluginAutoResponse::invokeWriteCookedBytes,
            this, &SAKDebuggerPlugins::invokeWriteCookedBytes);


    // Initialize menu psuh button
    QMenu *embedMenu = new QMenu(tr("Embed to Center"), menu);
    menu->addMenu(embedMenu);
    embedMenu->addAction(tr("Cancel Embedding"),
                         this,
                         &SAKDebuggerPlugins::cancelEmbedPlugin);
    embedMenu->addSeparator();
    struct SAKActionsContext {
        QString title;
        void (SAKDebuggerPlugins::*show)();
        void (SAKDebuggerPlugins::*embed)();
    };
    QVector<SAKActionsContext> actionsCtx;
    actionsCtx.append({tr("Transponders"),
                       &SAKDebuggerPlugins::showPluinTransponders,
                       &SAKDebuggerPlugins::embedPluinTransponders});
    actionsCtx.append({tr("Timed Sending"),
                       &SAKDebuggerPlugins::showPluginRegularlySending,
                       &SAKDebuggerPlugins::embedPluginRegularlySending});
    actionsCtx.append({tr("Automatically Response"),
                       &SAKDebuggerPlugins::showPluginAutoResponse,
                       &SAKDebuggerPlugins::embedPluginAutoResponse});
    auto addActionsToMenu = [=](QMenu *m,
            const QVector<SAKActionsContext> &ctxs,
            bool isShow){
        for (int i = 0; i < ctxs.count(); i++) {
            SAKActionsContext ctx = ctxs.at(i);
            if (isShow) {
                m->addAction(ctx.title, this, ctx.show);
            } else {
                m->addAction(ctx.title, this, ctx.embed);
            }
        }
    };
    addActionsToMenu(embedMenu, actionsCtx, false);
    menu->addSeparator();
    addActionsToMenu(menu, actionsCtx, true);


    mPluginDialog = new QDialog(sakMainWindow);
    mPluginDialog->setLayout(new QVBoxLayout(mPluginDialog));
    mPluginDialog->setModal(true);
    mPluginDialog->layout()->setContentsMargins(0, 0, 0, 0);
    mPluginDialog->setContentsMargins(6, 6, 6, 6);
    mPluginDialog->resize(800, 350);


    if (mPanelWidget) {
        if (!mPanelWidget->layout()) {
            mPanelWidget->setLayout(new QVBoxLayout(mPanelWidget));
            mPanelWidget->layout()->setContentsMargins(0, 0, 0, 0);
            mPanelWidget->setContentsMargins(0, 0, 0, 0);
        }
    } else {
        Q_ASSERT_X(mPanelWidget, __FUNCTION__, "Can not be null(mPanelWidget)!");
        mPanelWidget = new QWidget(sakMainWindow);
        mPanelWidget->hide();
    }

    // Some bus exist when reload plugins, I don't want to handle them yet.
    menu->addSeparator();
    menu->addAction(tr("Reload All"), this, [=](){
        this->loadPlugin(menu, embedMenu);
    });

#ifdef SAK_IMPORT_MODULE_PLUGIN
    loadPlugin(menu, embedMenu);
#endif
}

SAKDebuggerPlugins::~SAKDebuggerPlugins()
{
    QVector<QWidget*> pluginVector;
    pluginVector << mTransponders
                 << mTimedSending
                 << mAutoResponse;
    for (int i = 0; i < pluginVector.count(); i++) {
        auto w = pluginVector.at(i);
        if ((w != mActiveWidgetInPanel) && (w != mActiveWidgetInDialog)) {
            w->deleteLater();
        }
    }
}

void SAKDebuggerPlugins::showPluinTransponders()
{
    showPluginDialog(mTransponders);
}

void SAKDebuggerPlugins::showPluginAutoResponse()
{
    showPluginDialog(mAutoResponse);
}

void SAKDebuggerPlugins::showPluginRegularlySending()
{
    showPluginDialog(mTimedSending);
}

void SAKDebuggerPlugins::showPluginDialog(QWidget *contentWidget)
{
    clearPluginDialog();
    mActiveWidgetInDialog = contentWidget;
    mPluginDialog->layout()->addWidget(contentWidget);
    mPluginDialog->show();
    if (mActiveWidgetInPanel == mActiveWidgetInDialog) {
        cancelEmbedPlugin();
    }
}

void SAKDebuggerPlugins::embedPluinTransponders()
{
    embedPlugin(mTransponders);
}

void SAKDebuggerPlugins::embedPluginAutoResponse()
{
    embedPlugin(mAutoResponse);
}

void SAKDebuggerPlugins::embedPluginRegularlySending()
{
    embedPlugin(mTimedSending);
}

void SAKDebuggerPlugins::embedPlugin(QWidget *contentWidget)
{
    clearPluginPanel();
    mActiveWidgetInPanel = contentWidget;
    mPanelWidget->layout()->addWidget(contentWidget);
    mTitleLabel->show();
    mPanelWidget->show();
    contentWidget->show();

    if (mActiveWidgetInPanel == mActiveWidgetInDialog) {
        clearPluginDialog();
    }
}

void SAKDebuggerPlugins::cancelEmbedPlugin()
{
    clearPluginPanel();
    mTitleLabel->hide();
    mPanelWidget->hide();
}

void SAKDebuggerPlugins::clearPluginDialog()
{
    for (int i = 0; i < mPluginDialog->layout()->count(); i++) {
        auto ret = mPluginDialog->layout()->takeAt(i);
        ret->widget()->setParent(Q_NULLPTR);
    }

    mActiveWidgetInDialog = Q_NULLPTR;
}

void SAKDebuggerPlugins::clearPluginPanel()
{
    for (int i = 0; i < mPanelWidget->layout()->count(); i++) {
        auto ret = mPanelWidget->layout()->takeAt(i);
        ret->widget()->setParent(Q_NULLPTR);
    }

    mActiveWidgetInPanel = Q_NULLPTR;
}

void SAKDebuggerPlugins::loadPlugin(QMenu *menu, QMenu *embedMenu)
{
    clearPlugins(mPluginActionVector);
    clearPlugins(mEmbedPluginActionVector);
    QDir pluginsDir(QCoreApplication::applicationDirPath());
    pluginsDir.cd("plugins");
    const QStringList entries = pluginsDir.entryList(QDir::Files);
    for (const QString &fileName : entries) {
        const QString path = pluginsDir.absoluteFilePath(fileName);
        QPluginLoader pluginLoader(path);
        QObject *plugin = pluginLoader.instance();
        if (plugin) {
            QJsonObject jsonObj = pluginLoader.metaData();
            QJsonObject metaData = jsonObj.value("MetaData").toObject();
            QString name = metaData.value("name").toString();
            auto *sakPlugin = qobject_cast<SAKDebuggerPlugin*>(plugin);
            if (sakPlugin) {
                PluginUi *widget = sakPlugin->ui();
                if (widget) {
                    addPluginToMenu(menu, name, widget, false);
                    addPluginToMenu(embedMenu, name, widget, true);
                    connect(this, &SAKDebuggerPlugins::bytesRead,
                            widget, &PluginUi::onDataRead,
                            Qt::ConnectionType(Qt::AutoConnection|Qt::UniqueConnection));
                    connect(this, &SAKDebuggerPlugins::bytesWritten,
                            widget, &PluginUi::onDataWritten,
                            Qt::ConnectionType(Qt::AutoConnection|Qt::UniqueConnection));
                    connect(widget, &PluginUi::invokeWriteCookedData,
                            this, &SAKDebuggerPlugins::invokeWriteCookedBytes,
                            Qt::ConnectionType(Qt::AutoConnection|Qt::UniqueConnection));
                    connect(widget, &PluginUi::invokeWriteRawData,
                            this, &SAKDebuggerPlugins::invokeWriteRawBytes,
                            Qt::ConnectionType(Qt::AutoConnection|Qt::UniqueConnection));
                }
            }
        } else {
            pluginLoader.unload();
        }
    }
}

void SAKDebuggerPlugins::addPluginToMenu(QMenu *menu,
                                         QString name, QWidget *ui,
                                         bool embed)
{
    if (embed) {
        if (mEmbedPluginActionVector.isEmpty()) {
            mEmbedPluginActionVector.append(menu->addSeparator());
        }
    } else {
        if (mPluginActionVector.isEmpty()) {
            mPluginActionVector.append(menu->addSeparator());
        }
    }

    QAction *action = new QAction(name, menu);
    action->setData(QVariant::fromValue(ui));
    menu->addAction(action);
    if (embed) {
        mEmbedPluginActionVector.append(action);
    } else {
        mPluginActionVector.append(action);
    }

    connect(action, &QAction::triggered, this, [=]() {
        if (embed) {
            embedPlugin(ui);
        } else {
            if (mActiveWidgetInPanel == ui) {
                cancelEmbedPlugin();
            }

            if (ui->isHidden()) {
                ui->show();
            } else {
                ui->activateWindow();
            }
        }
    });
}

void SAKDebuggerPlugins::clearPlugins(QVector<QAction*> &actionVector)
{
    while (actionVector.count()) {
        QAction *action = actionVector.takeFirst();
        QObject *dataObj = action->data().value<QObject*>();
        if (dataObj) {
            auto *widget = qobject_cast<QWidget*>(dataObj);
            if (widget) {
               if (widget == this->mActiveWidgetInPanel)  {
                   cancelEmbedPlugin();
               }

               widget->close();
               widget->deleteLater();
            }
        }
        action->deleteLater();
    }
}
