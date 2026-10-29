/**
 * @file  test_main.cpp
 * @brief 计算器自动化检查工具。
 *
 * 两种用途：
 *   1) 直接运行          -> 执行界面结构、输入/运算、鼠标与键盘一致性检查，结果写入 test-report.txt；
 *   2) --screenshot 文件 -> 把主窗口渲染成 PNG，供实验报告使用。
 *
 * 之所以自己做这套检查：Qt Widgets 程序无法在无人值守环境下手工点按钮，
 * 通过模拟点击与按键事件，才能在命令行里验证“鼠标和键盘是否走同一套逻辑”。
 */

#include <QAbstractButton>
#include <QApplication>
#include <QFile>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QStringList>
#include <QTextStream>
#include <QVector>

#include "mainwindow.h"
#include "tokens.h"

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
    out << QStringLiteral("检查项：") << (g_passed + g_failed)
        << QStringLiteral("，通过：") << g_passed
        << QStringLiteral("，失败：") << g_failed << QStringLiteral("\n");
    for (const QString &line : g_report) {
        out << line << "\n";
    }
}

/// 记号 -> 按钮对象名，便于用“点击按钮”的方式驱动测试
static QString buttonNameForToken(const QString &token)
{
    if (token.size() == 1 && token.at(0).isDigit()) {
        return QStringLiteral("btn_number_") + token;
    }
    if (token == Token::dot())       { return QStringLiteral("btn_dot"); }
    if (token == Token::add())       { return QStringLiteral("btn_plus"); }
    if (token == Token::subtract())  { return QStringLiteral("btn_minus"); }
    if (token == Token::multiply())  { return QStringLiteral("btn_multiply"); }
    if (token == Token::divide())    { return QStringLiteral("btn_divide"); }
    if (token == Token::equals())    { return QStringLiteral("btn_equals"); }
    if (token == Token::clear())     { return QStringLiteral("btn_clear_all"); }
    if (token == Token::backspace()) { return QStringLiteral("btn_backspace"); }
    if (token == Token::percent())   { return QStringLiteral("btn_percent"); }
    if (token == Token::sign())      { return QStringLiteral("btn_sign"); }
    return QString();
}

/// 按顺序“点击”若干按钮，返回最后的显示内容（模拟鼠标输入）
static QString clickSequence(MainWindow &window, const QStringList &tokens)
{
    auto *clear = window.findChild<QPushButton *>(QStringLiteral("btn_clear_all"));
    if (clear != nullptr) {
        clear->click();
    }
    for (const QString &token : tokens) {
        auto *button = window.findChild<QPushButton *>(buttonNameForToken(token));
        if (button == nullptr) {
            return QStringLiteral("<没有对应 %1 的按钮>").arg(token);
        }
        button->click();
    }
    auto *display = window.findChild<QLineEdit *>(QStringLiteral("display"));
    return display != nullptr ? display->text() : QString();
}

