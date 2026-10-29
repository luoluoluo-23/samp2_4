#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QAbstractButton>
#include <QApplication>
#include <QButtonGroup>
#include <QFile>
#include <QKeyEvent>
#include <QPushButton>

#include "tokens.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setupConnections();
    setupFocusPolicy();
    loadStyleSheet();
    refreshDisplay();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupConnections()
{
    // 所有按键放进同一个按钮组，统一走 onButtonClicked()；
    // 按钮上的文字就是“记号”，键盘事件也产生同样的记号。
    auto *buttonGroup = new QButtonGroup(this);
    const QList<QPushButton *> buttons = findChildren<QPushButton *>();
    for (QPushButton *button : buttons) {
        buttonGroup->addButton(button);
    }
    connect(buttonGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),
            this, &MainWindow::onButtonClicked);
}

void MainWindow::loadStyleSheet()
{
    // 样式表放在 :/style.qss（resources.qrc），界面与样式分离，改配色不用动代码
    QFile styleFile(QStringLiteral(":/style.qss"));
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qApp->setStyleSheet(QString::fromUtf8(styleFile.readAll()));
    }
}

void MainWindow::setupFocusPolicy()
{
    // 按钮和显示区都不接受焦点：否则点过某个按钮后按回车会再次触发它，
    // 键盘事件也到不了窗口的 keyPressEvent()
    ui->display->setFocusPolicy(Qt::NoFocus);
    const QList<QPushButton *> buttons = findChildren<QPushButton *>();
    for (QPushButton *button : buttons) {
        button->setFocusPolicy(Qt::NoFocus);
    }
    setFocusPolicy(Qt::StrongFocus);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    const QString token = tokenForKey(event->key());
    if (token.isEmpty()) {
        QMainWindow::keyPressEvent(event);  // 不认识的键交回基类处理
        return;
    }
    applyToken(token);                     // 与鼠标点击完全相同的处理入口
    event->accept();
}

QString MainWindow::tokenForKey(int key)
{
    if (key >= Qt::Key_0 && key <= Qt::Key_9) {
        return QString::number(key - Qt::Key_0);
    }
    switch (key) {
    case Qt::Key_Period:    return Token::dot();
    case Qt::Key_Plus:      return Token::add();
    case Qt::Key_Minus:     return Token::subtract();
    case Qt::Key_Asterisk:  return Token::multiply();
    case Qt::Key_Slash:     return Token::divide();
    case Qt::Key_Equal:     // 主键盘的 = 和“等号”含义一致
    case Qt::Key_Return:
    case Qt::Key_Enter:     return Token::equals();
    case Qt::Key_Backspace: return Token::backspace();
    case Qt::Key_Escape:
    case Qt::Key_Delete:
    case Qt::Key_C:         return Token::clear();
    case Qt::Key_Percent:   return Token::percent();
    default:                break;
    }
    return QString();
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
