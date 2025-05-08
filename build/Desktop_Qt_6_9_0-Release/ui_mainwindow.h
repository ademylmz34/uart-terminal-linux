/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout_2;
    QFrame *frame_4;
    QVBoxLayout *verticalLayout;
    QFrame *frame;
    QHBoxLayout *horizontalLayout;
    QLabel *lblConnectStat;
    QLabel *lblPort;
    QComboBox *cmbPort;
    QLabel *lblBaudRate;
    QComboBox *cmbBaudRate;
    QPushButton *btnConnect;
    QPushButton *btnClear;
    QFrame *frame_2;
    QVBoxLayout *verticalLayout_3;
    QPlainTextEdit *plainTextEdit;
    QFrame *frame_3;
    QHBoxLayout *horizontalLayout_2;
    QLineEdit *lineEdit;
    QPushButton *btnSend;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(739, 644);
        MainWindow->setMinimumSize(QSize(0, 0));
        MainWindow->setMaximumSize(QSize(1920, 1080));
        MainWindow->setStyleSheet(QString::fromUtf8("background-color: rgb(223, 223, 223);"));
        MainWindow->setDocumentMode(false);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayout_2 = new QVBoxLayout(centralwidget);
        verticalLayout_2->setObjectName("verticalLayout_2");
        frame_4 = new QFrame(centralwidget);
        frame_4->setObjectName("frame_4");
        frame_4->setMinimumSize(QSize(0, 60));
        frame_4->setMaximumSize(QSize(16777215, 80));
        frame_4->setFrameShape(QFrame::Shape::StyledPanel);
        frame_4->setFrameShadow(QFrame::Shadow::Raised);
        verticalLayout = new QVBoxLayout(frame_4);
        verticalLayout->setObjectName("verticalLayout");
        frame = new QFrame(frame_4);
        frame->setObjectName("frame");
        frame->setStyleSheet(QString::fromUtf8("background-color: rgb(85, 170, 255);"));
        frame->setFrameShape(QFrame::Shape::StyledPanel);
        frame->setFrameShadow(QFrame::Shadow::Raised);
        horizontalLayout = new QHBoxLayout(frame);
        horizontalLayout->setObjectName("horizontalLayout");
        lblConnectStat = new QLabel(frame);
        lblConnectStat->setObjectName("lblConnectStat");
        QFont font;
        font.setPointSize(16);
        font.setBold(true);
        lblConnectStat->setFont(font);
        lblConnectStat->setStyleSheet(QString::fromUtf8("color: rgb(255, 255, 255);"));

        horizontalLayout->addWidget(lblConnectStat);

        lblPort = new QLabel(frame);
        lblPort->setObjectName("lblPort");
        QFont font1;
        font1.setPointSize(16);
        lblPort->setFont(font1);
        lblPort->setStyleSheet(QString::fromUtf8("color: rgb(255, 255, 255);"));

        horizontalLayout->addWidget(lblPort);

        cmbPort = new QComboBox(frame);
        cmbPort->setObjectName("cmbPort");
        cmbPort->setEnabled(false);
        cmbPort->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 255);\n"
"color: rgb(0, 0, 0);"));

        horizontalLayout->addWidget(cmbPort);

        lblBaudRate = new QLabel(frame);
        lblBaudRate->setObjectName("lblBaudRate");
        lblBaudRate->setFont(font1);
        lblBaudRate->setStyleSheet(QString::fromUtf8("color: rgb(255, 255, 255);"));

        horizontalLayout->addWidget(lblBaudRate);

        cmbBaudRate = new QComboBox(frame);
        cmbBaudRate->setObjectName("cmbBaudRate");
        QFont font2;
        font2.setPointSize(14);
        font2.setStyleStrategy(QFont::PreferDefault);
        font2.setHintingPreference(QFont::PreferDefaultHinting);
        cmbBaudRate->setFont(font2);
        cmbBaudRate->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 255);\n"
