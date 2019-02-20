#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QByteArray>
#include <QTimer>

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
    const static quint16 xbee_vendor_id = 1027;
    const static quint16 xbee_product_id = 24577;
    QString xbee_port_name;
    bool xbee_is_available;
    QByteArray serialData;
    QString serialBuffer;
    QStringList bufferSplit;
    QString teamId,packetCount,pressure,altitude,temperature,missionTime,temp,voltage,gpsTime,gpsLatitude,gpsLongitude,gpsAltitude,gpsSats,pitch,roll,rpm,softwareState,bonusDirection;
    QTimer dataTimer;

private slots:
    void readSerial();
    void updateData();
    void parseData();
    void writeData();
    void realtimeDataSlot();
    void plot();
};

#endif // MAINWINDOW_H
