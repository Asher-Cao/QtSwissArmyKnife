﻿/*
 * Copyright 2023 Qter(qsaker@foxmail.com). All rights reserved.
 *
 * The file is encoded using "utf8 with bom", it is a part
 * of QtSwissArmyKnife project.
 *
 * QtSwissArmyKnife is licensed according to the terms in
 * the file LICENCE in the root of the source code directory.
 */

#include <QApplication>
#include "SAKToolStringAssistant.h"

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);

  SAKToolStringAssistant assistant;
  assistant.show();

  return app.exec();
}