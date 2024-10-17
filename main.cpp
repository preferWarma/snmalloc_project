

#include "lyf.h"
#include "stopwatch.h"
#include "memCheck.h"

using namespace lyf;



int main(int agrc, char* argv[]) {
    auto_stopwatch sw(stopwatch::TimeType::ms);
    TRACE_OFF();
    MEM_ON();

    MEM_OFF();
    PrintTool::print_args("------------lyf--------------");
    return 0;
}
