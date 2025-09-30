#include "traverseparser.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QQueue>
#include <QDebug>

/*static double parseZValue(const QString &field)
{
    if (!field.startsWith("Z")) return 0.0;
    QString zStr = field.mid(1).trimmed();
    if (zStr.endsWith("m")) zStr.chop(1);
    bool ok;
    double val = zStr.toDouble(&ok);
    return ok ? val : 0.0;
}

static bool parseZLine(const QString &line, QString &pointName, double &Z)
{
    QStringList parts = line.split('|', Qt::SkipEmptyParts);
    if (parts.size() < 5) return false;

    QString field2 = parts[2].trimmed();
    if (!field2.startsWith("KD1")) return false;
    if (field2.contains("#####")) return false;

    QString clean = field2.mid(4).trimmed();
    QStringList tokens = clean.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    if (tokens.isEmpty()) return false;
    pointName = tokens[0];

    QString field3 = parts[3].trimmed();
    if (!field3.isEmpty()) return false;

    QString field5 = parts.size() > 5 ? parts[5].trimmed() : QString();
    QString field6 = parts.size() > 6 ? parts[6].trimmed() : QString();

    if (field5.startsWith("Z")) {
        Z = parseZValue(field5);
        return true;
    }
    if (field6.startsWith("Z")) {
        Z = parseZValue(field6);
        return true;
    }
    return false;
}

static bool parseObservationLine(const QString &line, QString &pointName, Observation &obs)
{
    QStringList parts = line.split('|', Qt::SkipEmptyParts);
    if (parts.size() < 5) return false;

    QString field2 = parts[2].trimmed();
    if (!field2.startsWith("KD1")) return false;
    if (field2.contains("#####")) return false;

    QString clean = field2.mid(4).trimmed();
    QStringList tokens = clean.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    if (tokens.isEmpty()) return false;
    pointName = tokens[0];

    QString field3 = parts[3].trimmed();
    if (field3.isEmpty()) return false;

    QStringList rd = field3.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    if (rd.size() < 2) return false;

    obs.pointName = pointName;
    obs.type = rd[0];
    QString val = rd[1];
    if (val.endsWith("m")) val.chop(1);
    bool ok;
    obs.reading = val.toDouble(&ok);
    if (!ok) return false;

    QString field4 = parts[4].trimmed();
    if (field4.startsWith("HD")) {
        QStringList hd = field4.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (hd.size() >= 2) {
            QString hdVal = hd[1];
            if (hdVal.endsWith("m")) hdVal.chop(1);
            obs.hd = hdVal.toDouble();
        }
    }

    return true;
}

static Station createStationFromSixLines(
    const QVector<QString> &sixLines,
    const QString &pattern,
    int lineNo,
    int stationIndex)
{
    Station st;
    st.name = QString("%1.%2").arg(lineNo).arg(stationIndex);

    QString pointBack, pointFore;
    double Z_back = 0.0, Z_fore = 0.0;

    if (!parseZLine(sixLines[0], pointBack, Z_back)) return st;
    st.pointBack = pointBack;
    st.Z_back = Z_back;

    QVector<Observation> obsList;
    for (int i = 1; i <= 4; ++i) {
        Observation obs;
        QString pt;
        if (!parseObservationLine(sixLines[i], pt, obs)) {
            return st;
        }
        obsList.append(obs);
    }

    if (!parseZLine(sixLines[5], pointFore, Z_fore)) return st;
    st.pointFore = pointFore;
    st.Z_fore = Z_fore;

    // Проверка соответствия точек
    if (obsList[0].pointName != pointBack || obsList[3].pointName != pointBack ||
        obsList[1].pointName != pointFore || obsList[2].pointName != pointFore) {
        return st;
    }

    st.backs = { obsList[0], obsList[3] };
    st.fores = { obsList[1], obsList[2] };

    // Проверка шаблона
    QString actualPattern = obsList[0].type + obsList[1].type + obsList[2].type + obsList[3].type;
    QString expectedPattern;
    for (QChar c : pattern) {
        expectedPattern += (c == 'B') ? "Rb" : "Rf";
    }
    st.isValid = (actualPattern == expectedPattern);

    return st;
}

static Traverse parseTraverseFromLines(const QVector<QString> &lines)
{
    Traverse trav;
    if (lines.isEmpty()) return trav;

    QString startLine = lines[0];
    QRegularExpression re(R"(Start-Line\s+(\w+)\s+(\d+))");
    auto match = re.match(startLine);
    if (!match.hasMatch()) return trav;
    trav.pattern = match.captured(1);
    trav.lineNo = match.captured(2).toInt();

    // Собираем все KD1-строки (без #####)
    QVector<QString> kd1Lines;
    for (int i = 1; i < lines.size(); ++i) {
        QString line = lines[i];
        QStringList parts = line.split('|', Qt::SkipEmptyParts);
        if (parts.size() < 5) continue;
        QString f2 = parts[2].trimmed();
        if (f2.startsWith("KD1") && !f2.contains("#####")) {
            kd1Lines.append(line);
        }
    }

    // Группируем по 6 строк
    int idx = 1;
    for (int i = 0; i + 5 < kd1Lines.size(); i += 6) {
        QVector<QString> block;
        for (int j = 0; j < 6; ++j) block.append(kd1Lines[i + j]);
        Station st = createStationFromSixLines(block, trav.pattern, trav.lineNo, idx++);
        if (st.isValid) {
            trav.stations.append(st);
        }
    }

    // Добавляем боковые измерения (Rz)
    int currentStationIndex = -1;
    bool insideIntermediate = false;

    for (int i = 1; i < lines.size(); ++i) {
        QString line = lines[i];
        QStringList parts = line.split('|', Qt::SkipEmptyParts);
        if (parts.size() < 5) continue;

        QString f2 = parts[2].trimmed();

        if (f2.startsWith("TO")) {
            if (f2.contains("Intermediate sight.")) {
                insideIntermediate = true;
                // Найти последнюю станцию, чья pointFore == точке в начале блока
                // Для простоты — привяжем к последней станции
                if (!trav.stations.isEmpty()) {
                    currentStationIndex = trav.stations.size() - 1;
                }
                continue;
            }
            if (f2.contains("End of interm. sight.")) {
                insideIntermediate = false;
                currentStationIndex = -1;
                continue;
            }
            continue;
        }

        if (insideIntermediate && f2.startsWith("KD1") && !f2.contains("#####")) {
            Observation obs;
            QString pt;
            if (parseObservationLine(line, pt, obs) && obs.type == "Rz") {
                if (currentStationIndex >= 0 && currentStationIndex < trav.stations.size()) {
                    trav.stations[currentStationIndex].sideshots.append(obs);
                }
            }
        }
    }

    return trav;
}
*/
QVector<Traverse> TraverseParser::parseFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file:" << filePath;
        return {};
    }

    QTextStream in(&file);
    QVector<Traverse> result;
    QStringList currentBlock;
    bool inside = false;
    static const QRegularExpression re_start_traverce("TO\\s*(?P<kewword>Start[- ]+Line)\\s+(?P<pattern>[BF]{4})\\s*(?P<number>(\\d+))");
    static const QRegularExpression re_end_traverce("TO\\s*(?P<kewword>End[- ]Line)\\s+(?P<number>(\\d+))");
    Traverse temp_traverce;
    while (!in.atEnd()) {

        QString line = in.readLine();
        QStringList parts=line.split('|');
        QRegularExpressionMatch start_match = re_start_traverce.match(parts[2]);

        if (start_match.hasMatch())
        {
            QString pattern = start_match.captured(QString::fromStdWString(L"pattern"));
            int line_no = start_match.captured(QString::fromStdWString(L"number")).toInt();
            temp_traverce=Traverse();
            temp_traverce.lineNo=line_no;
            temp_traverce.pattern=pattern;
            inside=true;
            currentBlock.clear();
            currentBlock.append(line);
            continue;

        }
        if (inside)
        {
            if (parts[2].contains(QString::fromStdWString(L"Station repeated"))||
                    parts[2].contains(QString::fromStdWString(L"Measurement repeated"))||
                    parts[2].contains(QString::fromStdWString(L"##")))
            {
                continue;
            }
            QRegularExpressionMatch end_match = re_end_traverce.match(parts[2]);
            if(end_match.hasMatch())
            {
                currentBlock.append(line);
                if (parseBlock(currentBlock,temp_traverce))
                {
                   result.append(temp_traverce);

                }
                inside=false;
                continue;

            }
            currentBlock.append(line);
        }



    }


    file.close();
    return result;
}

