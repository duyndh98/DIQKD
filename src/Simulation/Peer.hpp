#pragma once
#include "Define.hpp"
#include "QRNG.hpp"
#include "Atom.hpp"
#include "PublicChannel.hpp"

namespace DIQKD_ns
{
	class Peer
	{
		friend class DIQKD;

	protected:

		// Internal
		QRNG<uint8_t> _qrng;
		Atom _atom;
		PEER_TYPE _peer_type;
		size_t _n_polarizations;

		//ErrorCorrection _error_correction;

		// External
		PublicChannel& _public_channel;
		size_t _n_rounds;

		// Runtime
		POLARIZATION _polarization = POLARIZATION_NONE;
		STATE _state = STATE_SUPERPOSITION;
		std::atomic<PEER_STATUS> _status = PEER_STATUS_IDLING;

		// Storage
		std::vector<POLARIZATION> _inputs;
		std::vector<STATE> _outputs;
		std::vector<STATE> _sifted_key;

		/*std::vector<std::tuple<POLARIZATION, STATE>> _key_rounds_data;
		std::vector<std::tuple<POLARIZATION, STATE>> _test_rounds_data;*/

	public:

		Peer(PEER_TYPE peer_type_, size_t n_polarizations_, PublicChannel& public_channel_, size_t n_rounds_, float ionization_noise_)
			: _peer_type(peer_type_), _n_polarizations(n_polarizations_), _public_channel(public_channel_), _n_rounds(n_rounds_), _atom(ionization_noise_)
		{
			_inputs.reserve(n_rounds_);
			_outputs.reserve(n_rounds_);

			return;
		}

		Atom& GetAtom();

		virtual void Reset();
		virtual void WaitReadySignalTransmission();
		virtual void RandomNumberGeneration();
		virtual void StateSelectiveIonization();
		virtual void FluorescenceReadout();
		virtual void PostProcessing(ROUND_TYPE round_type_);
		virtual void SiftKey();
	};
}
