#include "Recorder.h"

Recorder::Recorder()
{
    initialization = false ;
}

Recorder::~Recorder()
{
    clearTempStorage() ;
}

Recorder& Recorder::getRecorder()
{
    static Recorder recorder ;
    return recorder ;
}

#define HASH_TABLE_SIZE 100
#define INIT_SIZE 10

void Recorder::init(int init_size)
{
    initialization = true ;

    table_instr.reserve(init_size) ;
    table_pseudo.reserve(init_size) ;
    table_pseudo.reserve(init_size) ;
    table_delimiter.reserve(init_size) ;

    table_symbol.reserve(HASH_TABLE_SIZE) ;
    table_intAndReal.reserve(HASH_TABLE_SIZE) ;
    table_string.reserve(HASH_TABLE_SIZE) ;

    for (unsigned int i = 0; i < HASH_TABLE_SIZE; i++)
    {
        Table tmp ;
        string emptyStr ;
        tmp.item = emptyStr ;
        table_symbol.push_back(tmp) ;
        table_intAndReal.push_back(tmp) ;
        table_string.push_back(tmp) ;
    }

    token_list.reserve(INIT_SIZE) ;
    comment_list.reserve(INIT_SIZE) ;
    lineStartPt.reserve(INIT_SIZE) ;
    commentStartPt = 0 ;

    // save lines info. after lexical analysis
    line_info_list.reserve(init_size) ;
}

void Recorder::clearTempStorage()
{
    initialization = false ;

    imported_file_name.clear() ;
    vector<string>().swap(code_segment) ;

    vector<Table>().swap(table_instr) ;
    vector<Table>().swap(table_pseudo) ;
    vector<Table>().swap(table_register) ;
    vector<Table>().swap(table_delimiter) ;
    vector<Table>().swap(table_symbol) ;
    vector<Table>().swap(table_intAndReal) ;
    vector<Table>().swap(table_string) ;

    vector<Token>().swap(token_list) ;
    vector<string>().swap(comment_list) ;
    vector<unsigned int>().swap(lineStartPt) ;
    commentStartPt = 0 ;

    vector<LineInfo>().swap(line_info_list) ;
}

void Recorder::strToUpper(string& src)
{
    for (unsigned int i = 0; i < src.size(); i++)
    {
        char lower = src[i] ;

        if ((lower >= 'a') && (lower <= 'z'))
            src[i] -= 32 ;
    }
}

void Recorder::strToLower(string& src)
{
    for (unsigned int i = 0; i < src.size(); i++)
    {
        char lower = src[i] ;

        if ((lower >= 'A') && (lower <= 'Z'))
            src[i] += 32 ;
    }
}

bool Recorder::isAllWhite(string& src)
{
    if (src[0] != ' ' && src[0] != '\t')
        return false ;

    bool allWhite = false ;

    for (unsigned int i = 0; i < src.size(); i++)
    {
        char ch = src[i] ;

        if (ch == ' ' || ch == '\t')
            allWhite = true ;
        else
        {
            allWhite = false ;
            break ;
        }
    }

    return allWhite ;
}

bool Recorder::importTables()
{
    if (!initialization)
    {
        cout << "Recorder::& need execute initialization first!\n" ;
        return true ;
    }

    fstream inFile ;
    inFile.open("Table1.table", ios::in) ; // import table 1 (instruction)

    string tmp_element ;

    if (!inFile.is_open())
    {
        cout << "Can not find the Table1.table, or some unknown error occurred." ;
        return true ;
    }

    while (getline(inFile, tmp_element))
    {
        if (tmp_element.empty() || isAllWhite(tmp_element))
            break ;
        Table tmp ;
        tmp.item = tmp_element ;
        table_instr.push_back(tmp) ;
    }

    inFile.close() ;

    inFile.open("Table2.table", ios::in) ; // import table 2 (pseudo code)
    if (!inFile.is_open())
    {
        cout << "Can not find the Table2.table, or some unknown error occurred." ;
        return true ;
    }

    while (getline(inFile, tmp_element))
    {
        if (tmp_element.empty() || isAllWhite(tmp_element))
            break ;
        Table tmp ;
        tmp.item = tmp_element ;
        table_pseudo.push_back(tmp) ;
    }

    inFile.close() ;

    inFile.open("Table3.table", ios::in) ; // import table 3 (register)
    if (!inFile.is_open())
    {
        cout << "Can not find the Table3.table, or some unknown error occurred." ;
        return true ;
    }

    while (getline(inFile, tmp_element))
    {
        if (tmp_element.empty() || isAllWhite(tmp_element))
            break ;
        Table tmp ;
        tmp.item = tmp_element ;
        table_register.push_back(tmp) ;
    }

    inFile.close() ;

    inFile.open("Table4.table", ios::in) ; // import table 4 (delimiter)
    if (!inFile.is_open())
    {
        cout << "Can not find the Table4.table, or some unknown error occurred." ;
        return true ;
    }

    while (getline(inFile, tmp_element))
    {
        if (tmp_element.empty() || isAllWhite(tmp_element))
            break ;
        Table tmp ;
        tmp.item = tmp_element ;
        table_delimiter.push_back(tmp) ;
    }

    inFile.close() ;

    return false ; // the process succeed in loading to memory
}

