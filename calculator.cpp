#include "calculator.h"

void Calculator::inputDigit(const QString &digit)
{
    // 已经达到长度上限时忽略这次输入，避免显示区被无限撑长
    if (m_currentInput.length() >= kMaxInputLength) {
        return;
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
    // 同一个操作数里只允许有一个小数点：已有小数点时直接忽略
    if (m_currentInput.contains(QLatin1Char('.'))) {
        return;
    }
    if (m_currentInput.length() >= kMaxInputLength) {
        return;
    }
    m_currentInput += QLatin1Char('.');
}

void Calculator::backspace()
{
    m_currentInput.chop(1);
    // 删空（或只剩一个负号）时回到 "0"，显示区不会空着
    if (m_currentInput.isEmpty() || m_currentInput == QLatin1String("-")) {
        m_currentInput = QStringLiteral("0");
    }
}

void Calculator::clearAll()
{
    m_currentInput = QStringLiteral("0");
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
    const double value = m_currentInput.toDouble() / 100.0;
    m_currentInput = QString::number(value, 'g', 12);
}
