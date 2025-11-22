/********************************************************************************
** Form generated from reading UI file 'ScreenCapture.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SCREENCAPTURE_H
#define UI_SCREENCAPTURE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ScreenCaptureClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *ScreenCaptureClass)
    {
        if (ScreenCaptureClass->objectName().isEmpty())
            ScreenCaptureClass->setObjectName(QString::fromUtf8("ScreenCaptureClass"));
        ScreenCaptureClass->resize(600, 400);
        menuBar = new QMenuBar(ScreenCaptureClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        ScreenCaptureClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(ScreenCaptureClass);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        ScreenCaptureClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(ScreenCaptureClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        ScreenCaptureClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(ScreenCaptureClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        ScreenCaptureClass->setStatusBar(statusBar);

        retranslateUi(ScreenCaptureClass);

        QMetaObject::connectSlotsByName(ScreenCaptureClass);
    } // setupUi

    void retranslateUi(QMainWindow *ScreenCaptureClass)
    {
        ScreenCaptureClass->setWindowTitle(QCoreApplication::translate("ScreenCaptureClass", "ScreenCapture", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ScreenCaptureClass: public Ui_ScreenCaptureClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SCREENCAPTURE_H
