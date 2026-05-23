// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <random>
#include <map>
#include <thread>
#include <vector>

void fnStaticLib1(size_t noise_100_id, size_t n_experiments);

#pragma comment (lib, "StaticLib1.lib")

int main(int argc, char* argv[])
{
    size_t start_noise_id = 0;
    size_t end_noise_id = 101;// start_noise_id + 1;
    
    /*std::vector<std::thread> threads;
    threads.reserve(end_noise_id - start_noise_id);*/

    for (size_t id = start_noise_id; id < end_noise_id; id++)
    {
		//std::cout << "Noise id: " << id << std::endl;

        /*threads.emplace_back([](size_t noise_100_id)
            {*/
        try
        {
            fnStaticLib1(id, 50);
        }
        catch (...)
        {

        }
        //}, id);
    }

    /*for (auto& thread : threads)
    {
        thread.join();
    }*/

    std::this_thread::sleep_for(std::chrono::hours(1));

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
