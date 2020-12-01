#include "Assembler.h"

Assembler::Assembler()
{}

Assembler::~Assembler()
{}

Assembler& Assembler::getAssembler()
{
    static Assembler assembler ;
    return assembler ;
}

void Assembler::clearTempStorage()
{
    undef_token_buf.clear() ;
    virtual_PC = 0 ; ;
    vector<SymbolEntry>().swap(symbol_table) ;
    vector<string>().swap(instrs_addr) ;
}

bool Assembler::isDigits(string& token)
{
    bool check = false ;

    int last = token.size() - 1 ;

    if (token[last] == 'H' || token[last] == 'h') // maybe hexadecimal
    {
        string exceptWord = token ;
        Recorder::strToUpper(exceptWord) ;
        if (!exceptWord.compare("AH") || !exceptWord.compare("BH") ||
            !exceptWord.compare("CH") || !exceptWord.compare("DH") )
            return false ; // these are register

        for (unsigned int i = 0; i < token.size() - 1; i++)
        {
            if ((token[i] >= '0' && token[i] <= '9') ||
                (token[i] >= 'A' && token[i] <= 'F') ||
                (token[i] >= 'a' && token[i] <= 'f'))
                check = true ;
            else
                return false ;
        }

        return check ; // is hexadecimal
    }
    else if (token[last] >= '0' && token[last] <= '9') // maybe decimal
    {
        for (unsigned int i = 0; i < token.size(); i++)
        {
            if (token[i] >= '0' && token[i] <= '9')
                check = true ;
            else
                return false ;
        }

        return check ; // is decimal
    }

    return false ;
}

bool Assembler::isDelimiter(char& src)
{
    if (src == ',' || src == ':' ||
        src == '\'' || src == '[' ||
        src == ']' || src == '+' ||
        src == '-' || src == '*' ||
        src == '/' || src == '?' ||
        src == '(' || src == ')' )
    {
        return true ;
    }
    else
        return false ;
}

bool Assembler::checkIsComment(Recorder::LineInfo &oneLine)
{
    if (oneLine.line_token.at(0).name.compare(";") == 0)
    {
        return true ;
    }

    return false ;
}

bool Assembler::assume_test(unsigned int lineNum, Recorder::LineInfo &oneLine)
{
    bool codeSegExist = false ;
    unsigned short processIdx = 1 ; // expect 1=seg_reg, 2=':', 3=seg_name, 4=','

    for (unsigned int i = 0; i < oneLine.line_token.size(); i++)
    {
        // Token: ASSUME

        if (oneLine.line_token.at(0).table_num == 2 &&
            oneLine.line_token.at(0).item_num == 5)
        {
            if (oneLine.line_token.at(i).name.compare("CS") == 0)
                codeSegExist = true ;
            else
            {
                if (!codeSegExist)
                    ;
            }

            // check syntax of seg_reg and seg_name
            if (oneLine.line_token.size() < 4)
            {
                cout << "line " << lineNum << ": expect complete syntax after 'ASSUME'.\n" ;
                return false ;
            }
            else if (i >= 1 && processIdx >= 1 && processIdx <= 4)
            {
                if (processIdx == 1) // expect a seg register
                {
                    if (oneLine.line_token.at(i).name.compare("CS") != 0 &&
                        oneLine.line_token.at(i).name.compare("DS") != 0 &&
                        oneLine.line_token.at(i).name.compare("SS") != 0 &&
                        oneLine.line_token.at(i).name.compare("ES") != 0 )
                    {
                        cout << "line " << lineNum << ": expect a segment register.\n" ;
                        return false ;
                    }

                    processIdx = 2 ;
                }
                else if (processIdx == 2) // expect a ':'
                {

                    if (oneLine.line_token.at(i).name.compare(":") != 0)
                    {
                        cout << "line " << lineNum << ": expect a ':' .\n" ;
                        return false ;
                    }

                    processIdx = 3 ;
                }
                else if (processIdx == 3) // expect a segment name
                {
                    if (oneLine.line_token.at(i).table_num != 5 &&
                        oneLine.line_token.at(i).name.compare("CODE") != 0 )
                    {
                        cout << "line " << lineNum << ": expect a segment name.\n" ;
                        return false ;
                    }

                    // if it's a correct syntax and CS is defined, record code segment name
                    if (oneLine.line_token.at(i-2).name.compare("CS") == 0)
                    {
                        syntaxCheckBuffer.code_seg_name = oneLine.line_token.at(i).name ;
                    }

                    processIdx = 4 ;
                }
                else // expect ',' , if it's has other declaration
                {
                    if (oneLine.line_token.at(i).name.compare(",") != 0)
                    {
                        cout << "line " << lineNum << ": expect a ',' .\n" ;
                        return false ;
                    }

                    processIdx = 1 ;
                }
            }

        }
        else if (oneLine.line_token.at(i).table_num == 2 &&
                 oneLine.line_token.at(i).item_num == 5 &&
                 i != 0)
        {
            cout << "line " << lineNum << ": it should not place anything before 'ASSUME'.\n" ;
            return false ;
        }

        if (oneLine.line_token.at(0).table_num == 2 &&
            oneLine.line_token.at(0).item_num == 5 &&
            !codeSegExist && i == oneLine.line_token.size() - 1)
        {
            cout << "line " << lineNum << ": code segment is undefined.\n" ;
            return false ;
        }
    } // end for

    if (oneLine.line_token.at(0).table_num == 2 &&
        oneLine.line_token.at(0).item_num == 5 &&
        (processIdx == 1 || processIdx == 2 || processIdx == 3))
    {
        cout << "line " << lineNum << ": expect complete syntax after 'ASSUME'.\n" ;
        return false ;
    }

    // end check

    return true ;
}

