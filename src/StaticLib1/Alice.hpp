#pragma once
#include "Define.hpp"
#include "QRNG.hpp"
#include "Atom.hpp"
#include "Peer.hpp"

namespace DIQKD_ns
{
	class Alice : public Peer
	{
		friend class DIQKD;

	public:
		Alice(PublicChannel& public_channel_, size_t n_rounds_, float ionization_noise_) : Peer(PEER_TYPE_ALICE, 4, public_channel_, n_rounds_, ionization_noise_) {}
	};
}
