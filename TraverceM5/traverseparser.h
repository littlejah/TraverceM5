#ifndef TRAVERSEPARSER_H
#define TRAVERSEPARSER_H

#include <QString>
#include <QVector>
#include <QMap>


struct Observation {  // Измерение на точку
    QString pointName; // Имя точки
    double temperature=0.0; // Температура воздуха
    int code=0;             // Полевой код
    int line_no=0;          // Номер нивелирного хода
    double r = 0.0;         // Измеренное превышение
    double hd = 0.0;        // Горизонтальное проложение
    double z=0.0;           // Измеренная вычисленная Z
    QString type; // Тип измерения "Rb" - назад, "Rf" - вперед, "Rz" - боковое
};

struct Station {     // Станция
    QString name; //  Имя станции "НомерХода.ИндексСтанции"
    QString pointBack; // Имя точки ориентирование назад
    QString pointFore; // Имя точки ориентирование вперед
    double Z_back = 0.0; // Z задней точки
    double Z_fore = 0.0; // Z передней точки
    QVector<Observation*> backs=QVector<Observation*>(); // Измерения на заднюю точку
    QVector<Observation*> fores=QVector<Observation*>(); // Измерения на переднюю точку
    QVector<Observation*> sideshots=QVector<Observation*>(); // Боковые измерения
    bool isValid = false;   // Валидность станции
    ~Station();
};

struct Traverse {    // Нивелирный ход
    int lineNo = -1; // Номер хода
    QString pattern; // Порядок измерения BFFB, FBBF, etc.
    QVector<Station*> stations =QVector<Station*>(); // станции хода
    double hd_front=0; // Горизонтальное проложение вперед
    double hd_back=0; // Горизонтальное проложение назад
    double sh=0;   // Суммарное превишение назад
    double dz=0; // Суммарное превишение вперед
    ~Traverse();
};

class TraverseParser
{
public:
    QVector<Traverse*> parseFile(const QString &filePath);
private:
    bool parseBlock(const QStringList &Block,Traverse *traverse);
    bool parseForesightBlock(const QStringList &block,Station *Station);
    double parseDouble(const QString &data,bool &ok);
    bool parceStation(QQueue<QString> &data,Station *station,const QString &pattern);
    Observation* parseLine(const QString &line);


};



#endif // TRAVERSEPARSER_H
