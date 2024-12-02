/**
 * 头文件：entry.h
 * 程序入口，以及垃圾处理。
 */
#ifndef __ENTRY_MINIDB_H__
#define __ENTRY_MINIDB_H__

#include "commands.h"

namespace minidb {

void deleteTempFiles();							// 清除程序中使用的临时文件。

return_status __Entry(int argc, char**& argv) {

	return_status status = return_status::success;
	ifstream ifile;
	ofstream ofile;
	bool f_UnacceptableCmdl = false;
	bool f_hasParsedCommand = false;

	// 这两行用于快速调试
	// argc = 3; argv[1] = "output/1.sql"; argv[2] = "output/2.txt";
	// argc = 5; argv[3] = "-lang"; argv[4] = "en";
	
	try {

		#ifdef __ENABLE_I18N__
			if (argc == 5) {
				if (argv[3] == string("-lang")) {
					g_LangCode = argv[4];
					argc = 3;
				}
				else {
					f_UnacceptableCmdl = true;
				}
			}
		#endif

		i18n::readKvPairs();

		if (argc != 3) {
			f_UnacceptableCmdl = true;
		}

		clog << endl << i18n::parseKey("welcome", {i18n::parseKey("authn").str()}) << endl;

		if (f_UnacceptableCmdl) {
			throw ArgumentCountError(2,argc-1,i18n::parseKey("unacptcmdl"));
		}

		#ifdef __STORE_LEGACY__
			readLegacyDatabases();
			clog << endl << i18n::parseKey("readlegfsuc") << endl;
		#endif
		
		ifile.open(argv[1], ios::in);
		ofile.open(argv[2], ios::out);

		if (!ifile.is_open()) {
			throw FailedFileOperation(i18n::parseKey("openifilef",{argv[1]}));
		}
		if (!ofile.is_open()) {
			throw FailedFileOperation(i18n::parseKey("openofilef",{argv[2]}));
		}

		f_hasParsedCommand = true;
		parseCommand(ifile, ofile);

		clog << endl << i18n::parseKey("atc") << endl;
	}
	catch (MiniDBExceptionBase& e) {
		cerr << endl << e.what() << g_LnCounter.where() << endl;
		status = e.status();
	} catch (exception& e) {
		cerr << endl << i18n::parseKey("unexpectederr", {e.what()}) << g_LnCounter.where() << endl;
		status = return_status::unexpt;
	}
	ifile.close();
	ofile.close();

	if (f_hasParsedCommand) {

		#ifdef __DEBUG_ENVIRONMENT__
			clog << endl << i18n::parseKey("h_dataexh") << endl;
			for (pair<string, Database> p_database : g_Databases) {
				clog << endl << "Database \"" << p_database.first << "\":" << endl;
				for (pstable p_table : p_database.second.getRaw()) {
					clog << endl << "Table \"" << p_table.first << "\" details:" << endl;
					p_table.second.print(clog);
				}
			}
			clog << endl << i18n::parseKey("h_dataexhend") << endl;
		#endif

		#ifdef __STORE_LEGACY__
			storeLegacyDatabases();
		#endif

		deleteTempFiles();

	}

	clog << endl << i18n::parseKey("exitstatus", {itos(static_cast<int>(status))}) << endl;

	return status;
}


// 删除临时文件
// 这个文件是在standardizeInputFile时被创建的，在程序结束时应被删除。
void deleteTempFiles() {
	vstring file_names = {g_TempFileName, legacy_tmp_file_name};
	for (string str : file_names) {
		if (str == "") continue;
		auto delete_status = remove(str.c_str());
		#ifdef __DEBUG_ENVIRONMENT__
			if (delete_status != 0) {
				clog << endl << i18n::parseKey("gcrmtmpf",{str}) << endl;
			}
			else clog << endl << i18n::parseKey("gcrmtmps",{str}) << endl;
		#endif
	}
}


}
#endif