#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <stdint.h>
#include <QDebug>
#include <QTimer>
#include <QFileDialog>
#include <QClipboard>
#include "opencv2/opencv.hpp"
#include <algorithm>

#ifdef _WIN32
#ifdef QT_NO_DEBUG
#pragma comment(lib, "opencv_core2413.lib")
#pragma comment(lib, "opencv_highgui2413.lib")
#else
#pragma comment(lib, "opencv_core2413d.lib")
#pragma comment(lib, "opencv_highgui2413d.lib")
#endif
#endif

# define M_PI           3.14159265358979323846  /* pi */

// solution 1: use cv::VideoCapture
class Capture1 {
private:
    cv::VideoCapture cap;
public:
    void open()
    {
        close();
        cap.open(0);
    }
    void close()
    {
        cap.release();
    }
    bool isOpened() const
    {
        return cap.isOpened();
    }
    QImage capture()
    {
        if (isOpened()) {
            cv::Mat frame;
            cap.read(frame);
            if (frame.dims == 2 && frame.channels() == 3) {
                int w = frame.cols;
                int h = frame.rows;
                QImage image(w, h, QImage::Format_RGB32);
                for (int i = 0; i < h; i++) {
                    uint8_t const *src = frame.ptr(i);
                    QRgb *dst = (QRgb *)image.scanLine(i);
                    for (int j = 0; j < w; j++) {
                        dst[j] = qRgb(src[2], src[1], src[0]);
                        src += 3;
                    }
                }
                return image;
            }
        }
        return QImage();
    }
};

// solution 2: use CvCapture
class Capture2 {
private:
    CvCapture *cap = nullptr;
public:
    void open()
    {
        close();
        cap = cvCreateCameraCapture(0);
    }
    void close()
    {
        if (isOpened()) {
            cvReleaseCapture(&cap);
            cap = nullptr;
        }
    }
    bool isOpened() const
    {
        return cap != nullptr;
    }
    QImage capture()
    {
        if (isOpened()) {
            cvQueryFrame(cap);
            IplImage *iplimage = cvQueryFrame(cap);
            int w = iplimage->width;
            int h = iplimage->height;
            QImage image(w, h, QImage::Format_RGB32);
            for (int i = 0; i < h; i++) {
                uint8_t const *src = (uint8_t const *)iplimage->imageData + i * w * 3;
                QRgb *dst = (QRgb *)image.scanLine(i);
                for (int j = 0; j < w; j++) {
                    dst[j] = qRgb(src[2], src[1], src[0]);
                    src += 3;
                }
            }
            return image;
        }
        return QImage();
    }
};

struct MainWindow::Private {
    //Capture1 cap;
    Capture2 cap;
    QTimer timer;
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    pv = new Private();
    ui->setupUi(this);

    pv->cap.open();

    connect(&pv->timer, SIGNAL(timeout()), this, SLOT(doCapture()));
    pv->timer.setInterval(0);
    pv->timer.start();

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
    pv->cap.close();
    delete pv;
    delete ui;
    if(arduino->isOpen()){
        arduino->close();
    }
}

