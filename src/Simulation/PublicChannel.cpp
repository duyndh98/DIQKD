#include "pch.h"
#include "PublicChannel.hpp"
using namespace DIQKD_ns;

//void DIQKD_ns::PublicChannel::StorePeerData(PEER_TYPE peer_type_, POLARIZATION input_, STATE output_)
//{
//	if (peer_type_ == PEER_TYPE_ALICE)
//	{
//		/*PLOG_INFO << magic_enum::enum_name(peer_type_) << " X=" << input_ << " A=" <<
//			(output_ == STATE_SUPERPOSITION ? "N/A" : magic_enum::enum_name(output_));*/
//
//		_alice_inputs.push_back(input_);
//		_alice_outputs.push_back(output_);
//	}
//	else if (peer_type_ == PEER_TYPE_BOB)
//	{
//		/*PLOG_INFO << magic_enum::enum_name(peer_type_) << " Y=" << input_ << " B=" <<
//			(output_ == STATE_SUPERPOSITION ? "N/A" : magic_enum::enum_name(output_));*/
//
//		_bob_inputs.push_back(input_);
//		_bob_outputs.push_back(output_);
//	}
//
//	return;
//}

//void DIQKD_ns::PublicChannel::UpdateRoundType(ROUND_TYPE round_type_)
//{
//	_round_type.store(round_type_);
//	_round_type.notify_all();
//
//	return;
//}
//
//DIQKD_ns::ROUND_TYPE DIQKD_ns::PublicChannel::FetchRoundType()
//{
//	if (_round_type.load() == ROUND_TYPE_UNKNOWN)
//		_round_type.wait(ROUND_TYPE_UNKNOWN);
//
//	return _round_type.load();
//}

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
	//PLOG_DEBUG << magic_enum::enum_name(peer_type_) << " " << magic_enum::enum_name(peer_status_);

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
	//PLOG_DEBUG << magic_enum::enum_name(peer_type_) << " " << magic_enum::enum_name(peer_status_);

	auto expected_status = GET_PREV(peer_status_, MAX_PEER_STATUS);

	if (peer_type_ == PEER_TYPE_ALICE)
	{		
		auto current_status = _alice_status.load();
		if (current_status != peer_status_)
			_alice_status.wait(current_status);
	}
	else if (peer_type_ == PEER_TYPE_BOB)
	{
		auto current_status = _bob_status.load();
		if (current_status != peer_status_)
			_bob_status.wait(current_status);
	}

	return;
}

void DIQKD_ns::PublicChannel::WaitAnotherPeerStatus(PEER_TYPE peer_type_, PEER_STATUS peer_status_)
{
	if (peer_type_ == PEER_TYPE_ALICE)
	{
		WaitPeerStatus(PEER_TYPE_BOB, peer_status_);
	}
	else if (peer_type_ == PEER_TYPE_BOB)
	{
		WaitPeerStatus(PEER_TYPE_ALICE, peer_status_);
	}
}

void DIQKD_ns::PublicChannel::PushPeerInputs(PEER_TYPE peer_type_, const std::vector<POLARIZATION>& polarizations_)
{
	if (peer_type_ == PEER_TYPE_ALICE)
	{
		_alice_inputs.assign(polarizations_.begin(), polarizations_.end());
	}
	else if (peer_type_ == PEER_TYPE_BOB)
	{
		_bob_inputs.assign(polarizations_.begin(), polarizations_.end());
	}

	return;
}

std::vector<DIQKD_ns::POLARIZATION> DIQKD_ns::PublicChannel::PullAnotherPeerInputs(PEER_TYPE peer_type_)
{
	if (peer_type_ == PEER_TYPE_ALICE)
	{
		return _bob_inputs;
	}
	else if (peer_type_ == PEER_TYPE_BOB)
	{
		return _alice_inputs;
	}

	return std::vector<POLARIZATION>();
}

