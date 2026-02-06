// StaticLib1.cpp : Defines the functions for the static library.
//

#include "pch.h"
#include "framework.h"

#include "DIQKD.hpp"
#include "ErrorCorrection.h"

#include <vector>
#include <list>
#include <fstream>
#include <string>
#include <cstdlib>

using namespace DIQKD_ns;

// TODO: This is an example of a library function
void fnStaticLib1(size_t noise_100_id, size_t n_experiments)
{
#if 0
	static constexpr float P[BOB_INPUT_COUNT][ALICE_INPUT_COUNT] =
	{
		{ 413.0 / 448, 220.0 / 425, 311.0 / 389, 361.0 / 434 },
		{ 210.0 / 408, 380.0 / 412, 77.0 / 403, 359.0 / 423 },
	};

	auto E_21 = (1.0 - P[1][2]) - P[1][2];
	auto E_20 = (1.0 - P[0][2]) - P[0][2];
	auto E_30 = (1.0 - P[0][3]) - P[0][3];
	auto E_31 = (1.0 - P[1][3]) - P[1][3];

	/*
		E_21	0.61786600947380066		double
		E_20	-0.59897172451019287	double
		E_30	-0.66359448432922363	double
		E_31	-0.69739949703216553	double
	*/

	auto S = E_21 - E_20 - E_30 - E_31;
	// 2.5778317153453827
#endif

	constexpr size_t N_ROUND = 10000;

	std::stringstream file_name;
	file_name << "test_result_";
	file_name << noise_100_id;
	file_name << ".csv";

	std::ifstream csv_file;
	int result_count = 0;
	csv_file.open(file_name.str(), std::ios::in);
	std::string line;
	while (std::getline(csv_file, line))
	{
		if (line.length() > 0)
			result_count++;
	}
	csv_file.close();

	std::ofstream result_file;
	result_file.open(file_name.str(), std::ios::app);

	for (int experiment_id = result_count; experiment_id < n_experiments; experiment_id++)
	{
		bool success = false;

		do
		{
			try
			{
				DIQKD protocol(N_ROUND, noise_100_id / 100.0);
				auto CHSH = protocol.Work();

				result_file << noise_100_id << "," << CHSH;

				result_file << "," << protocol.GetInputBinary(PEER_TYPE_ALICE);
				result_file << "," << protocol.GetOutputBinary(PEER_TYPE_ALICE);
				result_file << "," << protocol.GetInputBinary(PEER_TYPE_BOB);
				result_file << "," << protocol.GetOutputBinary(PEER_TYPE_BOB);

				result_file << std::endl;

				success = true;
			}
			catch (...)
			{
			}

		} while (!success);

		continue;
	}

	result_file.close();

	return;
}