void MainWindow::makePlotMeasurement(){

    // generate some data:
    QVector<double> x, y;
    if (data.size() == 0){
        x.resize(1000); y.resize(1000);
        for (int i=0; i<1000; ++i)
        {
            double noise1 = rand() % 3 + 1;
            double noise2 = rand() % 3 + 1;
            double noise3 = rand() % 3 + 1;
            double noise4 = rand() % 3 + 1;
            noise1 -=2;
            noise2 -=2;
            noise3 -=2;
            noise4 -=2;
            x[i] = 2.0*(1-noise1/30.0)*std::sin(i*2*M_PI/1000) + noise2/10.0;
            y[i] = 2.0*(1-noise3/30.0)*std::cos(i*2*M_PI/1000) + noise4/10.0;
        }

    }else{
        x.resize(data.size()); y.resize(data.size()); // initialize with entries 0..100
        for (int i=0; i<data.size(); ++i)
        {
           x[i] = i; // x goes from -1 to 1
           y[i] = data[i]; // let's plot a quadratic function
        }
    }
    double minx = *std::min_element(x.constBegin(), x.constEnd());
    double maxx = *std::max_element(x.constBegin(), x.constEnd());
    double miny = *std::min_element(y.constBegin(), y.constEnd());
    double maxy = *std::max_element(y.constBegin(), y.constEnd());
    // create graph and assign data to it:
    ui->customplot_1->clearGraphs();
    ui->customplot_1->addGraph(ui->customplot_1->yAxis, ui->customplot_1->xAxis);
    ui->customplot_1->graph(0)->setPen(QColor(255, 0, 0, 255));
    ui->customplot_1->graph(0)->setLineStyle(QCPGraph::lsNone);
    ui->customplot_1->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 4));
    ui->customplot_1->graph(0)->setData(x, y);
    // give the axes some labels:
    ui->customplot_1->xAxis->setLabel("Mx");
    ui->customplot_1->yAxis->setLabel("My");
    // set axes ranges, so we see all data:
    ui->customplot_1->xAxis->setRange(minx, maxx);
    ui->customplot_1->yAxis->setRange(miny, maxy);
    QCPTextElement *title = new QCPTextElement(ui->customplot_1, "Measurment", QFont("sans", 10, QFont::Bold));
    ui->customplot_1->plotLayout()->addElement(1, 0, title);
    ui->customplot_1->replot();
}

