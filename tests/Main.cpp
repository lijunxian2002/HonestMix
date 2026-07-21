/**
 * @file  Main.cpp
 * @brief Unit-test runner for HonestMix.
 *
 * Build & run:
 *   cmake --build build --target HonestMix_Tests
 *   ./build/Release/HonestMix_Tests.exe
 */

#include <juce_core/juce_core.h>
#include <iostream>

int main (int, char*[])
{
    juce::UnitTestRunner runner;
    runner.setPassesAreLogged (true);
    runner.runAllTests();

    int passed = 0, failed = 0;
    for (int i = 0; i < runner.getNumResults(); ++i)
    {
        auto* result = runner.getResult (i);
        if (result == nullptr)
            continue;
        if (result->failures > 0)
            ++failed;
        else
            ++passed;

        std::cout << "  " << result->unitTestName
                  << " / " << result->subcategoryName
                  << " — " << result->passes << " passed, "
                  << result->failures << " failed\n";
    }

    std::cout << "\n=== " << passed << " suites passed, "
              << failed << " suites failed ===\n";

    return failed > 0 ? 1 : 0;
}
