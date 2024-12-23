/**
 * 头文件：environment.h
 * 此头文件的内容是MiniDB用到的标准库内容、typedef、常用宏。
 */
#ifndef __ENVIRONMENT_MINIDB_H__
#define __ENVIRONMENT_MINIDB_H__

#include <string>
#include <vector>
#include <queue>
#include <stack>
#include <map>
#include <fstream>
#include <exception>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>

// 个人偏好，从Java借鉴了以extends表示继承的写法。我觉得这样一个有意义的单词更容易看懂一些。
#define extends :

// 程序需要的所有std内容
// 容器
using std::vector;
using std::queue;
using std::stack;
using std::string;		// 话说这个算容器吗……？
using std::map;
using std::pair;

// 异常
using std::exception;
using std::out_of_range;
using std::invalid_argument;

// 流
using std::ifstream;
using std::ofstream;
using std::fstream;
using std::istream;
using std::ostream;
using std::stringstream;

// 其他小物件
using std::ios;
using std::stoi;
using std::getline;
using std::clog;
using std::cerr;
using std::endl;
using std::noskipws;
using std::to_string;
using std::fmod;

// 整个Project大量使用了字符串数组，因此特别将这个过分长的类型名typedef成比较短的形式
typedef vector<string> vstring;

// 判断两double相等否的最低差值绝对值
const double g_DoubleEqCritDelta = 1e-6;

/**
 * 此外我还定义了一些宏，用于控制程序的一些特性。
 * 
 * 1. __DEBUG_ENVIRONMENT__ 
 * 		控制程序是否处在调试环境下。发布时默认关闭。
 * 		调试环境下额外会打开控制台log。不过，任何情况下都会将警告和错误信息输出到控制台。
 * 
 * 2. __ENABLE_I18N__
 * 		控制程序是否允许国际化功能。发布时默认关闭。
 * 		这个功能是写where解析的时候破防了一气之下写的，Project文档里没让做这个，所以我设置了一个宏开关。
 * 		打开的时候需要一个./i18n/xxx.ini文件作为语言文件，其中xxx是任意合法的文件名。
 * 		在原本的命令行结尾追加“ -lang xxx ”，就可以调用对应的语言文件进行翻译。
 * 		例如：minidb 1.sql 2.csv -lang zh_cn 可以调用./i18n/zh_cn.ini语言文件进行翻译。
 * 		如果没能成功打开语言文件，默认使用英语。
 * 		部分文本因特殊原因被硬编码至代码内，无法通过语言文件进行修改。
 * 		语言文件的格式为“key=value”，注意等号两侧无空格。value中使用“%n”以插入第n个参数，使用“%%”以插入单独的百分号“%”。
 * 
 * 3. __STORE_LEGACY__
 * 		控制程序是否生成历史数据记录。发布时默认开启。
 * 		调试的时候并不总是希望它保存历史记录，不然弄起来很费劲，所以开了这个宏来方便调试。
 * 
 * 4. __PRINT_FINAL_SEPARATOR__
 * 		控制程序在最后一个select语句后是否要加分隔用的横线。
 * 		默认不加，因为加了实在是看着很蠢。但是输出样例要求要加，那我只好顺从他。
 */

#define __DEBUG_ENVIRONMENT__
// #define __ENABLE_I18N__
#define __STORE_LEGACY__
#define __PRINT_FINAL_SEPARATOR__

#endif