bool TraverseParser::parseBlock(const QStringList &Block, Traverse &traverse)
{

    qDebug()<<Block.size()<<traverse.lineNo<<traverse.pattern;
    int station_index=0;

    Station* temp_station=nullptr;
    //bool inside_foresight = false;
    QStringList foresight_block;
    //bool begin_station=false;
    static const QRegularExpression st_re("KD\\d+\\s+(?P<name>\\S+)\\s+(?P<lineno>\\d+)$");
    static const QRegularExpression meas_re("KD\\d+\\s+(?P<name>\\S+)\\s+(?P<temperature>-?\\d+\\.\\d+)\\s+(C|F)\\s*(?P<code>\\d+)\\s+(?P<lineno>\\d+)");
    QQueue<QString> station_queue;
    for (int i=1;i<Block.size()-1;++i)
    {
        auto block_line=Block[i];




        auto data=block_line.split('|');

        if (station_queue.size()==(traverse.pattern.length()+2))
        {

            temp_station= new Station();
            temp_station->name=QString::fromStdWString(L"%1.%2").arg(traverse.lineNo).arg(station_index);
            parceStation(station_queue,temp_station,traverse.pattern);
            traverse.stations.append(temp_station);
            station_index++;




        }

        if (data[2].contains(QString::fromStdWString(L"Intermediate sight.")))
        {
            foresight_block.clear();
            for (int j=i+1;j<Block.size();j++)
            {
                auto data_fs=Block[j].split('|');
                if (data_fs[2].contains(QString::fromStdWString(L"End of interm. sight.")))
                {

                    parseForesightBlock(foresight_block,temp_station);
                    i=j;

                    break;
                }
                foresight_block.append(Block[j]);


            }
            continue;
        }





        if (data[3].contains(QString::fromStdWString(L"Sh"))&&
            data[4].contains(QString::fromStdWString(L"dz")))
        {
            bool ok = false;
            double tmp=parseDouble(data[3],ok);
            if(ok) traverse.sh=tmp;
            tmp=parseDouble(data[4],ok);
            if(ok) traverse.dz=tmp;
            continue;

        }
        if (data[3].contains(QString::fromStdWString(L"Db"))&&
            data[4].contains(QString::fromStdWString(L"Df")))
        {
            bool ok = false;
            double tmp=parseDouble(data[3],ok);
            if(ok) traverse.hd_back=tmp;
            tmp=parseDouble(data[4],ok);
            if(ok) traverse.hd_front=tmp;
            continue;

        }

        auto match_st=st_re.match(data[2]);

        if (match_st.hasMatch())
        {
            station_queue.enqueue(block_line);
            continue;
        }
        auto match_re=meas_re.match(data[2]);
        {
            station_queue.enqueue(block_line);
            continue;
        }





    }



    return true;
}

