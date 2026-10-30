# 计算器（Qt Widgets）

实验一「带键盘事件的计算器」的完整实现：四则运算、小数、退格、清除、连续运算、
除零处理，并且鼠标与键盘共用同一套处理逻辑。

## 目录结构

```
lab1.pro            主工程
main.cpp            程序入口
mainwindow.h/.cpp   主窗口：界面、按键/键盘事件分发
calculator.h/.cpp   计算状态机（不依赖界面，可单独测试）
tokens.h            鼠标与键盘共用的“记号”定义
style.qss           灰白简约主题样式表
resources.qrc       把 style.qss 打进 Qt 资源
tests/              自动化检查程序（110 项）
docs/               各步骤界面截图
实验日志.md          按实验步骤记录的实现、测试与问题修复过程
```

## 构建与运行

环境：Qt 5.12.11（MinGW 7.3.0 64 位）。

```powershell
mkdir build; cd build
qmake ..\lab1.pro
mingw32-make -j4
.\release\lab1.exe
```

自动化检查：

```powershell
mkdir build-tests; cd build-tests
qmake ..\tests\tests.pro
mingw32-make -j4
.\release\lab1_tests.exe          # 结果写入 test-report.txt
```

截图（可选，需要图形界面）：

```powershell
.\release\lab1_tests.exe --screenshot ..\docs\ui.png --input "1 2 + 3 4 ="
```

## 键盘操作

| 按键 | 功能 |
| --- | --- |
| `0`–`9` | 输入数字 |
| `.` | 小数点 |
| `+` `-` `*` `/` | 加、减、乘、除 |
| `Enter` / `=` | 计算 |
| `Backspace` | 退格 |
| `Esc` / `Delete` / `C` | 清除 |
| `%` | 百分号 |

按钮和显示区都设了 `Qt::NoFocus`，键盘事件直接送到窗口的 `keyPressEvent()`，
再翻译成与鼠标点击相同的记号交给 `Calculator` 处理，因此两条输入路径不会出现行为差异。

## 功能覆盖

- 四则运算、小数输入、退格、清除、正负号、百分号
- 连续小数点、前导零、连续运算符等异常输入
- 连续运算（`2+3+4=` → `9`）、连按等号（`2+3==` → `8`）
- 除数为 0 时提示“除数不能为0”，按数字或 `C` 即可恢复
- 结果后继续输入、退格修改第一个/第二个操作数、清除后重新计算
