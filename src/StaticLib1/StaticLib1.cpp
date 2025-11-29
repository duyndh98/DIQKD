// StaticLib1.cpp : Defines the functions for the static library.
//

#include "pch.h"
#include "framework.h"

#include "DIQKD.hpp"

using namespace DIQKD_ns;

// TODO: This is an example of a library function
void fnStaticLib1()
{
	static constexpr float P[BOB_INPUT_COUNT][ALICE_INPUT_COUNT] =
	{
		{ 413.0 / 448, 220.0 / 425, 311.0 / 389, 361.0 / 434 },
		{ 210.0 / 408, 380.0 / 412, 77.0 / 403, 359.0 / 423 },
	};

	auto E_21 = (1.0 - P[1][2]) - P[1][2];
	auto E_20 = (1.0 - P[0][2]) - P[0][2];
	auto E_30 = (1.0 - P[0][3]) - P[0][3];
	auto E_31 = (1.0 - P[1][3]) - P[1][3];

	/*
		E_21	0.61786600947380066		double
		E_20	-0.59897172451019287	double
		E_30	-0.66359448432922363	double
		E_31	-0.69739949703216553	double
	*/

	auto S = E_21 - E_20 - E_30 - E_31;
	// 2.5778317153453827

	DIQKD protocol(10000);
	protocol.Work();

	return;
}
