#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <cstdlib>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void makePlotMeasurement();
    void makePlotSystem();
    void readSerial();
    void doCapture();
    void on_action_file_save_as_triggered();
    void on_action_edit_copy_triggered();

private:
    struct Private;
    Private *pv;
    Ui::MainWindow *ui;
    QSerialPort *arduino;
    static const quint16 arduino_uno_vendor_id = 9025;
    static const quint16 arduino_uno_product_id = 67;
    QByteArray serialData;
    QString serialBuffer;
    QString parsed_data1;
    QString parsed_data2;
    QVector<double> data;

};

#endif // MAINWINDOW_H
