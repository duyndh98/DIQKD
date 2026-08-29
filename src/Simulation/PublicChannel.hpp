#pragma once
#include <vector>
#include <atomic>

#include "Define.hpp"

namespace DIQKD_ns
{
	class PublicChannel
	{
	private:

		std::vector<POLARIZATION> _alice_inputs, _bob_inputs;
		std::vector<STATE> _alice_outputs, _bob_outputs;

		std::atomic<PEER_STATUS> _alice_status, _bob_status;

		//std::atomic<ROUND_TYPE> _round_type = ROUND_TYPE_UNKNOWN;

	public:

		PublicChannel(size_t n_rounds_)
		{
			_alice_inputs.reserve(n_rounds_);
			_alice_outputs.reserve(n_rounds_);
			_bob_inputs.reserve(n_rounds_);
			_bob_outputs.reserve(n_rounds_);

			return;
		}

		~PublicChannel()
		{
			return;
		}

		//void StorePeerData(PEER_TYPE peer_type_, POLARIZATION input_, STATE output_);
		
		//void UpdateRoundType(ROUND_TYPE round_type_);
		//ROUND_TYPE FetchRoundType();

		void UpdatePeerStatus(PEER_TYPE peer_type_, PEER_STATUS peer_status_);
		void WaitPeerStatus(PEER_TYPE peer_type_, PEER_STATUS peer_status_);
		void WaitAnotherPeerStatus(PEER_TYPE peer_type_, PEER_STATUS peer_status_);

		void PushPeerInputs(PEER_TYPE peer_type_, const std::vector<POLARIZATION>& polarizations_);
		std::vector<POLARIZATION> PullAnotherPeerInputs(PEER_TYPE peer_type_);

		void PushPeerOutputs(PEER_TYPE peer_type_, const std::vector<STATE>& states_);
		std::vector<STATE> PullAnotherPeerOutputs(PEER_TYPE peer_type_);

		float ComputeCHSH(const std::vector<POLARIZATION>& X, const std::vector<POLARIZATION>& Y, const std::vector<STATE>& A, const std::vector<STATE>& B);
		float ComputeQBER(const std::vector<POLARIZATION>& X, const std::vector<POLARIZATION>& Y, const std::vector<STATE>& A, const std::vector<STATE>& B);
		float ComputeQBERs(const std::vector<uint8_t>& A, const std::vector<uint8_t>& B);
	};
}
