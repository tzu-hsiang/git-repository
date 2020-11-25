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
    // 1: digits or ' | 2: string | 3: ' | 4: , or DUP | 5: DUP-> ( | 6: DUP-> ) | 7: ,
    unsigned short expect_status = 0 ; // begin status
    bool is_dup_proccess = false ;

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

    } // end for

    /* exit(0), if expect_status = 4
     * other instructions, if expect_status = 0
     * DUP exit, if expect = 7
    */
    if (expect_status != 4 && expect_status != 7 && expect_status != 0)
    {
        cout << "line " << lineNum << ": expect complete assign pseudo.\n" ;
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
                cout << "line " << lineNum << ": it can only be a digits after 'ORG'\n" ;
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

void Assembler::pass1()
{
    Recorder &recorder = Recorder::getRecorder() ;
    vector<Recorder::LineInfo> &line_info_list = recorder.line_info_list ;


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
