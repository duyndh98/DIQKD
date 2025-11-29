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

	public:

		DIQKD(size_t n_rounds_)
			: _n_rounds(n_rounds_), _public_channel(n_rounds_), _alice(_public_channel, n_rounds_), _bob(_public_channel, n_rounds_)
		{
			static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
			plog::init(plog::verbose, &consoleAppender);
		}

		void Work();

		void AliceWork();
		void BobWork();
	};
}
