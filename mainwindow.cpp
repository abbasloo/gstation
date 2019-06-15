#include "mainwindow.h"
#include "ui_mainwindow.h"



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    MainWindow::makePlotMeasurement();
    MainWindow::makePlotSystem();

    arduino = new QSerialPort(this);
    serialBuffer = "";
    bool arduino_is_available = false;
    QString arduino_uno_port_name;
    foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()){
        if(serialPortInfo.hasProductIdentifier() && serialPortInfo.hasVendorIdentifier()){
            if((serialPortInfo.productIdentifier() == arduino_uno_product_id) && (serialPortInfo.vendorIdentifier() == arduino_uno_vendor_id)){
                arduino_is_available = true;
                arduino_uno_port_name = serialPortInfo.portName();
            }
        }
    }
    if(arduino_is_available){
        qDebug() << "Found the arduino port...\n";
        arduino->setPortName(arduino_uno_port_name);
        arduino->open(QSerialPort::ReadOnly);
        arduino->setBaudRate(QSerialPort::Baud9600);
        arduino->setDataBits(QSerialPort::Data8);
        arduino->setFlowControl(QSerialPort::NoFlowControl);
        arduino->setParity(QSerialPort::NoParity);
        arduino->setStopBits(QSerialPort::OneStop);
        QObject::connect(arduino, SIGNAL(readyRead()), this, SLOT(readSerial()));
    }else{
        qDebug() << "Couldn't find the correct port for the arduino.\n";
        QMessageBox::information(this, "Serial Port Error", "Couldn't open serial port to arduino.");
    }
    readSerial();
    makePlotMeasurement();
    makePlotSystem();
}

MainWindow::~MainWindow()
{
    delete ui;
    if(arduino->isOpen()){
        arduino->close();
    }
}

void MainWindow::makePlotMeasurement(){

    // generate some data:
    QVector<double> x(data.size()), y(data.size()); // initialize with entries 0..100
    for (int i=0; i<data.size(); ++i)
    {
      x[i] = i; // x goes from -1 to 1
      y[i] = data[i]; // let's plot a quadratic function
    }
    double min = *std::min_element(data.constBegin(), data.constEnd());
    double max = *std::max_element(data.constBegin(), data.constEnd());
    // create graph and assign data to it:
    ui->customplot_1->clearGraphs();
    ui->customplot_1->addGraph();
    ui->customplot_1->graph(0)->setData(x, y);
    // give the axes some labels:
    ui->customplot_1->xAxis->setLabel("t");
    ui->customplot_1->yAxis->setLabel("M");
    // set axes ranges, so we see all data:
    ui->customplot_1->xAxis->setRange(0, data.size());
    ui->customplot_1->yAxis->setRange(min, max);
    QCPTextElement *title = new QCPTextElement(ui->customplot_1, "Measurment", QFont("sans", 10, QFont::Bold));
    ui->customplot_1->plotLayout()->addElement(1, 0, title);
    ui->customplot_1->replot();
}

void MainWindow::makePlotSystem(){

    // generate some data:
    QVector<double> x(101), y(101); // initialize with entries 0..100
    for (int i=0; i<101; ++i)
    {
      x[i] = i/50.0 - 1; // x goes from -1 to 1
      y[i] = x[i]*x[i]*x[i]; // let's plot a quadratic function
    }
    // create graph and assign data to it:
    ui->customplot_2->addGraph();
    ui->customplot_2->graph(0)->setData(x, y);
    // give the axes some labels:
    ui->customplot_2->xAxis->setLabel("t");
    ui->customplot_2->yAxis->setLabel("P");
    // set axes ranges, so we see all data:
    ui->customplot_2->xAxis->setRange(-1, 1);
    ui->customplot_2->yAxis->setRange(-1, 1);
    QCPTextElement *title = new QCPTextElement(ui->customplot_2, "System Parameters", QFont("sans", 10, QFont::Bold));
    ui->customplot_2->plotLayout()->addElement(1, 0, title);
    ui->customplot_2->replot();
}

void MainWindow::readSerial()
{
    QStringList buffer_split = serialBuffer.split(",");
    if(buffer_split.length() < 3){
        serialData = arduino->readAll();
        serialBuffer = serialBuffer + QString::fromStdString(serialData.toStdString());
        serialData.clear();
    }else{
        serialBuffer = "";
        parsed_data1 = buffer_split[0];
        parsed_data2 = buffer_split[1];
        data.push_back(parsed_data1.toDouble());
        data.push_back(parsed_data2.toDouble());
        //qDebug() << parsed_data1.toDouble() << parsed_data2.toDouble() << "\n";
    }
    makePlotMeasurement();
}
