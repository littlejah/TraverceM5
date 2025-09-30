#include "traverseparser.h"
#include <QDebug>

QList<Traverse> TraverseParser::parseFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file:" << filePath;
        return {};
    }

    QTextStream in(&file);
    QList<Traverse> traverses;
    Traverse currentTraverse;
    Station currentStation;
    bool insideStation = false;
    bool insideIntermediate = false;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList parts = line.split('|', Qt::SkipEmptyParts);
        if (parts.size() < 5) continue;

        QString field2 = parts[2].trimmed();

        // === TO команды ===
        if (field2.startsWith("TO")) {
            if (field2.contains("Start-Line")) {
                // Завершаем предыдущую станцию, если есть
                if (insideStation) {
                    currentTraverse.stations.append(currentStation);
                    insideStation = false;
                }

                // Начинаем новый ход
                QRegularExpression reStart(R"(Start-Line\s+(\w+)\s+(\d+))");
                QRegularExpressionMatch m = reStart.match(field2);
                if (m.hasMatch()) {
                    QString pattern = m.captured(1);
                    int lineNo = m.captured(2).toInt();

                    if (pattern != "BFFB" && pattern != "FBBF" && pattern != "BFBF" && pattern != "FBFB") {
                        qWarning() << "Unknown pattern:" << pattern;
                        continue;
                    }

                    // Если уже есть ход — сохраняем
                    if (currentTraverse.lineNo != -1) {
                        traverses.append(currentTraverse);
                    }

                    currentTraverse = Traverse{lineNo, pattern, {}};
                }
                continue;
            }

            if (field2.contains("End-Line")) {
                if (insideStation) {
                    currentTraverse.stations.append(currentStation);
                    insideStation = false;
                }
                continue;
            }

            if (field2.contains("Station repeated")) {
                if (insideStation) {
                    // Сбрасываем измерения, но сохраняем stationPoint и Z (если уже заданы)
                    currentStation.backs.clear();
                    currentStation.fores.clear();
                    currentStation.sideshots.clear();
                    currentStation.actualSequence.clear();
                }
                continue;
            }

            if (field2.contains("Measurement repeated")) {
                // Игнорируем полностью
                continue;
            }

            if (field2.contains("Intermediate sight.")) {
                insideIntermediate = true;
                continue;
            }

            if (field2.contains("End of interm. sight.")) {
                insideIntermediate = false;
                continue;
            }

            continue;
        }

        // === KD1 ===
        if (field2.startsWith("KD1")) {
            // Проверка на #####
            if (field2.contains("#####")) {
                continue; // игнорируем полностью
            }

            // Извлечение данных
            QString cleanField2 = field2.mid(4).trimmed();
            QStringList tokens = cleanField2.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            if (tokens.isEmpty()) continue;

            Observation obs;
            obs.pointName = tokens[0];

            // Парсинг температуры и призмы (опционально)
            for (int i = 1; i < tokens.size(); ++i) {
                if (tokens[i] == "C") {
                    if (i >= 2) {
                        obs.temperature = tokens[i-1].toDouble();
                    }
                } else if (i == tokens.size() - 1 && tokens[i].toInt() > 0) {
                    // последний токен — номер станции, игнорируем
                } else if (tokens[i].toInt() > 0 && tokens[i].size() <= 2) {
                    obs.prism = tokens[i].toInt();
                }
            }

            // Поле 3: тип и отсчёт
            QString field3 = parts[3].trimmed();
            if (!field3.isEmpty()) {
                QStringList rd = field3.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
                if (rd.size() >= 2) {
                    obs.type = rd[0];
                    QString val = rd[1];
                    if (val.endsWith("m")) val.chop(1);
                    obs.reading = val.toDouble();
                }
            }

            // Поле 4: HD
            QString field4 = parts[4].trimmed();
            if (field4.startsWith("HD")) {
                QStringList hdParts = field4.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
                if (hdParts.size() >= 2) {
                    QString hdVal = hdParts[1];
                    if (hdVal.endsWith("m")) hdVal.chop(1);
                    obs.hd = hdVal.toDouble();
                }
            }

            // Поле 5: Z (если есть)
            if (parts.size() > 5) {
                QString field5 = parts[5].trimmed();
                if (field5.startsWith("Z")) {
                    QString zVal = field5.mid(1).trimmed();
                    if (zVal.endsWith("m")) zVal.chop(1);
                    double Z = zVal.toDouble();

                    // Если это строка без отсчёта — это точка стояния
                    if (field3.isEmpty()) {
                        if (!insideStation) {
                            // Начинаем новую станцию
                            currentStation = Station{};
                            currentStation.lineNo = currentTraverse.lineNo;
                            currentStation.stationNo = tokens.last().toInt();
                            currentStation.name = QString("%1.%2").arg(currentTraverse.lineNo).arg(currentStation.stationNo);
                            currentStation.stationPoint = obs.pointName;
                            currentStation.Z = Z;
                            currentStation.expectedPattern = currentTraverse.pattern;
                            insideStation = true;
                        } else {
                            // Уже внутри станции — обновляем Z
                            currentStation.Z = Z;
                        }
                    }
                }
            }

            // Добавление измерений
            if (insideStation) {
                if (obs.type == "Rb" || obs.type == "Rf") {
                    currentStation.actualSequence.append(obs.type);
                    if (obs.type == "Rb") {
                        currentStation.backs.append(obs);
                    } else if (obs.type == "Rf") {
                        currentStation.fores.append(obs);
                    }
                } else if (obs.type == "Rz" && insideIntermediate) {
                    currentStation.sideshots.append(obs);
                }
            }
            continue;
        }

        // KD2 и другие — игнорируем
    }

    // Завершаем последнюю станцию и ход
    if (insideStation) {
        currentTraverse.stations.append(currentStation);
    }
    if (currentTraverse.lineNo != -1) {
        // Валидация станций
        for (auto &st : currentTraverse.stations) {
            if (st.actualSequence.size() == st.expectedPattern.size()) {
                bool match = true;
                for (int i = 0; i < st.expectedPattern.size(); ++i) {
                    char c = st.expectedPattern[i].toLatin1();
                    QString expectedType = (c == 'B') ? "Rb" : "Rf";
                    if (st.actualSequence[i] != expectedType) {
                        match = false;
                        break;
                    }
                }
                st.isValid = match;
            } else {
                st.isValid = false;
            }
        }
        traverses.append(currentTraverse);
    }

    file.close();
    return traverses;
}