bool Recorder::importInstrSequence()
{
    if (!initialization)
    {
        cout << "Recorder::& need execute initialization first!\n" ;
        return true ;
    }

    string filename ;
    cout << "Please input a file name:" ;
    cin >> filename ;

    if (cin.fail())
    {
        cout << "illegal input, please input a new one.\n" ;
        cin.clear() ;
        cin.sync() ;

        return -1 ;
    }

    ifstream inFile(filename.c_str()) ;

    if (!inFile.is_open())
    {
        cout << "Can not find the file you inputted, or some unknown error occurred.\n" ;
        return true ;
    }

    for (unsigned int i = filename.size() - 1; i >= 0; i--) // truncate filename extension
    {
        if (filename[i] == '.')
        {
            filename.erase(i, filename.size() - i) ;
            break ;
        }
    }

    imported_file_name = filename ;
    string tmp_instr ;

    while (getline(inFile, tmp_instr))
    {
        code_segment.push_back(tmp_instr) ;
    }

    inFile.close() ;

    return false ;
}

unsigned short Recorder::hashFunc(string& src)
{
    unsigned short total = 0 ;

    for (unsigned int i = 0; i < src.size(); i++)
        total += (unsigned short) src[i] ;

    return total % HASH_TABLE_SIZE ;
}

Recorder::TableIndex Recorder::hashInStrTable(string& tokenName)
{
    TableIndex pak ;

    if (!initialization)
    {
        cout << "Recorder::& need execute initialization first!\n" ;
        pak.item_idx = 7 ;
        pak.table_idx = 999 ;
        return pak ;
    }

    unsigned short pos = hashFunc(tokenName) ;

    if ((table_string.at(pos).item).empty() ||
        !(table_string.at(pos).item).compare(tokenName) )
    {
        table_string.at(pos).item = tokenName ;
    }
    else
    {
        int exec_times = 0 ;
        while (!(table_string.at(pos).item).empty() &&
               (table_string.at(pos).item).compare(tokenName) != 0)
        {
            pos++ ;

            if (pos == 100) // out of range
                pos = 0 ;

            if (exec_times == 100) // all position is not empty, stop subroutine
            {
                cout << "hashInStrTable throw \"all position is not empty, stop subroutine!\"\n" ;
                pos = 999 ;
                break ;
            }

            exec_times++ ;
        }
    }

    pak.item_idx = pos ;
    pak.table_idx = 7 ;
    return pak ;
}

Recorder::TableIndex Recorder::hashInSymbolTable(string& tokenName)
{
    TableIndex pak ;

    if (!initialization)
    {
        cout << "Recorder::& need execute initialization first!\n" ;
        pak.item_idx = 5 ;
        pak.table_idx = 999 ;
        return pak ;
    }

    unsigned short pos = hashFunc(tokenName) ;

    if ((table_symbol.at(pos).item).empty() ||
        !(table_symbol.at(pos).item).compare(tokenName))
    {
        table_symbol.at(pos).item = tokenName ;
    }
    else
    {
        int exec_times = 0 ;
        while (!(table_symbol.at(pos).item).empty() &&
               (table_symbol.at(pos).item).compare(tokenName) != 0)
        {
            pos++ ;

            if (pos == 100) // out of range
                pos = 0 ;

            if (exec_times == 100) // all position is not empty, stop subroutine
            {
                cout << "hashInSymbolTable throw \"all position is not empty, stop subroutine!\"\n" ;
                pos = 999 ;
                break ;
            }

            exec_times++ ;
        }
    }

    pak.item_idx = pos ;
    pak.table_idx = 5 ;
    return pak ;
}

Recorder::TableIndex Recorder::hashInDigitsTable(string& tokenName)
{
    TableIndex pak ;

    if (!initialization)
    {
        cout << "Recorder::& need execute initialization first!\n" ;
        pak.item_idx = 6 ;
        pak.table_idx = 999 ;
        return pak ;
    }

    unsigned short pos =  hashFunc(tokenName) ;

    if ((table_intAndReal.at(pos).item).empty() ||
        !(table_intAndReal.at(pos).item).compare(tokenName))
    {
        table_intAndReal.at(pos).item = tokenName ;
    }
    else
    {
        int exec_times = 0 ;
        while (!(table_intAndReal.at(pos).item).empty() &&
               (table_intAndReal.at(pos).item).compare(tokenName) != 0)
        {
            pos++ ;

            if (pos == 100) // out of range
                pos = 0 ;

            if (exec_times == 100) // all position is not empty, stop subroutine
            {
                cout << "hashInDigitsTable throw \"all position is not empty, stop subroutine!\"\n" ;
                pos = 999 ;
                break ;
            }

            exec_times++ ;
        }
    }

    pak.item_idx = pos ;
    pak.table_idx = 6 ;
    return pak ;
}

