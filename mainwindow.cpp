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
#include <QPixmap>
#include <QMap>



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->teamId->setText("----");
    ui->missionTime->setText("----");
    ui->packetCount->setText("----");
    ui->gpsTime->setText("----");
    ui->gpsSatellites->setText("----");
    ui->batteryValue->setText("----");
    ui->stateValue->setText("----");
    ui->sammardgif->setScaledContents(true);
    ui->sammardgif->setMask((new QPixmap("Sammard.gif"))->mask());
    ui->sammardgif->show();
    QMovie *movie = new QMovie("Sammard.gif");
    ui->sammardgif->setMovie(movie);
    movie->start();

    ui->altitudeGraph->addGraph();
    ui->pressureGraph->addGraph();
    ui->tempGraph->addGraph();
    ui->rollGraph->addGraph();
    ui->pitchGraph->addGraph();


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
        xbee->open(QSerialPort::ReadWrite);
        xbee->setBaudRate(QSerialPort::Baud9600);
        xbee->setDataBits(QSerialPort::Data8);
        xbee->setParity(QSerialPort::NoParity);
        xbee->setStopBits(QSerialPort::OneStop); //One and a Half Stop is only for the windows platform
        xbee->setFlowControl(QSerialPort::NoFlowControl);
        QObject::connect(xbee,SIGNAL(readyRead()),this, SLOT(readSerial()));
    }
    else{
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
    //qDebug() << "Serial Port Works";
    semiColon = serialBuffer.split(";");
/*
    serialData = xbee->readAll();
    serialBuffer += QString::fromStdString(serialData.toStdString());
    serialData.clear();
    qDebug() << serialBuffer;
*/
    if(semiColon.length() <= 1){
        serialData = xbee->readAll();
        serialBuffer += QString::fromStdString(serialData.toStdString());
        serialData.clear();
        semiColon = serialBuffer.split(";");
    }
    else{
        semiColon = serialBuffer.split(";");
        bufferSplit = semiColon[0].split(",");
        if(bufferSplit.length() == 17){
            MainWindow::parseData();
        }
        //qDebug() << bufferSplit;
        serialBuffer.clear();
        serialBuffer = semiColon[1];
        serialData = xbee->readAll();
        serialBuffer += QString::fromStdString(serialData.toStdString());
        serialData.clear();
        bufferSplit.clear();
        semiColon.clear();
    }

}


void MainWindow::parseData()
{
    if(bufferSplit[0] == "\r\n5601")
        bufferSplit[0] = "5601";

    if(bufferSplit[0] == "5601"){
        teamId = bufferSplit[0];
        missionTime = bufferSplit[1];
        packetCount = bufferSplit[2];
        altitude = bufferSplit[3];
        pressure = bufferSplit[4];
        temp = bufferSplit[5];
        voltage = bufferSplit[6];
        gpsTime = bufferSplit[7];
        gpsLatitude = bufferSplit[8];
        gpsLongitude = bufferSplit[9];
        gpsAltitude = bufferSplit[10];
        gpsSats = bufferSplit[11];
        pitch = bufferSplit[12];
        roll = bufferSplit[13];
        rpm = bufferSplit[14];
        softwareState = bufferSplit[15];
        bonusDirection = bufferSplit[16];
        MainWindow::updateData();
    }
    MainWindow::writeData();
    qDebug() << bufferSplit;
}


void MainWindow::writeData()
{
    QFile file("data.csv");
    if (!file.open(QIODevice::Append | QIODevice::Text)){
        QMessageBox::warning(this,"Writing in file Error","File is not open");
    }

    QTextStream out(&file);
    for(int i=0;i<17;i++){
        out << bufferSplit[i] <<",";
        file.flush();
    }
    out << "\n";
    file.close();
}