"color: rgb(0, 0, 0);"));

        horizontalLayout->addWidget(cmbBaudRate);

        btnConnect = new QPushButton(frame);
        btnConnect->setObjectName("btnConnect");
        QFont font3;
        font3.setBold(true);
        btnConnect->setFont(font3);
        btnConnect->setStyleSheet(QString::fromUtf8("color:rgb(255, 0, 0);\n"
"background-color: rgb(255, 255, 255);"));

        horizontalLayout->addWidget(btnConnect);

        btnClear = new QPushButton(frame);
        btnClear->setObjectName("btnClear");
        btnClear->setFont(font3);
        btnClear->setStyleSheet(QString::fromUtf8("color:rgb(255, 0, 0);\n"
"background-color: rgb(255, 255, 255);"));

        horizontalLayout->addWidget(btnClear);


        verticalLayout->addWidget(frame);


        verticalLayout_2->addWidget(frame_4);

        frame_2 = new QFrame(centralwidget);
        frame_2->setObjectName("frame_2");
        frame_2->setFrameShape(QFrame::Shape::StyledPanel);
        frame_2->setFrameShadow(QFrame::Shadow::Raised);
        verticalLayout_3 = new QVBoxLayout(frame_2);
        verticalLayout_3->setObjectName("verticalLayout_3");
        plainTextEdit = new QPlainTextEdit(frame_2);
        plainTextEdit->setObjectName("plainTextEdit");
        plainTextEdit->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 255);\n"
"color: rgb(38, 162, 105);"));
        plainTextEdit->setReadOnly(true);

        verticalLayout_3->addWidget(plainTextEdit);


        verticalLayout_2->addWidget(frame_2);

        frame_3 = new QFrame(centralwidget);
        frame_3->setObjectName("frame_3");
        frame_3->setMaximumSize(QSize(16777215, 60));
        frame_3->setFrameShape(QFrame::Shape::StyledPanel);
        frame_3->setFrameShadow(QFrame::Shadow::Raised);
        horizontalLayout_2 = new QHBoxLayout(frame_3);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        lineEdit = new QLineEdit(frame_3);
        lineEdit->setObjectName("lineEdit");
        lineEdit->setMinimumSize(QSize(0, 40));
        lineEdit->setFont(font1);
#if QT_CONFIG(tooltip)
        lineEdit->setToolTip(QString::fromUtf8(""));
#endif // QT_CONFIG(tooltip)
        lineEdit->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 255);\n"
"border-radius: 0px;"));

        horizontalLayout_2->addWidget(lineEdit);

        btnSend = new QPushButton(frame_3);
        btnSend->setObjectName("btnSend");
        btnSend->setMinimumSize(QSize(0, 40));
        QFont font4;
        font4.setPointSize(24);
        font4.setWeight(QFont::ExtraBold);
        btnSend->setFont(font4);
        btnSend->setAutoFillBackground(false);
        btnSend->setStyleSheet(QString::fromUtf8("background-color: rgb(255, 255, 255);\n"
"border-radius: 0px; \n"
"\n"
""));
        btnSend->setIconSize(QSize(16, 16));

        horizontalLayout_2->addWidget(btnSend);


        verticalLayout_2->addWidget(frame_3);

        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Uart Terminal", nullptr));
        lblConnectStat->setText(QCoreApplication::translate("MainWindow", "UART TERMINAL", nullptr));
        lblPort->setText(QCoreApplication::translate("MainWindow", "Port:", nullptr));
        lblBaudRate->setText(QCoreApplication::translate("MainWindow", "Baudrate:", nullptr));
        btnConnect->setText(QCoreApplication::translate("MainWindow", "Connect", nullptr));
        btnClear->setText(QCoreApplication::translate("MainWindow", "Temizle", nullptr));
        lineEdit->setText(QString());
        btnSend->setText(QCoreApplication::translate("MainWindow", "\342\206\265", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
