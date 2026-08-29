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

		std::string _data_root_dir;
		std::string _readout_dir, _sifting_dir, _correction_dir;

		size_t _n_rounds;

		Alice _alice;
		Bob _bob;
		
		PublicChannel _public_channel;
		uint8_t _ionization_noise_pct;

	private:
		void StoreMeasurement();
		void KeySifting();
		void ErrorCorrection();

	public:

		DIQKD(size_t n_rounds_, uint8_t ionization_noise_pct_, const std::string& data_root_dir_);
		
		void Work();

		/*std::tuple<float, float>*/
		/*std::string GetInputBinary(PEER_TYPE peer_type_);
		std::string GetOutputBinary(PEER_TYPE peer_type_);*/

		void AliceWork();
		void BobWork();
	};
}