bool Assembler::segment_proc_test(unsigned int lineNum, Recorder::LineInfo &oneLine)
{
    for (unsigned int i = 0; i < oneLine.line_token.size(); i++)
    {
        // segment check
        if ((oneLine.line_token.size() != 2 &&
             oneLine.line_token.size() != 3 ) &&
            oneLine.line_token.at(0).name.compare("SEGMENT") == 0)
        {
            cout << "line " << lineNum << ": expect complete syntax after 'SEGMENT'\n" ;
            return false ;
        }

        if (oneLine.line_token.at(i).name.compare("SEGMENT") == 0 &&
            i != 1)
        {
            cout << "line " << lineNum << ": SEGMENT syntax error.\n" ;
            return false ;
        }

        if (oneLine.line_token.at(i).name.compare("SEGMENT") == 0 &&
            i == 1)
        {
            if (i+1 < oneLine.line_token.size())
            {
                if (oneLine.line_token.at(i+1).name.compare("PUBLIC") != 0 &&
                    oneLine.line_token.at(i+1).name.compare("PRIVATE") != 0 &&
                    oneLine.line_token.at(i+1).name.compare("STACK") != 0 &&
                    oneLine.line_token.at(i+1).name.compare("MEMORY") != 0 &&
                    oneLine.line_token.at(i+1).name.compare("COMMON") != 0 &&
                    oneLine.line_token.at(i+1).name.compare("VIRTUAL") != 0 &&
                    oneLine.line_token.at(i+1).name.compare("AT") != 0 &&
                    oneLine.line_token.at(i+1).name.compare("UNINIT") != 0)
                {
                    cout << "line " << lineNum << ": Unkown segment attribute.\n" ;
                    return false ;
                }
            }

            // if syntax is correct, but define PROC first
            if (syntaxCheckBuffer.proc_num > 0 && syntaxCheckBuffer.seg_num == 0)
            {
                cout << "line " << lineNum << ": SEGMENT needs definition before PROC.\n" ;
                return false ;
            }

            // use keyword as a segment name
            if (oneLine.line_token.at(i-1).table_num != 5 &&
                oneLine.line_token.at(i-1).name.compare("CODE") != 0)
            {
                cout << "line " << lineNum << ": can not use keyword as a segment name.\n" ;
                return false ;
            }

            // syntax is correct
            syntaxCheckBuffer.seg_name.push_back(oneLine.line_token.at(i-1).name) ;
            syntaxCheckBuffer.seg_num++ ;
            break ;
        }

        if (oneLine.line_token.at(i).name.compare("ENDS") == 0 && i != 1)
        {
            cout << "line " << lineNum << ": ENDS syntax error.\n" ;
            return false ;
        }

        if (oneLine.line_token.at(i).name.compare("ENDS") == 0 && i == 1)
        {
            if (oneLine.line_token.size() != 2)
            {
                cout << "line " << lineNum << ": ENDS syntax error.\n" ;
                return false ;
            }

            // check segment name is defined
            bool found = false ;

            for (unsigned int a = 0; a < syntaxCheckBuffer.seg_name.size(); a++)
            {
                if (syntaxCheckBuffer.seg_name.at(a).compare(oneLine.line_token.at(0).name) == 0)
                {
                    found = true ;
                    break ;
                }
                else
                {
                    found = false ;
                }
            }

            if (!found)
            {
                cout << "line " << lineNum << ": undefined segment name.\n" ;
                return false ;
            }
            else
                syntaxCheckBuffer.seg_num-- ;
        }
        // end segment check

        // proc check
        if (oneLine.line_token.size() != 3 &&
            oneLine.line_token.at(0).name.compare("PROC") == 0)
        {
            cout << "line " << lineNum << ": expect complete syntax after 'PROC'\n" ;
            return false ;
        }

        if (oneLine.line_token.at(i).name.compare("PROC") == 0 &&
            i != 1)
        {
            cout << "line " << lineNum << ": PROC syntax error.\n" ;
            return false ;
        }

        if (oneLine.line_token.at(i).name.compare("PROC") == 0 &&
            i == 1)
        {

            if (oneLine.line_token.at(i+1).name.compare("NEAR") != 0 &&
                oneLine.line_token.at(i+1).name.compare("FAR") != 0 )
            {
                cout << "line " << lineNum << ": Unkown proc attribute.\n" ;
                return false ;
            }

            // if syntax is correct, but define PROC first
            if (syntaxCheckBuffer.proc_num > 0 && syntaxCheckBuffer.seg_num == 0)
            {
                cout << "line " << lineNum << ": SEGMENT needs definition before PROC.\n" ;
                return false ;
            }

            // use keyword as a proc name
            if (oneLine.line_token.at(i-1).table_num != 5 &&
                oneLine.line_token.at(i-1).name.compare("CODE") != 0)
            {
                cout << "line " << lineNum << ": can not use keyword as a segment name.\n" ;
                return false ;
            }

            // syntax is correct
            syntaxCheckBuffer.proc_name.push_back(oneLine.line_token.at(i-1).name) ;
            syntaxCheckBuffer.proc_num++ ;

        }

        if (oneLine.line_token.at(i).name.compare("ENDP") == 0 && i != 1)
        {
            cout << "line " << lineNum << ": ENDS syntax error.\n" ;
            return false ;
        }

        if (oneLine.line_token.at(i).name.compare("ENDP") == 0 && i == 1)
        {
            if (oneLine.line_token.size() != 2)
            {
                cout << "line " << lineNum << ": ENDP syntax error.\n" ;
                return false ;
            }

            // check proc name is defined
            bool found = false ;

            for (unsigned int a = 0; a < syntaxCheckBuffer.proc_name.size(); a++)
            {
                if (syntaxCheckBuffer.proc_name.at(a).compare(oneLine.line_token.at(0).name) == 0)
                {
                    found = true ;
                    break ;
                }
                else
                {
                    found = false ;
                }
            }

            if (!found)
            {
                cout << "line " << lineNum << ": undefined proc name.\n" ;
                return false ;
            }
            else
                syntaxCheckBuffer.proc_num-- ;
        }
        // end proc check

        // end check
        if (oneLine.line_token.at(i).name.compare("END") == 0 && i != 0)
        {
            cout << "line " << lineNum << ": END syntax error.\n" ;
            return false ;
        }

        if (oneLine.line_token.at(i).name.compare("END") == 0 && i == 0)
        {
            if (oneLine.line_token.size() != 2)
            {
                cout << "line " << lineNum << ": END syntax error.\n" ;
                return false ;
            }

            if (!(syntaxCheckBuffer.proc_num == 0 && syntaxCheckBuffer.seg_num == 0))
            {
                cout << "line " << lineNum << ": END syntax error, it is defined before ENDS or ENDP.\n" ;
                return false ;
            }

            if (oneLine.line_token.at(i+1).table_num != 5 &&
                oneLine.line_token.at(i).name.compare("CODE") != 0)
            {
                cout << "line " << lineNum << ": END syntax error.\n" ;
                return false ;
            }

            // syntax correct
            break ;
        }

    } // end for

    // reach end, check segment and ends are symmetrical
    Recorder &recorder = Recorder::getRecorder() ;

    if ((lineNum == (recorder.line_info_list.size() - 1)) &&
        syntaxCheckBuffer.seg_num != 0)
    {
        if (syntaxCheckBuffer.seg_num > 0)
        {
            cout << "line " << lineNum << ": reach end of non-symmetrical segment.\n" ;
            return false ;
        }
        else
        {
            cout << "line " << lineNum << ": ENDS is redundant.\n" ;
            return false ;
        }
    }

    // reach end, check proc and endp are symmetrical
    if ((lineNum == (recorder.line_info_list.size() - 1)) &&
        syntaxCheckBuffer.proc_num != 0)
    {

        if (syntaxCheckBuffer.proc_num > 0)
        {
            cout << "line " << lineNum << ": reach end of non-symmetrical procedure.\n" ;
            return false ;
        }
        else
        {
            cout << "line " << lineNum << ": ENDP is redundant.\n" ;
            return false ;
        }
    } // end check proc and endp are symmetrical

    // reach end, check code segment is defined
    if (lineNum == (recorder.line_info_list.size() - 1))
    {
        bool found = false ;

        for (unsigned int a = 0; a < syntaxCheckBuffer.seg_name.size(); a++)
        {
            if (syntaxCheckBuffer.seg_name.at(a).compare(syntaxCheckBuffer.code_seg_name) == 0)
            {
                found = true ;
                break ;
            }
            else
            {
                found = false ;
            }
        }

        if (!found)
        {
            cout << "line " << lineNum << ": code segment is undefined.\n" ;
            return false ;
        }
    } // end check code segment is defined


    return true ;
}

