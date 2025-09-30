/********************************************************************************
** Form generated from reading UI file 'traverce.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TRAVERCE_H
#define UI_TRAVERCE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableView>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Traverce
{
public:
    QWidget *gridLayoutWidget;
    QGridLayout *gridLayout;
    QLabel *label;
    QTableView *StationTable;
    QLabel *label_3;
    QLabel *label_2;
    QHBoxLayout *horizontalLayout;
    QLineEdit *FileName;
    QPushButton *OpenBtn;
    QTableView *TraverseTable;
    QTableView *ObservationTable;

    void setupUi(QDialog *Traverce)
    {
        if (Traverce->objectName().isEmpty())
            Traverce->setObjectName(QString::fromUtf8("Traverce"));
        Traverce->resize(800, 600);
        gridLayoutWidget = new QWidget(Traverce);
        gridLayoutWidget->setObjectName(QString::fromUtf8("gridLayoutWidget"));
        gridLayoutWidget->setGeometry(QRect(20, 10, 761, 571));
        gridLayout = new QGridLayout(gridLayoutWidget);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(gridLayoutWidget);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 2, 0, 1, 1);

        StationTable = new QTableView(gridLayoutWidget);
        StationTable->setObjectName(QString::fromUtf8("StationTable"));

        gridLayout->addWidget(StationTable, 5, 0, 1, 1);

        label_3 = new QLabel(gridLayoutWidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 6, 0, 1, 1);

        label_2 = new QLabel(gridLayoutWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 4, 0, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        FileName = new QLineEdit(gridLayoutWidget);
        FileName->setObjectName(QString::fromUtf8("FileName"));

        horizontalLayout->addWidget(FileName);

        OpenBtn = new QPushButton(gridLayoutWidget);
        OpenBtn->setObjectName(QString::fromUtf8("OpenBtn"));

        horizontalLayout->addWidget(OpenBtn);


        gridLayout->addLayout(horizontalLayout, 1, 0, 1, 1);

        TraverseTable = new QTableView(gridLayoutWidget);
        TraverseTable->setObjectName(QString::fromUtf8("TraverseTable"));

        gridLayout->addWidget(TraverseTable, 3, 0, 1, 1);

        ObservationTable = new QTableView(gridLayoutWidget);
        ObservationTable->setObjectName(QString::fromUtf8("ObservationTable"));

        gridLayout->addWidget(ObservationTable, 7, 0, 1, 1);


        retranslateUi(Traverce);

        QMetaObject::connectSlotsByName(Traverce);
    } // setupUi

    void retranslateUi(QDialog *Traverce)
    {
        Traverce->setWindowTitle(QCoreApplication::translate("Traverce", "Traverce", nullptr));
        label->setText(QCoreApplication::translate("Traverce", "\320\245\320\276\320\264", nullptr));
        label_3->setText(QCoreApplication::translate("Traverce", "\320\230\320\267\320\274\320\265\321\200\320\265\320\275\320\270\321\217", nullptr));
        label_2->setText(QCoreApplication::translate("Traverce", "\320\241\321\202\320\260\320\275\321\206\320\270\321\217", nullptr));
        OpenBtn->setText(QCoreApplication::translate("Traverce", "\320\236\321\202\320\272\321\200\321\213\321\202\321\214", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Traverce: public Ui_Traverce {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TRAVERCE_H
