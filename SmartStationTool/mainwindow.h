#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
class RadarData;
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    quint16 total = 0;
    quint16 success = 0;
    quint16 failed = 0;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void addRadarData(RadarData radarData);
    void printDetail(QString detail);
private slots:
    void on_btnDld_clicked();

    void on_btnUpld_clicked();

    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
