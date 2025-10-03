#include "traverseparser.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QQueue>
#include <QDebug>


QVector<Traverse*> TraverseParser::parseFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file:" << filePath;
        return {};
    }

    QTextStream in(&file);
    QVector<Traverse*> result;
    QStringList currentBlock;
    bool inside = false;
    static const QRegularExpression re_start_traverce("TO\\s*(?P<kewword>Start[- ]+Line)\\s+(?P<pattern>[BF]{4})\\s*(?P<number>(\\d+))");
    static const QRegularExpression re_end_traverce("TO\\s*(?P<kewword>End[- ]Line)\\s+(?P<number>(\\d+))");
    Traverse *temp_traverce=nullptr;
    while (!in.atEnd()) {

        QString line = in.readLine();
        QStringList parts=line.split('|');
        QRegularExpressionMatch start_match = re_start_traverce.match(parts[2]);

        if (start_match.hasMatch())
        {
            QString pattern = start_match.captured(QString::fromStdWString(L"pattern"));
            int line_no = start_match.captured(QString::fromStdWString(L"number")).toInt();
            temp_traverce= new Traverse();
            temp_traverce->lineNo=line_no;
            temp_traverce->pattern=pattern;
            inside=true;
            currentBlock.clear();
            currentBlock.append(line);
            continue;

        }
        if (inside)
        {
            if (//parts[2].contains(QString::fromStdWString(L"Station repeated"))||
                    parts[2].contains(QString::fromStdWString(L"Measurement repeated"))||
                    parts[2].contains(QString::fromStdWString(L"##"))
                        )
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

bool TraverseParser::parseBlock(const QStringList &Block, Traverse *traverse)
{
    if (traverse==nullptr) return false;
    qDebug()<<Block.size()<<traverse->lineNo<<traverse->pattern;
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
        if (data[2].contains(QString::fromStdWString(L"Station repeated"))) continue;

        if (station_queue.size()==(traverse->pattern.length()+2))
        {

            temp_station= new Station();
            temp_station->name=QString::fromStdWString(L"%1.%2").arg(traverse->lineNo).arg(station_index);
            parceStation(station_queue,temp_station,traverse->pattern);
            traverse->stations.append(temp_station);
            station_index++;




        }

        if (data[2].contains(QString::fromStdWString(L"Intermediate sight.")))
        {
            foresight_block.clear();
            for (int j=i+1;j<Block.size();j++)
            {
                auto data_fs=Block[j].split('|');
                if (data_fs[2].contains(QString::fromStdWString(L"End of interm. sight."))||
                    data_fs[2].contains(QString::fromStdWString(L"Station repeated")))
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
            if(ok) traverse->sh=tmp;
            tmp=parseDouble(data[4],ok);
            if(ok) traverse->dz=tmp;
            continue;

        }
        if (data[3].contains(QString::fromStdWString(L"Db"))&&
            data[4].contains(QString::fromStdWString(L"Df")))
        {
            bool ok = false;
            double tmp=parseDouble(data[3],ok);
            if(ok) traverse->hd_back=tmp;
            tmp=parseDouble(data[4],ok);
            if(ok) traverse->hd_front=tmp;
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

Station::~Station()
{
    for (int i=0;i<fores.count();++i)
    {
        if (fores[i]!=nullptr) delete fores[i];
    }
    for (int i=0;i<backs.count();++i)
    {
        if (backs[i]!=nullptr) delete backs[i];
    }
    for (int i=0;i<sideshots.count();++i)
    {
        if (sideshots[i]!=nullptr) delete sideshots[i];
    }
}

Traverse::~Traverse()
{
    for(int i=0;i<stations.count();++i)
    {
        if (stations[i]!=nullptr) delete stations[i];
    }
}
