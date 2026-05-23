#pragma once
#include <vector>
#include <stdint.h>

#include "Alice.hpp"
#include "Bob.hpp"
#include "PublicChannel.hpp"
#include "ErrorCorrection.h"

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

		DIQKD(size_t n_rounds_, float ionization_noise_);
		
		std::tuple<float, float> Work();
		std::string GetInputBinary(PEER_TYPE peer_type_);
		std::string GetOutputBinary(PEER_TYPE peer_type_);

		void AliceWork();
		void BobWork();
	};
}
