#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QString>

/**
 * @brief 计算器核心状态机，与界面完全解耦。
 *
 * 鼠标点击和键盘按键最终都翻译成同样的“记号”（数字、小数点、运算符……）再交给这里处理，
 * 因此两条输入路径共用同一套逻辑，不会出现鼠标能用、键盘不能用的情况。
 *
 * 状态说明（双操作数模型）：
 *   m_storedValue       第一个操作数；
 *   m_pendingOperator   已按下但还没执行的运算符；
 *   m_currentInput      第二个操作数（显示区里正在编辑的数字），或者计算结果；
 *   m_waitingForOperand 刚按完运算符，下一次输入应当开始一个新的操作数；
 *   m_resultShown       刚按完等号，显示的是计算结果；
 *   m_hasError          处于错误状态（例如除数为 0），除数字/小数点/清除外一律忽略；
 *   m_lastOperator/m_lastOperand 上一次运算，用于连按等号时重复计算。
 */
class Calculator
{
public:
    /// 四则运算符
    enum class Operator { None, Add, Subtract, Multiply, Divide };

    // ---- 输入 ----
    void inputDigit(const QString &digit);
    void inputDot();
    void inputOperator(Operator op);
    void inputEquals();
    void backspace();
    void clearAll();
    void toggleSign();
    void inputPercent();

    // ---- 输出 ----
    /// 主显示区文本（当前操作数、计算结果或错误提示）
    QString displayText() const { return m_currentInput; }
    /// 显示区上方的小字，例如 "12 +" 或 "12 + 34 ="
    QString expressionText() const { return m_expression; }
    /// 是否处于错误状态
    bool hasError() const { return m_hasError; }

private:
    /// 单个操作数允许的最大字符数，防止无限输入把显示区挤爆
    static const int kMaxInputLength = 16;

    double currentValue() const;
    /// 执行一次运算；除数为 0 时返回 false，由调用者进入错误状态
    static bool tryApply(double lhs, double rhs, Operator op, double *result);
    static QString operatorSymbol(Operator op);
    static QString formatNumber(double value);

    /// 开始一个新的操作数（按下运算符或等号之后调用）
    void beginNewOperand();
    /// 进入错误状态：清空待执行运算，显示错误提示
    void enterErrorState(const QString &message);

    QString m_currentInput = QStringLiteral("0");  ///< 当前操作数 / 计算结果显示
    double m_storedValue = 0.0;                    ///< 第一个操作数
    Operator m_pendingOperator = Operator::None;   ///< 待执行的运算符
    bool m_waitingForOperand = false;              ///< 等待输入第二个操作数
    bool m_resultShown = false;                    ///< 刚显示完计算结果
    bool m_hasError = false;                       ///< 错误状态
    Operator m_lastOperator = Operator::None;      ///< 上一次执行的运算符
    double m_lastOperand = 0.0;                    ///< 上一次运算的右操作数
    QString m_expression;                          ///< 上方小字显示的算式
};

#endif // CALCULATOR_H
