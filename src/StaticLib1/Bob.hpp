#pragma once
#include "Define.hpp"
#include "QRNG.hpp"
#include "Atom.hpp"
#include "Peer.hpp"

namespace DIQKD_ns
{
	class Bob : public Peer
	{
		friend class DIQKD;

	public:
		Bob(PublicChannel& public_channel_, size_t n_rounds_) : Peer(PEER_TYPE_BOB, 2, public_channel_, n_rounds_) {}
	};
}
