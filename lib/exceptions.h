/**
 * 头文件：exceptions.h
 * 本头文件定义了数个新异常类，在处理语法过程中可能被抛出。
 */
#ifndef __CLASSES_MINIDB_H__
#define __CLASSES_MINIDB_H__

#include "i18n.h"

namespace minidb {

const string g_TempFileName = "std.temp";

const int g_ArgCntMax = INT32_MAX;	// 仅仅是一个形式上的作用。实际上懒得限制最大参数个数。

class ArgumentCountError extends public MiniDBExceptionBase {
	private:
		int expt;		//	expected
		int recv;		//	received
	public:
		ArgumentCountError(const int e, const int r, const i18nstring s = ""):expt(e),recv(r) { msg = s; };
		virtual const string what() const override {
			stringstream ss;
			ss << i18n::parseKey("argscerr",{msg.prints(),(expt<recv?i18n::parseKey("much").prints():i18n::parseKey("less").prints()),itos(expt)});
			if (recv != g_ArgCntMax) ss << i18n::parseKey("argscerr_r",{itos(recv)});
			ss << i18n::parseKey("argscerr_e");
			return ss.str().c_str();
		}
		virtual return_status status() const override { return return_status::argscerr; }
};
class InvalidArgument extends public MiniDBExceptionBase {
	public:
		InvalidArgument(const i18nstring s = "") { msg = s; };
		virtual const string what() const override { return i18n::parseKey("invalidarg",{msg.prints()}).prints().c_str(); }
		virtual return_status status() const override { return return_status::invarg; }
};
class SyntaxError extends public MiniDBExceptionBase {
	public:
		SyntaxError(const i18nstring s = "") { msg = s; };
		virtual const string what() const override { return i18n::parseKey("syntaxerr",{msg.prints()}).prints().c_str(); } 
		virtual return_status status() const override { return return_status::syntaxerr; }

};
class FailedFileOperation extends public MiniDBExceptionBase {
	public:
		FailedFileOperation(const i18nstring s = "") { msg = s; };
		virtual const string what() const override { return i18n::parseKey("ferr",{msg.prints()}).prints().c_str(); }
		virtual return_status status() const override { return return_status::ferr; }
};

ostream& operator<< (ostream& os, return_status rs) {
	return os << static_cast<int>(rs);
}

// namespace minidb end
}
#endif