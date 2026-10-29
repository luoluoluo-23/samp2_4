#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 十个数字键共用一个槽：用 sender() 取出被点击按钮上的文字
    const QList<QPushButton *> digitButtons = {
        ui->btn_number_0, ui->btn_number_1, ui->btn_number_2, ui->btn_number_3,
        ui->btn_number_4, ui->btn_number_5, ui->btn_number_6, ui->btn_number_7,
        ui->btn_number_8, ui->btn_number_9
    };
    for (QPushButton *button : digitButtons) {
        connect(button, &QPushButton::clicked, this, &MainWindow::onDigitClicked);
    }

    connect(ui->btn_dot,       &QPushButton::clicked, this, &MainWindow::onDotClicked);
    connect(ui->btn_backspace, &QPushButton::clicked, this, &MainWindow::onBackspaceClicked);
    connect(ui->btn_clear_all, &QPushButton::clicked, this, &MainWindow::onClearAllClicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onDigitClicked()
{
    const QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (button == nullptr) {
        return;
    }

    QString text = ui->display->text();
    // 显示区起始值是 "0"，直接追加会变成 "05" 这种形式，这里把孤立的 0 顶掉
    if (text == QLatin1String("0")) {
        text.clear();
    }
    text += button->text();
    ui->display->setText(text);
}

void MainWindow::onDotClicked()
{
    // 同一个操作数里只允许出现一个小数点
    QString text = ui->display->text();
    if (!text.contains(QLatin1Char('.'))) {
        text += QLatin1Char('.');
        ui->display->setText(text);
    }
}

void MainWindow::onBackspaceClicked()
{
    QString text = ui->display->text();
    text.chop(1);
    if (text.isEmpty()) {
        text = QLatin1String("0");
    }
    ui->display->setText(text);
}

void MainWindow::onClearAllClicked()
{
    ui->display->setText(QLatin1String("0"));
}
