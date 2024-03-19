﻿/***************************************************************************************************
 * Copyright 2023 x-tools-author(x-tools@outlook.com). All rights reserved.
 *
 * The file is encoded using "utf8 with bom", it is a part of xTools project.
 *
 * xTools is licensed according to the terms in the file LICENCE(GPL V3) in the root of the source
 * code directory.
 **************************************************************************************************/
#include <QFileDialog>
#include <QLineEdit>

#include "xToolsStorerTool.h"
#include "sakstorertoolui.h"
#include "ui_sakstorertoolui.h"

SAKStorerToolUi::SAKStorerToolUi(QWidget *parent)
    : SAKBaseToolUi{parent}
    , ui(new Ui::SAKStorerToolUi)
{
    ui->setupUi(this);
    connect(ui->pushButtonSelectFile,
            &QPushButton::clicked,
            this,
            &SAKStorerToolUi::onPushButtonSelectFileClicked);
}

SAKStorerToolUi::~SAKStorerToolUi()
{
    delete ui;
}

void SAKStorerToolUi::onBaseToolUiInitialized(xToolsBaseTool *tool, const QString &settingsGroup)
{
    if (!tool) {
        qCWarning((*mLoggingCategory)) << "The tool value is nullptr!";
        return;
    }

    xToolsStorerTool *cookedTool = qobject_cast<xToolsStorerTool *>(tool);
    if (!cookedTool) {
        qCWarning((*mLoggingCategory)) << "The cookedTool value is nullptr!";
        return;
    }

    connect(ui->checkBoxEnable, &QCheckBox::clicked, this, [=]() {
        cookedTool->setIsEnable(ui->checkBoxEnable->isChecked());
    });
    connect(ui->checkBoxRx, &QCheckBox::clicked, this, [=]() {
        cookedTool->setSaveRx(ui->checkBoxRx->isChecked());
    });
    connect(ui->checkBoxTx, &QCheckBox::clicked, this, [=]() {
        cookedTool->setSaveTx(ui->checkBoxTx->isChecked());
    });
    connect(ui->checkBoxDate, &QCheckBox::clicked, this, [=]() {
        cookedTool->setSaveDate(ui->checkBoxDate->isChecked());
    });
    connect(ui->checkBoxDate, &QCheckBox::clicked, this, [=]() {
        cookedTool->setSaveTime(ui->checkBoxTime->isChecked());
    });
    connect(ui->checkBoxMs, &QCheckBox::clicked, this, [=]() {
        cookedTool->setSaveMs(ui->checkBoxMs->isChecked());
    });
    connect(ui->comboBoxFormat,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
            this,
            [=]() {
                int format = ui->comboBoxFormat->currentData().toInt();
                cookedTool->setSaveFormat(format);
            });
    connect(ui->lineEditStorerPath, &SAKLineEdit::textChanged, this, [=]() {
        cookedTool->setFileName(ui->lineEditStorerPath->text());
    });

    ui->checkBoxEnable->setGroupKey(settingsGroup, "enable");
    ui->checkBoxDate->setGroupKey(settingsGroup, "date");
    ui->checkBoxTime->setGroupKey(settingsGroup, "time");
    ui->checkBoxMs->setGroupKey(settingsGroup, "ms");
    ui->checkBoxRx->setGroupKey(settingsGroup, "rx");
    ui->checkBoxTx->setGroupKey(settingsGroup, "tx");
    ui->comboBoxFormat->setGroupKey(settingsGroup, "format");
    ui->lineEditStorerPath->setGroupKey(settingsGroup, "path");
    
    cookedTool->setIsEnable(ui->checkBoxEnable->isChecked());
    cookedTool->setSaveTx(ui->checkBoxTx->isChecked());
    cookedTool->setSaveRx(ui->checkBoxRx->isChecked());
    cookedTool->setSaveDate(ui->checkBoxDate->isChecked());
    cookedTool->setSaveTime(ui->checkBoxTime->isChecked());
    cookedTool->setSaveMs(ui->checkBoxMs->isChecked());
    cookedTool->setSaveFormat(ui->comboBoxFormat->currentData().toInt());
    cookedTool->setFileName(ui->lineEditStorerPath->text());
}

void SAKStorerToolUi::onPushButtonSelectFileClicked()
{
    auto str = QFileDialog::getSaveFileName(Q_NULLPTR,
                                            tr("Save file"),
                                            ".",
                                            tr("txt (*.txt);;All (*)"));
    if (!str.isEmpty()) {
        ui->lineEditStorerPath->setText(str);
    }
}