double TraverseParser::parseDouble(const QString &data, bool &ok)
{
    static const QRegularExpression double_re("(-?\\d+\\.\\d+)");
    QRegularExpressionMatch dmatch=double_re.match(data);
    if (dmatch.hasMatch())
    {
        QString d_data=dmatch.captured();
        ok=true;
        return d_data.toDouble();
    }
    else
    {
        ok=false;
        return 0.0;
    }

}

bool TraverseParser::parceStation(QQueue<QString> &data, Station *station, const QString &pattern)
{
   QList<Observation*> obs;
   QString pat=QString();
   for (const auto &d:data)
   {
       Observation* temp=parseLine(d);
       obs.push_back(temp);
   }
   for (const auto &observation:obs)
   {
     if (observation==nullptr) continue;
     if (observation->type=="Rb")
     {
     station->backs.push_back(observation);
     pat+="B";
     }
    if (observation->type=="Rf")
    {
    station->fores.push_back(observation);
    pat+="F";
    }
   }
   for(const auto &st:std::as_const(station->fores))
   {
        for (const auto &observation:obs)
        {
            if(observation->pointName==st->pointName && observation->type=="Z")
            {
                station->pointFore=observation->pointName;
                station->Z_fore=observation->z;
                break;
            }
        }
   }
   for(const auto &st:std::as_const(station->backs))
   {
        for (const auto &observation:obs)
        {
            if(observation->pointName==st->pointName && observation->type=="Z")
            {
                station->pointBack=observation->pointName;
                station->Z_back=observation->z;
                break;
            }
        }
   }
   if (pat==pattern)
   {
       station->isValid=true;
   }



    while (data.size()!=1)
    {
        data.dequeue();
    }
    return true;
}



