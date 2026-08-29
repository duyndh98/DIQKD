// StaticLib1.cpp : Defines the functions for the static library.
//

#include "pch.h"
#include "framework.h"

#include "DIQKD.hpp"
#include "ErrorCorrection.h"

#include "algorithm.h"
#include "key.h"
#include "mock_classical_session.h"
#include "reconciliation.h"

using namespace Cascade;

#include <vector>
#include <list>
#include <fstream>
#include <string>
#include <cstdlib>
#include <format>

using namespace DIQKD_ns;

void ExperimentWithNoise(uint8_t noise_pct, size_t n_rounds, size_t n_experiments)
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

	std::string root_dir = "../../data_defense";

	std::ifstream sifting_file;
	int result_count_ = 0;
	sifting_file.open(root_dir + "/sifting/" + std::format("{:03}", noise_pct) + ".csv", std::ios::in);
	std::string line_;
	while (std::getline(sifting_file, line_))
	{
		if (line_.length() > 0)
			result_count_++;

		auto delim_pos = line_.find_first_of(',');
		auto alice_key = line_.substr(0, delim_pos);
		auto bob_key = line_.substr(delim_pos + 1);

		size_t n_bits = alice_key.length();
		Key correct_key(n_bits);
		Key noisy_key(n_bits);

		for (size_t i = 0; i < n_bits; ++i)
		{
			correct_key.set_bit(i, alice_key[i] - '0');
			noisy_key.set_bit(i, bob_key[i] - '0');
		}
		
		auto err_origin = correct_key.compute_bit_error_count(noisy_key);
		auto Q = (float)err_origin / n_bits;
		
		const Algorithm* algorithm = Algorithm::get_by_name("original");
		MockClassicalSession classical_session(correct_key, algorithm->cache_shuffles);
		Reconciliation reconciliation(*algorithm, classical_session, noisy_key, Q);
		reconciliation.reconcile();
		Key& reconciled_key = reconciliation.get_reconciled_key();

		auto recon_err_count = correct_key.compute_bit_error_count(reconciled_key);

		std::ofstream correction_file;
		correction_file.open(root_dir + "/correction/" + std::format("{:03}", noise_pct) + ".csv", std::ios::app);
		correction_file << n_bits;

		for (auto& iter_key : reconciliation.get_iter_keys())
		{
			auto iter_err_count = correct_key.compute_bit_error_count(iter_key);
			correction_file << "," << iter_err_count;
			continue;
		}

		correction_file << std::endl;
		correction_file.close();

		continue;
	}
	sifting_file.close();

	return;

	std::stringstream file_name;
	file_name << root_dir + "/readout/";
	file_name << std::format("{:03}", noise_pct);
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

	//std::ofstream result_file;
	//result_file.open(file_name.str(), std::ios::app);

	for (int experiment_id = result_count; experiment_id < n_experiments; experiment_id++)
	{
		bool success = false;

		do
		{
			try
			{
				DIQKD protocol(n_rounds, noise_pct, root_dir);
				protocol.Work();

				/*auto result*/ 
				/*auto CHSH = std::get<0>(result);
				auto QBER = std::get<1>(result);*/

				/*result_file << noise_100_id << "," << CHSH << "," << QBER;

				result_file << "," << protocol.GetInputBinary(PEER_TYPE_ALICE);
				result_file << "," << protocol.GetInputBinary(PEER_TYPE_BOB);
				result_file << "," << protocol.GetOutputBinary(PEER_TYPE_ALICE);
				result_file << "," << protocol.GetOutputBinary(PEER_TYPE_BOB);

				result_file << std::endl;*/

				success = true;
			}
			catch (...)
			{
			}

		} while (!success);

		continue;
	}

	//result_file.close();

	return;
}
