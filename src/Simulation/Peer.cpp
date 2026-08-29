#include "pch.h"

#include <string>
#include <sstream>

#include "Peer.hpp"
#include "Define.hpp"

using namespace DIQKD_ns;

Atom& DIQKD_ns::Peer::GetAtom()
{
	return _atom;
}

void DIQKD_ns::Peer::Reset()
{
	_public_channel.WaitPeerStatus(_peer_type, PEER_STATUS_CLEANING);

	_polarization = POLARIZATION_NONE;
	_state = STATE_SUPERPOSITION;
	_atom.Reset();

	/*_status.store(PEER_STATUS_IDLING);
	_status.notify_all();*/

	_public_channel.UpdatePeerStatus(_peer_type, PEER_STATUS_IDLING);

	return;
}

void DIQKD_ns::Peer::WaitReadySignalTransmission()
{
	//PLOG_INFO << _peer_type << ": Wating ROUND_STATUS_HERALD";

	_public_channel.WaitPeerStatus(_peer_type, PEER_STATUS_HERALD);

	//PLOG_INFO << _peer_type << ": Got ROUND_STATUS_HERALD";

	return;
}

void DIQKD_ns::Peer::RandomNumberGeneration()
{
	auto n_bits = std::lround(std::log2(_n_polarizations));
	_polarization = (POLARIZATION)_qrng.Generate(n_bits);

	//PLOG_INFO << _peer_type << " " << _polarization;
}

void DIQKD_ns::Peer::StateSelectiveIonization()
{
	//PLOG_INFO << _peer_type << ": Ionizing " << _polarization;

	//PLOG_DEBUG << "StateSelectiveIonization";

	_atom.Ionize(_polarization, _peer_type);

	//PLOG_INFO << _peer_type << ": Ionized";

	return;
}

void DIQKD_ns::Peer::FluorescenceReadout()
{
	//PLOG_INFO << _peer_type << ": Reading out";

	_state = _atom.Readout();

	//PLOG_INFO << _peer_type << ": Read " << _state;

	return;
}

void DIQKD_ns::Peer::Store(/*ROUND_TYPE round_type_*/)
{
	//auto input = _polarization;
	//auto output = round_type_ == ROUND_TYPE_TEST ? _state : STATE_SUPERPOSITION;

	//_public_channel.StorePeerData(_peer_type, input, output);

	_inputs.push_back(_polarization);
	_outputs.push_back(_state);

	_public_channel.UpdatePeerStatus(_peer_type, PEER_STATUS_READOUT);
	
	return;
}

void DIQKD_ns::Peer::QuantumStage()
{
	for (size_t round_id = 0; round_id < _n_rounds; round_id++)
	{
		WaitReadySignalTransmission();

		RandomNumberGeneration();
		StateSelectiveIonization();
		FluorescenceReadout();

		Store();
		Reset();

		continue;
	}

	return;
}

void DIQKD_ns::Peer::ClassicalStage()
{
	SharePublic();
	SiftKey();

	return;
}

void DIQKD_ns::Peer::SharePublic()
{
	_public_channel.PushPeerInputs(_peer_type, _inputs);

	_public_channel.UpdatePeerStatus(_peer_type, PEER_STATUS_SHARED_INPUTS);
	_public_channel.WaitAnotherPeerStatus(_peer_type, PEER_STATUS_SHARED_INPUTS);
	
	auto another_inputs = _public_channel.PullAnotherPeerInputs(_peer_type);
	if (_inputs.size() != another_inputs.size())
		return;

	std::vector<STATE> shared_outputs;
	shared_outputs.reserve(_outputs.size());

	for (size_t round_id = 0; round_id < _inputs.size(); round_id++)
	{
		auto shared_output = STATE_SUPERPOSITION;
		if (_inputs[round_id] > 1 || another_inputs[round_id] > 1)
			shared_output = _outputs[round_id];
		
		shared_outputs.push_back(shared_output);
	}

	_public_channel.PushPeerOutputs(_peer_type, shared_outputs);

	_public_channel.UpdatePeerStatus(_peer_type, PEER_STATUS_SHARED_OUTPUTS);
	_public_channel.WaitAnotherPeerStatus(_peer_type, PEER_STATUS_SHARED_OUTPUTS);

	return;
}

void DIQKD_ns::Peer::SiftKey()
{
	auto another_inputs = _public_channel.PullAnotherPeerInputs(_peer_type);
	if (_inputs.size() != another_inputs.size())
		return;

	_sifted_key.reserve(_inputs.size());

	for (size_t round_id = 0; round_id < _inputs.size(); round_id++)
	{
		if (_inputs[round_id] != another_inputs[round_id])
			continue;

		if (_outputs[round_id] == STATE_SUPERPOSITION)
			continue;

		auto key_bit = _outputs[round_id] == STATE_UP ? 1 : 0;
		if (_peer_type == PEER_TYPE_BOB)
			key_bit = ~key_bit & 1;

		_sifted_key.push_back(key_bit);
		continue;
	}

	/*std::stringstream sifted_key_bin_str;
	for (auto x : _sifted_key)
	{
		uint8_t bit_value = 0xff;
		if (x == STATE_UP)
			bit_value = 1;
		else if (x == STATE_DOWN)
			bit_value = 0;
		
		if (_peer_type != PEER_TYPE_ALICE)
			bit_value = (~bit_value) & 1;

		char bit_ch = '*';
		if (bit_value != 0xff)
			bit_ch = '0' + bit_value;

		sifted_key_bin_str << bit_ch;
	}*/

	//PLOG_INFO << magic_enum::enum_name(_peer_type) << " (" << _sifted_key.size() << "): "
	//	<< (_peer_type == PEER_TYPE_ALICE ? "" : "(flipping)") << std::endl << sifted_key_bin_str.str();

	return;
}
