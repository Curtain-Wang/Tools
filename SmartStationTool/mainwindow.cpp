#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <xlsxdocument.h>
#include "radar_data.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QByteArray>
#include <QStandardPaths>
#include <QRegularExpression>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/icons/images/icon.webp"));
    ui->pushButton->setEnabled(false);
    ui->progressBar->setFixedHeight(25);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addRadarData(RadarData radarData, bool isCovered)
{
    if(!verify(radarData))
    {
        return;
    }

    QNetworkAccessManager *manager = new QNetworkAccessManager();
    // 创建 HTTP 请求
    QNetworkRequest request(QUrl("http://" + radarData.stationIp + ":8888/smart_station/device/radar/add"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    // 构建 JSON 数据
    QJsonObject json;
    json["ver"] = "1.0";
    json["user"] = "2a9a43b58c1ad02ebe1920c63df693c4";
    json["id"] = "1728546698527";
    json["data"] = radarData.toJson();  // 嵌套的 data 部分

    QJsonDocument jsonDoc(json);
    QByteArray jsonData = jsonDoc.toJson(QJsonDocument::Compact);

    qDebug() << "请求体: " << jsonData;
    // 发送 POST 请求
    QNetworkReply *reply = manager->post(request, jsonData);

    // 处理服务器的响应
    QObject::connect(reply, &QNetworkReply::finished, [reply, this, radarData, isCovered]() mutable {
        // 检查是否有错误
        if (reply->error() == QNetworkReply::NoError) {
            // 获取服务器返回的响应数据
            QByteArray responseData = reply->readAll();

            // 打印原始响应数据
            qDebug() << "服务器响应：" << responseData;

            // 解析JSON响应
            QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
            if (jsonResponse.isObject()) {
                QJsonObject jsonObject = jsonResponse.object();

                // 获取 JSON 数据中的各个字段
                QString id = jsonObject.value("id").toString();
                int code = jsonObject.value("code").toInt();
                QString desc = jsonObject.value("desc").toString();
                // 打印解析后的数据
                qDebug() << "返回的Code:" << code;
                qDebug() << "返回的描述:" << desc;

                if (code == 0) {
                    success++;
                    if (isCovered) {
                        covered++;
                        printDetail(QString("覆盖【%1%2】进口的雷达成功!").arg(radarData.junctionName).arg(radarData.radarPosition));
                    } else {
                        printDetail(QString("添加【%1%2】进口的雷达成功!").arg(radarData.junctionName).arg(radarData.radarPosition));
                    }
                } else if (desc == "object already persistent" && radarData.ifCover == "是") {
                    //可以覆盖的雷达信息，先删除再添加
                    delAndAdd(radarData);
                } else {
                    failed++;
                    radarData.errDesc = desc;  // 将错误描述存储到 radarData 的 errDesc 字段
                    printDetail(QString("添加【%1%2】进口的雷达失败, 错误信息: %3").arg(radarData.junctionName).arg(radarData.radarPosition).arg(desc));
                    failedList.append(radarData);
                }
            }
        } else {
            failed++;
            radarData.errDesc = reply->errorString();  // 将网络错误描述存储到 radarData 的 errDesc 字段
            printDetail(QString("添加【%1%2】进口的雷达失败, 错误信息: %3").arg(radarData.junctionName).arg(radarData.radarPosition).arg(reply->errorString()));
            failedList.append(radarData);
        }

        // 更新进度条
        qDebug() << "success: " << success;
        qDebug() << "failed: " << failed;
        qDebug() << "total: " << total;
        ui->progressBar->setValue((success + failed) * 100 / total);

        // 当所有操作完成时，输出统计信息
        if (success + failed == total) {
            printDetail(QString("本次导入雷达信息结束, 共%1个, 成功%2个(其中覆盖%3个), 失败%4个.")
                            .arg(total)
                            .arg(success)
                            .arg(covered)
                            .arg(failed));
            if (failed > 0) {
                ui->pushButton->setEnabled(true);
            }
        }

        reply->deleteLater();
    });
}

void MainWindow::printDetail(QString detail)
{
    ui->plainTextEdit->appendPlainText(detail);
}

void MainWindow::delAndAdd(RadarData radarData)
{
    QNetworkAccessManager *manager = new QNetworkAccessManager();
    // 创建 HTTP 请求
    QNetworkRequest request(QUrl("http://" + radarData.stationIp + ":8888/smart_station/device/radar/del"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    // 构建 JSON 数据
    QJsonObject json;
    json["ver"] = "1.0";
    json["user"] = "2a9a43b58c1ad02ebe1920c63df693c4";
    json["id"] = "1728546698527";
    json["data"] = radarData.toDelJson();  // 嵌套的 data 部分

    QJsonDocument jsonDoc(json);
    QByteArray jsonData = jsonDoc.toJson(QJsonDocument::Compact);

    // 发送 POST 请求
    QNetworkReply *reply = manager->post(request, jsonData);

    // 处理服务器的响应
    QObject::connect(reply, &QNetworkReply::finished, [reply, this, radarData]() mutable {
        // 检查是否有错误
        if (reply->error() == QNetworkReply::NoError) {
            // 获取服务器返回的响应数据
            QByteArray responseData = reply->readAll();
            // 解析JSON响应
            QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);
            if (jsonResponse.isObject()) {
                QJsonObject jsonObject = jsonResponse.object();

                // 获取 JSON 数据中的各个字段
                int code = jsonObject.value("code").toInt();
                QString desc = jsonObject.value("desc").toString();
                // 打印解析后的数据
                qDebug() << "返回的Code:" << code;
                qDebug() << "返回的描述:" << desc;

                // 判断接口返回的 code 是否为错误码
                if (code != 0) {
                    failed++;
                    radarData.errDesc = desc;  // 将错误描述存入 RadarData 的 errDesc 字段
                    printDetail(QString("覆盖【%1%2】进口的雷达失败, 错误信息: %3")
                                    .arg(radarData.junctionName)
                                    .arg(radarData.radarPosition)
                                    .arg(desc));
                    failedList.append(radarData);
                } else {
                    addRadarData(radarData, true);
                }
            }
        } else {
            failed++;
            radarData.errDesc = reply->errorString();  // 将网络错误描述存入 RadarData 的 errDesc 字段
            printDetail(QString("覆盖【%1%2】进口的雷达失败, 错误信息: %3")
                            .arg(radarData.junctionName)
                            .arg(radarData.radarPosition)
                            .arg(reply->errorString()));
            failedList.append(radarData);
        }

        // 判断是否处理完成
        if (success + failed == total) {
            printDetail(QString("本次导入雷达信息结束, 共%1个, 成功%2个(其中覆盖%3个), 失败%4个.")
                            .arg(total)
                            .arg(success)
                            .arg(covered)
                            .arg(failed));
            if (failed > 0) {
                ui->pushButton->setEnabled(true);
            }
        }

        reply->deleteLater();  // 清理 reply 对象
    });

}

bool MainWindow::verify(RadarData& radarData)
{
    if(radarData.junctionName.isEmpty())
    {
        printDetail(QString("添加【%1%2】进口的雷达失败, 错误信息: %3").arg(radarData.junctionName).arg(radarData.radarPosition).arg("路口名称为空!"));
        radarData.errDesc = "路口名称为空!";
        failedDeal(radarData);
        return false;
    }
    bool ok;
    int id = radarData.junctionId.toInt(&ok);
    if(!ok || id < 100000 || id > 999999)
    {
        printDetail(QString("添加【%1%2】进口的雷达失败, 错误信息: %3").arg(radarData.junctionName).arg(radarData.radarPosition).arg("路口id有误!"));
        radarData.errDesc = "路口id有误!";
        failedDeal(radarData);
        return false;
    }
    QRegularExpression ipRegex(R"(^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$)");
    if(!ipRegex.match(radarData.stationIp).hasMatch())
    {
        printDetail(QString("添加【%1%2】进口的雷达失败, 错误信息: %3").arg(radarData.junctionName).arg(radarData.radarPosition).arg("小站ip有误!"));
        radarData.errDesc = "小站ip有误!";
        failedDeal(radarData);
        return false;
    }
    if(QString::number(RadarDirection::toInt(RadarDirection::fromString(radarData.radarPosition))) == "9")
    {
        printDetail(QString("添加【%1%2】进口的雷达失败, 错误信息: %3").arg(radarData.junctionName).arg(radarData.radarPosition).arg("未知的雷达安装方向!"));
        radarData.errDesc = "未知的雷达安装方向!";
        failedDeal(radarData);
        return false;
    }
    if(QString::number(RadarDirection::toInt(RadarDirection::fromString(radarData.radarDirection))) == "9")
    {
        printDetail(QString("添加【%1%2】进口的雷达失败, 错误信息: %3").arg(radarData.junctionName).arg(radarData.radarPosition).arg("未知的雷达照射方向!"));
        radarData.errDesc = "未知的雷达照射方向!";
        failedDeal(radarData);
        return false;
    }
    if(!ipRegex.match(radarData.radarIp).hasMatch())
    {
        printDetail(QString("添加【%1%2】进口的雷达失败, 错误信息: %3").arg(radarData.junctionName).arg(radarData.radarPosition).arg("雷达ip有误!"));
        radarData.errDesc = "雷达ip有误!";
        failedDeal(radarData);
        return false;
    }
    radarData.radarPort.toInt(&ok);
    if(!ok)
    {
        printDetail(QString("添加【%1%2】进口的雷达失败, 错误信息: %3").arg(radarData.junctionName).arg(radarData.radarPosition).arg("雷达端口有误!"));
        radarData.errDesc = "雷达端口有误!";
        failedDeal(radarData);
        return false;
    }
    if(radarData.ifCover.isEmpty() || radarData.ifCover != "否")
    {
        radarData.ifCover = "是";
    }
    return true;
}

void MainWindow::failedDeal(RadarData& radarData)
{
    failed++;
    failedList.append(radarData);
    ui->progressBar->setValue((success + failed) * 100/ total);
    if(success + failed == total)
    {
        printDetail(QString("本次导入雷达信息结束, 共%1个, 成功%2个(其中覆盖%3个), 失败%4个.").arg(total).arg(success).arg(covered).arg(failed));
        if(failed > 0)
        {
            ui->pushButton->setEnabled(true);
        }
    }
}

void MainWindow::on_btnDld_clicked()
{
    QString sourcePath = QDir::currentPath() + "/雷达导入模板.xlsx"; // 模板文件在当前目录下
    qDebug() << "路径1: " << QDir::currentPath();
    // 获取桌面路径
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString targetPath = QFileDialog::getSaveFileName(this, "保存模板文件", desktopPath + "/雷达导入模板.xlsx", "Excel 文件 (*.xlsx)");

    if (!targetPath.isEmpty()) {
        QFile sourceFile(sourcePath);
        QFile targetFile(targetPath);

        if (sourceFile.exists()) {
            // 如果目标文件已经存在，先删除它
            if (targetFile.exists()) {
                targetFile.remove();
            }

            // 进行复制操作
            if (sourceFile.copy(targetPath)) {
                QMessageBox::information(this, "成功", "模板文件下载成功！");
            } else {
                QMessageBox::warning(this, "错误", "文件复制失败！");
            }
        } else {
            QMessageBox::warning(this, "错误", "模板文件不存在！");
        }
    }
}



void MainWindow::on_btnUpld_clicked()
{
    // 获取桌面路径
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString filePath = QFileDialog::getOpenFileName(this, "选择文件", desktopPath, "Excel 文件 (*.xlsx)");
    if (!filePath.isEmpty()) {
        // 使用 QXlsx 库读取 Excel 文件中的数据
        QXlsx::Document xlsx(filePath);

        QString tag = "以下是一个路口雷达配置的示例(灰色部分),可参照该示例进行配置";
        if(tag != xlsx.read(1, 1).toString())
        {
            QMessageBox::warning(this, "错误", "请导入下载下来的模板！");
            return;
        }
        //清空错误列表
        failedList.clear();
        ui->pushButton->setEnabled(false);
        ui->plainTextEdit->clear();
        totalRadarList.clear();
        // 假设数据从第3行开始，如之前讨论的那样
        // 数据从第7行开始
        int startRow = 7;
        QString junctionName = "";   // 用于存储合并的路口名称
        QString junctionId = "";     // 用于存储合并的路口ID
        QString stationIp = "";      // 用于存储合并的小站IP
        printDetail("开始添加雷达...");
        total = 0;
        success = 0;
        failed = 0;
        while (true) {
            // 读取路口名称，若为空则保持前一个路口名称
            QString currentJunctionName = xlsx.read(startRow, 1).toString();
            if (!currentJunctionName.isEmpty()) {
                junctionName = currentJunctionName;
            }

            // 读取路口ID，若为空则保持前一个路口ID
            QString currentJunctionId = xlsx.read(startRow, 2).toString();
            if (!currentJunctionId.isEmpty()) {
                junctionId = currentJunctionId;
            }

            // 读取小站IP，若为空则保持前一个小站IP
            QString currentStationIp = xlsx.read(startRow, 3).toString();
            if (!currentStationIp.isEmpty()) {
                stationIp = currentStationIp;
            }

            // 读取其他数据
            QString radarPosition = xlsx.read(startRow, 4).toString().trimmed(); // 雷达安装位置
            QString radarDirection = xlsx.read(startRow, 5).toString().trimmed();// 雷达照射方向
            QString radarIp = xlsx.read(startRow, 6).toString().trimmed();       // 雷达ip
            QString radarPort = xlsx.read(startRow, 7).toString().trimmed();     // 雷达端口
            QString radarType = xlsx.read(startRow, 8).toString().trimmed();     // 雷达类型
            QString ifCover = xlsx.read(startRow, 9).toString().trimmed();      //是否覆盖

            // 如果没有更多数据（雷达安装位置为空），则退出循环
            if (radarPosition.isEmpty()) {
                break;
            }

            // 创建 RadarData::Data 对象（代表嵌套的 data 部分）
            RadarData radarDataDetails;
            radarDataDetails.junctionName = junctionName;
            radarDataDetails.radarIp = radarIp;
            radarDataDetails.radarPort = radarPort;
            radarDataDetails.junctionId = junctionId;
            radarDataDetails.radarType = radarType;
            radarDataDetails.radarDirection = radarDirection;
            radarDataDetails.radarPosition = radarPosition;
            radarDataDetails.stationIp = stationIp;
            radarDataDetails.ifCover = ifCover;
            totalRadarList.append(radarDataDetails);
            // 打印或保存提取的数据
            qDebug() << "路口名称:" << junctionName;
            qDebug() << "路口ID:" << junctionId;
            qDebug() << "小站IP:" << stationIp;
            qDebug() << "雷达安装位置:" << radarPosition;
            qDebug() << "雷达照射方向:" << radarDirection;
            qDebug() << "雷达IP:" << radarIp;
            qDebug() << "雷达端口:" << radarPort;
            qDebug() << "雷达类型:" << radarDataDetails.radarType;
            qDebug() << "是否覆盖:" << radarDataDetails.ifCover;
            // 处理完当前行后移动到下一行
            startRow++;
            total++;
        }
        if(total > 0)
        {
            for(const RadarData& item : totalRadarList)
            {
                addRadarData(item);
            }
        }
    } else {
        QMessageBox::warning(this, "错误", "未选择模板文件！");
    }
}


void MainWindow::on_pushButton_clicked()
{
    // 1. 获取当前目录下的模板文件路径
    QString sourcePath = QDir::currentPath() + "/添加失败的雷达信息.xlsx";
    // 2. 选择目标路径
    QString targetPath = QFileDialog::getSaveFileName(this, "选择保存路径", QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/添加失败的雷达信息.xlsx", "Excel 文件 (*.xlsx)");
    if (targetPath.isEmpty()) {
        QMessageBox::warning(this, "错误", "未选择保存路径！");
        return;
    }

    // 3. 拷贝模板文件到目标路径
    QFile sourceFile(sourcePath);
    if (!sourceFile.exists()) {
        QMessageBox::warning(this, "错误", "添加失败的雷达信息模板文件不存在！");
        return;
    }

    QFile targetFile(targetPath);
    if (targetFile.exists()) {
        targetFile.remove();  // 如果目标文件存在，删除它
    }

    if (!sourceFile.copy(targetPath)) {
        QMessageBox::warning(this, "错误", "添加失败的雷达信息模板文件复制失败！");
        return;
    }

    // 4. 打开拷贝后的 Excel 文件，并修改数据
    QXlsx::Document xlsx(targetPath);  // 使用目标路径打开文件
    int startRow = 7;
    if (!failedList.isEmpty()) {
        std::sort(failedList.begin(), failedList.end(), RadarData::compareByNameAndPosition);
        QString previousJunctionName;
        QString previousJunctionId;
        QString previousStationIp;
        int mergeStartRow = startRow;

        // 创建一个格式，用于居中显示并添加边框
        QXlsx::Format format;
        format.setHorizontalAlignment(QXlsx::Format::AlignHCenter);  // 水平居中
        format.setVerticalAlignment(QXlsx::Format::AlignVCenter);    // 垂直居中
        format.setBorderStyle(QXlsx::Format::BorderThin);            // 设置单元格边框为细线

        int maxErrDescWidth = 10;  // 用于记录错误描述列的最大宽度

        for (const RadarData &item : failedList) {
            xlsx.write(startRow, 1, item.junctionName, format);
            xlsx.write(startRow, 2, item.junctionId, format);
            xlsx.write(startRow, 3, item.stationIp, format);
            xlsx.write(startRow, 4, item.radarPosition, format);
            xlsx.write(startRow, 5, item.radarDirection, format);
            xlsx.write(startRow, 6, item.radarIp, format);
            xlsx.write(startRow, 7, item.radarPort, format);
            xlsx.write(startRow, 8, item.radarType, format);
            xlsx.write(startRow, 9, item.ifCover, format);
            xlsx.write(startRow, 10, item.errDesc);

            // 获取错误描述的长度，并更新列宽
            int errDescLength = item.errDesc.size();
            if (errDescLength > maxErrDescWidth) {
                maxErrDescWidth = errDescLength;
            }

            // 判断是否需要合并单元格
            if (item.junctionName != previousJunctionName || item.junctionId != previousJunctionId || item.stationIp != previousStationIp) {
                // 如果是新的一组数据，先合并之前的相同数据
                if (startRow > mergeStartRow) {
                    if (!previousJunctionName.isEmpty()) {
                        xlsx.mergeCells(QXlsx::CellRange(mergeStartRow, 1, startRow - 1, 1), format); // 合并路口名称并居中
                        xlsx.mergeCells(QXlsx::CellRange(mergeStartRow, 2, startRow - 1, 2), format); // 合并路口ID并居中
                        xlsx.mergeCells(QXlsx::CellRange(mergeStartRow, 3, startRow - 1, 3), format); // 合并小站IP并居中
                    }
                }
                mergeStartRow = startRow; // 记录新的合并起始行
            }

            previousJunctionName = item.junctionName;
            previousJunctionId = item.junctionId;
            previousStationIp = item.stationIp;
            startRow++;
        }

        // 最后一组数据需要单独处理合并
        if (startRow > mergeStartRow) {
            xlsx.mergeCells(QXlsx::CellRange(mergeStartRow, 1, startRow - 1, 1), format); // 合并路口名称并居中
            xlsx.mergeCells(QXlsx::CellRange(mergeStartRow, 2, startRow - 1, 2), format); // 合并路口ID并居中
            xlsx.mergeCells(QXlsx::CellRange(mergeStartRow, 3, startRow - 1, 3), format); // 合并小站IP并居中
        }

        // 根据错误描述的最大长度调整列宽
        xlsx.setColumnWidth(10, maxErrDescWidth + 30);  // 适当增加宽度
    }

    // 5. 保存修改
    if (xlsx.save()) {
        QMessageBox::information(this, "成功", "文件保存成功！");
    } else {
        QMessageBox::warning(this, "错误", "文件保存失败！");
    }
}

