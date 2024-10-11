#ifndef RADARDIRECTION_H
#define RADARDIRECTION_H

#include <QString>
#include <QMap>

class RadarDirection {
public:
    enum Direction {
        EAST = 1,
        SOUTH = 2,
        WEST = 3,
        NORTH = 4,
        D1 = 5,
        D2 = 6,
        D3 = 7,
        D4 = 8
    };

    // 静态函数：根据字符串返回对应的枚举值
    static Direction fromString(const QString& directionStr) {
        static const QMap<QString, Direction> directionMap = {
            {"东", EAST},
            {"南", SOUTH},
            {"西", WEST},
            {"北", NORTH},
            {"方向一", D1},
            {"方向二", D2},
            {"方向三", D3},
            {"方向", D4}
        };

        // 默认返回 EAST，如果找不到对应的方向
        return directionMap.value(directionStr, EAST);
    }

    // 静态函数：返回方向枚举的整数值
    static int toInt(Direction direction) {
        return static_cast<int>(direction);
    }
};

#endif // RADARDIRECTION_H
