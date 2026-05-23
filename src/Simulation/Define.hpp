#pragma once

#include <vector>
#include <string>

namespace DIQKD_ns
{
#define GET_PREV(x, n) (decltype(x))(((long)x - 1 + (long)n) % (long)n)

	enum PEER_TYPE
	{
		PEER_TYPE_UNKNOWN = 0,
		PEER_TYPE_ALICE = 1,
		PEER_TYPE_BOB = 2,
	};
	/*const std::vector<std::string> PEER_TYPE_NAME =
	{
		"UNKNOWN",
		"ALICE",
		"BOB",
	};*/

	enum PEER_STATUS
	{
		PEER_STATUS_IDLING,
		PEER_STATUS_HERALD,
		//PEER_STATUS_PROCESSED,
		PEER_STATUS_READOUT,
		PEER_STATUS_CLEANING,

		MAX_PEER_STATUS
	};

	/*enum ROUND_STATUS
	{
		ROUND_STATUS_IDLING,
		ROUND_STATUS_HERALD,

		MAX_ROUND_STATUS
	};*/

	enum ROUND_TYPE
	{
		ROUND_TYPE_UNKNOWN,
		ROUND_TYPE_KEY,
		ROUND_TYPE_TEST,
	};

	const size_t ALICE_INPUT_COUNT = 4;
	const size_t BOB_INPUT_COUNT = 2;

	enum POLARIZATION
	{
		POLARIZATION_NONE = -1,

		POLARIZATION_0 = 0,
		POLARIZATION_1 = 1,
		POLARIZATION_2 = 2,
		POLARIZATION_3 = 3,

		//POLARIZATION_MAX_2 = 2,
		//POLARIZATION_MAX_4 = 4,

		/*POLARIZATION_H = 0,
		POLARIZATION_V = 1,
		POLARIZATION_D = 2,
		POLARIZATION_A = 3,*/
	};

	enum STATE
	{
		STATE_DOWN = -1,
		STATE_SUPERPOSITION = 0,
		STATE_UP = 1,
	};
#define OPPOSITED_STATE(state) (STATE)(-(state))

	enum BELL_STATE
	{
		BELL_STATE_PHI_PLUS = 0,
		BELL_STATE_PHI_MINUS = 1,
		BELL_STATE_PSI_PLUS = 2,
		BELL_STATE_PSI_MINUS = 3,
	};

#define RAND_BIT_COUNT 16

	/*enum STATE
	{
		STATE_NEGATIVE = -1,
		STATE_SUPERPOSITION = 0,
		STATE_POSITIVE = 1,
	};*/

	//enum SPIN
	//{
	//	SPIN_UP = 0,	// |0>
	//	SPIN_DOWN = 1,	// |1>
	//};
}