Recorder::TableIndex Recorder::searchStaticTable(string& tokenName, bool& delimiterF)
{
    bool found = false ;
    TableIndex pak ;

    if (!initialization)
    {
        cout << "Recorder::& need execute initialization first!\n" ;
        pak.item_idx = 0 ;
        pak.table_idx = 999 ;
        return pak ;
    }

    if (delimiterF) // search: table4 (delimiter)
    {
        pak.table_idx = 4 ;
        // search
        for (unsigned int i = 0; i < table_delimiter.size(); i++)
        {
            if (!(table_delimiter.at(i).item).compare(tokenName))
            {
                found = true ;
                pak.item_idx = i + 1 ;
                break ;
            }
        }
    }
    else
    {
        // for handle exception that token has upper letters
        // and also has lower letters, or all Upper/lower form.
        // if it upper form or lower form is a element of table 1~3,
        // it will be treated as the same thing.
        string tokenName_uniform = tokenName ;
        Recorder::strToUpper(tokenName_uniform) ;

        // search: table1 (instruction)
        for (unsigned int i = 0; i < table_instr.size(); i++)
        {
            if (!(table_instr.at(i).item).compare(tokenName_uniform))
            {
                found = true ;
                pak.table_idx = 1 ;
                pak.item_idx = i + 1 ;
                break ;
            }
        }

        // search: table2 (pseudo)
        for (unsigned int i = 0; i < table_pseudo.size(); i++)
        {
            if (!(table_pseudo.at(i).item).compare(tokenName_uniform))
            {
                found = true ;
                pak.table_idx = 2 ;
                pak.item_idx = i + 1 ;
                break ;
            }
        }

        // search: table3 (register)
        for (unsigned int i = 0; i < table_register.size(); i++)
        {
            if (!(table_register.at(i).item).compare(tokenName_uniform) )
            {
                found = true ;
                pak.table_idx = 3 ;
                pak.item_idx = i + 1 ;
                break ;
            }
        }
    }

    if (!found) // is symbol
    {
        pak.table_idx = 0 ;
        pak.item_idx = 0 ;
    }

    return pak ;
}

bool Recorder::exportOneLineRes(unsigned int code_line_idx)
{
    if (!initialization)
    {
        cout << "Recorder::& need execute initialization first!\n" ;
        return true ;
    }

    ofstream outFile ;
    outFile.open((imported_file_name + " (Token_output).txt").c_str(), ios::out | ios::app) ;

    if (!outFile.is_open())
    {
        cout << "can not export record, or some unknown error occurred.\n" ;
        return true ;
    }

    if (code_line_idx >= code_segment.size())
    {
        cout << "error: exportOneLineRes throw \"idx_out_of_range\"\n" ;
        return true ;
    }

    if (code_line_idx == 0) // first write out to this file after assembler every initiation
    {
        cout << "Start with writing (clear this file records):\n" ;
        outFile.close() ;
        outFile.open((imported_file_name + " (Token_output).txt").c_str(), ios::out | ios::trunc) ;
        outFile.close() ;
        outFile.open((imported_file_name + " (Token_output).txt").c_str(), ios::out | ios::app) ;
    }

    cout << "writing line " << code_line_idx + 1 << "..\n" ;
    outFile << code_segment[code_line_idx] << "\n" ;
    Recorder &recorder = Recorder::getRecorder() ;

    // TODO: assign result to line_info_list for translate to machine code
    LineInfo oneline ;
    oneline.origin_insr = code_segment.at(code_line_idx) ;
    // end

    if (!code_segment[code_line_idx].empty() && !isAllWhite(code_segment[code_line_idx]))
    {
        for (unsigned int startPt = recorder.lineStartPt.at(code_line_idx); startPt < recorder.token_list.size(); startPt++)
        {
            outFile << "(" << recorder.token_list[startPt].table_num << ","
                    << recorder.token_list[startPt].item_num << ")" ;

            // TODO: assign result to line_info_list for translate to machine code
            oneline.line_token.push_back(recorder.token_list[startPt]) ;
            // end

            if (!(recorder.token_list[startPt].name).compare(";"))
            {
                outFile << recorder.comment_list[recorder.commentStartPt] ;
                recorder.commentStartPt++ ;
            }
        }

        // TODO: assign result to line_info_list for translate to machine code
        recorder.line_info_list.push_back(oneline) ;
        // end

        outFile << endl ;
    }
    else
    {
        // TODO: assign result to line_info_list for translate to machine code
        Token empty_sign ;
        empty_sign.table_num = 999 ;
        empty_sign.item_num = 999 ;
        empty_sign.name = "null" ;
        empty_sign.isDelimiter = false ;
        empty_sign.isDigits = false ;
        empty_sign.isString = false ;

        oneline.line_token.push_back(empty_sign) ;
        recorder.line_info_list.push_back(oneline) ;
        // end
    }

    recorder.lineStartPt.push_back(recorder.token_list.size()) ;

    return false ;
}

void Recorder::freeLexicalUsedMem()
{
    vector<string>().swap(code_segment) ;

    vector<Token>().swap(token_list) ;
    vector<string>().swap(comment_list) ;
    vector<unsigned int>().swap(lineStartPt) ;
    commentStartPt = 0 ;
}
