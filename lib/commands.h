/**
 * 头文件：commands.h
 * 主要的流程和处理大多位于此处。此外，存储历史数据库记录的函数也位于此。
 */
#ifndef __MAIN_MINIDB_H__
#define __MAIN_MINIDB_H__

#include "paramanalysis.h"
#include "operations.h"

#ifdef __DEBUG_ENVIRONMENT__
	#include "loggers.h"
#endif

namespace minidb {

// 设置为true以让loggers闭嘴
bool gf_SilentLoggers = false;

cmd_type judgeCmdType(vstring&);					// 判别参数列表指定了什么类型的命令，同时处理参数列表
void callCommand(vstring, ostream&);			// 根据参数列表调用对应的函数

void parseCommand (ifstream&, ofstream&);		// 解析命令的主要逻辑流程

#ifndef __STORE_LEGACY__
	const string legacy_tmp_file_name = "";
#endif
#ifdef __STORE_LEGACY__
	const string legacy_file_name = "legacy.sql";
	const string legacy_tmp_file_name = "legacy.tmp";

	void storeLegacyDatabases();
	void readLegacyDatabases();
	/**
	 * 存储的思路简单粗暴：既然都写过parser了，那自然是反复利用最简单。
	 * 所以存储的时候直接以SQL语句输出至新文件，读取的时候直接过parser就行。
	 */
#endif

// 函数体定义在下方

void parseCommand(ifstream& ifile, ofstream& ofile) {
	string line;
	standardizeInputFile(ifile);
	g_LnCounter.clearAll();
	g_LnCounter.newl();
	while (getsUntil(ifile, line, symbols::cmdend)) {
		vstring params;
		params = splitParameters(line, ' ');
		
		#ifdef __DEBUG_ENVIRONMENT__
			if (!gf_SilentLoggers) {
				clog << endl << i18n::parseKey("h_debug_rawcmd");
				for (string str : params) {
					clog << str << ' ';
				}
				clog << endl;
			}
		#endif

		callCommand(params,ofile);
	}
	ifile.close();
}

cmd_type judgeCmdType(vstring& params) {
	eraseNewlFront(params);
	if (params.size() == 0) return cmd_type::null;
	g_LnCounter.increment();
	switch(getKeywordIndex(params.at(0))) {
		case keyword_name::create:
			params.erase(params.begin());		// 删去开头的"create"
			return parseCreateStParams(params);
		case keyword_name::use:
			params.erase(params.begin());		// 删去开头的"delete"
			return parseUseStParams(params);
		case keyword_name::drop:
			params.erase(params.begin());		// 删去开头的"drop"
			return parseDropStParams(params);
		case keyword_name::insert:
			params.erase(params.begin());		// 删去开头的"insert"
			return parseInsertStParams(params);
		case keyword_name::select:
			params.erase(params.begin());		// 删去开头的"select"
			return parseSelectStParams(params);
		case keyword_name::update:
			params.erase(params.begin());		// 删去开头的"update"
			return parseUpdateParams(params);
		case keyword_name::_delete:
			params.erase(params.begin());		// 删去开头的"delete"
			return parseDeletionStParams(params);
		default:
			throw SyntaxError(i18n::parseKey("unexptstr",{params.at(0)}));
	}

}

void callCommand(vstring params, ostream& os) {
	int size = params.size();
	cmd_type cmd_type;
	if (size == 0) {
		cmd_type = cmd_type::null;
	}
	else cmd_type = judgeCmdType(params);
	switch (cmd_type) {
		case cmd_type::createdb:
			runStCreateDatabase(params);
			#ifdef __DEBUG_ENVIRONMENT__
				if (!gf_SilentLoggers) logCreateDatabase(params);
			#endif
			break;
		case cmd_type::createtab:
			runStCreateTable(params);
			#ifdef __DEBUG_ENVIRONMENT__
				if (!gf_SilentLoggers) logCreateTable(params);
			#endif
			break;
		case cmd_type::usedb:
			runStUseDatabase(params);
			#ifdef __DEBUG_ENVIRONMENT__
				if (!gf_SilentLoggers) logUseDatabase(params);
			#endif
			break;
		case cmd_type::droptab:
			runStDropTable(params);
			#ifdef __DEBUG_ENVIRONMENT__
				if (!gf_SilentLoggers) logDropTable(params);
			#endif
			break;
		case cmd_type::insertion:
			runStInsertion(params);
			#ifdef __DEBUG_ENVIRONMENT__
				if (!gf_SilentLoggers) logInsertion(params);
			#endif
			break;
		case cmd_type::selection:		// 仅select ... (where)
			runStSelection(params, os);
			#ifdef __DEBUG_ENVIRONMENT__
				if (!gf_SilentLoggers) logSelection(params);
			#endif
			break;
		case cmd_type::innerjoin:		// 仅select ... inner join ...
			runStInnerJoin(params, os);
			#ifdef __DEBUG_ENVIRONMENT__
				if (!gf_SilentLoggers) logInnerJoin(params);
			#endif
			break;
		case cmd_type::update:
			runStUpdate(params);
			#ifdef __DEBUG_ENVIRONMENT__
				if (!gf_SilentLoggers) logUpdate(params);
			#endif
			break;
		case cmd_type::delfrom:
			runStDeleteFrom(params);
			#ifdef __DEBUG_ENVIRONMENT__
				if (!gf_SilentLoggers) logDeleteFrom(params);
			#endif
			break;
		case cmd_type::null:
			#ifdef __DEBUG_ENVIRONMENT__
				if (!gf_SilentLoggers) logNullStm();
			#endif
			break;
	}
}

// 以下内容仅在定义了宏__STORE_MINIDB_H__时生效
#ifdef __STORE_LEGACY__

void storeLegacyDatabases() {
	ofstream ofile;
	ofile.open(legacy_file_name, ios::out);
	if (!ofile.is_open()) {
		throw FailedFileOperation(i18n::parseKey("openlegfilef", {legacy_file_name}));
	}
	for (pair<string, Database> p_database : g_Databases) {
		ofile << "create database " << p_database.first << ';' << endl;
		ofile << "use database " << p_database.first << ';' << endl;
		for (pstable p_table : p_database.second.getRaw()) {
			ofile << "create table " << p_table.first << " ( ";

			// 处理标题行
			bool f_isFirst = true;
			for (psterm title : p_table.second.getTitle().getRaw()) {
				if (f_isFirst) f_isFirst = false;
				else ofile << " , ";
				ofile << title.first << ' ' << title.second.getType();
			}
			ofile << " );" << endl;

			// 处理其他行
			for (Row r : p_table.second.getRaw()) {
				ofile << "insert into " << p_table.first << " values ( ";
				bool f_isFirst = true;
				for (psterm p_term : r.getRaw()) {
					if (f_isFirst) f_isFirst = false;
					else ofile << " , ";
					p_term.second.print(ofile, true);
				}
				ofile << " );" << endl;
			}
		}
	}
}

void readLegacyDatabases() {
	
	ifstream ifile;
	ifile.open(legacy_file_name, ios::in);
	if (!ifile.is_open()) {						// 没开成功有可能是压根没这文件，也可能是打不开
		ofstream otemp;
		otemp.open(legacy_file_name, ios::trunc);		// 这是为了创建一个空文件
		if (!otemp.is_open()) {					// 文件都创建不了再报错
			throw FailedFileOperation(i18n::parseKey("openlegfilef", {legacy_file_name}));
		}
		otemp.close();
		ifile.open(legacy_file_name, ios::in);
		if (!ifile.is_open()) {
			throw FailedFileOperation(i18n::parseKey("openlegfilef", {legacy_file_name}));
		}
	}

	// 生成的历史查询记录不人为修改一定无误，这里懒得做try-catch了，直接读就完事了
	ofstream ofile;
	ofile.open(legacy_tmp_file_name, ios::out);
	if (!ofile.is_open()) {
		throw FailedFileOperation(i18n::parseKey("opentmpf", {legacy_tmp_file_name}));
	}

	gf_SilentLoggers = true;			// 让loggers闭嘴
	parseCommand(ifile, ofile);		// 复用parser加载历史内容
	gf_SilentLoggers = false;			// 让loggers恢复正常

	ifile.close();
	ofile.close();

	// 最后还要清除计数器，因为这个计数器是全局的，在上一步的加载中已经产生了内容，不清理掉会影响本次解析行号的正确性。
	g_LnCounter.clearAll();
	// 还有当前使用的database也要清除
	g_CurrentDatabaseName = "";
}

// 其实我想给legacy.sql上一层带校验的加密，这样可以使其被人为变更时不一定能解出合理的内容。
// 但是这也太麻烦了，能跑就行，反正Project没要求。
// 希望使用者们素质高一点不要乱动程序文件喵。

#endif

}

#endif