bool Assembler::db_dw_byte_word_test(unsigned int lineNum, Recorder::LineInfo &oneLine)
{
    // 1: digits or ' | 2: string | 3: ' | 4: , or DUP or ) | 5: DUP-> ( | 6: DUP-> ) | 7: ,
    // if '+', '-', than it must be digits after '+/-'
    // 8: digits
    // 9: + or - or ( or digits
    // 10: + or - or ) or ,
    unsigned short expect_status = 0 ; // begin status
    bool is_dup_proccess = false ;
    int left_bracket = 0 ;

    for (unsigned int i = 0; i < oneLine.line_token.size(); i++)
    {
        // skip normal instructions
        if (oneLine.line_token.at(0).table_num == 1)
            break ;

        if ((oneLine.line_token.at(i).name.compare("DB") == 0 ||
            oneLine.line_token.at(i).name.compare("DW") == 0 ||
            oneLine.line_token.at(i).name.compare("BYTE") == 0 ||
            oneLine.line_token.at(i).name.compare("WORD") == 0 ) &&
            (i != 0 && i != 1))
        {
            cout << "line " << lineNum << ": assign pseudo syntax error.\n" ;
            return false ;
        }

        if ((oneLine.line_token.at(i).name.compare("DB") == 0 ||
            oneLine.line_token.at(i).name.compare("DW") == 0 ||
            oneLine.line_token.at(i).name.compare("BYTE") == 0 ||
            oneLine.line_token.at(i).name.compare("WORD") == 0 ))
        {
            // at(0) must ba a symbol name
            if (i == 1 && oneLine.line_token.at(0).table_num != 5)
            {
                cout << "line " << lineNum << ": assign pseudo syntax error, expect symbol or empty.\n" ;
                return false ;
            }
            else
            {
                expect_status = 1 ;

                if (i+1 < oneLine.line_token.size())
                {
                    if (oneLine.line_token.at(i+1).name.compare("-") == 0)
                    {
                        expect_status = 8 ;
                        i++ ;
                    }

                    if (oneLine.line_token.at(i+1).name.compare("(") == 0)
                    {
                        left_bracket++ ;
                        expect_status = 9 ;
                        i++ ;
                    }
                }

                continue ;
            }
        }

        if (expect_status == 1) // expect to get digits or a '(string prefix)
        {
            if (oneLine.line_token.at(i).table_num == 6) // case:digits
            {
                if (is_dup_proccess)
                    expect_status = 6 ; // next get )
                else
                    expect_status = 4 ; // next get ,

                continue ;
            }
            else if (oneLine.line_token.at(i).name.compare("\'") == 0)// case:str
            {
                expect_status = 2 ;
                continue ;
            }
            else
            {
                cout << "line " << lineNum << ": invalid assign pseudo.\n" ;
                return false ;
            }
        } // end status=1

        if (expect_status == 2) // expect to get string
        {
            if (oneLine.line_token.at(i).table_num != 7)
            {
                cout << "errorname: " << oneLine.line_token.at(i).table_num << endl ;
                cout << "line " << lineNum << ": invalid assign pseudo.\n" ;
                return false ;
            }
            else
            {
                expect_status = 3 ;
                continue ;
            }
        } // end status=2

        if (expect_status == 3) // expect to get a '(string postfix)
        {
            if (oneLine.line_token.at(i).name.compare("\'") != 0)
            {
                cout << "line " << lineNum << ": invalid assign pseudo.\n" ;
                return false ;
            }
            else
            {
                if (is_dup_proccess)
                    expect_status = 6 ; // next get )
                else
                    expect_status = 4 ; // next get ,

                continue ;
            }
        } // end status=3

        if (expect_status == 4) // expect to get a , or a DUP
        {
            if (oneLine.line_token.at(i).name.compare(",") == 0)
            {
                expect_status = 1 ;
                continue ;
            }
            else if (oneLine.line_token.at(i).name.compare("DUP") == 0)
            {
                // before DUP, there is a string
                if (oneLine.line_token.at(i-1).name.compare("\'") == 0)
                {
                    cout << "line " << lineNum << ": DUP needs a digits before declaration it.\n" ;
                    return false ;
                }

                expect_status = 5 ;
                is_dup_proccess = true ;
                continue ;
            }
            else if (oneLine.line_token.at(i).name.compare(")") == 0)
            {
                left_bracket-- ;
                expect_status = 10 ;
                continue ;
            }
            else
            {
                cout << "line " << lineNum << ": invalid assign pseudo(expect complete syntax).\n" ;
                return false ;
            }

        } // end status=4

        if (expect_status == 5) // status=4 is a DUP, expect to get a (
        {
            if (oneLine.line_token.at(i).name.compare("(") != 0)
            {
                cout << "line " << lineNum << ": invalid assign pseudo(DUP).\n" ;
                return false ;
            }
            else
            {
                expect_status = 1 ;
                continue ;
            }
        } // end status=5

        if (expect_status == 6) // expect to get a ) to end DUP
        {
            if (oneLine.line_token.at(i).name.compare(")") != 0)
            {
                cout << "line " << lineNum << ": invalid assign pseudo(DUP).\n" ;
                return false ;
            }
            else
            {
                is_dup_proccess = false ; // reinitialize process status
                expect_status = 7 ; // dup test success, it's next should be a ,
                continue ;
            }
        } // end status=6

        if (expect_status == 7) // expect to get a ) to end DUP
        {
            if (oneLine.line_token.at(i).name.compare(",") != 0)
            {
                cout << "line " << lineNum << ": invalid assign pseudo.\n" ;
                return false ;
            }
            else
            {
                expect_status = 1 ;
                continue ;
            }
        } // end status=7

        if (expect_status == 8) // expect to digits
        {
            if (oneLine.line_token.at(i).table_num == 6) // case:digits
            {
                if (is_dup_proccess)
                    expect_status = 6 ; // next get )
                else
                    expect_status = 4 ; // next get ,

                continue ;
            }
            else
            {
                cout << "line " << lineNum << ": invalid assign pseudo.\n" ;
                return false ;
            }
        } // end status=8

        if (expect_status == 9) // expect a + or - or ( or digits
        {
            if (oneLine.line_token.at(i).table_num == 6) // case:digits
            {
                if (is_dup_proccess)
                    expect_status = 6 ; // next get )
                else
                    expect_status = 4 ; // next get ,

                continue ;
            }
            else if (oneLine.line_token.at(i).name.compare("+") == 0)
            {
                expect_status = 8 ;
                continue ;
            }
            else if (oneLine.line_token.at(i).name.compare("-") == 0)
            {
                expect_status = 8 ;
                continue ;
            }
            else if (oneLine.line_token.at(i).name.compare("(") == 0)
            {
                expect_status = 9 ;
                continue ;
            }
            else
            {
                cout << "line " << lineNum << ": invalid assign pseudo.\n" ;
                return false ;
            }
        } // end status=9

         // 10: + or - or ) or ,
        if (expect_status == 10) // expect a + or - or ) or ,
        {
            if (oneLine.line_token.at(i).name.compare("+") == 0)
            {
                expect_status = 8 ;
                continue ;
            }
            else if (oneLine.line_token.at(i).name.compare("-") == 0)
            {
                expect_status = 8 ;
                continue ;
            }
            else if (oneLine.line_token.at(i).name.compare(")") == 0)
            {
                expect_status = 10 ;
                continue ;
            }
            else if (oneLine.line_token.at(i).name.compare(",") == 0)
            {
                expect_status = 1 ;

                if (i+1 < oneLine.line_token.size())
                {
                    if (oneLine.line_token.at(i+1).name.compare("-") == 0)
                    {
                        expect_status = 8 ;
                        i++ ;
                    }

                    if (oneLine.line_token.at(i+1).name.compare("(") == 0)
                    {
                        left_bracket++ ;
                        expect_status = 9 ;
                        i++ ;
                    }
                }

                continue ;
            }
            else
            {
                cout << "line " << lineNum << ": invalid assign pseudo.\n" ;
                return false ;
            }
        } // end status=10

    } // end for

    /* exit(0), if expect_status = 4
     * other instructions, if expect_status = 0
     * DUP exit, if expect = 7
    */
    if (expect_status != 4 && expect_status != 7 && expect_status != 10 &&expect_status != 0)
    {
        cout << "line " << lineNum << ": expect complete assign pseudo.\n" ;
        return false ;
    }

    if (left_bracket != 0)
    {
        cout << "line " << lineNum << ": it looks like less or more brackets here.\n" ;
        return false ;
    }

    return true ;
}

