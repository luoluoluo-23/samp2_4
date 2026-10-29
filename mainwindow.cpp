#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->btn_number_0,SIGNAL(clicked()),this,SLOT(btn_number_clicked()));
    connect(ui->btn_number_1,SIGNAL(clicked()),this,SLOT(btn_number_clicked()));
    connect(ui->btn_number_2,SIGNAL(clicked()),this,SLOT(btn_number_clicked()));
    connect(ui->btn_number_3,SIGNAL(clicked()),this,SLOT(btn_number_clicked()));
    connect(ui->btn_number_4,SIGNAL(clicked()),this,SLOT(btn_number_clicked()));
    connect(ui->btn_number_5,SIGNAL(clicked()),this,SLOT(btn_number_clicked()));
    connect(ui->btn_number_6,SIGNAL(clicked()),this,SLOT(btn_number_clicked()));
    connect(ui->btn_number_7,SIGNAL(clicked()),this,SLOT(btn_number_clicked()));
    connect(ui->btn_number_8,SIGNAL(clicked()),this,SLOT(btn_number_clicked()));
    connect(ui->btn_number_9,SIGNAL(clicked()),this,SLOT(btn_number_clicked()));
}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::btn_number_clicked()
{
    QString str=ui->display->text();
    str+=qobject_cast<QPushButton*>(sender())->text();
    ui->display->setText(str);
}

void MainWindow::on_btn_period_clicked()
{
    QString str=ui->display->text();
    if(!str.contains("."))
        str+=qobject_cast<QPushButton*>(sender())->text();
    ui->display->setText(str);
}


void MainWindow::on_btn_clear_clicked()
{
    QString str=ui->display->text();
    str = str.left(str.length()-1);
    ui->display->setText(str);
}

