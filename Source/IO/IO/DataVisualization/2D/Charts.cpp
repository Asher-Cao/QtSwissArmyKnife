﻿/***************************************************************************************************
 * Copyright 2024 x-tools-author(x-tools@outlook.com). All rights reserved.
 *
 * The file is encoded using "utf8 with bom", it is a part of eTools project.
 *
 * eTools is licensed according to the terms in the file LICENCE(GPL V3) in the root of the source
 * code directory.
 **************************************************************************************************/
#include "Charts.h"

#include <QPointF>
#include <QTimer>

namespace xTools {

Charts::Charts(QObject *parent)
    : AbstractIO(parent)
    , m_binaryTail(QByteArray::fromHex("0000807f"))
{}

Charts::~Charts() {}

void Charts::inputBytes(const QByteArray &bytes)
{
    if (!bytes.isEmpty() && isRunning()) {
        emit input2run(bytes);
    }
}

void Charts::run()
{
    QByteArray tmp;
    QTimer *timer = new QTimer();

    connect(this, &Charts::input2run, timer, [&tmp](const QByteArray &bytes) { tmp.append(bytes); });
    connect(timer, &QTimer::timeout, timer, [&tmp, this, timer] {
        QVariantMap parameters = save();
        ChartDataKeys keys;
        int dataFormat = parameters.value(keys.dataFormat).toInt();
        if (dataFormat == static_cast<int>(DataFormat::BinaryY)) {
            handleBinaryY(tmp);
        } else if (dataFormat == static_cast<int>(DataFormat::TextY)) {
            handleTextY(tmp);
        } else if (dataFormat == static_cast<int>(DataFormat::BinaryXY)) {
            handleBinaryXY(tmp);
        } else if (dataFormat == static_cast<int>(DataFormat::TextXY)) {
            handleTextXY(tmp);
        } else {
            emit outputBytes(QByteArray("Invalid data format!"));
        }

        timer->start();
    });

    timer->setSingleShot(true);
    timer->setInterval(50);
    timer->start();

    exec();

    timer->stop();
    timer->deleteLater();
}

void Charts::handleBinaryY(QByteArray &bytes)
{
    while (bytes.indexOf(m_binaryTail) != -1 && !bytes.isEmpty()) {
        int index = bytes.indexOf(m_binaryTail);
        QByteArray yOfPoints = bytes.left(index);
        bytes.remove(0, index + m_binaryTail.size());

        if (yOfPoints.size() % 4 != 0) {
            emit outputBytes(QByteArray("Data error: ") + yOfPoints);
            continue;
        }

        QList<double> values;
        for (int i = 0; i < yOfPoints.size(); i += 4) {
            QByteArray tmp = yOfPoints.mid(i, 4);
            values.append(*reinterpret_cast<float *>(tmp.data()));
        }
        emit newValues(values);
    }
}

void Charts::handleTextY(QByteArray &bytes)
{
    // 1.000, 2.000, 3.000, 4.000, 5.000\n
    while (bytes.indexOf('\n') != -1 && !bytes.isEmpty()) {
        int index = bytes.indexOf('\n');
        QByteArray yOfPoints = bytes.left(index);
        bytes.remove(0, index + 1);

        QString str = QString::fromLatin1(yOfPoints);
        if (str.isEmpty()) {
            emit outputBytes(QByteArray("Data error: ") + yOfPoints);
            continue;
        }

        QStringList yList = str.split(',');
        QList<double> values;
        for (const QString &y : yList) {
            values.append(y.toDouble());
        }
        emit newValues(values);
    }
}

void Charts::handleBinaryXY(QByteArray &bytes)
{
    while (bytes.indexOf(m_binaryTail) != -1 && !bytes.isEmpty()) {
        int index = bytes.indexOf(m_binaryTail);
        QByteArray xyOfPoints = bytes.left(index);
        bytes.remove(0, index + m_binaryTail.size());

        if (xyOfPoints.size() % 8 != 0) {
            emit outputBytes(QByteArray("Data error: ") + xyOfPoints);
            continue;
        }

        QList<QPointF> points;
        for (int i = 0; i < xyOfPoints.size(); i += 8) {
            QByteArray tmp = xyOfPoints.mid(i, 8);
            float x = *reinterpret_cast<float *>(tmp.data());

            tmp.remove(0, 4);
            float y = *reinterpret_cast<float *>(tmp.data());
            points.append(QPointF(x, y));
        }
        emit newPoints(points);
    }
}

void Charts::handleTextXY(QByteArray &bytes)
{
    //1.000,2.000,3.000,4.000\n
    while (bytes.indexOf('\n') != -1 && !bytes.isEmpty()) {
        int index = bytes.indexOf('\n');
        QByteArray xyOfPoints = bytes.left(index);
        bytes.remove(0, index + 1);

        QString str = QString::fromLatin1(xyOfPoints);
        if (str.isEmpty()) {
            emit outputBytes(QByteArray("Data error: ") + xyOfPoints);
            continue;
        }

        QStringList xyList = str.split(',');
        QList<QPointF> points;
        for (int i = 0; i < xyList.size(); i += 2) {
            points.append(QPointF(xyList.at(i).toDouble(), xyList.at(i + 1).toDouble()));
        }
        emit newPoints(points);
    }
}

} // namespace xTools
