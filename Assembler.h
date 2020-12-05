#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "Recorder.h"
#include <cstdlib>
#include <algorithm>
#include <stack>

using namespace std ;

class Assembler
{
    public:
        static Assembler& getAssembler() ;

        void lexicalAnalysis() ;
        bool syntaxAnalysis() ;
        void clearTempStorage() ;

        // translate process
        bool pass1() ;
        // pass2()

        // end

    private:
        Assembler() ;
        Assembler(const Assembler& src) ;
        const Assembler& operator = (const Assembler&) ;
        ~Assembler() ;

        typedef struct SynChBuf
        {
            string code_seg_name ; // code segment name
            short seg_num ;
            short proc_num ;
            vector<string> seg_name ;
            vector<string> proc_name ;
            bool end_defined ;
        } SyntaxCheckBuffer ;

        // for translate process
        typedef struct SymbolEntry
        {
            string seg_scope ; // cs, ds, ss, es
            string name ;
            string address ; // (__hex)
        } SymbolEntry ;

        unsigned int virtual_PC ;
        vector<SymbolEntry> symbol_table ;

        // each instruction addr
        vector<string> instrs_addr ;
        // end

        // small temporary buffer list
        string undef_token_buf ;
        SyntaxCheckBuffer syntaxCheckBuffer ;
        vector<string> err_msg_list ;
        // end

        void addTokenFrmBuf(bool DelimiterFlag, bool DigitFlag, bool StringFlag) ;
        void addTokenDelimiter(char) ;
        bool isDigits(string&) ;
        bool isDelimiter(char&) ;
        void cutOneLineToTokenBuf(string& code_line) ;
        void setIndexForTokenRecords(unsigned int line_idx) ;
        bool checkIsComment(Recorder::LineInfo oneLine) ;
        bool pseudoCheck(unsigned int lineNum, Recorder::LineInfo oneLine) ;

        // pseudo check
        bool assume_test(unsigned int lineNum, Recorder::LineInfo oneLine) ;
        bool segment_proc_test(unsigned int lineNum, Recorder::LineInfo oneLine) ;
        bool db_dw_byte_word_test(unsigned int lineNum, Recorder::LineInfo oneLine) ;
        bool org_test(unsigned int lineNum, Recorder::LineInfo oneLine) ;
        bool equ_test(unsigned int lineNum, Recorder::LineInfo oneLine) ;
        bool ptr_test(unsigned int lineNum, Recorder::LineInfo oneLine) ;
        bool offset_test(unsigned int lineNum, Recorder::LineInfo oneLine) ;
        // end

        // pass1 process
        bool pseudo_filter(Recorder::LineInfo oneLine, unsigned int line_num, string &cs_onto_seg_name,
                           string &ds_onto_seg_name, string &ss_onto_seg_name,
                           string &es_onto_seg_name, string &using_seg_name,
                           string &using_seg_reg, unsigned short &error_status) ;
        int atoi_hex_to_dec(string number) ;
        void display_addr(unsigned int pc, string &addr) ;
        bool add_symbol_entry(unsigned int pc, string &name, string &using_seg_reg,
                              bool dup_check) ;
        void infix_to_postfix(string &src, string &result) ;
        int priority(char &op) ;
        string calculate_postfix(string infix, string &postfix) ;

        bool normal_addressing_syntax_check(Recorder::LineInfo oneLine, unsigned int begin, unsigned int end,
                                     unsigned int line_num, bool &disp) ;
        bool ptr_addressing_syntax_check(Recorder::LineInfo oneLine, unsigned int begin, unsigned int end,
                                     unsigned int line_num, bool &disp) ;

        bool instr_type_1_filter(Recorder::LineInfo oneLine) ;
        bool instr_type_2_filter(Recorder::LineInfo oneLine) ;
        bool instr_type_3_filter(Recorder::LineInfo oneLine, unsigned int line_num,
                                 unsigned short &error_status) ;
        // type 4 is same as 3 for check

        bool is_expression(Recorder::LineInfo oneLine, unsigned int begin, unsigned int end,
                                     unsigned int line_num, string &expression, unsigned short &error_status) ;
        bool instr_type_5_filter(Recorder::LineInfo oneLine, unsigned int line_num,
                                 unsigned short &error_status) ;

        bool instr_type_6_filter(Recorder::LineInfo oneLine, unsigned int line_num,
                                 unsigned short &error_status) ;
        bool instr_type_7_filter(Recorder::LineInfo oneLine, unsigned int line_num,
                                 unsigned short &error_status) ;
        bool instr_type_8_filter(Recorder::LineInfo oneLine, unsigned int line_num,
                                 unsigned short &error_status) ;
        void instr_type_9_filter(Recorder::LineInfo &oneLine) ;
        void instr_type_10_filter(Recorder::LineInfo &oneLine) ;
        void instr_type_11_filter(Recorder::LineInfo &oneLine) ;
        void instr_type_12_filter(Recorder::LineInfo &oneLine) ;
        void instr_type_13_filter(Recorder::LineInfo &oneLine) ;
        void instr_type_14_filter(Recorder::LineInfo &oneLine) ;
        void instr_type_15_filter(Recorder::LineInfo &oneLine) ;
        void instr_type_16_filter(Recorder::LineInfo &oneLine) ;
        void instr_type_17_filter(Recorder::LineInfo &oneLine) ;
        void instr_type_18_filter(Recorder::LineInfo &oneLine) ;
        void instr_type_19_filter(Recorder::LineInfo &oneLine) ;
        void instr_type_20_filter(Recorder::LineInfo &oneLine) ;
        // end

} ;

#endif // ASSEMBLER_H
