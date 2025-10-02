#ifndef TRAVERSEPARSER_H
#define TRAVERSEPARSER_H

#include <QString>
#include <QVector>
#include <QMap>


struct Observation {
    QString pointName;
    double temperature=0.0;
    int code=0;
    int line_no=0;
    double r = 0.0;
    double hd = 0.0;
    double z=0.0;
    QString type; // "Rb", "Rf", "Rz"
};

struct Station {
    QString name; // e.g. "168.1"
    QString pointBack;
    QString pointFore;
    double Z_back = 0.0;
    double Z_fore = 0.0;
    QVector<Observation*> backs=QVector<Observation*>();
    QVector<Observation*> fores=QVector<Observation*>();
    QVector<Observation*> sideshots=QVector<Observation*>();
    bool isValid = false;
    ~Station();
};

struct Traverse {
    int lineNo = -1;
    QString pattern; // BFFB, FBBF, etc.
    QVector<Station*> stations =QVector<Station*>();
    double hd_front=0;
    double hd_back=0;
    double sh=0;
    double dz=0;
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
