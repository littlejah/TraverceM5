// traverseparser.h
#ifndef TRAVERSEPARSER_H
#define TRAVERSEPARSER_H

#include <QString>
#include <QList>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>

struct Observation {
    QString pointName;
    double reading = 0.0;
    double hd = 0.0;
    QString type; // "Rb", "Rf", "Rz"
    int prism = -1;
    double temperature = 0.0;
};

struct Station {
    QString name; // LineNo.StationNo
    int lineNo = -1;
    int stationNo = -1;
    QString stationPoint;
    double Z = 0.0;
    QList<Observation> backs;
    QList<Observation> fores;
    QList<Observation> sideshots;
    QString expectedPattern; // BFFB и т.д.
    QList<QString> actualSequence; // порядок Rb/Rf
    bool isValid = false;
};

struct Traverse {
    int lineNo = -1;
    QString pattern;
    QList<Station> stations;
};

class TraverseParser
{
public:
    static QList<Traverse> parseFile(const QString &filePath);
};

#endif // TRAVERSEPARSER_H