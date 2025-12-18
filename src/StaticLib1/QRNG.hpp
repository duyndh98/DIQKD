#pragma once
#include <random>

namespace DIQKD_ns
{
	template<typename T>
	class QRNG
	{		
	private:
		std::random_device _rd;

	public:
		inline T Generate(size_t n_bits_)
		{
#if 0
			auto x = (T)rand();			
#else
			auto x = (T)_rd();
#endif

			if (!n_bits_)
				return x;

			x &= ((T)1 << n_bits_) - 1;
			return x;
		}
	};
}
