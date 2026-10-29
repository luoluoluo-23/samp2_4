#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @brief 计算器主窗口。
 *
 * 当前（实验步骤 1）只完成了 UI 布局与最基础的数字键/小数点/退格/清除接线，
 * 四则运算、键盘事件等在后续实验步骤中逐步补充。
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onDigitClicked();      ///< 0-9 十个数字键共用的槽，用 sender() 区分
    void onDotClicked();        ///< 小数点
    void onBackspaceClicked();  ///< 退格
    void onClearAllClicked();   ///< 清除

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
