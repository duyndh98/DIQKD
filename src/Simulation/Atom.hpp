#pragma once
#include <atomic>

#include "Define.hpp"

namespace DIQKD_ns
{
	class Entanglement;

	class Atom
	{
		friend class Entanglement;

	private:

		Entanglement* _entanglement_ptr = nullptr;
		Atom* _entangled_atom_ptr = nullptr;

		POLARIZATION _selected_polarization_ = POLARIZATION_NONE;
		std::atomic<STATE> _projected_state = STATE_SUPERPOSITION;
		std::atomic<STATE> _ionized_state = STATE_SUPERPOSITION;
		
		float _ionization_noise = 0.0f;

	public:
		Atom(float ionization_noise_) : _ionization_noise(ionization_noise_) {}

		void Ionize(POLARIZATION polarization_);
		STATE Readout();
		void Reset();
	};

	class Entanglement
	{
		friend class Atom;

	private:

		static constexpr float IN_OUT_CORRELATION_PROBABILITY_TABLE[BOB_INPUT_COUNT][ALICE_INPUT_COUNT] =
		{
			{ 413.0 / 448, 220.0 / 425, 311.0 / 389, 361.0 / 434 },
			{ 210.0 / 408, 380.0 / 412, 77.0 / 403, 359.0 / 423 },
		};

		BELL_STATE _bell_state;
		std::atomic<bool> _projected = false;

	public:
		
		Entanglement(BELL_STATE bell_state_, Atom& atom_A_, Atom& atom_B_);

		void Project(Atom* atom_ptr_, POLARIZATION polarization_);
	};
}
