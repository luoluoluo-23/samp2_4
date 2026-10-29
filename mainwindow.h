#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "calculator.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QAbstractButton;

/**
 * @brief 计算器主窗口。
 *
 * 界面上的所有按键都先转换成统一的“记号”（数字、"+"、"."、"C" 等）再交给
 * applyToken() 处理，鼠标点击和键盘按键因此共用一条代码路径。
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    /// 所有按键共用的槽，参数是被点击的按钮
    void onButtonClicked(QAbstractButton *button);

private:
    /// 建立按键到处理逻辑的连接
    void setupConnections();
    /// 从 Qt 资源加载样式表
    void loadStyleSheet();
    /// 把一个记号交给核心逻辑，并刷新显示
    void applyToken(const QString &token);
    /// 把核心逻辑的当前状态同步到显示区
    void refreshDisplay();

    Ui::MainWindow *ui;
    Calculator m_calculator;
};

#endif // MAINWINDOW_H
