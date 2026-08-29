// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <random>
#include <map>
#include <thread>
#include <vector>

void ExperimentWithNoise(uint8_t noise_pct, size_t n_rounds, size_t n_experiments);

#pragma comment (lib, "StaticLib1.lib")

int main(int argc, char* argv[])
{
    srand(42);

    size_t n_experiments = 10;// 100;
    size_t n_rounds = 1e5;
    size_t start_noise_pct = 0;
    size_t end_noise_pct = 101;// start_noise_id + 1;
    
    for (size_t noise_pct = start_noise_pct; noise_pct < end_noise_pct; noise_pct++)
    {
        try
        {
            ExperimentWithNoise(noise_pct, n_rounds, n_experiments);
        }
        catch (...)
        {

        }
    }

	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
