#include <tut/tut.hpp>
#include <tut/tut_reporter.hpp>
#include "../src/configure.hpp"

namespace tut
{
    struct configure_data
    {
    };

    typedef test_group<configure_data> factory;
    typedef factory::object object;

    factory test_group("configure");

    template<>
    template<>
    void object::test<1>()
    {
        Config* config = Config::instance();
        ensure(!!config);
        ensure(config->checkKey() == 0);
        ensure(config->shellCopy() == true);
        ensure(config->showMenu() == false);
        ensure(config->useShellObject() == false);
    }

    template<>
    template<>
    void object::test<2>()
    {
        Config* config = Config::instance();
        ensure(!!config);

        config->checkKey(1);
        ensure(config->checkKey() == 1);

        config->shellCopy(false);
        ensure(!config->shellCopy());

        config->showMenu(false);
        ensure(!config->showMenu());

        config->useShellObject(false);
        ensure(!config->useShellObject());

        config->checkKey(0);
        ensure(config->checkKey() == 0);

        config->shellCopy(true);
        ensure(config->shellCopy());

        config->showMenu(true);
        ensure(config->showMenu());

        config->useShellObject(true);
        ensure(config->useShellObject());
    }
    test_runner_singleton runner;
}

int main(int argc, const char** argv)
{
    tut::reporter reporter;

    tut::runner.get().set_callback(&reporter);

    tut::runner.get().run_tests();

    return 0;
}

// vim: set et ts=4 sw=4 ai :