bool Assembler::org_test(unsigned int lineNum, Recorder::LineInfo &oneLine)
{
    for (unsigned int i = 0; i < oneLine.line_token.size(); i++)
    {
        if (oneLine.line_token.at(i).name.compare("ORG") == 0)
        {
            if (oneLine.line_token.size() != 2)
            {
                cout << "line " << lineNum << ": syntax error(ORG)\n" ;
                return false ;
            }

            if (i != 0)
            {
                cout << "line " << lineNum << ": it can not place anything before 'ORG'\n" ;
                return false ;
            }
            else if (i == 0 && oneLine.line_token.at(i+1).table_num != 6)
            {
                cout << "line " << lineNum << ": it can only be a positive digits after 'ORG'\n" ;
                return false ;
            }
            else
            {
                break ; // test success
            }
        }

    } // end for

    return true ;
}

bool Assembler::equ_test(unsigned int lineNum, Recorder::LineInfo &oneLine)
{
    bool equ_process = false ;
    unsigned int i = 0 ;

    for (i = 0 ; i < oneLine.line_token.size(); i++)
    {
        if (oneLine.line_token.at(i).name.compare("EQU") == 0)
        {
            if (oneLine.line_token.size() < 3)
            {
                cout << "line " << lineNum << ": syntax error(EQU)\n" ;
                return false ;
            }

            if (i == 0)
            {
                cout << "line " << lineNum << ": it must place a symbol name before 'EQU'\n" ;
                return false ;
            }
            else if (i == 1 && oneLine.line_token.at(i-1).table_num != 5)
            {
                cout << "line " << lineNum << ": it can only be a symbol name before 'EQU'\n" ;
                return false ;
            }
            else if (i == 1 && oneLine.line_token.at(i+1).name.compare("(") != 0
                            && oneLine.line_token.at(i+1).name.compare("+") != 0
                            && oneLine.line_token.at(i+1).name.compare("-") != 0
                            && oneLine.line_token.at(i+1).table_num != 5
                            && oneLine.line_token.at(i+1).table_num != 6
                            && oneLine.line_token.at(i+1).name.compare("\'") != 0)
            {
                cout << "line " << lineNum << ": it can only be a symbol, digits or a string after 'EQU'\n" ;
                return false ;
            }
            else if (i != 1)
            {
                cout << "line " << lineNum << ": syntax error(EQU)\n" ;
                return false ;
            }
            else
            {
                i++ ;

                if (oneLine.line_token.size() == i+1)
                    equ_process = false ;
                else
                    equ_process = true ;

                break ; // test success
            }
        }

    } // end for

    unsigned int s = i ;

    if (equ_process)
    {
        short left_bracket = 0 ;

        if (oneLine.line_token.at(i).table_num == 5 ||
            oneLine.line_token.at(i).table_num == 6 ||
            oneLine.line_token.at(i).name.compare("(") == 0 ||
            oneLine.line_token.at(i).name.compare("+") == 0 ||
            oneLine.line_token.at(i).name.compare("-") == 0)
        {
            for ( ;i < oneLine.line_token.size(); i++)
            {
                if (oneLine.line_token.at(i).table_num == 5 ||
                    oneLine.line_token.at(i).table_num == 6)
                {
                    if (i+1 < oneLine.line_token.size() &&
                        oneLine.line_token.at(i+1).name.compare("+") != 0 &&
                        oneLine.line_token.at(i+1).name.compare("-") != 0 &&
                        oneLine.line_token.at(i+1).name.compare(")") != 0)
                    {
                        cout << "line " << lineNum << ": syntax error(EQU),"
                         "it expect a delimiter, such as '+', '-', or ')'\n" ;
                        return false ;
                    }

                }

                if (oneLine.line_token.at(i).name.compare("(") == 0)
                {
                    if (i+1 < oneLine.line_token.size() &&
                        oneLine.line_token.at(i+1).name.compare("+") != 0 &&
                        oneLine.line_token.at(i+1).name.compare("-") != 0 &&
                        oneLine.line_token.at(i+1).name.compare("(") != 0 &&
                        oneLine.line_token.at(i+1).table_num != 5 &&
                        oneLine.line_token.at(i+1).table_num != 6)
                    {
                        cout << "line " << lineNum << ": syntax error(EQU),"
                         "it expect symbol or digits or a delimiter, such as '+', '-'\n" ;
                        return false ;
                    }

                    if (i == oneLine.line_token.size()-1)
                    {
                        cout << "line " << lineNum << ": syntax error(EQU),"
                         "it doesn't expect a ')'\n" ;
                        return false ;
                    }

                    left_bracket++ ;
                }

                if (oneLine.line_token.at(i).name.compare(")") == 0)
                {
                    if (i+1 < oneLine.line_token.size() &&
                        oneLine.line_token.at(i+1).name.compare("+") != 0 &&
                        oneLine.line_token.at(i+1).name.compare("-") != 0 &&
                        oneLine.line_token.at(i+1).name.compare(")") != 0)
                    {
                        cout << "line " << lineNum << ": syntax error(EQU),"
                         "it expect a delimiter, such as '+', '-', ')'\n" ;
                        return false ;
                    }

                    left_bracket-- ;
                }

                if (oneLine.line_token.at(i).name.compare("+") == 0 ||
                    oneLine.line_token.at(i).name.compare("-") == 0)
                {
                    if (i+1 < oneLine.line_token.size() &&
                        oneLine.line_token.at(i+1).table_num != 5 &&
                        oneLine.line_token.at(i+1).table_num != 6 &&
                        oneLine.line_token.at(i+1).name.compare("(") != 0)
                    {
                        cout << "line " << lineNum << ": syntax error(EQU),"
                         "it expect symbol or digits or a '('\n" ;
                        return false ;
                    }

                    if (i == oneLine.line_token.size()-1)
                    {
                        cout << "line " << lineNum << ": syntax error(EQU),"
                         "it doesn't expect a '+' or '-'\n" ;
                        return false ;
                    }
                }

            } // end inner for

            if (left_bracket != 0)
            {
                cout << "line " << lineNum << ": syntax error(EQU),"
                         "it looks like more or less bracket\n" ;
                return false ;
            }
        }

        // check string
        if (oneLine.line_token.at(s).name.compare("\'") == 0)
        {
            if (s+2 >= oneLine.line_token.size() ||
                s+1 >= oneLine.line_token.size())
            {
                cout << "line " << lineNum << ":1 syntax error(EQU)\n" ;
                return false ;
            }

            if (oneLine.line_token.at(s+1).table_num != 7)
            {
                cout << "line " << lineNum << ":2 syntax error(EQU)\n" ;
                return false ;
            }

            if (oneLine.line_token.at(s+2).name.compare("\'") != 0)
            {
                cout << "line " << lineNum << ":3 syntax error(EQU)\n" ;
                return false ;
            }

            if (s+3 > oneLine.line_token.size())
            {
                cout << "line " << lineNum << ":4 syntax error(EQU)\n" ;
                return false ;
            }
        }
    }

    return true ;
}

