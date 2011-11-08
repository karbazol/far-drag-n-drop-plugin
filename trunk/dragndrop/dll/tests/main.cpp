#include <tut/tut.hpp>
#include <tut/tut_reporter.hpp>

int main(int argc, const char** argv)
{
    tut::reporter reporter;

    tut::runner.get().set_callback(&reporter);

    tut::runner.get().run_tests();

    return 0;
}

// vim: set et ts=4 sw=4 ai :