void DIQKD_ns::PublicChannel::PushPeerOutputs(PEER_TYPE peer_type_, const std::vector<STATE>& states_)
{
	if (peer_type_ == PEER_TYPE_ALICE)
	{
		_alice_outputs.assign(states_.begin(), states_.end());
	}
	else if (peer_type_ == PEER_TYPE_BOB)
	{
		_bob_outputs.assign(states_.begin(), states_.end());
	}
}

std::vector<STATE> DIQKD_ns::PublicChannel::PullAnotherPeerOutputs(PEER_TYPE peer_type_)
{
	return std::vector<STATE>();
}

//void DIQKD_ns::PublicChannel::WaitAnotherPeerStatus(PEER_TYPE peer_type_, PEER_STATUS peer_status_)
//{
//	if (peer_type_ == PEER_TYPE_ALICE)
//	{
//		this->WaitPeerStatus(PEER_TYPE_BOB, peer_status_);
//	}
//	else if (peer_type_ == PEER_TYPE_BOB)
//	{
//		this->WaitPeerStatus(PEER_TYPE_ALICE, peer_status_);
//	}
//	
//	return;
//}

float DIQKD_ns::PublicChannel::ComputeCHSH(const std::vector<POLARIZATION>& X, const std::vector<POLARIZATION>& Y, const std::vector<STATE>& A, const std::vector<STATE>& B)
{
	auto n_rounds = X.size();

	size_t correlation_frequencies[ALICE_INPUT_COUNT][BOB_INPUT_COUNT][2];

	for (size_t i = 0; i < ALICE_INPUT_COUNT; i++)
		for (size_t j = 0; j < BOB_INPUT_COUNT; j++)
			correlation_frequencies[i][j][0] = correlation_frequencies[i][j][1] = 0;

	for (size_t round_id = 0; round_id < n_rounds; round_id++)
	{
		// output of key rounds are not used for CHSH computation
		/*if (_alice_outputs[round_id] == STATE_SUPERPOSITION || _bob_outputs[round_id] == STATE_SUPERPOSITION)
			continue;*/

		auto x = X[round_id];
		auto y = Y[round_id];
		auto a = A[round_id];
		auto b = B[round_id];

		if (x < BOB_INPUT_COUNT)
			continue;

		correlation_frequencies[x][y][a == b ? 0 : 1]++;
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

float DIQKD_ns::PublicChannel::ComputeQBER(const std::vector<POLARIZATION>& X, const std::vector<POLARIZATION>& Y, const std::vector<STATE>& A, const std::vector<STATE>& B)
{
	auto n_rounds = X.size();

	size_t N_0 = 0;
	size_t E_0 = 0;
	size_t N_1 = 0;
	size_t E_1 = 0;

	for (size_t round_id = 0; round_id < n_rounds; round_id++)
	{
		auto x = X[round_id];
		auto y = Y[round_id];
		
		if (x != y || x >= BOB_INPUT_COUNT)
			continue;

		auto a = A[round_id];
		auto b = B[round_id];
		bool error = a == b;

		if (X[round_id] == POLARIZATION_0)
		{
			N_0++;

			if (error)
				E_0++;
		}
		else if (X[round_id] == POLARIZATION_1)
		{
			N_1++;

			if (error)
				E_1++;
		}

		continue;
	}

	auto Q_0 = (float)E_0 / N_0;
	auto Q_1 = (float)E_1 / N_1;
	auto QBER = (Q_0 + Q_1) / 2;
	//auto QBER = (E_0 + E_1) / (float)(N_0 + N_1);

	return QBER;
}

float DIQKD_ns::PublicChannel::ComputeQBERs(const std::vector<uint8_t>& alice_sifted_key_, const std::vector<uint8_t>& bob_sifted_key_)
{
	float QBER = 1.0f;

	if (alice_sifted_key_.size() != bob_sifted_key_.size())
		return QBER;

	size_t n_rounds = alice_sifted_key_.size();
	size_t n_err = 0;

	for (size_t round_id = 0; round_id < n_rounds; round_id++)
	{
		auto error = alice_sifted_key_[round_id] != bob_sifted_key_[round_id];

		if (error)
			n_err += 1;
	}

	QBER = n_err / (float)n_rounds;
	return QBER;
}