Observation* TraverseParser::parseLine(const QString &line)
{
    Observation* result=new Observation();
    auto data = line.split('|');
    static const QRegularExpression z_line_re ("KD\\d+\\s+(?P<name>\\S+)\\s+(?P<lineno>\\d+)$");
    static const QRegularExpression end_line_re  ("KD\\d+\\s+(?P<name>\\S+)\\s+(?P<temperature>-?\\d+\\.\\d+)\\s+(C|F)\\s*\\s+(?P<lineno>\\d+)$");
    static const QRegularExpression meas_line_re ("KD\\d+\\s+(?P<name>\\S+)\\s+(?P<temperature>-?\\d+\\.\\d+)\\s+(C|F)\\s*(?P<code>\\d+)\\s+(?P<lineno>\\d+)");
    auto z_line_match=z_line_re.match(data[2]);
    auto meas_line_match=meas_line_re.match(data[2]);
    auto end_line_match=end_line_re.match(data[2]);
    bool ok;
    if (z_line_match.hasMatch())
    {

        result->pointName=z_line_match.captured("name");
        result->line_no=z_line_match.captured("lineno").toInt();
        result->type=QString::fromStdWString(L"Z");
        result->z=parseDouble(data[5],ok);
        return result;

    }

    if(meas_line_match.hasMatch())
    {
        if (data[3].contains("Rb"))
        {
           result->pointName= meas_line_match.captured("name");
           result->temperature=meas_line_match.captured("temperature").toDouble();
           result->code=meas_line_match.captured("code").toInt();
           result->line_no = meas_line_match.captured("lineno").toInt();
           result->r=parseDouble(data[3],ok);
           result->hd=parseDouble(data[4],ok);
           result->type=QString::fromStdWString(L"Rb");
        }
        if (data[3].contains("Rf"))
        {
           result->pointName= meas_line_match.captured("name");
           result->temperature=meas_line_match.captured("temperature").toDouble();
           result->code=meas_line_match.captured("code").toInt();
           result->line_no = meas_line_match.captured("lineno").toInt();
           result->r=parseDouble(data[3],ok);
           result->hd=parseDouble(data[4],ok);
           result->type=QString::fromStdWString(L"Rf");
        }
        if (data[3].contains("Rz"))
        {
           result->pointName= meas_line_match.captured("name");
           result->temperature=meas_line_match.captured("temperature").toDouble();
           result->code=meas_line_match.captured("code").toInt();
           result->line_no = meas_line_match.captured("lineno").toInt();
           result->r=parseDouble(data[3],ok);
           result->hd=parseDouble(data[4],ok);
           result->z=parseDouble(data[5],ok);
           result->type=QString::fromStdWString(L"Rz");
        }


        return result;

    }
    if (end_line_match.hasMatch())
    {
          result->pointName=end_line_match.captured("name");
          result->line_no=end_line_match.captured("lineno").toInt();
          result->type=QString::fromStdWString(L"Z");
          result->z=parseDouble(data[5],ok);
          return result;



    }
    delete result;


    return nullptr;

}

bool TraverseParser::parseForesightBlock(const QStringList &block, Station *Station)
{
    if (Station==nullptr) return false;
    for (const auto &line:block)
    {
    Station->sideshots.push_back(parseLine(line));
    }
    return true;

}

//KD\d+\s+(?P<name>\S+)\s+(?P<temperature>-?\d+\.\d+)\s+(C|F)\s*(?P<code>\d+)\s+(?P<lineno>\d+)
//KD\d+\s+(?P<name>\S+)\s+(?P<lineno>\d+)$
