#ifndef TOKENS_H
#define TOKENS_H

#include <QString>

/**
 * @brief 计算器内部统一的“记号”。
 *
 * 按钮上的文字就是记号，键盘按键也会被翻译成同样的记号，
 * 这样鼠标与键盘才能共用 applyToken() 这唯一一条处理路径。
 * 统一定义在头文件里，避免主程序和测试各写一份字符串导致不一致。
 */
namespace Token {

inline QString clear()      { return QStringLiteral("C"); }
inline QString backspace()  { return QStringLiteral("←"); }
inline QString percent()    { return QStringLiteral("%"); }
inline QString sign()       { return QStringLiteral("±"); }
inline QString add()        { return QStringLiteral("+"); }
inline QString subtract()   { return QStringLiteral("-"); }
inline QString multiply()   { return QStringLiteral("×"); }
inline QString divide()     { return QStringLiteral("÷"); }
inline QString equals()     { return QStringLiteral("="); }
inline QString dot()        { return QStringLiteral("."); }

} // namespace Token

#endif // TOKENS_H