/// --screenshot 模式：渲染主窗口（可先用 --input "1 2 + 3 4 =" 点击一串按键）
static int shoot(QApplication &app, const QString &path, const QStringList &tokens)
{
    MainWindow window;
    window.show();
    app.processEvents();
    if (!tokens.isEmpty()) {
        clickSequence(window, tokens);
        app.processEvents();
    }
    const QPixmap pixmap = window.grab();
    return pixmap.save(path) ? 0 : 1;
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

    // 按钮上的文字必须是 tokens.h 里定义过的记号，否则键盘/鼠标映射会静默失配
    for (const QString &name : expectedButtons) {
        auto *button = window.findChild<QPushButton *>(name);
        if (button == nullptr) {
            continue;
        }
        const QString mapped = buttonNameForToken(button->text());
        check(mapped == name,
              QStringLiteral("按钮文字与记号映射一致：%1").arg(name),
              QStringLiteral("文字“%1”映射到“%2”").arg(button->text(), mapped));
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
}

/// 输入相关检查：数字、小数点、退格、清除以及异常输入
static void checkInputHandling(MainWindow &window)
{
    struct Case {
        QStringList tokens;
        QString expected;
        QString note;
    };
    const QVector<Case> cases = {
        {{"1", "2", "3"},                         QStringLiteral("123"),
         QStringLiteral("连续输入数字")},
        {{"0", "0", "7"},                          QStringLiteral("7"),
         QStringLiteral("前导零不累积")},
        {{"0", "5"},                               QStringLiteral("5"),
         QStringLiteral("0 开头的数字顶替")},
        {{"1", ".", "2"},                          QStringLiteral("1.2"),
         QStringLiteral("正常小数")},
        {{"1", ".", ".", "2"},                     QStringLiteral("1.2"),
         QStringLiteral("连续按两次小数点，第二个被忽略")},
        {{".", "5"},                               QStringLiteral("0.5"),
         QStringLiteral("直接按小数点补 0")},
        {{"1", ".", "2", "."},                     QStringLiteral("1.2"),
         QStringLiteral("已有小数点时再按无效")},
        {{"1", "2", "3", "←"},                     QStringLiteral("12"),
         QStringLiteral("退格删除一位")},
        {{"5", "←"},                               QStringLiteral("0"),
         QStringLiteral("退格删空后回到 0")},
        {{"1", "←", "←", "←"},                     QStringLiteral("0"),
         QStringLiteral("退格到底仍为 0，不会出现空串")},
        {{"1", "2", "C"},                          QStringLiteral("0"),
         QStringLiteral("清除")},
        {{"1", ".", "5", "C", "7"},                QStringLiteral("7"),
         QStringLiteral("清除后重新输入")},
        {{"5", "±"},                               QStringLiteral("-5"),
         QStringLiteral("正负号切换")},
        {{"5", "±", "±"},                          QStringLiteral("5"),
         QStringLiteral("连续切换两次回到正数")},
        {{"5", "0", "%"},                          QStringLiteral("0.5"),
         QStringLiteral("百分号")}
    };

    for (const Case &item : cases) {
        const QString actual = clickSequence(window, item.tokens);
        check(actual == item.expected,
              QStringLiteral("输入处理：%1（%2）").arg(item.tokens.join(QLatin1Char(' ')), item.note),
              QStringLiteral("期望 %1，实际 %2").arg(item.expected, actual));
    }
}

/// 读取显示区上方的小字（算式）
static QString expressionText(MainWindow &window)
{
    auto *label = window.findChild<QLabel *>(QStringLiteral("expressionLabel"));
    return label != nullptr ? label->text() : QStringLiteral("<没有 expressionLabel>");
}

/// 四则运算与状态管理检查
static void checkArithmetic(MainWindow &window)
{
    struct Case {
        QStringList tokens;
        QString expected;
        QString note;
    };
    const QVector<Case> cases = {
        {{"1", "2", "+", "3", "4", "="},           QStringLiteral("46"),
         QStringLiteral("12 + 34")},
        {{"7", "×", "8", "="},                     QStringLiteral("56"),
         QStringLiteral("7 × 8")},
        {{"9", "-", "4", "="},                     QStringLiteral("5"),
         QStringLiteral("9 - 4")},
        {{"8", "÷", "2", "="},                     QStringLiteral("4"),
         QStringLiteral("8 ÷ 2")},
        {{"1", ".", "5", "+", "2", ".", "5", "="}, QStringLiteral("4"),
         QStringLiteral("小数相加 1.5 + 2.5")},
        {{"1", "0", "÷", "4", "="},                QStringLiteral("2.5"),
         QStringLiteral("10 ÷ 4 保留小数")},
        {{"0", ".", "1", "+", "0", ".", "2", "="}, QStringLiteral("0.3"),
         QStringLiteral("0.1 + 0.2 不出现浮点尾巴")},
        {{"2", "5", "-", "3", "0", "="},           QStringLiteral("-5"),
         QStringLiteral("25 - 30 得到负数")},
        {{"5", "+", "3"},                          QStringLiteral("3"),
         QStringLiteral("按下运算符后再输入，显示第二个操作数")},
        {{"5", "="},                               QStringLiteral("5"),
         QStringLiteral("没有运算符时按等号不改变显示")}
    };

    for (const Case &item : cases) {
        const QString actual = clickSequence(window, item.tokens);
        check(actual == item.expected,
              QStringLiteral("运算：%1（%2）").arg(item.tokens.join(QLatin1Char(' ')), item.note),
              QStringLiteral("期望 %1，实际 %2").arg(item.expected, actual));
    }

    // 算式提示：按下运算符后应显示第一个操作数和运算符
    clickSequence(window, {"1", "2", "+"});
    check(expressionText(window) == QStringLiteral("12 +"),
          QStringLiteral("算式提示：按下 + 后显示“12 +”"), expressionText(window));

    // 等号之后应显示完整算式
    clickSequence(window, {"6", "×", "7", "="});
    check(expressionText(window) == QStringLiteral("6 × 7 ="),
          QStringLiteral("算式提示：等号后显示“6 × 7 =”"), expressionText(window));

    // 清除后算式提示也要清空
    clickSequence(window, {"1", "2", "+", "3", "C"});
    check(expressionText(window).isEmpty(),
          QStringLiteral("清除后算式提示为空"), expressionText(window));
}

/// 连续运算与边界情况检查
static void checkEdgeCases(MainWindow &window)
{
    struct Case {
        QStringList tokens;
        QString expected;
        QString note;
    };
    const QVector<Case> cases = {
        // —— 连续运算 ——
        {{"2", "+", "3", "+", "4", "="},           QStringLiteral("9"),
         QStringLiteral("连续加法 2+3+4")},
        {{"1", "0", "-", "3", "×", "2", "="},      QStringLiteral("14"),
         QStringLiteral("混合运算 (10-3)×2，按顺序计算")},
        {{"8", "÷", "2", "+", "1", "="},           QStringLiteral("5"),
         QStringLiteral("先除后加")},
        {{"5", "+", "3", "+"},                     QStringLiteral("8"),
         QStringLiteral("连续运算时中间结果立即显示")},

        // —— 除零 ——
        {{"5", "÷", "0", "="},                     QStringLiteral("除数不能为0"),
         QStringLiteral("除数为 0 给出错误提示")},
        {{"5", "÷", "0", "=", "7"},                QStringLiteral("7"),
         QStringLiteral("除零后按数字即重新开始")},
        {{"5", "÷", "0", "=", "+"},                QStringLiteral("除数不能为0"),
         QStringLiteral("除零后按运算符被忽略，提示保留")},
        {{"5", "÷", "0", "=", "C", "8", "+", "1", "="}, QStringLiteral("9"),
         QStringLiteral("除零后按 C 可正常重新计算")},
        {{"9", "÷", "0", ".", "5", "="},           QStringLiteral("18"),
         QStringLiteral("除数 0.5 不是零，正常计算 9÷0.5")},

        // —— 计算结果后继续输入 ——
        {{"2", "+", "3", "=", "4"},                QStringLiteral("4"),
         QStringLiteral("出结果后按数字开始新的一轮")},
        {{"2", "+", "3", "=", "4", "×", "2", "="}, QStringLiteral("8"),
         QStringLiteral("用上一次的结果继续运算")},
        {{"2", "+", "3", "=", "="},                QStringLiteral("8"),
         QStringLiteral("连按等号重复上一次运算")},
        {{"2", "+", "3", "=", "=", "="},           QStringLiteral("11"),
         QStringLiteral("连按三次等号")},

        // —— 退格 ——
        {{"1", "2", "+", "3", "4", "←"},           QStringLiteral("3"),
         QStringLiteral("退格修改第二个操作数")},
        {{"1", "2", "+", "←"},                     QStringLiteral("1"),
         QStringLiteral("刚按完运算符时退格修改第一个操作数")},
        {{"1", "2", "+", "3", "=", "←"},           QStringLiteral("15"),
         QStringLiteral("结果不允许退格修改")},

        // —— 清除后重新计算 ——
        {{"1", "2", "+", "3", "C", "8", "+", "2", "="}, QStringLiteral("10"),
         QStringLiteral("清除后重新计算 8+2")}
    };

    for (const Case &item : cases) {
        const QString actual = clickSequence(window, item.tokens);
        check(actual == item.expected,
              QStringLiteral("边界：%1（%2）").arg(item.tokens.join(QLatin1Char(' ')), item.note),
              QStringLiteral("期望 %1，实际 %2").arg(item.expected, actual));
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    const QStringList args = QCoreApplication::arguments();
    const int shotIndex = args.indexOf(QStringLiteral("--screenshot"));
    if (shotIndex > 0 && shotIndex + 1 < args.size()) {
        QStringList tokens;
        const int inputIndex = args.indexOf(QStringLiteral("--input"));
        if (inputIndex > 0 && inputIndex + 1 < args.size()) {
            tokens = args.at(inputIndex + 1).split(QLatin1Char(' '), QString::SkipEmptyParts);
        }
        return shoot(app, args.at(shotIndex + 1), tokens);
    }

    MainWindow window;
    window.show();
    app.processEvents();

    checkUiStructure(window);
    checkInputHandling(window);
    checkArithmetic(window);
    checkEdgeCases(window);

    flushReport(QStringLiteral("test-report.txt"));
    QTextStream(stdout) << "checks=" << (g_passed + g_failed)
                        << " passed=" << g_passed
                        << " failed=" << g_failed << "\n";
    return g_failed == 0 ? 0 : 1;
}