bool Assembler::ptr_test(unsigned int lineNum, Recorder::LineInfo &oneLine)
{

    for (unsigned int i = 0; i < oneLine.line_token.size(); i++)
    {
        if (oneLine.line_token.at(i).name.compare("PTR") == 0)
        {
            if (oneLine.line_token.size() < 3) // at least be a format such as '(b/w) ptr label'
            {
                cout << "line " << lineNum << ": syntax error(PTR)\n" ;
                return false ;
            }

            if (i == 0)
            {
                cout << "line " << lineNum << ": syntax error(PTR)\n" ;
                return false ;
            }

            if (oneLine.line_token.at(i-1).name.compare("BYTE") != 0 &&
                oneLine.line_token.at(i-1).name.compare("WORD") != 0)
            {
                cout << "line " << lineNum << ": unknown prefix(PTR)\n" ;
                return false ;
            }

            if (i+1 >= oneLine.line_token.size())
            {
                cout << "line " << lineNum << ": syntax error(PTR)\n" ;
                return false ;
            }

            if (oneLine.line_token.at(i+1).table_num != 5 &&
                oneLine.line_token.at(i+1).name.compare("CODE") != 0)
            {
                cout << "line " << lineNum << ": it must be a symbol name after 'PTR'\n" ;
                return false ;
            }

            i += 2 ;
            // check double ptr exist or not
            for ( ;i < oneLine.line_token.size(); i++)
            {
                if (oneLine.line_token.at(i).name.compare("PTR") == 0)
                {
                    cout << "line " << lineNum << ": syntax error(PTR)\n" ;
                    return false ;
                }
            }

            break ; // test success

        }

    } // end for

    return true ;
}

bool Assembler::offset_test(unsigned int lineNum, Recorder::LineInfo &oneLine)
{
    for (unsigned int i = 0; i < oneLine.line_token.size(); i++)
    {
        if (oneLine.line_token.at(i).name.compare("OFFSET") == 0)
        {
            if (oneLine.line_token.size() < 2)
            {
                cout << "line " << lineNum << ": syntax error(OFFSET)\n" ;
                return false ;
            }

            if (i+1 >= oneLine.line_token.size())
            {
                cout << "line " << lineNum << ": syntax error(OFFSET)\n" ;
                return false ;
            }

            if (oneLine.line_token.at(i+1).table_num != 5)
            {
                cout << "line " << lineNum << ": it must be a symbol name after 'OFFSET'\n" ;
                return false ;
            }

            if (i-1 >= 0)
            {
                // if it is placed something, should be a ,
                if (oneLine.line_token.at(i-1).name.compare(",") != 0)
                {
                    cout << "line " << lineNum << ": syntax error(OFFSET)\n" ;
                    return false ;
                }
            }

            break ; // test success
        }
    } // end for

    return true ;
}

bool Assembler::pseudoCheck(unsigned int lineNum, Recorder::LineInfo &oneLine)
{
    // if one error occur, set testSuccess = false
    bool testSuccess = true ;
    bool oneSuccess = false ;

    // check 'assume' position and cs is defined
    oneSuccess = assume_test(lineNum, oneLine) ;
    if (!oneSuccess)
        testSuccess = false ;

    oneSuccess = segment_proc_test(lineNum, oneLine) ;
    if (!oneSuccess)
        testSuccess = false ;

    oneSuccess = db_dw_byte_word_test(lineNum, oneLine) ;
    if (!oneSuccess)
        testSuccess = false ;

    oneSuccess = org_test(lineNum, oneLine) ;
    if (!oneSuccess)
        testSuccess = false ;

    oneSuccess = equ_test(lineNum, oneLine) ;
    if (!oneSuccess)
        testSuccess = false ;

    oneSuccess = ptr_test(lineNum, oneLine) ;
    if (!oneSuccess)
        testSuccess = false ;

    oneSuccess = offset_test(lineNum, oneLine) ;
    if (!oneSuccess)
        testSuccess = false ;

    return testSuccess ;
}

bool Assembler::syntaxAnalysis()
{
    Recorder &recorder = Recorder::getRecorder() ;
    bool success = true ;

    vector<Recorder::LineInfo> &line_info = recorder.line_info_list ;

    // check segment/proc/ syntax and code segment is really defined
    syntaxCheckBuffer.proc_num = 0 ;
    syntaxCheckBuffer.seg_num = 0 ;

    for (unsigned int i = 0; i < line_info.size(); ) // a instruction
    {
        // check this line is a comment or not
        // if yes, skip this line
        bool isComment = checkIsComment(line_info.at(i)) ;

        if (isComment)
        {
            i++ ;
        }
        else
        {
            bool tmp_success = pseudoCheck(i+1, line_info.at(i)) ;
            if (!tmp_success)
                success = false ;

            i++ ;
        }

    }

    // clear used memory
    syntaxCheckBuffer.code_seg_name.clear() ;
    syntaxCheckBuffer.proc_num = 0 ;
    syntaxCheckBuffer.seg_num = 0 ;

    return success ;
}

int Assembler::atoi_hex_to_dec(string number)
{
    unsigned int start_pt = 0 ;

    for (unsigned int i = 0 ; i < number.size() ; )
    {
        if (number[i] == '0')
            i++ ;
        else
        {
            start_pt = i ;
            break ;
        }
    }

    int ans = 0 ;
    for (start_pt = 0 ; start_pt < number.size() ; start_pt++)
    {
        char t = number[start_pt] ;
        if (t == 'h' || t == 'H')
            break ;

        if (t >= '0' && t <= '9')
            ans = ans * 16 + t - '0' ;
        else
            ans = ans * 16 + t - 'A' + 10 ;
    }

    return ans;
}

void Assembler::display_addr(unsigned int pc, string &addr)
{
    unsigned int quotient = pc, remainder = 0 ;

    while (quotient > 0)
    {
        remainder = quotient % 16 ;
        switch (remainder)
        {
        case 10:
            addr += 'A' ;
            break ;
        case 11:
            addr += 'B' ;
            break ;
        case 12:
            addr += 'C' ;
            break ;
        case 13:
            addr += 'D' ;
            break ;
        case 14:
            addr += 'E' ;
            break ;
        case 15:
            addr += 'F' ;
            break ;
        default:
            char digit = remainder + '0' ;
            addr += digit ;
            break ;
        } // end switch

        quotient /= 16 ;
    }

    while (addr.size() < 4)
        addr += '0' ;

    reverse(addr.begin(), addr.end()) ;

    if (addr.size() > 4) // for '-' prefix
        addr.assign(addr, addr.size()-4, 4) ;

}

bool Assembler::add_symbol_entry(unsigned int pc, string &name, string &using_seg_reg,
                                 bool dup_check)
{
    // check duplicate defined symbol
    if (dup_check)
    {
        for (unsigned int i = 0; i < symbol_table.size(); i++)
        {
            if (name.compare(symbol_table.at(i).name) == 0)
                return false ;
        }
    }

    SymbolEntry one_entry ;

    string addr ;
    display_addr(pc, addr) ; // int pc change to string addr

    one_entry.seg_scope = using_seg_reg ;
    one_entry.name = name ;
    one_entry.address = addr ; // (__hex)
    symbol_table.push_back(one_entry) ;

    return true ;
}

int Assembler::priority(char &op)
{
    switch(op)
    {
        case '+':
        case '-':
            return 1 ;
        case '*':
        case '/':
            return 2 ;
        default:
            return 0;
    }
}

void Assembler::infix_to_postfix(string &src, string &result)
{
    if (src[0] == '+' || src[0] == '-')
        src.insert(0, "0") ;

    stack<char> buf ;

    for (unsigned int i = 0; i < src.size(); i++)
    {
        switch(src[i])
        {
        case '(':
            buf.push(src[i]) ;
            break ;

        case '+':
        case '-':
        case '*':
        case '/':
            while (!buf.empty() && (priority(buf.top()) >= priority(src[i])))
            {
                result += buf.top() ;
                buf.pop() ;
            }

            buf.push(src[i]) ;
            break ;

        case ')':
            while (buf.top() != '(')
            {
                result += buf.top() ;
                buf.pop() ;
            }

            buf.pop() ;
            break ;

        default:
            result += src[i] ;
            break ;
        }
    }

    while (!buf.empty())
    {
        result += buf.top() ;
        buf.pop() ;
    }
}

