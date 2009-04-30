/********************************************************************************
** Form generated from reading ui file 'blokus.ui'
**
** Created: Sun Apr 26 19:42:11 2009
**      by: Qt User Interface Compiler version 4.4.3
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_BLOKUS_H
#define UI_BLOKUS_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_BlokusUi
{
public:
    QVBoxLayout *verticalLayout;

    void setupUi(QWidget *BlokusUi)
    {
    if (BlokusUi->objectName().isEmpty())
        BlokusUi->setObjectName(QString::fromUtf8("BlokusUi"));
    BlokusUi->resize(806, 468);
    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(BlokusUi->sizePolicy().hasHeightForWidth());
    BlokusUi->setSizePolicy(sizePolicy);
    verticalLayout = new QVBoxLayout(BlokusUi);
    verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));

    retranslateUi(BlokusUi);

    QMetaObject::connectSlotsByName(BlokusUi);
    } // setupUi

    void retranslateUi(QWidget *BlokusUi)
    {
    BlokusUi->setWindowTitle(QApplication::translate("BlokusUi", "Form", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(BlokusUi);
    } // retranslateUi

};

namespace Ui {
    class BlokusUi: public Ui_BlokusUi {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_BLOKUS_H
