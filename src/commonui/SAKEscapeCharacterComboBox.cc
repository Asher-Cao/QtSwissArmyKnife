/******************************************************************************
 * Copyright 2023 Qsaker(wuuhaii@outlook.com). All rights reserved.
 *
 * The file is encoded using "utf8 with bom", it is a part
 * of QtSwissArmyKnife project.
 *
 * QtSwissArmyKnife is licensed according to the terms in
 * the file LICENCE in the root of the source code directory.
 *****************************************************************************/
#include "SAKDataStructure.hh"
#include "SAKEscapeCharacterComboBox.hh"

SAKEscapeCharacterComboBox::SAKEscapeCharacterComboBox(QWidget *parent)
    : SAKComboBox(parent)
{
    addItem(tr("None"), SAKDataStructure::EscapeCharacterOptionNone);
    addItem("\\r", SAKDataStructure::EscapeCharacterOptionR);
    addItem("\\n", SAKDataStructure::EscapeCharacterOptionN);
    addItem("\\r\\n", SAKDataStructure::EscapeCharacterOptionRN);
    addItem("\\n\\r", SAKDataStructure::EscapeCharacterOptionNR);
    addItem("\\r + \\n", SAKDataStructure::EscapeCharacterOptionRAndN);
}