string Assembler::calculate_postfix(string infix, string &postfix)
{
    vector<string> num_list ;
    string num ; // for buffering

    for (unsigned int i = 0; i < infix.size(); i++)
    {
        if (infix[i] != '+' && infix[i] != '-' && infix[i] != '*' && infix[i] != '/' &&
            infix[i] != '(' && infix[i] != ')' )
            num += infix[i] ;
        else
        {
            if (!num.empty())
            {
                num_list.push_back(num) ;
                num.clear() ;
            }
        }
    }

    if (!num.empty())
    {
        num_list.push_back(num) ;
        num.clear() ;
    }


    stack<string> buf ;
    unsigned int compare_st = 0;

    for (unsigned int i = 0; i < postfix.size(); i++)
    {
        if (postfix[i] != '+' && // is operand
            postfix[i] != '-' &&
            postfix[i] != '*' &&
            postfix[i] != '/' )
        {
            num += postfix[i] ;

            // compare num list

            if (num.compare(num_list[compare_st]) == 0)
            {
                buf.push(num) ;
                num.clear() ;
                compare_st++ ;
            }

        }
        else
        {
            int total = 0 ;
            int num_b = atoi((buf.top()).c_str()) ;
            buf.pop() ;
            int num_a = atoi((buf.top()).c_str()) ;
            buf.pop() ;

            if (postfix[i] == '+')
                total = num_a + num_b ;
            else if (postfix[i] == '-')
                total = num_a - num_b ;
            else if (postfix[i] == '*')
                total = num_a * num_b ;
            else
                total = num_a / num_b ;

            buf.push(to_string(total)) ;
        }

    }

    return buf.top() ;
}

