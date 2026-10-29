#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QString>

/**
 * @brief 计算器核心状态机，与界面完全解耦。
 *
 * 鼠标点击和键盘按键最终都翻译成同样的“记号”（数字、小数点、运算符……）再交给这里处理，
 * 因此两条输入路径共用同一套逻辑，不会出现鼠标能用、键盘不能用的情况。
 *
 * 当前（实验步骤 2）只管理“正在输入的操作数”，四则运算状态在步骤 3 补充。
 */
class Calculator
{
public:
    /// 输入一个数字字符（"0"～"9"）
    void inputDigit(const QString &digit);
    /// 输入小数点
    void inputDot();
    /// 退格：删除当前操作数的最后一位
    void backspace();
    /// 清除：回到初始状态
    void clearAll();
    /// 正负号切换
    void toggleSign();
    /// 百分号：当前操作数除以 100
    void inputPercent();

    /// 应该显示在窗口上的文本
    QString displayText() const { return m_currentInput; }

private:
    /// 单个操作数允许的最大字符数，防止无限输入把显示区挤爆
    static const int kMaxInputLength = 16;

    QString m_currentInput = QStringLiteral("0");  ///< 当前正在输入的操作数
};

#endif // CALCULATOR_H