void MainWindow::updateData()
{
    ui->teamId->setText(teamId);
    ui->missionTime->setText(missionTime+" Seconds");
    ui->packetCount->setText(packetCount+" Packets");
    ui->gpsTime->setText(gpsTime);
    ui->gpsSatellites->setText(gpsSats);
    ui->batteryValue->setText(voltage+" Volts");

    if(softwareState == "G")
        ui->stateValue->setText("Launch Pad");
    else if(softwareState == "A")
        ui->stateValue->setText("Acending");
    else if(softwareState == "D")
        ui->stateValue->setText("Decending");
    else if(softwareState == "P")
        ui->stateValue->setText("Probe Deployed");
    else if(softwareState == "L")
        ui->stateValue->setText("Landed");
    else
        ui->stateValue->setText("UNKNOWN");

    if(packetCount == "1"){
        if(!ui->altitudeGraph->graph(0)->data().isNull())
            ui->altitudeGraph->graph(0)->data()->clear();
        if(!ui->pressureGraph->graph(0)->data().isNull())
            ui->pressureGraph->graph(0)->data()->clear();
        if(!ui->rollGraph->graph(0)->data().isNull())
            ui->rollGraph->graph(0)->data()->clear();
        if(!ui->pitchGraph->graph(0)->data().isNull())
            ui->pitchGraph->graph(0)->data()->clear();
        if(!ui->tempGraph->graph(0)->data().isNull())
            ui->tempGraph->graph(0)->data()->clear();
    }


    altitudePlot();
    pressurePlot();
    tempPlot();
    rollPlot();
    pitchPlot();
}


void MainWindow::altitudePlot()
{
    ui->altitudeGraph->graph(0)->addData(missionTime.toDouble(), altitude.toDouble());
    ui->altitudeGraph->graph(0)->setPen(QPen(Qt::blue)); //blue line
    ui->altitudeGraph->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross));
    ui->altitudeGraph->axisRect()->setupFullAxesBox();
    ui->altitudeGraph->xAxis->setRange(0,450);
    ui->altitudeGraph->yAxis->setRange(0,50);
    ui->altitudeGraph->xAxis->setLabel("Time in Seconds");
    ui->altitudeGraph->yAxis->setLabel("Altitude");
    ui->altitudeGraph->replot();
}

void MainWindow::pressurePlot()
{
    ui->pressureGraph->graph(0)->addData(missionTime.toDouble(), pressure.toDouble());
    ui->pressureGraph->graph(0)->setPen(QPen(Qt::black)); //black line
    ui->pressureGraph->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDot));
    ui->pressureGraph->axisRect()->setupFullAxesBox();
    ui->pressureGraph->xAxis->setRange(0,450);
    ui->pressureGraph->yAxis->setRange(97000,99000);
    ui->pressureGraph->xAxis->setLabel("Time in Seconds");
    ui->pressureGraph->yAxis->setLabel("Pressure");
    ui->pressureGraph->replot();
}

void MainWindow::tempPlot()
{
    ui->tempGraph->graph(0)->addData(missionTime.toDouble(), temp.toDouble());
    ui->tempGraph->graph(0)->setPen(QPen(Qt::blue)); //blue line
    ui->tempGraph->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle));
    ui->tempGraph->axisRect()->setupFullAxesBox();
    ui->tempGraph->xAxis->setRange(0,450);
    ui->tempGraph->yAxis->setRange(20,60); //Degree celsius
    ui->tempGraph->xAxis->setLabel("Time in Seconds");
    ui->tempGraph->yAxis->setLabel("Temperature");
    ui->tempGraph->replot();
}

void MainWindow::rollPlot()
{
    ui->rollGraph->graph(0)->addData(missionTime.toDouble(), roll.toDouble());
    ui->rollGraph->graph(0)->setPen(QPen(Qt::black)); //black line
    ui->rollGraph->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle));
    ui->rollGraph->axisRect()->setupFullAxesBox();
    ui->rollGraph->xAxis->setRange(0,450);
    ui->rollGraph->yAxis->setRange(-360,360);
    ui->rollGraph->xAxis->setLabel("Time in Seconds");
    ui->rollGraph->yAxis->setLabel("Roll Values");
    ui->rollGraph->replot();
}

void MainWindow::pitchPlot()
{
    ui->pitchGraph->graph(0)->addData(missionTime.toDouble(), pitch.toDouble());
    ui->pitchGraph->graph(0)->setPen(QPen(Qt::red)); //red line
    ui->pitchGraph->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle));
    ui->pitchGraph->axisRect()->setupFullAxesBox();
    ui->pitchGraph->xAxis->setRange(0,450);
    ui->pitchGraph->yAxis->setRange(-360,360);
    ui->pitchGraph->xAxis->setLabel("Time in Seconds");
    ui->pitchGraph->yAxis->setLabel("Pitch Values");
    ui->pitchGraph->replot();
}



void MainWindow::on_pushButton_clicked()
{
    QMessageBox::information(this,"Calibration Command","The Clibration Command has been sent");
    xbee->write("T");
}


