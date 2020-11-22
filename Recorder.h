#ifndef RECORDER_H
#define RECORDER_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

using namespace std ;

class Recorder
{
    public:
        typedef struct TB{
            string item ;
        } Table ;

        typedef struct TBIDX{
            unsigned short table_idx ;
            unsigned short item_idx ;
        } TableIndex ;

        typedef struct Tkn
        {
            string name ;
            unsigned short table_num ;
            unsigned short item_num ;
            bool isDelimiter ;
            bool isDigits ;
            bool isString ;
        } Token ;

        typedef struct LIF
        {
            string origin_insr ;
            vector<Token> line_token ;
        } LineInfo ;

        // record each line starting with the nth token.
        // can also know how many tokens there are in each line.
        vector<unsigned int> lineStartPt ;
        int commentStartPt ;

        vector<string> code_segment ;
        vector<Token> token_list ;
        vector<string> comment_list ;
        vector<LineInfo> line_info_list ;

        static Recorder& getRecorder() ;
        void init(int init_size) ;
        bool importTables() ;
        bool importInstrSequence() ;
        static void strToUpper(string&) ;
        static void strToLower(string&) ;
        static bool isAllWhite(string&) ;
        TableIndex searchStaticTable(string& tokenName, bool& delimiterF) ;
        TableIndex hashInStrTable(string& tokenName) ;
        TableIndex hashInSymbolTable(string& tokenName) ;
        TableIndex hashInDigitsTable(string& tokenName) ;

        bool exportOneLineRes(unsigned int code_line_idx) ;
        void clearTempStorage() ;
        void freeLexicalUsedMem() ;

    private:
        Recorder() ;
        Recorder(const Recorder& src) ;
        const Recorder& operator = (const Recorder&) ;
        ~Recorder() ;

        bool initialization ;

        // Not Allowed for Modification
        vector<Table> table_instr ;
        vector<Table> table_pseudo ;
        vector<Table> table_register ;
        vector<Table> table_delimiter ;
        // End

        vector<Table> table_symbol ;
        vector<Table> table_intAndReal ;
        vector<Table> table_string ;

        string imported_file_name ;

        unsigned short hashFunc(string& src) ;


} ;

#endif // RECORDER_H
