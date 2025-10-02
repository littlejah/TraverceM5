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
    m_StationModel=new QStandardItemModel(ui->StationTable);
    ui->StationTable->setModel(m_StationModel);
    m_ObservationModel = new QStandardItemModel(ui->ObservationTable);
    ui->ObservationTable->setModel(m_ObservationModel);
    UpdateTablesHeaders();
    connect(ui->OpenBtn,&QPushButton::clicked,this,&Traverce::OpenFile_btn_clicked);
    connect(ui->TraverseTable->selectionModel(),&QItemSelectionModel::selectionChanged,this,&Traverce::TraverseSelection_changed);
    connect(ui->StationTable->selectionModel(),&QItemSelectionModel::selectionChanged,this,&Traverce::StationSelection_changed);
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
    m_selected_traverse=-1;
    TraverseParser *parser = new TraverseParser();
    if(m_Traverses.size()!=0)
    {
        for (int i=0;i<m_Traverses.size();++i)
        {
            if(m_Traverses[i]!=nullptr) delete m_Traverses[i];
        }
        m_Traverses.clear();
    }
    m_Traverses=parser->parseFile(fileName);
    UpdateTraverces();
    delete parser;


}

void Traverce::UpdateTraverces()
{
    if (m_Traverses.isEmpty()) return;
    m_TraverceModel->clear();
    UpdateTablesHeaders();
    QList<QStandardItem*> row;

    for (int i=0;i<m_Traverses.size();++i)
    {
        row.clear();
        auto name_item = new QStandardItem(QString::number(m_Traverses[i]->lineNo));
        name_item->setData(i,Qt::UserRole);
        row.append(name_item);
        auto pattern_item = new QStandardItem(m_Traverses[i]->pattern);
        row.append(pattern_item);
        auto stations_count = new QStandardItem(QString::number(m_Traverses[i]->stations.count()));
        row.append(stations_count);
        m_TraverceModel->appendRow(row);
    }

}

void Traverce::TraverseSelection_changed(const QItemSelection &selected, const QItemSelection &deselected)
{
    if(!selected.isEmpty())
    {
        QModelIndexList selected_indexes=selected.indexes();
        if (!selected_indexes.isEmpty())
        {
            QModelIndex selected_index=selected_indexes.first();
            int traverse_index=m_TraverceModel->item(selected_index.row(),0)->data(Qt::UserRole).toInt();
            m_selected_traverse=traverse_index;
            UpdateStationModel(m_Traverses[traverse_index]);

        }
    }


}

void Traverce::StationSelection_changed(const QItemSelection &selected, const QItemSelection &deselected)
{
    if(!selected.isEmpty())
    {
        QModelIndexList selected_indexes=selected.indexes();
        if (!selected_indexes.isEmpty())
        {
            QModelIndex selected_index=selected_indexes.first();
            int station_index=m_StationModel->item(selected_index.row(),0)->data(Qt::UserRole).toInt();
            UpdateObservationModel(m_Traverses[m_selected_traverse]->stations[station_index]);

        }
    }

}

void Traverce::UpdateTablesHeaders()
{
    QStringList TraversesHeader = {QString::fromStdWString(L"Ход"),
                                   QString::fromStdWString(L"Измерение"),
                                   QString::fromStdWString(L"Станции")};
    m_TraverceModel->setHorizontalHeaderLabels(TraversesHeader);
    QStringList StationsHeader = {QString::fromStdWString(L"Станция"),
                                   QString::fromStdWString(L"Задняя"),
                                  QString::fromStdWString(L"Передняя"),
                                  QString::fromStdWString(L"Z зад"),
                                   QString::fromStdWString(L"Z пер"),

                                   QString::fromStdWString(L"Z ст")};
    m_StationModel->setHorizontalHeaderLabels(StationsHeader);
    QStringList ObservationHeader = {QString::fromStdWString(L"Имя"),
                                   QString::fromStdWString(L"Направление"),
                                  QString::fromStdWString(L"R"),
                                  QString::fromStdWString(L"HD"),
                                   QString::fromStdWString(L"Z")
                                   };
    m_ObservationModel->setHorizontalHeaderLabels(ObservationHeader);

}

