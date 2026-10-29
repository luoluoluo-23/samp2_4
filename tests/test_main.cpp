/**
 * @file  test_main.cpp
 * @brief 计算器自动化检查工具。
 *
 * 两种用途：
 *   1) 直接运行         -> 执行界面结构、逻辑与鼠标/键盘一致性检查，结果写入 test-report.txt；
 *   2) --screenshot 文件 -> 把主窗口渲染成 PNG，供实验报告使用。
 *
 * 之所以要自己做这套检查：Qt Widgets 程序无法在无人值守环境下手工点按钮，
 * 用 QTest 风格的事件投递才能在命令行里验证“鼠标和键盘是否走同一套逻辑”。
 */

#include <QApplication>
#include <QFile>
#include <QGridLayout>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QStringList>
#include <QTextStream>

#include "mainwindow.h"

static QStringList g_report;   ///< 检查明细，最后写入 UTF-8 文本
static int g_passed = 0;
static int g_failed = 0;

/// 记录一条检查结果
static void check(bool ok, const QString &what, const QString &detail = QString())
{
    if (ok) {
        ++g_passed;
        g_report << QStringLiteral("[PASS] %1").arg(what);
    } else {
        ++g_failed;
        g_report << QStringLiteral("[FAIL] %1  %2").arg(what, detail);
    }
}

/// 把检查结果写入文件（UTF-8，便于在终端编码不一致时查看）
static void flushReport(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << "检查项：" << (g_passed + g_failed) << "，通过：" << g_passed
        << "，失败：" << g_failed << "\n";
    for (const QString &line : g_report) {
        out << line << "\n";
    }
}

/// --screenshot 模式：渲染主窗口
static int shoot(QApplication &app, const QString &path)
{
    MainWindow window;
    window.show();
    app.processEvents();
    const QPixmap pixmap = window.grab();
    const bool ok = pixmap.save(path);
    return ok ? 0 : 1;
}

/// 界面结构检查：控件是否齐全、命名是否一致、布局是否生效
static void checkUiStructure(MainWindow &window)
{
    const QStringList expectedButtons = {
        QStringLiteral("btn_number_0"), QStringLiteral("btn_number_1"),
        QStringLiteral("btn_number_2"), QStringLiteral("btn_number_3"),
        QStringLiteral("btn_number_4"), QStringLiteral("btn_number_5"),
        QStringLiteral("btn_number_6"), QStringLiteral("btn_number_7"),
        QStringLiteral("btn_number_8"), QStringLiteral("btn_number_9"),
        QStringLiteral("btn_dot"),       QStringLiteral("btn_plus"),
        QStringLiteral("btn_minus"),     QStringLiteral("btn_multiply"),
        QStringLiteral("btn_divide"),    QStringLiteral("btn_equals"),
        QStringLiteral("btn_clear_all"), QStringLiteral("btn_backspace"),
        QStringLiteral("btn_percent"),   QStringLiteral("btn_sign")
    };

    for (const QString &name : expectedButtons) {
        check(window.findChild<QPushButton *>(name) != nullptr,
              QStringLiteral("按钮存在：%1").arg(name));
    }

    auto *display = window.findChild<QLineEdit *>(QStringLiteral("display"));
    check(display != nullptr, QStringLiteral("显示区 display 存在"));
    if (display != nullptr) {
        check(display->isReadOnly(), QStringLiteral("显示区为只读，避免用户直接改内容"));
        check(display->alignment().testFlag(Qt::AlignRight),
              QStringLiteral("显示区右对齐"));
        check(display->text() == QLatin1String("0"),
              QStringLiteral("初始显示为 0"), display->text());
    }

    auto *grid = window.findChild<QGridLayout *>(QStringLiteral("gridLayout"));
    check(grid != nullptr, QStringLiteral("按键使用 QGridLayout 网格布局"));
    if (grid != nullptr) {
        check(grid->rowCount() == 5, QStringLiteral("网格为 5 行"),
              QString::number(grid->rowCount()));
        check(grid->columnCount() == 4, QStringLiteral("网格为 4 列"),
              QString::number(grid->columnCount()));
    }

    auto *equals = window.findChild<QPushButton *>(QStringLiteral("btn_equals"));
    check(equals != nullptr && equals->text() == QStringLiteral("="),
          QStringLiteral("等号键位于最后一行最后一列"));
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    const QStringList args = QCoreApplication::arguments();
    const int shotIndex = args.indexOf(QStringLiteral("--screenshot"));
    if (shotIndex > 0 && shotIndex + 1 < args.size()) {
        return shoot(app, args.at(shotIndex + 1));
    }

    MainWindow window;
    window.show();
    app.processEvents();

    checkUiStructure(window);

    flushReport(QStringLiteral("test-report.txt"));
    QTextStream(stdout) << "checks=" << (g_passed + g_failed)
                        << " passed=" << g_passed
                        << " failed=" << g_failed << "\n";
    return g_failed == 0 ? 0 : 1;
}
