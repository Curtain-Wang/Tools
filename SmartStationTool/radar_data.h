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
    QString radarPort;      // 雷达端口
    QString radarType;      // 雷达类型
    QString ifCover;          //是否覆盖
    QString errDesc;        //错误描述

    // 构造函数
    RadarData(const QString& junctionName = "", const QString& junctionId = "",
              const QString& stationIp = "", const QString& radarPosition = "",
              const QString& radarDirection = "", const QString& radarIp = "",
              const QString& radarPort = "0", const QString& radarType = "", const QString& ifCover = "")
        : junctionName(junctionName), junctionId(junctionId), stationIp(stationIp),
        radarPosition(radarPosition), radarDirection(radarDirection),
        radarIp(radarIp), radarPort(radarPort), radarType(radarType),
        ifCover(ifCover){
        errDesc = "";

    }

    // 将数据转换为JSON格式
    QJsonObject toJson() const {
        QJsonObject json;
        json["clazz"] = 206;
        json["direction"] = QString::number(RadarDirection::toInt(RadarDirection::fromString(radarDirection)));
        json["id"] = junctionId + QString::number(RadarDirection::toInt(RadarDirection::fromString(radarDirection))) + "301";
        json["ip"] = radarIp;
        if(radarPosition == "11")
        {
            json["location"] = QString::number(RadarDirection::toInt(RadarDirection::fromString(radarPosition)));
        }
        json["location"] = QString::number(RadarDirection::toInt(RadarDirection::fromString(radarPosition)));
        if(radarType == "中控")
        {
            json["manufactor"] = "supcon";
        }else if(radarType == "惠尔视(部分数据)")
        {
            json["manufactor"] = "hrs";
        }else if(radarType == "通用(全量数据)")
        {
            json["manufactor"] = "general-all";
        }else if(radarType == "通用(部分数据)")
        {
            json["manufactor"] = "general-part";
        }
        json["name"] = radarPosition + "照" + radarDirection;
        json["port"] = radarPort.toInt();
        json["protocol"] = "TCP";
        json["type"] = "radar";
        return json;
    }

    // 将数据转换为JSON格式
    QJsonObject toDelJson() const {
        QJsonObject json;
        json["id"] = junctionId + QString::number(RadarDirection::toInt(RadarDirection::fromString(radarDirection))) + "301";
        return json;
    }

    //自定义排序规则
    static bool compareByNameAndPosition(const RadarData &a, const RadarData &b){
        if(a.junctionName != b.junctionName){
            return a.junctionName < b.junctionName;
        }else{
            return QString::number(RadarDirection::toInt(RadarDirection::fromString(a.radarPosition))) < QString::number(RadarDirection::toInt(RadarDirection::fromString(b.radarPosition)));
        }
    }
};

#endif // RADARDATA_H
