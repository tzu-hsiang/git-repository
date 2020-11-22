#include "Recorder.h"
#include "Assembler.h"

#define INIT_SIZE 25

using namespace std ;

void proc_exec() ;
void cleanException(string&) ;

int main()
{
    cout << "Input 1 to execute process, any to quit:" ;
    string comment ;
    cin >> comment ;

    while (!comment.compare("1"))
    {
        proc_exec() ;

        cout << "\nInput 1 to execute process, any to quit:" ;
        cin >> comment ;
        cleanException(comment) ; // handle illegal input
    }

    cout << "Quit." << endl ;
    return 0 ;
}

void proc_exec()
{
    Recorder &recorder = Recorder::getRecorder() ;
    recorder.init(INIT_SIZE) ;

    bool errorFlag = false ;
    errorFlag = recorder.importTables() ;

    if (errorFlag)
        return ;

    errorFlag = recorder.importInstrSequence() ;

    if (errorFlag)
        return ;

    Assembler &assembler = Assembler::getAssembler() ;
    assembler.lexicalAnalysis() ;

    // test assign to line_info_list (success)
    for (unsigned int x = 0; x < recorder.line_info_list.size(); x++)
    {
        cout << recorder.line_info_list.at(x).origin_insr << "\n" ;

        for (unsigned int y = 0; y < recorder.line_info_list.at(x).line_token.size() &&
                            recorder.line_info_list.at(x).line_token.at(y).name.compare("null") != 0; y++)
            cout << "(" << recorder.line_info_list.at(x).line_token.at(y).table_num << ","
                 << recorder.line_info_list.at(x).line_token.at(y).item_num << ")" ;

        cout << endl ;
    }
    // end test

    // to write >> syntaxAnalysis() ;
    bool success = assembler.syntaxAnalysis() ;

    if (success) // do pass1 pass2
    {
        cout << "test success!\n" ;
    }
    else
    {
        cout << "test not success!\n" ;
    }
        // end
    /* roughly
    pass1() ;
    pass2() ;
    */

    recorder.clearTempStorage() ;
    assembler.clearTempStorage() ;
}

void cleanException(string& comment)
{
    if (cin.fail())
    {
        comment.clear() ;
        cin.clear() ;
        cin.sync() ;
    }
}
