#include "calculator.h"

#include <QtMath>

#include <limits>

void Calculator::inputDigit(const QString &digit)
{
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
    // 连按两次运算符时，只更新运算符，不丢失已经输入的第一个操作数
    if (m_pendingOperator == Operator::None) {
        m_storedValue = currentValue();
    }

    m_pendingOperator = op;
    m_waitingForOperand = true;
    m_resultShown = false;
    m_expression = formatNumber(m_storedValue) + QLatin1Char(' ') + operatorSymbol(op);
}

void Calculator::inputEquals()
{
    if (m_pendingOperator == Operator::None) {
        return;  // 没有待执行的运算符，等号没有意义
    }

    const double lhs = m_storedValue;
    const double rhs = currentValue();
    const double result = applyOperation(lhs, rhs, m_pendingOperator);

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
    m_currentInput.chop(1);
    // 删空（或只剩一个负号）时回到 "0"，显示区不会空着
    if (m_currentInput.isEmpty() || m_currentInput == QLatin1String("-")) {
        m_currentInput = QStringLiteral("0");
    }
    m_resultShown = false;
}

void Calculator::clearAll()
{
    m_currentInput = QStringLiteral("0");
    m_storedValue = 0.0;
    m_pendingOperator = Operator::None;
    m_waitingForOperand = false;
    m_resultShown = false;
    m_expression.clear();
}

void Calculator::toggleSign()
{
    if (m_currentInput == QLatin1String("0")) {
        return;  // 0 取反没有意义
    }
    if (m_currentInput.startsWith(QLatin1Char('-'))) {
        m_currentInput.remove(0, 1);
    } else {
        m_currentInput.prepend(QLatin1Char('-'));
    }
}

void Calculator::inputPercent()
{
    const double value = currentValue() / 100.0;
    m_currentInput = formatNumber(value);
}

void Calculator::beginNewOperand()
{
    m_currentInput = QStringLiteral("0");
    m_waitingForOperand = false;
    // 上一次是“等号出结果”，说明这一轮计算已经结束，上方的算式也一并清掉
    if (m_resultShown) {
        m_resultShown = false;
        m_expression.clear();
    }
}

double Calculator::currentValue() const
{
    return m_currentInput.toDouble();
}

double Calculator::applyOperation(double lhs, double rhs, Operator op)
{
    switch (op) {
    case Operator::Add:      return lhs + rhs;
    case Operator::Subtract: return lhs - rhs;
    case Operator::Multiply: return lhs * rhs;
    case Operator::Divide:   return rhs == 0.0 ? std::numeric_limits<double>::quiet_NaN() : lhs / rhs;
    case Operator::None:     break;
    }
    return rhs;
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