bool Assembler::pseudo_filter(Recorder::LineInfo &oneLine, unsigned int line_num, string &cs_onto_seg_name,
                           string &ds_onto_seg_name, string &ss_onto_seg_name,
                           string &es_onto_seg_name, string &using_seg_name,
                           string &using_seg_reg, unsigned short &error_status)
{

    for (unsigned int i = 0 ; i < oneLine.line_token.size(); i++)
    {
        // def symbol
        if ((oneLine.line_token.at(i).table_num == 5 ||
            oneLine.line_token.at(i).name.compare("CODE") == 0) &&
            i == 0 &&
            oneLine.line_token.at(i+1).name.compare("ENDS") != 0 &&
            oneLine.line_token.at(i+1).name.compare("ENDP") != 0)
        {
            bool not_duplicate = false ;

            if (oneLine.line_token.at(i+1).name.compare("EQU") == 0)
            {
                if (oneLine.line_token.at(i+2).table_num != 5 ||
                    i+2 != oneLine.line_token.size() - 1)
                {
                    ;
                }
                else
                {
                    not_duplicate = add_symbol_entry(virtual_PC, oneLine.line_token.at(i).name, using_seg_reg,
                                             true) ;
                    if (!not_duplicate)
                    {
                        cout << "line " << line_num << ": duplicate defined symbol(" << oneLine.line_token.at(i).name << ")\n" ;
                        error_status = 1 ;
                        return true ;
                    }

                    if (i == oneLine.line_token.size()-1)
                        return true ;
                    else if (i == oneLine.line_token.size()-2)
                    {
                        if (oneLine.line_token.at(i+1).name.compare(":") == 0)
                            return true ;
                    }
                }

            }
            else
            {
                not_duplicate = add_symbol_entry(virtual_PC, oneLine.line_token.at(i).name, using_seg_reg,
                                             true) ;
                if (!not_duplicate)
                {
                    cout << "line " << line_num << ": duplicate defined symbol(" << oneLine.line_token.at(i).name << ")\n" ;
                    error_status = 1 ;
                    return true ;
                }

                if (i == oneLine.line_token.size()-1)
                    return true ;
                else if (i == oneLine.line_token.size()-2)
                {
                    if (oneLine.line_token.at(i+1).name.compare(":") == 0)
                        return true ;
                }
            }

        }

        // segment
        if (oneLine.line_token.at(i).name.compare("SEGMENT") == 0)
        {
            // record segment name
            using_seg_name = oneLine.line_token.at(i-1).name ;

            if (using_seg_name.compare(cs_onto_seg_name) == 0)
                using_seg_reg = "CS" ;
            else if (using_seg_name.compare(ds_onto_seg_name) == 0)
                using_seg_reg = "DS" ;
            else if (using_seg_name.compare(ss_onto_seg_name) == 0)
                using_seg_reg = "SS" ;
            else if (using_seg_name.compare(es_onto_seg_name) == 0)
                using_seg_reg = "ES" ;
            else
                using_seg_reg = "CS" ;

            // reset seg symbol from default CS
            symbol_table.at(symbol_table.size()-1).seg_scope = using_seg_reg ;

            return true ;
        }

        // org
        if (oneLine.line_token.at(i).name.compare("ORG") == 0)
        {
            string digits = oneLine.line_token.at(i+1).name ;
            Recorder::strToUpper(digits) ;
            if (digits.at(digits.length() - 1) == 'H') // hex
                virtual_PC = atoi_hex_to_dec(digits) ;
            else
                virtual_PC = atoi(digits.c_str()) ;

            string addr ;
            display_addr(virtual_PC, addr) ;

            return true ;
        }

        // assume
        if (oneLine.line_token.at(i).name.compare("ASSUME") == 0)
        {
            i++ ;
            while (i < oneLine.line_token.size())
            {
                if (oneLine.line_token.at(i).name.compare("CS") == 0)
                {
                    i += 2 ;
                    cs_onto_seg_name = oneLine.line_token.at(i).name ;
                }
                else if (oneLine.line_token.at(i).name.compare("DS") == 0)
                {
                    i += 2 ;
                    ds_onto_seg_name = oneLine.line_token.at(i).name ;
                }
                else if (oneLine.line_token.at(i).name.compare("SS") == 0)
                {
                    i += 2 ;
                    ss_onto_seg_name = oneLine.line_token.at(i).name ;
                }
                else if (oneLine.line_token.at(i).name.compare("ES") == 0)
                {
                    i += 2 ;
                    es_onto_seg_name = oneLine.line_token.at(i).name ;
                }
                else
                    ;

                i++ ;
            } // end while

            return true ;
        }

        // byte && db
        if (oneLine.line_token.at(i).name.compare("DB") == 0 ||
            oneLine.line_token.at(i).name.compare("BYTE") == 0)
        {
            unsigned int keep_length = 0 ; // will adding for pc
            i++ ;

            while (i < oneLine.line_token.size())
            {
                if (oneLine.line_token.at(i).name.compare("DUP") == 0)
                {
                    // str to digits(u_int)
                    unsigned int digits = 0 ;

                    string digits_str = oneLine.line_token.at(i-1).name ;
                    Recorder::strToUpper(digits_str) ;

                    if (digits_str.at(digits_str.size()-1) == 'H') // hex
                        digits = atoi_hex_to_dec(digits_str) ;
                    else // dec
                        digits = atoi(digits_str.c_str()) ;

                    keep_length = keep_length + (1 * digits) ;
                }

                if (oneLine.line_token.at(i).table_num == 6) // digits
                {
                    if (i+1 < oneLine.line_token.size())
                    {
                        if (oneLine.line_token.at(i+1).name.compare("DUP") != 0)
                        {
                            keep_length += 1 ;
                        }
                    }
                    else if (i == oneLine.line_token.size()-1)
                        keep_length += 1 ;
                }

                if (oneLine.line_token.at(i).name.compare("\'") == 0) // string
                {
                    i++ ;

                    for (unsigned int str_idx = 0 ; str_idx < oneLine.line_token.at(i).name.size(); str_idx++)
                    {
                        keep_length += 1 ;
                    }

                    i++ ; // postfix '
                }

                i++ ;
            } // end while

            virtual_PC += keep_length ;
            return true ;
        } // end db && byte

        // dw && word
        if (oneLine.line_token.at(i).name.compare("DW") == 0 ||
            oneLine.line_token.at(i).name.compare("WORD") == 0)
        {
            unsigned int keep_length = 0 ; // will adding for pc
            i++ ;

            while (i < oneLine.line_token.size())
            {
                if (oneLine.line_token.at(i).name.compare("DUP") == 0)
                {
                    // str to digits(u_int)
                    unsigned int digits = 0 ;

                    string digits_str = oneLine.line_token.at(i-1).name ;
                    Recorder::strToUpper(digits_str) ;

                    if (digits_str.at(digits_str.size()-1) == 'H') // hex
                        digits = atoi_hex_to_dec(digits_str) ;
                    else // dec
                        digits = atoi(digits_str.c_str()) ;

                    keep_length = keep_length + (2 * digits) ;
                }

                if (oneLine.line_token.at(i).table_num == 6) // digits
                {
                    if (i+1 < oneLine.line_token.size())
                    {
                        if (oneLine.line_token.at(i+1).name.compare("DUP") != 0)
                        {
                            keep_length += 2 ;
                        }
                    }
                    else if (i == oneLine.line_token.size()-1)
                        keep_length += 2 ;
                }

                if (oneLine.line_token.at(i).name.compare("\'") == 0) // string
                {
                    i++ ;

                    for (unsigned int str_idx = 0 ; str_idx < oneLine.line_token.at(i).name.size(); str_idx++)
                    {
                        keep_length += 2 ;
                    }

                    i++ ; // postfix '
                }

                i++ ;
            } // end while

            virtual_PC += keep_length ;
            return true ;
        } // end dw && word

        // equ
        if (oneLine.line_token.at(i).name.compare("EQU") == 0)
        {
            string expression ;

            for (unsigned int j = i+1; j < oneLine.line_token.size(); j++)
            {
                if (oneLine.line_token.at(j).table_num == 6) // is digits
                {
                    string target_digits = oneLine.line_token.at(j).name ;
                    if (target_digits.at(target_digits.size()-1) == 'H' ||
                        target_digits.at(target_digits.size()-1) == 'h') // hex
                    {
                        unsigned int dec_num = atoi_hex_to_dec(oneLine.line_token.at(j).name) ;
                        string tmp = to_string(dec_num) ;
                        expression += tmp ;
                    }
                    else // dec
                    {
                        expression += oneLine.line_token.at(j).name ;
                    }

                }
                else if (oneLine.line_token.at(j).name.compare("\'") == 0) // is string
                {
                    if (!expression.empty()) // error: using str as operand
                    {
                        cout << "line " << line_num << ": using str as a operand for calculate\n" ;
                        error_status = 3 ;
                        return true ;
                    }

                    // same as db string
                    bool not_duplicate = false ;
                    j++ ; // idx: str
                    not_duplicate = add_symbol_entry(virtual_PC, oneLine.line_token.at(j).name, using_seg_reg,
                                                     true) ;

                    if (!not_duplicate)
                    {
                        cout << "line " << line_num << ": duplicate defined symbol\n" ;
                        error_status = 1 ;
                        return true ;
                    }

                    unsigned int keep_length = 0 ;

                    for (unsigned int str_idx = 0 ; str_idx < oneLine.line_token.at(i).name.size(); str_idx++)
                    {
                        keep_length += 1 ;
                    }

                    virtual_PC += keep_length ;
                    j ++ ;

                    return true ; // end string case
                }
                else if (i+1 == oneLine.line_token.size() - 1 &&
                         oneLine.line_token.at(j).table_num == 5) // is a symbol and only it exist
                {
                    string src_symbol = oneLine.line_token.at(j).name ;
                    string target_symbol = oneLine.line_token.at(j-2).name ; // find addr of this symbol
                    unsigned int addr_to_pc = 0 ;

                    for (unsigned int search_idx = 0; search_idx < symbol_table.size(); search_idx++)
                    {
                        if (symbol_table.at(search_idx).name.compare(target_symbol) == 0)
                        {
                            addr_to_pc = (unsigned int)atoi_hex_to_dec(symbol_table.at(search_idx).address) ;
                            add_symbol_entry(addr_to_pc, src_symbol, symbol_table.at(search_idx).seg_scope, false) ;

                            break ;
                        }
                    } // end for

                    return true ;
                }
                else if (oneLine.line_token.at(j).table_num == 5) // is a express which including symbol
                {
                    string src_symbol = oneLine.line_token.at(j).name ;

                    // find this symbol from symbol table
                    bool found = false ;

                    for (unsigned int search_idx = 0; search_idx < symbol_table.size(); search_idx++)
                    {
                        // if exist: get addr of the symbol, and change it from __hex addr string to __dec string
                        // add result to expression
                        if (symbol_table.at(search_idx).name.compare(src_symbol) == 0)
                        {
                            unsigned int addr_to_pc = (unsigned int)atoi_hex_to_dec(symbol_table.at(search_idx).address) ;
                            string dec_addr = to_string(addr_to_pc) ;
                            expression += dec_addr ;
                            found = true ;
                            break ;
                        }
                    } // end for

                    if (!found) // not found: error status = 2. after all commands are checked, check all again
                    {
                        error_status = 2 ;
                        return true ;
                    }
                }
                else if (oneLine.line_token.at(j).name.compare("+") == 0 ||
                         oneLine.line_token.at(j).name.compare("-") == 0 ||
                         oneLine.line_token.at(j).name.compare("(") == 0 ||
                         oneLine.line_token.at(j).name.compare(")") == 0)  // + or - or ( or )
                {
                    expression += oneLine.line_token.at(j).name ;
                }
                else
                    ;

            } // end search (for)

            // calculate expression
            if (!expression.empty())
            {
                cout << "\nexpression: " << expression << endl ;

                string res ;
                infix_to_postfix(expression, res) ;
                cout << "postfix: " << res << endl ;
                res = calculate_postfix(expression, res) ; // get __dec addr

                // change from __dec to __hex
                string addr ;
                display_addr(atoi(res.c_str()), addr) ;
                cout << "addr: " << addr << endl ;

                // add result to symbol table
                SymbolEntry one_entry ;

                one_entry.seg_scope = using_seg_reg ;
                one_entry.name = oneLine.line_token.at(i-1).name ;
                one_entry.address = addr ; // (__hex)
                symbol_table.push_back(one_entry) ;
            }

        }

    } // end for

    return false ;
}

bool instr_type_1_filter(Recorder::LineInfo &oneLine)
{
    return false ;
}

bool Assembler::pass1()
{
    Recorder &recorder = Recorder::getRecorder() ;
    vector<Recorder::LineInfo> &line_info_list = recorder.line_info_list ;

    virtual_PC = 0 ;

    // according to ASSUME pseudo,
    // it decides what name is used to connect to segment reg
    string cs_onto_seg_name = "empty",
           ds_onto_seg_name = "empty",
           ss_onto_seg_name = "empty",
           es_onto_seg_name = "empty" ;

    string using_seg_name ;
    string using_seg_reg = "CS" ; // scope: default = CS

    for (unsigned int line_num = 0; line_num < line_info_list.size(); line_num++)
    {
        bool all_exec = false ;
        unsigned short error_status = 0 ; // not error

        string addr ;
        display_addr(virtual_PC, addr) ;
        cout << "addr= " << addr << "\t" ;

        // if it is segment declaration, set segment configuration
        all_exec = pseudo_filter(line_info_list.at(line_num), line_num, cs_onto_seg_name, ds_onto_seg_name,
                                 ss_onto_seg_name, es_onto_seg_name, using_seg_name, using_seg_reg,
                                 error_status) ;

        cout << line_info_list.at(line_num).origin_insr << endl ;

        if (error_status == 1) // symbol duplicate defined
            break ;

        if (error_status == 2) // equ symbol not found or not defined

        if (error_status == 3) // using str as operand
            break ;

        if (error_status == 4) // redefine symbol(EQU)
            break ;

        if (all_exec) continue ;

    }

    // test
    for (unsigned int i = 0; i < symbol_table.size(); i++)
    {
        cout << symbol_table.at(i).seg_scope << ", " << symbol_table.at(i).name << ", " << symbol_table.at(i).address << endl ;
    }

}

