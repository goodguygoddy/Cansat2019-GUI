#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QByteArray>
#include <QTimer>
#include <QVector>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QSerialPort *xbee;
    const static quint16 xbee_vendor_id = 4292;
    const static quint16 xbee_product_id = 60000;
    QString xbee_port_name;
    bool xbee_is_available;
    QByteArray serialData;
    QString serialBuffer;
    QStringList semiColon;
    QStringList bufferSplit;
    QString teamId,packetCount,pressure,altitude,temperature,missionTime,temp,voltage,gpsTime,gpsLatitude,gpsLongitude,gpsAltitude,gpsSats,pitch,roll,rpm,softwareState,bonusDirection;
    QTimer dataTimer;
    bool x;

private slots:
    void readSerial();
    void updateData();
    void parseData();
    void writeData();
    void altitudePlot();
    void pressurePlot();
    void tempPlot();
    void rollPlot();
    void pitchPlot();
    void on_pushButton_clicked();
};

#endif // MAINWINDOW_H
