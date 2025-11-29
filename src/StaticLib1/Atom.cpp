#include <atomic>
#include <vector>

#include "pch.h"
#include "QRNG.hpp"
#include "Atom.hpp"

using namespace DIQKD_ns;

namespace DIQKD_ns
{
	QRNG<uint64_t> g_qrng;
}

DIQKD_ns::Entanglement::Entanglement(BELL_STATE bell_state_, Atom& atom_A_, Atom& atom_B_)
{
	_bell_state = bell_state_;
	atom_A_._entanglement_ptr = atom_B_._entanglement_ptr = this;

	atom_A_._entangled_atom_ptr = &atom_B_;
	atom_B_._entangled_atom_ptr = &atom_A_;

	return;
}

void DIQKD_ns::Entanglement::Project(Atom* atom_ptr_, POLARIZATION polarization_)
{
	auto expected_projective = false;
	auto projected_changed = _projected.compare_exchange_strong(expected_projective, true);

	//PLOG_INFO << "Projected changed ? " << projected_changed;

	if (projected_changed)
	{
		uint8_t selected_state = g_qrng.Generate(1);
		atom_ptr_->_state.store(selected_state ? STATE_UP : STATE_DOWN);
		atom_ptr_->_state.notify_all();

		//PLOG_INFO << "Selected state: " << atom_ptr_->_state.load();
	}
	else
	{
		auto another_polarization = atom_ptr_->_entangled_atom_ptr->_selected_polarization_;

		atom_ptr_->_entangled_atom_ptr->_state.wait(STATE_SUPERPOSITION);
		auto another_state = atom_ptr_->_entangled_atom_ptr->_state.load();

		auto X = std::max(polarization_, another_polarization);
		auto Y = std::min(polarization_, another_polarization);

		auto correlation_prob = IN_OUT_CORRELATION_PROBABILITY_TABLE[Y][X];

		auto constexpr RAND_BIT_COUNT = 10;
		auto threshold = correlation_prob * (1 << RAND_BIT_COUNT);

		auto selected_value = g_qrng.Generate(RAND_BIT_COUNT);
		auto correlated = selected_value < threshold;

		STATE current_state = STATE_SUPERPOSITION;

		switch (_bell_state)
		{
		case DIQKD_ns::BELL_STATE_PHI_PLUS:
		case DIQKD_ns::BELL_STATE_PHI_MINUS:
			current_state = correlated ? another_state : OPPOSITED_STATE(another_state);
			break;
		case DIQKD_ns::BELL_STATE_PSI_PLUS:
		case DIQKD_ns::BELL_STATE_PSI_MINUS:
			current_state = correlated ? OPPOSITED_STATE(another_state) : another_state;
			break;
		default:
			break;
		}

		atom_ptr_->_state.store(current_state);		
		//PLOG_INFO << "Measured state: " << current_state;

		return;
	}

	return;
}

void DIQKD_ns::Atom::Ionize(POLARIZATION polarization_)
{
	if (_state == STATE_SUPERPOSITION)
	{
		_selected_polarization_ = polarization_;
		_entanglement_ptr->Project(this, polarization_);
	}
	
	//_ionized_timepoint = g_timepoint.fetch_add(1);

	//auto self_changed = _selected_polarization_.compare_exchange_strong(expected_state, self_state);
	/*auto self_state = g_qrng.Generate(1) ? STATE_NEGATIVE : STATE_POSITIVE;
	auto expected_state = STATE_SUPERPOSITION;
	auto self_changed = _state.compare_exchange_strong(expected_state, self_state);
	if (self_changed)
	{
		while (true)
		{
			expected_state = STATE_SUPERPOSITION;
			auto another_changed = _entangled_atom->_state.compare_exchange_strong(expected_state, another_state);
			if (another_changed)
				break;
	
			if (this->_ionized_timepoint > _entangled_atom->_ionized_timepoint)
			{
				_state.exchange(STATE_SUPERPOSITION);
				break;
			}
		}
	}*/

	//auto ionized = _state == STATE_POSITIVE;
	//_exist = !ionized;

	//switch (polarization_)
	//{
	//case DIQKD_ns::POLARIZATION_H:
	//	break;
	//case DIQKD_ns::POLARIZATION_V:
	//	break;
	//case DIQKD_ns::POLARIZATION_D:
	//case DIQKD_ns::POLARIZATION_A:
	//	break;
	//default:
	//	break;
	//}

	return;
}

STATE DIQKD_ns::Atom::Readout()
{
	return _state;
}

void DIQKD_ns::Atom::Reset()
{
	_entanglement_ptr = nullptr;
	_entangled_atom_ptr = nullptr;
	_selected_polarization_ = POLARIZATION_NONE;
	_state = STATE_SUPERPOSITION;

	return;
}
