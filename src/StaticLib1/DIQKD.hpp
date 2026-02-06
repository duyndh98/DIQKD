#pragma once
#include <vector>

#include "Alice.hpp"
#include "Bob.hpp"
#include "PublicChannel.hpp"

namespace DIQKD_ns
{
	class DIQKD
	{	
	private:

		size_t _n_rounds;

		Alice _alice;
		Bob _bob;
		
		PublicChannel _public_channel;
		float _ionization_noise;

	private:
		void ErrorCorrection();

	public:

		DIQKD(size_t n_rounds_, float ionization_noise_)
			: _n_rounds(n_rounds_), _public_channel(n_rounds_), _alice(_public_channel, n_rounds_, ionization_noise_), _bob(_public_channel, n_rounds_, ionization_noise_)
		{
			_ionization_noise = ionization_noise_;

			static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
			plog::init(plog::verbose, &consoleAppender);
		}

		float Work();
		std::string GetInputBinary(PEER_TYPE peer_type_);
		std::string GetOutputBinary(PEER_TYPE peer_type_);

		void AliceWork();
		void BobWork();
	};
}
