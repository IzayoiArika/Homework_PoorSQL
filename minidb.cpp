/**
 * Hello! This is the main MiniDB section. Though it's really just providing a place to compile, plus an embedded comment document.
 * -------------------------------------------------------------------------------------------------------------------------------------
 * I promise:
 * 		The code in this Project submission was almost entirely done by myself, independently, with no one else involved in the process.
 * 
 * 		I used ChatGPT to ask about syntactic questions (e.g. "when does class polymorphism take effect?" ),
 * 			and did not use any code generated by it.
 * 
 * 		ChatGPT was not used for other steps such as [[logic build and debug]].
 * 
 * 		I asked my professor about the meaning of the unspecified part of the project manual by email.
 * 				However, I did not communicate about the implementation.
 * -------------------------------------------------------------------------------------------------------------------------------------
 * ※ It is recommended to read the detailed description of the project. It could be found below the main function.
 * ※ It is recommended to use 4 spaces as the width of a tab.
 */

/**				中文版说明
 * 你好！这是MiniDB主部分。虽然其实只是提供一个编译的位置，外加内嵌一个注释文档。
 * 我保证：
 * 		此Project提交的代码几乎全部由我自己独立完成，过程中无其他任何人参与。
 * 		【句法Debug】时使用了ChatGPT，但只进行错误原因的问询（例如询问“类的多态何时生效”之类语法层面的问题），未使用其生成的代码。
 * 		【逻辑构建、Debug】等其他步骤均未使用ChatGPT。
 * 		Project手册中未明部分通过邮件向教授问询了含义，但无相关实现的交流。
 * ※ 推荐阅读位于main函数下方的Project介绍。
 * ※ 推荐使用4空格作为Tab宽度。
 */

#include "lib/entry.h"

using namespace minidb;

int main(int argc, char** argv) {
	// __Entry(int, char**) 是程序实际开始的地方
	return static_cast<int>(__Entry(argc, argv));
}

// 本文件仅提供一个编译入口。
// This file is just for compilation.

/**				文件结构 File Structure
 * ---------------------------------------------------- *
 * 	minidb.cpp						-> lib/entry.h		*
 * ---------------------------------------------------- *
 * 	lib/												*
 * 		entry.h											*
 * 		->	commands.h									*
 * 			->	loggers.h			-> exceptions.h		*
 * 			->	operations.h		-> calculator.h		*
 * 			->	paramsanlys.h		-> stringop.h		*
 * ---------------------------------------------------- *
 * 			->	calculator.h							*
 * 				->	objects.h							*
 * 					->	stringop.h						*
 * 						->	auxiliaries.h				*
 * 							->	exceptions.h			*
 * 								->	i18n.h				*
 * 									->	environments.h	*
 * ---------------------------------------------------- *
 */


/**				命名风格 Name Styles
 * ---------------------------------------------------------------------------------------------------------------------
 * 类 / classes						CapitalizedCamel
 * 多数变量 / most variables		lowercase_underline
 * 命名空间 / namespaces			lowercase_underline
 * 枚举 / enums						lowercase_underline
 * 函数 / functions					doSthWithCamel		（有少量例外 There are few exceptions）
 * 宏 / macros						__FULL_LOWERCASE__
 * ---------------------------------------------------------------------------------------------------------------------
 * 迭代器总是叫做it、jt。			Iterators are always named after "it" or "jt". 
 * ---------------------------------------------------------------------------------------------------------------------
 * 所有名称都可能含有缩写。			All these names may contain abbreviations.
 * 
 * p_ 开头的变量是pair。			Variables begin with p_ are pairs.
 * ---------------------------------------------------------------------------------------------------------------------
 * 以下变量使用前缀+大驼峰命名，除非第一个词是虚词。
 * 			Variables below are named in prefix_CapitalizedCamel format, unless the first word is an empty word.
 * 
 * g_ 开头的变量是全局变量。		Variables begin with g_ are global variables.
 * f_ 开头的变量是bool类型的flag。	Variables begin with f_ are bool flags.
 * ---------------------------------------------------------------------------------------------------------------------
 * 特别的，i18n的key起得很随意。	Specially, keys for i18n is quite randomly named.
 */

