#include "calculator.h"

#include <QtMath>

void Calculator::inputDigit(const QString &digit)
{
    // 出错之后随便按一个数字即重新开始
    if (m_hasError) {
        clearAll();
    }
    if (m_currentInput.length() >= kMaxInputLength) {
        return;
    }

    // 刚按完运算符，或者刚按完等号：这次输入要开始一个新的操作数
    if (m_waitingForOperand || m_resultShown) {
        beginNewOperand();
    }

    // 显示为 "0" 时，新数字直接顶替它，避免出现 "05"、"007" 这类前导零
    if (m_currentInput == QLatin1String("0")) {
        m_currentInput = digit;
    } else {
        m_currentInput += digit;
    }
}

void Calculator::inputDot()
{
    if (m_hasError) {
        clearAll();
    }
    if (m_waitingForOperand || m_resultShown) {
        beginNewOperand();
    }
    // 同一个操作数里只允许有一个小数点：已有小数点时直接忽略
    if (m_currentInput.contains(QLatin1Char('.'))) {
        return;
    }
    if (m_currentInput.length() >= kMaxInputLength) {
        return;
    }
    m_currentInput += QLatin1Char('.');
}

void Calculator::inputOperator(Operator op)
{
    if (m_hasError) {
        return;  // 错误状态下先按 C 或数字重新开始
    }

    if (m_pendingOperator != Operator::None && !m_waitingForOperand) {
        // 连续运算：先把上一步算出来，再把结果作为新的第一个操作数
        double result = 0.0;
        if (!tryApply(m_storedValue, currentValue(), m_pendingOperator, &result)) {
            enterErrorState(QStringLiteral("除数不能为0"));
            return;
        }
        m_storedValue = result;
        m_currentInput = formatNumber(result);
    } else if (m_pendingOperator == Operator::None) {
        // 普通情况，或者刚按完等号（此时显示的就是结果）
        m_storedValue = currentValue();
    }
    // 连按两次运算符时既不计算也不覆盖第一个操作数，只替换运算符

    m_pendingOperator = op;
    m_waitingForOperand = true;
    m_resultShown = false;
    m_expression = formatNumber(m_storedValue) + QLatin1Char(' ') + operatorSymbol(op);
}

void Calculator::inputEquals()
{
    if (m_hasError) {
        return;
    }

    if (m_pendingOperator == Operator::None) {
        // 连按等号：用上一次的运算符和右操作数再算一次，例如 2 + 3 = = 得 8
        if (!m_resultShown || m_lastOperator == Operator::None) {
            return;  // 还没算过任何东西，等号没有意义
        }
        double repeated = 0.0;
        if (!tryApply(currentValue(), m_lastOperand, m_lastOperator, &repeated)) {
            enterErrorState(QStringLiteral("除数不能为0"));
            return;
        }
        m_expression = formatNumber(currentValue()) + QLatin1Char(' ')
                       + operatorSymbol(m_lastOperator) + QLatin1Char(' ')
                       + formatNumber(m_lastOperand) + QStringLiteral(" =");
        m_currentInput = formatNumber(repeated);
        m_storedValue = repeated;
        return;
    }

    const double lhs = m_storedValue;
    const double rhs = currentValue();
    double result = 0.0;
    if (!tryApply(lhs, rhs, m_pendingOperator, &result)) {
        enterErrorState(QStringLiteral("除数不能为0"));
        return;
    }

    m_lastOperator = m_pendingOperator;
    m_lastOperand = rhs;
    m_expression = formatNumber(lhs) + QLatin1Char(' ') + operatorSymbol(m_pendingOperator)
                   + QLatin1Char(' ') + formatNumber(rhs) + QStringLiteral(" =");
    m_currentInput = formatNumber(result);
    m_storedValue = result;
    m_pendingOperator = Operator::None;
    m_waitingForOperand = false;
    m_resultShown = true;
}

void Calculator::backspace()
{
    if (m_hasError) {
        clearAll();
        return;
    }
    if (m_resultShown) {
        return;  // 结果是算出来的，不允许按退格去改
    }

    m_currentInput.chop(1);
    // 删空（或只剩一个负号）时回到 "0"，显示区不会空着
    if (m_currentInput.isEmpty() || m_currentInput == QLatin1String("-")) {
        m_currentInput = QStringLiteral("0");
    }
    // 刚按完运算符时，显示区里其实是第一个操作数，退格要同步改掉它
    if (m_waitingForOperand) {
        m_storedValue = currentValue();
    }
}

void Calculator::clearAll()
{
    m_currentInput = QStringLiteral("0");
    m_storedValue = 0.0;
    m_pendingOperator = Operator::None;
    m_waitingForOperand = false;
    m_resultShown = false;
    m_hasError = false;
    m_lastOperator = Operator::None;
    m_lastOperand = 0.0;
    m_expression.clear();
}

void Calculator::toggleSign()
{
    if (m_hasError) {
        return;
    }
    if (m_currentInput == QLatin1String("0")) {
        return;  // 0 取反没有意义
    }
    if (m_currentInput.startsWith(QLatin1Char('-'))) {
        m_currentInput.remove(0, 1);
    } else {
        m_currentInput.prepend(QLatin1Char('-'));
    }
    if (m_waitingForOperand) {
        m_storedValue = currentValue();
    }
}

void Calculator::inputPercent()
{
    if (m_hasError) {
        return;
    }
    m_currentInput = formatNumber(currentValue() / 100.0);
}

void Calculator::beginNewOperand()
{
    m_currentInput = QStringLiteral("0");
    m_waitingForOperand = false;
    // 上一次是“等号出结果”，说明这一轮计算已经结束，上方的算式也一并清掉
    if (m_resultShown) {
        m_resultShown = false;
        m_expression.clear();
        m_lastOperator = Operator::None;
    }
}

void Calculator::enterErrorState(const QString &message)
{
    m_hasError = true;
    m_currentInput = message;
    m_storedValue = 0.0;
    m_pendingOperator = Operator::None;
    m_waitingForOperand = false;
    m_resultShown = false;
    m_lastOperator = Operator::None;
    m_expression.clear();
}

double Calculator::currentValue() const
{
    return m_currentInput.toDouble();
}

bool Calculator::tryApply(double lhs, double rhs, Operator op, double *result)
{
    switch (op) {
    case Operator::Add:
        *result = lhs + rhs;
        return true;
    case Operator::Subtract:
        *result = lhs - rhs;
        return true;
    case Operator::Multiply:
        *result = lhs * rhs;
        return true;
    case Operator::Divide:
        if (rhs == 0.0) {
            return false;  // 除数为 0
        }
        *result = lhs / rhs;
        return true;
    case Operator::None:
        break;
    }
    *result = rhs;
    return true;
}

QString Calculator::operatorSymbol(Operator op)
{
    switch (op) {
    case Operator::Add:      return QStringLiteral("+");
    case Operator::Subtract: return QStringLiteral("-");
    case Operator::Multiply: return QStringLiteral("×");
    case Operator::Divide:   return QStringLiteral("÷");
    case Operator::None:     break;
    }
    return QString();
}

QString Calculator::formatNumber(double value)
{
    if (qIsNaN(value)) {
        return QStringLiteral("错误");
    }
    if (qIsInf(value)) {
        return QStringLiteral("溢出");
    }
    QString text = QString::number(value, 'g', 12);
    if (text == QLatin1String("-0")) {
        text = QStringLiteral("0");
    }
    return text;
}
