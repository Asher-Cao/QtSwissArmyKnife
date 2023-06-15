/******************************************************************************
 * Copyright 2023 Qsaker(wuuhaii@outlook.com). All rights reserved.
 *
 * The file is encoded using "utf8 with bom", it is a part
 * of QtSwissArmyKnife project.
 *
 * QtSwissArmyKnife is licensed according to the terms in
 * the file LICENCE in the root of the source code directory.
 *****************************************************************************/
#include "SAKInterface.hh"
#include "SAKAnalyzerTool.hh"

SAKAnalyzerTool::SAKAnalyzerTool(QObject *parent)
    : SAKBaseTool{"SAK.AnalyzerTool", parent}
{
    mEnable = false;
}

void SAKAnalyzerTool::setFixed(bool fixed)
{
    mParameters.fixed = fixed;
}

void SAKAnalyzerTool::setFrameBytes(int bytes)
{
    mParameters.frameBytes = bytes;
}

void SAKAnalyzerTool::setSeparationMark(const QByteArray &mark)
{
    mParameters.separationMark = mark;
}

void SAKAnalyzerTool::setMaxTempBytes(int maxBytes)
{
    mParameters.maxTempBytes = maxBytes;
}

void SAKAnalyzerTool::inputBytes(const QByteArray &bytes,
                                 const QVariant &context)
{
    if (bytes.isEmpty()) {
        outputMessage(QtInfoMsg, "Empty input bytes, "
                                 "the operation will be ignored!");
        return;
    }

    QString hex = QString::fromLatin1(SAKInterface::arrayToHex(bytes, ' '));
    outputMessage(QtInfoMsg, QString("%1<-%2").arg(mToolName, hex));
    emit bytesInputted(bytes, context);

    if (!enable()) {
        QString hex = QString::fromLatin1(SAKInterface::arrayToHex(bytes, ' '));
        outputMessage(QtInfoMsg, QString("%1->%2").arg(mToolName, hex));
        emit bytesOutputted(bytes, context);
    } else {
        mInputtedBytesMutex.lock();
        mInputtedBytes.append(bytes);
        mInputtedBytesMutex.unlock();
    }
}

void SAKAnalyzerTool::run()
{
    QTimer *handleTimer = new QTimer();
    handleTimer->setInterval(5);
    handleTimer->setSingleShot(true);
    connect(handleTimer, &QTimer::timeout, handleTimer, [=](){
        if (mEnable) {
            mInputtedBytesMutex.lock();
            analyze();
            mInputtedBytesMutex.unlock();
        }
        handleTimer->start();
    });
    handleTimer->start();

    exec();

    if (handleTimer && handleTimer->isActive()) {
        mInputtedBytesMutex.lock();
        mInputtedBytes.clear();
        mInputtedBytesMutex.unlock();

        handleTimer->stop();
        handleTimer->deleteLater();
    }

    handleTimer = nullptr;
}

void SAKAnalyzerTool::analyze()
{
    if (mInputtedBytes.length() > mParameters.maxTempBytes) {
        outputMessage(QtInfoMsg,
                      "clear bytes: " + QString(SAKInterface::arrayToHex(mInputtedBytes, ' ')));
        mInputtedBytes.clear();

        return;
    }

    if (mParameters.fixed) {
        while (mInputtedBytes.length() >= mParameters.frameBytes) {
            QByteArray frame(mInputtedBytes.data(),
                             mParameters.frameBytes);
            mInputtedBytes.remove(0, mParameters.frameBytes);

            QString hex = QString::fromLatin1(SAKInterface::arrayToHex(frame, ' '));
            outputMessage(QtInfoMsg, QString("Analyzer->%1").arg(hex));
            emit bytesOutputted(frame, QJsonObject());
        }

        return;
    }

    if (mParameters.separationMark.isEmpty()) {
        if (!mInputtedBytes.isEmpty()) {
            QString hex = QString::fromLatin1(SAKInterface::arrayToHex(mInputtedBytes, ' '));
            QString msg = QString("Analyzer->%1").arg(hex);
            outputMessage(QtInfoMsg, msg);
            emit bytesOutputted(mInputtedBytes, QJsonObject());
            mInputtedBytes.clear();
        }
    } else {
        auto ret = mInputtedBytes.indexOf(mParameters.separationMark);
        if (ret != -1) {
            int len = ret + mParameters.separationMark.length();
            QByteArray frame(mInputtedBytes.constData(), len);
            mInputtedBytes.remove(0, len);

            QString hex = QString::fromLatin1(SAKInterface::arrayToHex(frame, ' '));
            QString msg = QString("Analyzer->%1").arg(hex);
            outputMessage(QtInfoMsg, msg);
            emit bytesOutputted(frame, QJsonObject());
        }
    }
}
