#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "./QSettingsUtf8/QSettingsUtf8.h"
#include "tsafequeue.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButtonQSettingsUtf8_clicked()
{
    QSettingsUtf8 settings("QSettingsUtf8.ini",QSettings::IniFormat);//settings.setIniCodec("UTF-8");
    settings.beginGroup("CollectForms");
    settings.setValue(QString("中文测试"),"中文测试");
    settings.endGroup();
    settings.setValue(QString("Modbus采集-未命名/容器项目"),"中文测试");

    {
        QSettingsUtf8 settings2("QSettingsUtf8.ini",QSettings::IniFormat);//settings.setIniCodec("UTF-8");
        settings2.setValue(QString("Modbus采集-未命名/类名"),"中文测试2");
    }
}

void MainWindow::on_pushButtonTSafeQueue_clicked()
{
    TSafeQueue<int> queue;
    for (int i=0;i<10000;i++) {
        queue.enqueue(i);

        ui->labelTSafeQueue->setText(QString::number(queue.size()));
        if(i%5 == 0)
            qApp->processEvents();
    }

    for (int i=0;i<10000;i++) {
        queue.dequeue();

        ui->labelTSafeQueue->setText(QString::number(queue.size()));
        if(i%5 == 0)
            qApp->processEvents();
    }
}
