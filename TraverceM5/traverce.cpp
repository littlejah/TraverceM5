#include "traverce.h"
#include "ui_traverce.h"
#include <QFileDialog>

Traverce::Traverce(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Traverce)
{
    ui->setupUi(this);
    m_TraverceModel= new QStandardItemModel(ui->TraverseTable);
    ui->TraverseTable->setModel(m_TraverceModel);
    QStringList TraversesHeader = {QString::fromStdWString(L"Ход"),
                                   QString::fromStdWString(L"Измерение"),
                                   QString::fromStdWString(L"Станции")};
    m_TraverceModel->setHorizontalHeaderLabels(TraversesHeader);

    connect(ui->OpenBtn,&QPushButton::clicked,this,&Traverce::OpenFile_btn_clicked);
}

Traverce::~Traverce()
{
    delete ui;
    if (m_TraverceModel) delete m_TraverceModel;
    if (m_StationModel) delete m_StationModel;
    if (m_ObservationModel) delete m_ObservationModel;
}

void Traverce::OpenFile_btn_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, QString::fromStdWString(L"Открыть"),
                                                    QString::fromStdWString(L""),
                                                    QString::fromStdWString(L"DiNi M5 (*.dat *.txt)"));
    if (fileName.isEmpty()) return;
    TraverseParser *parser = new TraverseParser();
    m_Traverses=parser->parseFile(fileName);
    UpdateTraverces();
    delete parser;


}

void Traverce::UpdateTraverces()
{
    if (m_Traverses.isEmpty()) return;
    m_TraverceModel->clear();
    QList<QStandardItem*> row;

    for (int i=0;i<m_Traverses.size();++i)
    {
        row.clear();
        auto name_item = new QStandardItem(QString::number(m_Traverses[i].lineNo));
        name_item->setData(i,Qt::UserRole);
        row.append(name_item);
        auto pattern_item = new QStandardItem(m_Traverses[i].pattern);
        row.append(pattern_item);
        auto stations_count = new QStandardItem(QString::number(m_Traverses[i].stations.count()));
        row.append(stations_count);
        m_TraverceModel->appendRow(row);
    }

}

