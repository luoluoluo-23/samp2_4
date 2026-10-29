#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QAbstractButton>
#include <QButtonGroup>
#include <QPushButton>

#include "tokens.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 所有按键放进同一个按钮组，统一走 onButtonClicked()；
    // 按钮上的文字就是“记号”，键盘事件稍后也产生同样的记号。
    auto *buttonGroup = new QButtonGroup(this);
    const QList<QPushButton *> buttons = findChildren<QPushButton *>();
    for (QPushButton *button : buttons) {
        buttonGroup->addButton(button);
    }
    connect(buttonGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),
            this, &MainWindow::onButtonClicked);

    refreshDisplay();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onButtonClicked(QAbstractButton *button)
{
    if (button == nullptr) {
        return;
    }
    applyToken(button->text());
}

void MainWindow::applyToken(const QString &token)
{
    if (token.isEmpty()) {
        return;
    }

    if (token.length() == 1 && token.at(0).isDigit()) {
        m_calculator.inputDigit(token);
    } else if (token == Token::dot()) {
        m_calculator.inputDot();
    } else if (token == Token::backspace()) {
        m_calculator.backspace();
    } else if (token == Token::clear()) {
        m_calculator.clearAll();
    } else if (token == Token::sign()) {
        m_calculator.toggleSign();
    } else if (token == Token::percent()) {
        m_calculator.inputPercent();
    } else if (token == Token::add()) {
        m_calculator.inputOperator(Calculator::Operator::Add);
    } else if (token == Token::subtract()) {
        m_calculator.inputOperator(Calculator::Operator::Subtract);
    } else if (token == Token::multiply()) {
        m_calculator.inputOperator(Calculator::Operator::Multiply);
    } else if (token == Token::divide()) {
        m_calculator.inputOperator(Calculator::Operator::Divide);
    } else if (token == Token::equals()) {
        m_calculator.inputEquals();
    }

    refreshDisplay();
}

void MainWindow::refreshDisplay()
{
    ui->display->setText(m_calculator.displayText());
    ui->expressionLabel->setText(m_calculator.expressionText());
}