void Assembler::addTokenFrmBuf(bool DelimiterFlag, bool DigitFlag, bool StringFlag)
{
    Recorder::Token token_tmp ;

    if (undef_token_buf.empty())
        return ;

    token_tmp.name = undef_token_buf ;
    token_tmp.table_num = 0 ;
    token_tmp.item_num = 0 ;
    token_tmp.isDelimiter = DelimiterFlag ;
    token_tmp.isDigits = DigitFlag ;
    token_tmp.isString = StringFlag ;

    Recorder &recorder = Recorder::getRecorder() ;
    recorder.token_list.push_back(token_tmp) ;
}

void Assembler::addTokenDelimiter(char delimiter)
{
    Recorder::Token token_tmp ; // the delimiter itself is also a token

    token_tmp.name = delimiter ;
    token_tmp.table_num = 4 ;
    token_tmp.item_num = 0 ;
    token_tmp.isDelimiter = true ;
    token_tmp.isDigits = false ;
    token_tmp.isString = false ;

    Recorder &recorder = Recorder::getRecorder() ;
    recorder.token_list.push_back(token_tmp) ;
}

void Assembler::lexicalAnalysis()
{
    Recorder &recorder = Recorder::getRecorder() ;
    recorder.lineStartPt.push_back(0) ;
    vector<string>& code_segment = recorder.code_segment ;

    bool errorF = false ;

    for (unsigned int line_idx = 0; line_idx < code_segment.size(); line_idx++)
    {
        string code_line = code_segment[line_idx] ;

        if (!code_line.empty() || Recorder::isAllWhite(code_line))
        {
            cutOneLineToTokenBuf(code_line) ;
            setIndexForTokenRecords(line_idx) ;
        }

        errorF = recorder.exportOneLineRes(line_idx) ;

        if (errorF) // error occurred, stop process
        {
            cout << "error occurred, stop process!" << endl ;
            return ;
        }

    } // end all instruction

    if (!errorF)
        cout << "all lines wrote out successfully." << endl ;

    recorder.freeLexicalUsedMem() ;
    undef_token_buf.clear() ;
}

void Assembler::cutOneLineToTokenBuf(string& code_line)
{
    Recorder &recorder = Recorder::getRecorder() ;
    bool isAllcomment = false ;

    if (code_line[0] == ';') // this line is all comment
    {
        addTokenDelimiter(code_line[0]) ;

        string comment = code_line ;
        comment.erase(0, 1) ;
        recorder.comment_list.push_back(comment) ;

        isAllcomment = true ; // skip this instruction line
    }

    bool string_process = false ;

    for (unsigned int idx = 0; idx < code_line.size() && !isAllcomment; idx++)
    {

        if ((idx == (code_line.size() - 1)) && // this command line approach end
            (code_line.at(idx) != ';' ) &&
            (!isDelimiter(code_line.at(idx))) )
        {
            if (!undef_token_buf.empty())
            {
                undef_token_buf += code_line.at(idx) ;

                bool digitsFlag = isDigits(undef_token_buf) ;
                addTokenFrmBuf(false, digitsFlag, false) ;
                undef_token_buf.clear() ;
            }
            else // is a only one digit or symbol
            {
                // maybe a one digit decimal or a character symbol
                string lastChar ;
                lastChar += code_line.at(idx) ;

                if (isDigits(lastChar))
                {
                    undef_token_buf += code_line.at(idx) ;
                    addTokenFrmBuf(false, true, false) ;
                }
                else // is symbol
                {
                    undef_token_buf += code_line.at(idx) ;
                    addTokenFrmBuf(false, false, false) ;
                }

                undef_token_buf.clear() ;
            }
        }
        else if (!string_process && (code_line.at(idx) == ' ' || code_line.at(idx) == '\t')) // white space
        {
            if (!undef_token_buf.empty())
            {
                bool digitsFlag = isDigits(undef_token_buf) ;
                addTokenFrmBuf(false, digitsFlag, false) ;
                undef_token_buf.clear() ;
            }
        }
        else if(isDelimiter(code_line.at(idx)))
        {

            if (!string_process && !undef_token_buf.empty())
            {
                if (undef_token_buf.size() == 1 && isDelimiter(undef_token_buf.at(0)))
                {
                    addTokenDelimiter(undef_token_buf.at(0)) ;
                }
                else
                {
                    bool digitsFlag = isDigits(undef_token_buf) ;
                    addTokenFrmBuf(false, digitsFlag, false) ;
                }

                undef_token_buf.clear() ;
            }

            if (code_line.at(idx) == '\'')
            {
                if (string_process)
                {
                    addTokenFrmBuf(false, false, true) ;
                    addTokenDelimiter('\'') ;
                    undef_token_buf.clear() ;
                    string_process = false ;
                }
                else
                {
                    addTokenDelimiter(code_line.at(idx)) ;
                    string_process = true ;
                }

            }
            else if (!string_process)
                addTokenDelimiter(code_line.at(idx)) ;
            else
                undef_token_buf += code_line.at(idx) ;

        }
        else if (!string_process && code_line.at(idx) == ';')
        {
            if (!undef_token_buf.empty())
            {
                bool digitsFlag = isDigits(undef_token_buf) ;
                addTokenFrmBuf(false, digitsFlag, false) ;
                undef_token_buf.clear() ;
            }

            addTokenDelimiter(code_line.at(idx)) ;

            // after this delimiter, it is comment line
            string comment = code_line ;
            comment.erase(0, idx + 1) ;
            recorder.comment_list.push_back(comment) ;

            break ; // for skip comment
        }
        else
        {
            undef_token_buf += code_line.at(idx) ;
        }

    } // end one instruction
}

void Assembler::setIndexForTokenRecords(unsigned int line_idx)
{
    Recorder &recorder = Recorder::getRecorder() ;

    for (unsigned int startPt = recorder.lineStartPt.at(line_idx); startPt < recorder.token_list.size(); startPt++)
    {
        Recorder::TableIndex pak ;
        string search_token_name = recorder.token_list[startPt].name ;

        if (recorder.token_list[startPt].isDigits) // is digits
        {
            pak = recorder.hashInDigitsTable(search_token_name) ;
        }
        else if (recorder.token_list[startPt].isString) // is string
        {
            pak = recorder.hashInStrTable(search_token_name) ;
        }
        else // is element of table 1~4 or symbol
        {
            pak = recorder.searchStaticTable(search_token_name, recorder.token_list[startPt].isDelimiter) ;

            // could not find in table1~4, and not a string or digits. means it's symbol
            if (!(pak.table_idx >= 1  && pak.table_idx <= 4))
            {
                pak = recorder.hashInSymbolTable(search_token_name) ;
            }
        }

        recorder.token_list[startPt].table_num = pak.table_idx ;
        recorder.token_list[startPt].item_num = pak.item_idx ;


    }
}
