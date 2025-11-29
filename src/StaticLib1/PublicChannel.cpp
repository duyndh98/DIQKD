#include "pch.h"
#include "PublicChannel.hpp"

void DIQKD_ns::PublicChannel::StorePeerData(PEER_TYPE peer_type_, POLARIZATION input_, STATE output_)
{
	if (peer_type_ == PEER_TYPE_ALICE)
	{
		PLOG_INFO << magic_enum::enum_name(peer_type_) << " X=" << input_ << " A=" <<
			(output_ == STATE_SUPERPOSITION ? "N/A" : magic_enum::enum_name(output_));

		_alice_inputs.push_back(input_);
		_alice_outputs.push_back(output_);
	}
	else if (peer_type_ == PEER_TYPE_BOB)
	{
		PLOG_INFO << magic_enum::enum_name(peer_type_) << " Y=" << input_ << " B=" <<
			(output_ == STATE_SUPERPOSITION ? "N/A" : magic_enum::enum_name(output_));

		_bob_inputs.push_back(input_);
		_bob_outputs.push_back(output_);
	}

	return;
}

void DIQKD_ns::PublicChannel::UpdateRoundType(ROUND_TYPE round_type_)
{
	_round_type.store(round_type_);
	_round_type.notify_all();

	return;
}

DIQKD_ns::ROUND_TYPE DIQKD_ns::PublicChannel::FetchRoundType()
{
	_round_type.wait(ROUND_TYPE_UNKNOWN);

	return _round_type.load();
}

//bool DIQKD_ns::PublicChannel::SendSignal(ROUND_STATUS round_status_)
//{
//	auto expected_round_status = (round_status_ == ROUND_STATUS_HERALD) ? ROUND_STATUS_IDLING : ROUND_STATUS_HERALD;
//	auto changed = _round_status.compare_exchange_strong(expected_round_status, round_status_);
//
//	_round_status.notify_all();
//
//	return changed;
//}
//
//void DIQKD_ns::PublicChannel::WaitSignal(ROUND_STATUS round_status_)
//{
//	_round_status.wait(GET_PREV(round_status_, MAX_ROUND_STATUS));
//	
//	return;
//}

void DIQKD_ns::PublicChannel::UpdatePeerStatus(PEER_TYPE peer_type_, PEER_STATUS peer_status_)
{
	if (peer_type_ == PEER_TYPE_ALICE)
	{
		_alice_status.store(peer_status_);
		_alice_status.notify_all();
	}
	else if (peer_type_ == PEER_TYPE_BOB)
	{
		_bob_status.store(peer_status_);
		_bob_status.notify_all();
	}

	return;
}

void DIQKD_ns::PublicChannel::WaitPeerStatus(PEER_TYPE peer_type_, PEER_STATUS peer_status_)
{
	auto expected_status = GET_PREV(peer_status_, MAX_PEER_STATUS);

	if (peer_type_ == PEER_TYPE_ALICE)
		_alice_status.wait(expected_status);
	else if (peer_type_ == PEER_TYPE_BOB)
		_bob_status.wait(expected_status);

	return;
}

std::vector<DIQKD_ns::POLARIZATION> DIQKD_ns::PublicChannel::GetAnotherPeerInputs(PEER_TYPE peer_type_)
{
	auto expected_status = GET_PREV(PEER_STATUS_IDLING, MAX_PEER_STATUS);

	if (peer_type_ == PEER_TYPE_ALICE)
	{
		_bob_status.wait(expected_status);
		return _bob_inputs;
	}
	else if (peer_type_ == PEER_TYPE_BOB)
	{
		_alice_status.wait(expected_status);
		return _alice_inputs;
	}

	return std::vector<POLARIZATION>();
}

float DIQKD_ns::PublicChannel::ComputeCHSH()
{
	auto n_rounds = _alice_inputs.size();

	size_t correlation_frequencies[ALICE_INPUT_COUNT][BOB_INPUT_COUNT][2];

	for (size_t i = 0; i < ALICE_INPUT_COUNT; i++)
		for (size_t j = 0; j < BOB_INPUT_COUNT; j++)
			correlation_frequencies[i][j][0] = correlation_frequencies[i][j][1] = 0;

	for (size_t round_id = 0; round_id < n_rounds; round_id++)
	{
		if (_alice_outputs[round_id] == STATE_SUPERPOSITION || _bob_outputs[round_id] == STATE_SUPERPOSITION)
			continue;

		auto X = _alice_inputs[round_id];
		auto Y = _bob_inputs[round_id];
		auto A = _alice_outputs[round_id];
		auto B = _bob_outputs[round_id];

		correlation_frequencies[X][Y][A == B ? 0 : 1]++;
		continue;
	}

	auto ComputeQuantumCorrelation_fn = [](size_t correlation_freq[2])
	{
		auto anti_prob = (float)correlation_freq[1] / (correlation_freq[1] + correlation_freq[0]);
		return (1.0 - anti_prob) - anti_prob;
	};

	auto E_21 = ComputeQuantumCorrelation_fn(correlation_frequencies[2][1]);
	auto E_20 = ComputeQuantumCorrelation_fn(correlation_frequencies[2][0]);
	auto E_30 = ComputeQuantumCorrelation_fn(correlation_frequencies[3][0]);
	auto E_31 = ComputeQuantumCorrelation_fn(correlation_frequencies[3][1]);

	float CHSH = E_21 - E_20 - E_30 - E_31;

	return CHSH;
}
