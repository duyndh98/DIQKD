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

	_projected = false;

	//PLOG_DEBUG << "Entanglement";

	return;
}

void DIQKD_ns::Entanglement::Project(Atom* atom_ptr_, POLARIZATION polarization_)
{
	//PLOG_DEBUG << "Project";

	auto expected_projective = false;
	auto projected_changed = _projected.compare_exchange_strong(expected_projective, true);
	_projected.notify_all();

	/*{
		auto state = atom_ptr_->_entangled_atom_ptr->_projected_state.load();
		if (state != STATE_SUPERPOSITION)
			atom_ptr_->_entangled_atom_ptr->_projected_state.wait(state);
	}*/

	//PLOG_INFO << "Projected changed ? " << projected_changed;

	if (projected_changed)
	{
		uint8_t selected_state = g_qrng.Generate(1);

		atom_ptr_->_projected_state.store(selected_state ? STATE_UP : STATE_DOWN);
		atom_ptr_->_projected_state.notify_all();

		//PLOG_INFO << "Selected state: " << atom_ptr_->_state.load();
	}
	else
	{
		auto another_polarization = atom_ptr_->_entangled_atom_ptr->_selected_polarization_;

		auto another_state = atom_ptr_->_entangled_atom_ptr->_projected_state.load();

		auto X = std::max(polarization_, another_polarization);
		auto Y = std::min(polarization_, another_polarization);

		auto correlated = g_qrng.Generate(RAND_BIT_COUNT) <= IN_OUT_CORRELATION_PROBABILITY_TABLE[Y][X] * (1 << RAND_BIT_COUNT);

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

		atom_ptr_->_projected_state.store(current_state);
		atom_ptr_->_projected_state.notify_all();
		//PLOG_INFO << "Measured state: " << current_state;

		return;
	}

	return;
}

void DIQKD_ns::Atom::Ionize(POLARIZATION polarization_)
{
	if (_projected_state.load() == STATE_SUPERPOSITION)
	{
		_selected_polarization_ = polarization_;
		_entanglement_ptr->Project(this, polarization_);

		auto ionization_inefficiency = g_qrng.Generate(RAND_BIT_COUNT) <= _ionization_noise * (1 << RAND_BIT_COUNT);
		if (ionization_inefficiency)
			_ionized_state.store(OPPOSITED_STATE(_projected_state.load()));
		else
			_ionized_state.store(_projected_state.load());

		_ionized_state.notify_all();
	}
	
	return;
}

STATE DIQKD_ns::Atom::Readout()
{
	return _ionized_state.load();
}

void DIQKD_ns::Atom::Reset()
{
	//_entanglement_ptr = nullptr;
	//_entangled_atom_ptr = nullptr;
	_selected_polarization_ = POLARIZATION_NONE;

	_projected_state = STATE_SUPERPOSITION;
	_projected_state.notify_all();

	_ionized_state = STATE_SUPERPOSITION;
	_ionized_state.notify_all();

	return;
}