void Traverce::UpdateStationModel(Traverse *traverse)
{
    if(traverse==nullptr) return;
    m_StationModel->clear();
    UpdateTablesHeaders();
    for (int i=0;i<traverse->stations.count();++i)
    {
        Station *station=traverse->stations[i];
        if (station==nullptr) continue;
        QList<QStandardItem*> row;
        auto name_item = new QStandardItem(station->name);
        name_item->setData(i,Qt::UserRole);
        row.append(name_item);
        auto name_back = new QStandardItem(station->pointBack);
        row.append(name_back);
        auto name_fore = new QStandardItem(station->pointFore);
        row.append(name_fore);
        auto z_back = new QStandardItem(QString::number(station->Z_back,'f',6));
        row.append(z_back);
        auto z_fore = new QStandardItem(QString::number(station->Z_fore,'f',6));
        row.append(z_fore);
        auto z_station = new QStandardItem(QString::number(ZStation(station),'f',6));
        row.append(z_station);
        m_StationModel->appendRow(row);


    }

}

void Traverce::UpdateObservationModel(Station *station)
{
    if (station==nullptr) return;
    m_ObservationModel->clear();
    UpdateTablesHeaders();
    for (int i=0;i<station->backs.count();++i)
    {
        Observation *obs=station->backs[i];
        if (obs==nullptr) continue;
        QList<QStandardItem *> row;
        auto name_item = new QStandardItem(obs->pointName);
        name_item->setData(i,Qt::UserRole);
        row.append(name_item);
        auto dir_item = new QStandardItem(QString::fromStdWString(L"Назад"));
        row.append(dir_item);
        auto R_item = new QStandardItem(QString::number(obs->r,'f',6));
        row.append(R_item);
        auto HD_item = new QStandardItem(QString::number(obs->hd,'f',6));
        row.append(HD_item);
        auto Z_item = new QStandardItem(QString());
        row.append(Z_item);
        m_ObservationModel->appendRow(row);

    }
    for (int i=0;i<station->fores.count();++i)
    {
        Observation *obs=station->fores[i];
        if (obs==nullptr) continue;
        QList<QStandardItem *> row;
        auto name_item = new QStandardItem(obs->pointName);
        name_item->setData(i,Qt::UserRole);
        row.append(name_item);
        auto dir_item = new QStandardItem(QString::fromStdWString(L"Вперед"));
        row.append(dir_item);
        auto R_item = new QStandardItem(QString::number(obs->r,'f',6));
        row.append(R_item);
        auto HD_item = new QStandardItem(QString::number(obs->hd,'f',6));
        row.append(HD_item);
        auto Z_item = new QStandardItem(QString());
        row.append(Z_item);
        m_ObservationModel->appendRow(row);

    }
    for (int i=0;i<station->sideshots.count();++i)
    {
        Observation *obs=station->sideshots[i];
        if (obs==nullptr) continue;
        QList<QStandardItem *> row;
        auto name_item = new QStandardItem(obs->pointName);
        name_item->setData(i,Qt::UserRole);
        row.append(name_item);
        auto dir_item = new QStandardItem(QString::fromStdWString(L"Вбок"));
        row.append(dir_item);
        auto R_item = new QStandardItem(QString::number(obs->r,'f',6));
        row.append(R_item);
        auto HD_item = new QStandardItem(QString::number(obs->hd,'f',6));
        row.append(HD_item);
        auto Z_item = new QStandardItem(QString::number(obs->z,'f',6));
        row.append(Z_item);
        m_ObservationModel->appendRow(row);

    }


}

double Traverce::ZStation(Station *station)
{
    if(station==nullptr) return std::numeric_limits<double>().infinity();
    double z_back=station->Z_back;
    if (!station->backs.isEmpty())
    {
        double R_backs=std::accumulate(station->backs.begin(),station->backs.end(),0.0,
                                       [](double sum,Observation * obs_b)
                                         {return sum+obs_b->r;}
                                        );
        R_backs=R_backs/station->backs.count();
        return z_back+R_backs;
    }
    else
    {
        return std::numeric_limits<double>().infinity();
    }
}

