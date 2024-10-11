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
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/icons/images/icon.webp"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addRadarData(RadarData radarData)
{
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
    QObject::connect(reply, &QNetworkReply::finished, [reply, this, radarData]() {

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
                if(code == 0)
                {
                    success++;
                    printDetail(QString("添加【%1%2】进口的雷达成功!").arg(radarData.junctionName).arg(radarData.radarPosition));
                }else
                {
                    failed++;
                    printDetail(QString("添加【%1%2】进口的雷达失败, 错误信息: %3").arg(radarData.junctionName).arg(radarData.radarPosition).arg(desc));
                }
            }
        } else {
            failed++;
            printDetail(QString("添加【%1%2】进口的雷达失败, 错误信息: %3").arg(radarData.junctionName).arg(radarData.radarPosition).arg(reply->errorString()));
        }
        if(success + failed == total)
        {
            printDetail(QString("本次导入雷达信息共%1个, 成功%2个, 失败%3个.").arg(total).arg(success).arg(failed));
        }
        reply->deleteLater();
    });
}

void MainWindow::printDetail(QString detail)
{
    ui->plainTextEdit->appendPlainText(detail);
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

        // 假设数据从第3行开始，如之前讨论的那样
        // 数据从第3行开始
        int startRow = 3;
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
            QString radarPosition = xlsx.read(startRow, 4).toString(); // 雷达安装位置
            QString radarDirection = xlsx.read(startRow, 5).toString();// 雷达照射方向
            QString radarIp = xlsx.read(startRow, 6).toString();       // 雷达ip
            quint16 radarPort = xlsx.read(startRow, 7).toInt();     // 雷达端口
            QString radarType = xlsx.read(startRow, 8).toString();     // 雷达类型

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
            addRadarData(radarDataDetails);

            // 打印或保存提取的数据
            qDebug() << "路口名称:" << junctionName;
            qDebug() << "路口ID:" << junctionId;
            qDebug() << "小站IP:" << stationIp;
            qDebug() << "雷达安装位置:" << radarPosition;
            qDebug() << "雷达照射方向:" << radarDirection;
            qDebug() << "雷达IP:" << radarIp;
            qDebug() << "雷达端口:" << radarPort;
            qDebug() << "雷达类型:" << radarType;

            // 处理完当前行后移动到下一行
            startRow++;
            total++;
        }
    } else {
        QMessageBox::warning(this, "错误", "未选择模板文件！");
    }
}


void MainWindow::on_pushButton_clicked()
{
    ui->plainTextEdit->clear();
}

