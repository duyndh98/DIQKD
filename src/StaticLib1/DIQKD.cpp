#include "pch.h"
#include <thread>

#include "DIQKD.hpp"
#include "Atom.hpp"

void DIQKD_ns::DIQKD::Work()
{
	std::thread alice_work(&DIQKD::AliceWork, this);
	std::thread bob_work(&DIQKD::BobWork, this);

	for (size_t round_id = 0; round_id < _n_rounds; round_id++)
	{
		//_public_channel.WaitSignal(ROUND_STATUS_IDLING);

		PLOG_INFO << "Round #" << round_id + 1;
		
		_public_channel.UpdateRoundType(ROUND_TYPE_UNKNOWN);
		Entanglement entanglement(BELL_STATE_PSI_PLUS, _alice.GetAtom(), _bob.GetAtom());

		//_public_channel.SendSignal(ROUND_STATUS_HERALD);
		_public_channel.UpdatePeerStatus(PEER_TYPE_ALICE, PEER_STATUS_RUNNING);
		_public_channel.UpdatePeerStatus(PEER_TYPE_BOB, PEER_STATUS_RUNNING);
		
		_public_channel.WaitPeerStatus(PEER_TYPE_ALICE, PEER_STATUS_IDLING);
		_public_channel.WaitPeerStatus(PEER_TYPE_BOB, PEER_STATUS_IDLING);

		continue;
	}

	alice_work.join();
	bob_work.join();

	PLOG_INFO << "N rounds: " << _n_rounds;

	auto CHSH = _public_channel.ComputeCHSH();
	PLOG_INFO << "CHSH = " << CHSH;

	size_t N_eq_0 = 0;
	size_t N_0 = 0;
	size_t N_eq_1 = 0;
	size_t N_1 = 0;

	for (size_t round_id = 0; round_id < _alice._inputs.size(); round_id++)
	{
		if (_alice._inputs[round_id] != _bob._inputs[round_id])
			continue;

		bool output_matched = _alice._outputs[round_id] == _bob._outputs[round_id];

		if (_alice._inputs[round_id] == 0)
		{
			N_0++;

			if (output_matched)
				N_eq_0++;
		}
		else if (_alice._inputs[round_id] == 1)
		{
			N_1++;

			if (output_matched)
				N_eq_1++;
		}

		continue;
	}

	auto Q_0 = (float)N_eq_0 / N_0;
	auto Q_1 = (float)N_eq_1 / N_1;

	PLOG_INFO << "Q_0 = " << Q_0;
	PLOG_INFO << "Q_1 = " << Q_1;

	system("pause");

	return;
}

void DIQKD_ns::DIQKD::AliceWork()
{
	for (size_t round_id = 0; round_id < _n_rounds; round_id++)
	{
		_alice.Reset();
		_alice.WaitReadySignalTransmission();

		_alice.RandomNumberGeneration();
		_alice.StateSelectiveIonization();
		_alice.FluorescenceReadout();
		
		auto round_type = (_alice._polarization > 1) ? ROUND_TYPE_TEST : ROUND_TYPE_KEY;
		_public_channel.UpdateRoundType(round_type);
		
		_alice.PostProcessing(round_type);
	}

	_alice.SiftKey();

	return;
}

void DIQKD_ns::DIQKD::BobWork()
{
	for (size_t round_id = 0; round_id < _n_rounds; round_id++)
	{
		_bob.Reset();
		_bob.WaitReadySignalTransmission();
		
		_bob.RandomNumberGeneration();
		_bob.StateSelectiveIonization();
		_bob.FluorescenceReadout();

		auto round_type = _public_channel.FetchRoundType();
		_bob.PostProcessing(round_type);
	}

	_bob.SiftKey();

	return;
}