void MainWindow::makePlotSystem(){

    ui->customplot_2->setLocale(QLocale(QLocale::English, QLocale::UnitedKingdom)); // period as decimal separator and comma as thousand separator
    ui->customplot_2->legend->setVisible(true);
    QFont legendFont = font();  // start out with MainWindow's font..
    legendFont.setPointSize(9); // and make a bit smaller for legend
    ui->customplot_2->legend->setFont(legendFont);
    ui->customplot_2->legend->setBrush(QBrush(QColor(255,255,255,230)));
    // by default, the legend is in the inset layout of the main axis rect. So this is how we access it to change legend placement:
    ui->customplot_2->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignBottom|Qt::AlignRight);

    // generate some data:
    QVector<double> x, y;
    if (data.size() == 0){
        x.resize(1000); y.resize(1000);
        for (int i=0; i<1000; ++i)
        {
            double noise1 = rand() % 3 + 1;
            double noise2 = rand() % 3 + 1;
            noise1 -=2;
            noise2 -=2;
            x[i] = i;
            y[i] = 2.0*(1-noise1/30.0)*std::cos(i*2*M_PI/1000) + noise2/10.0;
        }

    }else{
        x.resize(data.size()); y.resize(data.size()); // initialize with entries 0..100
        for (int i=0; i<data.size(); ++i)
        {
           x[i] = i; // x goes from -1 to 1
           y[i] = data[i]; // let's plot a quadratic function
        }
    }
    double min1 = *std::min_element(y.constBegin(), y.constEnd());
    double max1 = *std::max_element(y.constBegin(), y.constEnd());
    // create graph and assign data to it:
    ui->customplot_2->clearGraphs();
    ui->customplot_2->addGraph();
    ui->customplot_2->graph(0)->setData(x, y);
    ui->customplot_2->graph(0)->setPen(QPen(QColor(255, 0, 0)));
    ui->customplot_2->graph(0)->setName("X");
    // give the axes some labels:
    ui->customplot_2->xAxis->setLabel("t");
    ui->customplot_2->yAxis->setLabel("P");
    // set axes ranges, so we see all data:
    ui->customplot_2->xAxis->setRange(0, y.size());
    //ui->customplot_2->yAxis->setRange(min1, max1);
    QCPTextElement *title = new QCPTextElement(ui->customplot_2, "System Parameters", QFont("sans", 10, QFont::Bold));
    ui->customplot_2->plotLayout()->addElement(1, 0, title);
    ui->customplot_2->replot();

    // generate some data:
    if (data.size() == 0){
        for (int i=0; i<1000; ++i)
        {
            double noise1 = rand() % 3 + 1;
            double noise2 = rand() % 3 + 1;
            noise1 -=2;
            noise2 -=2;
            x[i] = i;
            y[i] = 2.0*(1-noise1/30.0)*std::sin(i*2*M_PI/1000) + noise2/10.0;
        }

    }else{
        for (int i=0; i<data.size(); ++i)
        {
           x[i] = i; // x goes from -1 to 1
           y[i] = data[i]; // let's plot a quadratic function
        }
    }
    double min2 = *std::min_element(y.constBegin(), y.constEnd());
    double max2 = *std::max_element(y.constBegin(), y.constEnd());
    // create graph and assign data to it:
    ui->customplot_2->addGraph();
    ui->customplot_2->graph(1)->setData(x, y);
    ui->customplot_2->graph(1)->setPen(QPen(QColor(0, 255, 0)));
    ui->customplot_2->graph(1)->setName("Y");
    // give the axes some labels:
    ui->customplot_2->xAxis->setLabel("t");
    ui->customplot_2->yAxis->setLabel("P");
    // set axes ranges, so we see all data:
    ui->customplot_2->xAxis->setRange(0, y.size());
    //ui->customplot_2->yAxis->setRange(min2, max2);
    title = new QCPTextElement(ui->customplot_2, "System Parameters", QFont("sans", 10, QFont::Bold));
    ui->customplot_2->plotLayout()->addElement(1, 0, title);
    ui->customplot_2->replot();

    // generate some data:
    if (data.size() == 0){
        for (int i=0; i<1000; ++i)
        {
            double noise1 = rand() % 3 + 1;
            noise1 -= 2;
            double noise2 = rand() % 3 + 1;
            noise2 -= 2;
            x[i] = i;
            y[i] = 1.5*(1-noise1/20.0) + noise2/30.0;
        }

    }else{
        for (int i=0; i<data.size(); ++i)
        {
           x[i] = i; // x goes from -1 to 1
           y[i] = data[i]; // let's plot a quadratic function
        }
    }
    double min3 = *std::min_element(y.constBegin(), y.constEnd());
    double max3 = *std::max_element(y.constBegin(), y.constEnd());
    // create graph and assign data to it:
    ui->customplot_2->addGraph();
    ui->customplot_2->graph(2)->setData(x, y);
    ui->customplot_2->graph(2)->setPen(QPen(QColor(0, 0, 255)));
    ui->customplot_2->graph(2)->setName("Z");
    // give the axes some labels:
    ui->customplot_2->xAxis->setLabel("t");
    ui->customplot_2->yAxis->setLabel("P");
    // set axes ranges, so we see all data:
    ui->customplot_2->xAxis->setRange(0, y.size());

    ui->customplot_2->yAxis->setRange(std::min(std::min(min1,min2),min3), std::max(std::max(max1,max2),max3));
    title = new QCPTextElement(ui->customplot_2, "System Parameters", QFont("sans", 10, QFont::Bold));
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
    makePlotSystem();
}

void MainWindow::doCapture()
{
    QImage image = pv->cap.capture();
    int w = image.width();
    int h = image.height();
    if (w > 0 && h > 0) {
        //setFixedSize(w, h);
        ui->imagewidget->setFixedSize(w, h);
        ui->imagewidget->setImage(image);
    }
}

void MainWindow::on_action_file_save_as_triggered()
{
    QString path = QFileDialog::getSaveFileName(this, tr("Save as"), QString(), "JPEG files (*.jpg);;PNG files (*.png)");
    if (path.isEmpty()) return;

    QImage image = ui->imagewidget->getImage();
    image.save(path);
}

void MainWindow::on_action_edit_copy_triggered()
{
    QImage image = ui->imagewidget->getImage();
    qApp->clipboard()->setImage(image);
}
