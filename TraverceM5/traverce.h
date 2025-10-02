#ifndef TRAVERCE_H
#define TRAVERCE_H

#include <QDialog>
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include "traverseparser.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Traverce; }
QT_END_NAMESPACE

class Traverce : public QDialog
{
    Q_OBJECT

public:
    Traverce(QWidget *parent = nullptr);
    ~Traverce();

private:
    void OpenFile_btn_clicked(void);
    void UpdateTraverces();
    void TraverseSelection_changed(const QItemSelection &selected, const QItemSelection &deselected);
    void StationSelection_changed(const QItemSelection &selected, const QItemSelection &deselected);
    void UpdateTablesHeaders();
    void UpdateStationModel(Traverse *traverse=nullptr);
    void UpdateObservationModel(Station *station=nullptr);
    double ZStation(Station * station=nullptr);
    Ui::Traverce *ui;
    QStandardItemModel *m_TraverceModel=nullptr;
    QStandardItemModel *m_StationModel=nullptr;
    QStandardItemModel *m_ObservationModel=nullptr;

    QVector<Traverse*> m_Traverses;
    int m_selected_traverse=-1;


};
#endif // TRAVERCE_H
