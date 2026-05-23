#include "pch.h"
#include <thread>

#include "DIQKD.hpp"
#include "Atom.hpp"

DIQKD_ns::DIQKD::DIQKD(size_t n_rounds_, float ionization_noise_) : _n_rounds(n_rounds_), _public_channel(n_rounds_), _alice(_public_channel, n_rounds_, ionization_noise_), _bob(_public_channel, n_rounds_, ionization_noise_)
{
	_ionization_noise = ionization_noise_;

	static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
	plog::init(plog::debug, &consoleAppender);
	return;
}

void DIQKD_ns::DIQKD::ErrorCorrection()
{
	for (size_t round_id = 0; round_id < _n_rounds; round_id++)
	{
		if (_alice._outputs[round_id] == STATE_SUPERPOSITION || _bob._outputs[round_id] == STATE_SUPERPOSITION)
			continue;

		/*auto X = _alice_inputs[round_id];
		auto Y = _bob_inputs[round_id];
		auto A = _alice_outputs[round_id];
		auto B = _bob_outputs[round_id];

		correlation_frequencies[X][Y][A == B ? 0 : 1]++;*/

		continue;
	}

	return;
}

std::tuple<float, float> DIQKD_ns::DIQKD::Work()
{
	std::thread alice_work(&DIQKD::AliceWork, this);
	std::thread bob_work(&DIQKD::BobWork, this);

	for (size_t round_id = 0; round_id < _n_rounds; round_id++)
	{
		//PLOG_INFO << "Round #" << round_id + 1;
		
		_public_channel.UpdateRoundType(ROUND_TYPE_UNKNOWN);

		_public_channel.WaitPeerStatus(PEER_TYPE_ALICE, PEER_STATUS_IDLING);
		_public_channel.WaitPeerStatus(PEER_TYPE_BOB, PEER_STATUS_IDLING);

		Entanglement entanglement(BELL_STATE_PSI_PLUS, _alice.GetAtom(), _bob.GetAtom());

		//_public_channel.SendSignal(ROUND_STATUS_HERALD);
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

	//PLOG_INFO << "N rounds: " << _n_rounds;

	auto CHSH = _public_channel.ComputeCHSH();

	//auto noise_id = (int)(this->_ionization_noise * 100);
	//PLOG_INFO << noise_id << " | " << "CHSH = " << CHSH;

	auto QBER = _public_channel.ComputeQBER(_alice._inputs, _bob._inputs, _alice._outputs, _bob._outputs);

	//size_t N_eq_0 = 0;
	//size_t N_0 = 0;
	//size_t N_eq_1 = 0;
	//size_t N_1 = 0;

	//for (size_t round_id = 0; round_id < _alice._inputs.size(); round_id++)
	//{
	//	if (_alice._inputs[round_id] != _bob._inputs[round_id])
	//		continue;

	//	bool output_matched = _alice._outputs[round_id] == _bob._outputs[round_id];

	//	if (_alice._inputs[round_id] == 0)
	//	{
	//		N_0++;

	//		if (output_matched)
	//			N_eq_0++;
	//	}
	//	else if (_alice._inputs[round_id] == 1)
	//	{
	//		N_1++;

	//		if (output_matched)
	//			N_eq_1++;
	//	}

	//	continue;
	//}

	//auto Q_0 = (float)N_eq_0 / N_0;
	//auto Q_1 = (float)N_eq_1 / N_1;
	//auto QBER = (Q_0 + Q_1) / 2;

	//PLOG_INFO << "Q_0 = " << Q_0;
	//PLOG_INFO << "Q_1 = " << Q_1;

	//system("pause");

	return { CHSH, QBER };
}

void DIQKD_ns::DIQKD::AliceWork()
{
	for (size_t round_id = 0; round_id < _n_rounds; round_id++)
	{
		//PLOG_INFO << "[ALICE] round #" << round_id;

		_alice.WaitReadySignalTransmission();

		_alice.RandomNumberGeneration();
		_alice.StateSelectiveIonization();
		_alice.FluorescenceReadout();
		
		auto round_type = (_alice._polarization > 1) ? ROUND_TYPE_TEST : ROUND_TYPE_KEY;
		_public_channel.UpdateRoundType(round_type);
		
		//PLOG_INFO << "[ALICE] PostProcessing";

		_alice.PostProcessing(round_type);
		_alice.Reset();

		continue;
	}

	//_alice.SiftKey();

	return;
}

void DIQKD_ns::DIQKD::BobWork()
{
	for (size_t round_id = 0; round_id < _n_rounds; round_id++)
	{
		//PLOG_INFO << "[BOB] round #" << round_id;

		_bob.WaitReadySignalTransmission();

		_bob.RandomNumberGeneration();
		_bob.StateSelectiveIonization();
		_bob.FluorescenceReadout();

		auto round_type = _public_channel.FetchRoundType();
		
		//PLOG_INFO << "[BOB] PostProcessing";

		_bob.PostProcessing(round_type);
		_bob.Reset();

		continue;
	}

	//_bob.SiftKey();

	return;
}

std::string DIQKD_ns::DIQKD::GetInputBinary(PEER_TYPE peer_type_)
{
	auto inputs = peer_type_ == PEER_TYPE_ALICE ? _alice._inputs : _bob._inputs;

	std::stringstream ss;
	for (auto input : inputs)
		ss << (int)input;

	return ss.str();
}

std::string DIQKD_ns::DIQKD::GetOutputBinary(PEER_TYPE peer_type_)
{
	auto outputs = peer_type_ == PEER_TYPE_ALICE ? _alice._outputs : _bob._outputs;

	std::stringstream ss;
	for (auto output : outputs)
		ss << (int)(output == STATE_UP ? 1 : 0);

	return ss.str();
}
