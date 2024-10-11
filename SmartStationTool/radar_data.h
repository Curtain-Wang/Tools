#ifndef RADARDATA_H
#define RADARDATA_H
#include "radar_direction.h"
#include <QString>
#include <QJsonObject>
class RadarData
{
public:
    QString junctionName;   // 路口名称
    QString junctionId;     // 路口ID
    QString stationIp;      // 小站IP
    QString radarPosition;  // 雷达安装位置
    QString radarDirection; // 雷达照射方向
    QString radarIp;        // 雷达IP
    qint16 radarPort;      // 雷达端口
    QString radarType;      // 雷达类型

    // 构造函数
    RadarData(const QString& junctionName = "", const QString& junctionId = "",
              const QString& stationIp = "", const QString& radarPosition = "",
              const QString& radarDirection = "", const QString& radarIp = "",
              const quint16& quint16 = 0, const QString& radarType = "")
        : junctionName(junctionName), junctionId(junctionId), stationIp(stationIp),
        radarPosition(radarPosition), radarDirection(radarDirection),
        radarIp(radarIp), radarPort(radarPort), radarType(radarType) {}

    // 将数据转换为JSON格式
    QJsonObject toJson() const {
        QJsonObject json;
        json["clazz"] = 206;
        json["direction"] = QString::number(RadarDirection::toInt(RadarDirection::fromString(radarDirection)));
        json["id"] = QString("%1%2301").arg(junctionId).arg(RadarDirection::toInt(RadarDirection::fromString(radarDirection)));
        json["ip"] = radarIp;
        json["location"] = QString::number(RadarDirection::toInt(RadarDirection::fromString(radarPosition)));
        json["manufactor"] = radarType;
        json["name"] = junctionName + radarPosition;
        json["port"] = radarPort;
        json["protocol"] = "TCP";
        json["type"] = "radar";
        return json;
    }
};

#endif // RADARDATA_H
