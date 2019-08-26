﻿/*
 * Copyright (C) 2018-2019 wuuhii. All rights reserved.
 *
 * The file is encoding with utf-8 (with BOM). It is a part of QtSwissArmyKnife
 * project. The project is a open source project, you can get the source from:
 *     https://github.com/wuuhii/QtSwissArmyKnife
 *     https://gitee.com/wuuhii/QtSwissArmyKnife
 *
 * If you want to know more about the project, please join our QQ group(952218522).
 * In addition, the email address of the project author is wuuhii@outlook.com.
 * Welcome to bother.
 *
 * I write the comment in English, it's not because that I'm good at English,
 * but for "installing B".
 */
#include "SaveOutputDataSettings.hh"
#include "ui_SaveOutputDataSettings.h"

#include <QFile>
#include <QDialog>
#include <QDateTime>
#include <QFileDialog>
#include <QStandardPaths>

SaveOutputDataSettings::SaveOutputDataSettings(QWidget *parent)
    :QDialog (parent)
    ,ui (new Ui::SaveOutputDataSettings)
{
    ui->setupUi(this);
    setModal(true);

    pathLineEdit        = ui->pathLineEdit;
    setFilePushButton   = ui->setFilePushButton;
    binRadioButton      = ui->binRadioButton;
    utf8RadioButton     = ui->utf8RadioButton;
    hexRadioButton      = ui->hexRadioButton;
    closePushButton     = ui->closePushButton;
    clearFilePushButton = ui->clearFilePushButton;

    defaultPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    pathLineEdit->setText(defaultPath.append("/default.txt"));
}

SaveOutputDataSettings::~SaveOutputDataSettings()
{
    delete ui;
}

void SaveOutputDataSettings::inputData(QByteArray data)
{
    int format;
    if (binRadioButton->isChecked()){
        format = Bin;
    }else if (utf8RadioButton->isChecked()){
        format = Utf8;
    }else{
        format = Hex;
    }

    emit writeDataToFile(data, format);
}

void SaveOutputDataSettings::on_setFilePushButton_clicked()
{
    QString datetime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
    QString fileName;
    if (binRadioButton->isChecked()){
        datetime.append(".bin");
        fileName = QFileDialog::getSaveFileName(this, tr("文件设置"), QString("%1/%2").arg(defaultPath).arg(datetime), QString("bin (*.bin)"));
    }else{
        datetime.append(".txt");
        fileName = QFileDialog::getSaveFileName(this, tr("文件设置"), QString("%1/%2").arg(defaultPath).arg(datetime), QString("txt (*.txt)"));
    }

    if (!fileName.isEmpty()){
        pathLineEdit->setText(fileName);
    }
}

void SaveOutputDataSettings::on_clearFilePushButton_clicked()
{
    QString fileName = pathLineEdit->text();
    if (fileName.isEmpty()){
        return;
    }

    QFile file(fileName);
    if (file.open(QFile::ReadWrite | QFile::Truncate)){
        file.close();
    }
}
