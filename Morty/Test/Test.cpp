#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest/doctest.h"


int main(int argc, char** argv) {
    doctest::Context context;

    context.applyCommandLine(argc, argv);
    context.setOption("abort-after", 5);              // stop test execution after 5 failed assertions
    context.setOption("order-by", "name");            // sort the test cases by their name
    context.setOption("no-breaks", true);             // don't break in the debugger when assertions fail

    const int res = context.run();

    if (context.shouldExit())
    {
        return res;
    }

    //other...
    
    return res;
}