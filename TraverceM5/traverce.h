#ifndef TRAVERCE_H
#define TRAVERCE_H

#include <QDialog>
#include <QStandardItemModel>
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
    Ui::Traverce *ui;
    QStandardItemModel *m_TraverceModel=nullptr;
    QStandardItemModel *m_StationModel=nullptr;
    QStandardItemModel *m_ObservationModel=nullptr;

    QVector<Traverse> m_Traverses;

};
#endif // TRAVERCE_H
