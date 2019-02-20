#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qcustomplot.h"
#include <string>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QtWidgets>
#include <QFile>
#include <QTextStream>
#include <QDateTime>



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->teamId->setText("5608");
    ui->missionTime->setText("68");
    ui->packetCount->setText("67");
    ui->gpsTime->setText("15:8:20");
    ui->gpsSatellites->setText("10");

    xbee_is_available = false;
    xbee_port_name = "";
    xbee = new QSerialPort();
    serialBuffer = "";

    //This part of the code is to Identify the Vendor ID & the Product ID for Serial Communication
    //Testing Code
    /*
        qDebug() << "Number of available ports: " << QSerialPortInfo::availablePorts().length();
        foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()){
            qDebug() << "Has vendor ID: " << serialPortInfo.hasVendorIdentifier();
            if(serialPortInfo.hasVendorIdentifier()){
                qDebug() << "Vendor ID: " << serialPortInfo.vendorIdentifier();
            }
            qDebug() << "Has Product ID: " << serialPortInfo.hasProductIdentifier();
            if(serialPortInfo.hasProductIdentifier()){
                qDebug() << "Product ID: " << serialPortInfo.productIdentifier();
            }
        }
    */


//This part is to check if the xbee is connected on to the specified port
    foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()){
            if(serialPortInfo.hasVendorIdentifier() && serialPortInfo.hasProductIdentifier()){
                if(serialPortInfo.vendorIdentifier() == xbee_vendor_id){
                    if(serialPortInfo.productIdentifier() == xbee_product_id){
                        xbee_port_name = serialPortInfo.portName();
                        xbee_is_available = true;
                    }
                }
            }
        }

    if(xbee_is_available){
        //To open and configure the serialport
        qDebug() << "Found the XBee Port!!";
        xbee->setPortName(xbee_port_name);
        xbee->open(QSerialPort::ReadOnly);
        xbee->setBaudRate(QSerialPort::Baud9600);
        xbee->setDataBits(QSerialPort::Data8);
        xbee->setParity(QSerialPort::NoParity);
        xbee->setStopBits(QSerialPort::OneStop); //One and a Half Stop is only for the windows platform
        xbee->setFlowControl(QSerialPort::NoFlowControl);
        QObject::connect(xbee,SIGNAL(readyRead()),this, SLOT(readSerial()));
    }else{
        //To give error message if not available
        qDebug() << "The XBee is not connected to the Computer";
        QMessageBox::critical(this, "Serial Port Error", "Couldn't find the XBee!");
    }


}

MainWindow::~MainWindow()
{
    if(xbee->isOpen()){
        xbee->close();
    }
    delete ui;
}

void MainWindow::readSerial()
{
    qDebug() << "Serial Port Works";
    serialData = xbee->readAll();
    serialBuffer += QString::fromStdString(serialData.toStdString());
    qDebug() << serialBuffer;

    //parseData();

    //qDebug() << bufferSplit;

    //serialData.clear();
    //qDeleteAll(serialBuffer.begin(), serialBuffer.end());
    //serialBuffer.clear();
}


void MainWindow::parseData()
{
    bufferSplit = serialBuffer.split(",");
    teamId = bufferSplit[1];
    missionTime = bufferSplit[2];
    packetCount = bufferSplit[3];
    altitude = bufferSplit[4];
    pressure = bufferSplit[5];
    temp = bufferSplit[6];
    voltage = bufferSplit[7];
    gpsTime = bufferSplit[8];
    gpsLatitude = bufferSplit[9];
    gpsLongitude = bufferSplit[10];
    gpsAltitude = bufferSplit[11];
    gpsSats = bufferSplit[12];
    pitch = bufferSplit[13];
    roll = bufferSplit[14];
    rpm = bufferSplit[15];
    softwareState = bufferSplit[16];
    bonusDirection = bufferSplit[17];

    updateData();
    writeData();
    //qDeleteAll(bufferSplit.begin(), bufferSplit.end());
    bufferSplit.clear();
    serialBuffer.clear();
}


void MainWindow::writeData()
{
    QFile file("data.csv");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        QMessageBox::warning(this,"Writing in file Error","File is not open");
    }

    QTextStream out(&file);
    out << serialBuffer << "\n";
    file.flush();
    file.close();
}


void MainWindow::updateData()
{
    ui->teamId->setText(teamId);
    ui->missionTime->setText(missionTime);
    ui->packetCount->setText(packetCount);
    ui->gpsTime->setText(gpsTime);
    ui->gpsSatellites->setText(gpsSats);
    plot();
}


void MainWindow::plot()
{
    ui->altitudeGraph->addGraph(); // blue line
    ui->altitudeGraph->graph(0)->setPen(QPen(QColor(40, 110, 255)));

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    ui->altitudeGraph->xAxis->setTicker(timeTicker);
    ui->altitudeGraph->axisRect()->setupFullAxesBox();
    ui->altitudeGraph->yAxis->setRange(0, 1000); //Maximum altitude that the CanSat can reach

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(ui->altitudeGraph->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->altitudeGraph->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->altitudeGraph->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->altitudeGraph->yAxis2, SLOT(setRange(QCPRange)));

    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
    connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
    dataTimer.start(0); // Interval 0 means to refresh as fast as possible
}


void MainWindow::realtimeDataSlot(){
    static QTime time(QTime::currentTime());
    // calculate two new data points:
    double key = time.elapsed()/1000.0; // time elapsed since start of demo, in seconds
    static double lastPointKey = 0;
    if (key-lastPointKey > 0.002) // at most add point every 2 ms
    {
      // add data to lines:
      ui->altitudeGraph->graph(0)->addData(key, altitude.toDouble());
      //ui->altitudeGraph->graph(1)->addData(key, qCos(key)+qrand()/RAND_MAX*0.5*qSin(key/0.4364));
      //rescale value (vertical) axis to fit the current data:
      ui->altitudeGraph->graph(0)->rescaleValueAxis();
      //ui->altitudeGraph->graph(1)->rescaleValueAxis(true);
      lastPointKey = key;
    }
    // make key axis range scroll with the data (at a constant range size of 8):
    ui->altitudeGraph->xAxis->setRange(key, 8, Qt::AlignRight);
    ui->altitudeGraph->replot();

    // calculate frames per second:
    static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key-lastFpsKey > 2) // average fps over 2 seconds
    {
      ui->statusBar->showMessage(
            QString("%1 FPS, Total Data points: %2").arg(frameCount/(key-lastFpsKey), 0, 'f', 0).arg(ui->altitudeGraph->graph(0)->data()->size()));
      lastFpsKey = key;
      frameCount = 0;
    }

}







