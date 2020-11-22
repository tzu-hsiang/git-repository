#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "Recorder.h"

using namespace std ;

class Assembler
{
    public:
        static Assembler& getAssembler() ;

        void lexicalAnalysis() ;
        bool syntaxAnalysis() ;
        void clearTempStorage() ;

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


        // small temporary buffer list
        string undef_token_buf ;
        SyntaxCheckBuffer syntaxCheckBuffer ;
        // end

        void addTokenFrmBuf(bool DelimiterFlag, bool DigitFlag, bool StringFlag) ;
        void addTokenDelimiter(char) ;
        bool isDigits(string&) ;
        bool isDelimiter(char&) ;
        void cutOneLineToTokenBuf(string& code_line) ;
        void setIndexForTokenRecords(unsigned int line_idx) ;
        bool checkIsComment(Recorder::LineInfo &oneLine) ;
        bool pseudoCheck(unsigned int lineNum, Recorder::LineInfo &oneLine) ;

        // pseudo check
        bool assume_test(unsigned int lineNum, Recorder::LineInfo &oneLine) ;
        bool segment_proc_test(unsigned int lineNum, Recorder::LineInfo &oneLine) ;
        bool db_dw_byte_word_test(unsigned int lineNum, Recorder::LineInfo &oneLine) ;
        // end
} ;

#endif // ASSEMBLER_H
