#include "pch.h"
#include <thread>
#include <fstream>
#include <format>
#include <filesystem>

#include "DIQKD.hpp"
#include "Atom.hpp"
#include "Helper.h"

#include "algorithm.h"
#include "key.h"
#include "mock_classical_session.h"
#include "reconciliation.h"

using namespace Cascade;
namespace fs = std::filesystem;

DIQKD_ns::DIQKD::DIQKD(size_t n_rounds_, uint8_t ionization_noise_pct_, const std::string& data_root_dir_) : _n_rounds(n_rounds_), _ionization_noise_pct(ionization_noise_pct_), _data_root_dir(data_root_dir_), _public_channel(n_rounds_), _alice(_public_channel, n_rounds_, ionization_noise_pct_ / 100.0), _bob(_public_channel, n_rounds_, ionization_noise_pct_ / 100.0)
{
	_readout_dir = data_root_dir_ + "/readout";
	if (!fs::exists(_readout_dir))
		fs::create_directories(_readout_dir);

	_sifting_dir = data_root_dir_ + "/sifting";
	if (!fs::exists(_sifting_dir))
		fs::create_directories(_sifting_dir);

	_correction_dir = data_root_dir_ + "/correction";
	if (!fs::exists(_correction_dir))
		fs::create_directories(_correction_dir);

	static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
	plog::init(plog::debug, &consoleAppender);
	return;
}

void DIQKD_ns::DIQKD::StoreMeasurement()
{
	auto CHSH = _public_channel.ComputeCHSH(_alice._inputs, _bob._inputs, _alice._outputs, _bob._outputs);
	auto QBER = _public_channel.ComputeQBER(_alice._inputs, _bob._inputs, _alice._outputs, _bob._outputs);

	std::ofstream readout_file;
	readout_file.open(_readout_dir + "/" + std::format("{:03}", _ionization_noise_pct) + ".csv", std::ios::app);
	readout_file << (size_t)_ionization_noise_pct << "," << CHSH << "," << QBER;
	readout_file << "," << Helper::GetNumericString(_alice._inputs);
	readout_file << "," << Helper::GetNumericString(_bob._inputs);
	readout_file << "," << Helper::GetNumericString(_alice._outputs);
	readout_file << "," << Helper::GetNumericString(_bob._outputs);
	readout_file << std::endl;
	readout_file.close();

	return;
}

void DIQKD_ns::DIQKD::KeySifting()
{
	size_t n_rounds = _alice._inputs.size();

	_alice._sifted_key.reserve(n_rounds);
	_bob._sifted_key.reserve(n_rounds);

	for (size_t i = 0; i < n_rounds; i++)
	{
		if (_alice._inputs[i] != _bob._inputs[i])
			continue;

		_alice._sifted_key.push_back(_alice._outputs[i] == STATE_UP ? 0 : 1);
		_bob._sifted_key.push_back(_bob._outputs[i] == STATE_UP ? 1 : 0);

		continue;
	}

	std::ofstream sifting_file;
	sifting_file.open(_sifting_dir + "/" + std::format("{:03}", _ionization_noise_pct) + ".csv", std::ios::app);
	sifting_file << Helper::GetNumericString(_alice._sifted_key) << "," << Helper::GetNumericString(_bob._sifted_key);
	sifting_file << std::endl;
	sifting_file.close();

	//auto QBERs = _public_channel.ComputeQBERs(_alice._sifted_key, _bob._sifted_key);

	return;
}

void DIQKD_ns::DIQKD::ErrorCorrection()
{
	size_t n_bits = _alice._sifted_key.size();
	Key correct_key(n_bits);
	Key noisy_key(n_bits);

	for (size_t i = 0; i < n_bits; ++i)
	{
		correct_key.set_bit(i, _alice._sifted_key[i]);
		noisy_key.set_bit(i, _bob._sifted_key[i]);
	}

	auto root_err_count = correct_key.compute_bit_error_count(noisy_key);
	auto root_error_rate = (float)root_err_count / n_bits;

	const Algorithm* algorithm = Algorithm::get_by_name("original");
	MockClassicalSession classical_session(correct_key, algorithm->cache_shuffles);
	Reconciliation reconciliation(*algorithm, classical_session, noisy_key, root_error_rate);
	reconciliation.reconcile();
	Key& reconciled_key = reconciliation.get_reconciled_key();

	auto recon_err_count = correct_key.compute_bit_error_count(reconciled_key);

	std::ofstream correction_file;
	correction_file.open(_correction_dir + "/" + std::format("{:03}", _ionization_noise_pct) + ".csv", std::ios::app);
	correction_file << n_bits;

	for (auto& iter_key : reconciliation.get_iter_keys())
	{
		auto iter_err_count = correct_key.compute_bit_error_count(iter_key);
		correction_file << "," << iter_err_count;
		continue;
	}

	correction_file << std::endl;
	correction_file.close();

	return;
}

void DIQKD_ns::DIQKD::Work()
{
	std::thread alice_work(&DIQKD::AliceWork, this);
	std::thread bob_work(&DIQKD::BobWork, this);

	for (size_t round_id = 0; round_id < _n_rounds; round_id++)
	{
		_public_channel.WaitPeerStatus(PEER_TYPE_ALICE, PEER_STATUS_IDLING);
		_public_channel.WaitPeerStatus(PEER_TYPE_BOB, PEER_STATUS_IDLING);

		Entanglement entanglement(BELL_STATE_PSI_PLUS, _alice.GetAtom(), _bob.GetAtom());

		_public_channel.UpdatePeerStatus(PEER_TYPE_ALICE, PEER_STATUS_HERALD);
		_public_channel.UpdatePeerStatus(PEER_TYPE_BOB, PEER_STATUS_HERALD);
		
		_public_channel.WaitPeerStatus(PEER_TYPE_ALICE, PEER_STATUS_READOUT);
		_public_channel.WaitPeerStatus(PEER_TYPE_BOB, PEER_STATUS_READOUT);

		_public_channel.UpdatePeerStatus(PEER_TYPE_ALICE, PEER_STATUS_CLEANING);
		_public_channel.UpdatePeerStatus(PEER_TYPE_BOB, PEER_STATUS_CLEANING);

		continue;
	}

	alice_work.join();
	bob_work.join();

	StoreMeasurement();

	KeySifting();

	ErrorCorrection();

	return /*{ CHSH, QBER }*/;
}

void DIQKD_ns::DIQKD::AliceWork()
{
	_alice.QuantumStage();
	//_alice.ClassicalStage();

	return;
}

void DIQKD_ns::DIQKD::BobWork()
{
	_bob.QuantumStage();
	//_bob.ClassicalStage();

	return;
}

//std::string DIQKD_ns::DIQKD::GetInputBinary(PEER_TYPE peer_type_)
//{
//	auto inputs = peer_type_ == PEER_TYPE_ALICE ? _alice._inputs : _bob._inputs;
//
//	std::stringstream ss;
//	for (auto input : inputs)
//		ss << (int)input;
//
//	return ss.str();
//}

//std::string DIQKD_ns::DIQKD::GetOutputBinary(PEER_TYPE peer_type_)
//{
//	auto outputs = peer_type_ == PEER_TYPE_ALICE ? _alice._outputs : _bob._outputs;
//
//	std::stringstream ss;
//	for (auto output : outputs)
//		ss << (int)(output == STATE_UP ? 1 : 0);
//
//	return ss.str();
//}
