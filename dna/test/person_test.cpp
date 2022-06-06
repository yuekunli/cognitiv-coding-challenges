#include "catch.hpp"
#include <array>
#include "sequence_buffer.hpp"
#include "person.hpp"

dna::base telo[6] = { dna::T, dna::T, dna::A, dna::G, dna::G, dna::G };


TEST_CASE()
{
	std::vector<std::byte> data(512);

	data[0] = dna::pack(dna::G, dna::T, dna::T, dna::A);
                        //   |
                        //   match the end of a telomere

	data[1] = dna::pack(dna::G, dna::G, dna::G, dna::G);
                                                //   |
                                                //   doesn't match the start of a telomere
	for (int i = 2; i < 512; i++)
	{
		data[i] = dna::pack(dna::T, dna::T, dna::A, dna::A);
	}

	dna::HelixStream hs(data, 512);

	int count = dna::Person::getTelomereBasesCountBegin(hs);

	REQUIRE(count == 7);
}


/*
* a stream of 512 bytes, there should be 2048 bases, which consists of
* 1 base (G) + 341 complete telomere sequences + 1 base (A).
* The construction of the test case is in a way so that the very 1st base
* matches the end of a telomere, the 2046 bases in the middle match 341
* complete telomeres, but the 1 base at the very end doesn't match the start
* of a telomere. Therefore the number of bases of the total padding sequence
* is 2047
*/
TEST_CASE()
{
	std::vector<std::byte> data(512);

	data[0] = dna::pack(dna::G, dna::T, dna::T, dna::A);
						//   |
						//   match the end of a telomere
	data[1] = dna::pack(dna::G, dna::G, dna::G, dna::T);

	data[511] = dna::pack(dna::G, dna::G, dna::G, dna::A);
													// |
													// doesn't match the start of a telomere

	int k = 1;
	for (int i = 2; i <= 510; i++)
	{
		dna::base b1 = telo[k % 6];
		k++;
		dna::base b2 = telo[k % 6];
		k++;
		dna::base b3 = telo[k % 6];
		k++;
		dna::base b4 = telo[k % 6];
		k++;

		data[i] = dna::pack(b1, b2, b3, b4);
	}
	
	dna::HelixStream hs(data, 512);

	int count = dna::Person::getTelomereBasesCountBegin(hs);

	REQUIRE(count == 2047);
}


TEST_CASE()
{
	std::vector<std::byte> data(512);

	data[510] = dna::pack(dna::T, dna::T, dna::A, dna::G);
	                       //  |
	                       //  doesn't match the end of a telomere
	data[511] = dna::pack(dna::G, dna::G, dna::T, dna::T);
	                                        // \       /
	                                        //  these two match the start of a telomere

	for (int i = 0; i <= 509; i++)
	{
		data[i] = dna::pack(dna::A, dna::T, dna::C, dna::G);
	}

	dna::HelixStream hs(data, 512);

	int count = dna::Person::getTelomereBasesCountEnd(hs);

	REQUIRE(count == 8);
}

/*
*       340 * 6 = 2040
*    4 bases + 340 complete telomeres + 4 bases
*        |                              (T T A G) match the start of a telomere
*       make these 4 not match the end of a telomere
*/
TEST_CASE()
{
	std::vector<std::byte> data(512);

	data[0] = dna::pack(dna::A, dna::C, dna::G, dna::G);
	
	int k = 0;
	for (int i = 1; i <= 510; i++)
	{
		dna::base b1 = telo[k % 6];
		k++;
		dna::base b2 = telo[k % 6];
		k++;
		dna::base b3 = telo[k % 6];
		k++;
		dna::base b4 = telo[k % 6];
		k++;

		data[i] = dna::pack(b1, b2, b3, b4);
	}

	data[511] = dna::pack(dna::T, dna::T, dna::A, dna::G);

	dna::HelixStream hs(data, 512);
	int count = dna::Person::getTelomereBasesCountEnd(hs);
	REQUIRE(count == 2044);
}

/*
* 4 bases at the beginning, as if a telomere is cut between the 2nd and 3rd base,
* then I add 1, 2, 3, 4, .... 600 complete telomeres
*/
TEST_CASE()
{
	std::vector<std::byte>data(1024);

	for (int completeTelo = 1; completeTelo <= 600; completeTelo++)
	{
		data.assign(1024, std::byte(0));

		data[0] = dna::pack(dna::A, dna::G, dna::G, dna::G);

		int k = 0;
		int completeTeloAdded = 0;
		int i = 1;
		for (i = 1; i < 1024; i++)
		{
			dna::base b1 = telo[k % 6];
			if (k % 6 == 5) completeTeloAdded++;
			k++;
			dna::base b2 = telo[k % 6];
			if (k % 6 == 5) completeTeloAdded++;
			k++;
			dna::base b3 = telo[k % 6];
			if (k % 6 == 5) completeTeloAdded++;
			k++;
			dna::base b4 = telo[k % 6];
			if (k % 6 == 5) completeTeloAdded++;
			k++;

			data[i] = dna::pack(b1, b2, b3, b4);
			if (completeTeloAdded == completeTelo)
				break;
		}
		i++;
		for (; i < 1024; i++)
		{
			data[i] = dna::pack(dna::C, dna::C, dna::T, dna::T);
		}
		dna::HelixStream hs(data, 512);
		int count = dna::Person::getTelomereBasesCountBegin(hs);
		REQUIRE(count == (4 + completeTelo*6));
	}
}

/*
* At the very end of this data stream, there are 4 bases, TTAG which match the start of a telomere
* then add 1, 2, 3, ..., 600 complete telomeres in front of it.
*/
TEST_CASE()
{
	std::vector<std::byte>data(1024);

	for (int completeTelo = 1; completeTelo <= 600; completeTelo++)
	{
		data.assign(1024, std::byte(0));

		data[1023] = dna::pack(dna::T, dna::T, dna::A, dna::G);

		int k = 6005;
		int completeTeloAdded = 0;
		int i = 1022;
		for (i = 1022; i >= 0; i--)
		{
			dna::base b4 = telo[k % 6];
			if (k % 6 == 0) completeTeloAdded++;
			k--;

			dna::base b3 = telo[k % 6];
			if (k % 6 == 0) completeTeloAdded++;
			k--;

			dna::base b2 = telo[k % 6];
			if (k % 6 == 0) completeTeloAdded++;
			k--;

			dna::base b1 = telo[k % 6];
			if (k % 6 == 0) completeTeloAdded++;
			k--;

			data[i] = dna::pack(b1, b2, b3, b4);
			if (completeTeloAdded == completeTelo)
				break;
		}
		i--;
		for (; i >= 0; i--)
		{
			data[i] = dna::pack(dna::C, dna::T, dna::T, dna::C);
		}
		dna::HelixStream hs(data, 512);
		int count = dna::Person::getTelomereBasesCountEnd(hs);
		REQUIRE(count == (4 + completeTelo * 6));
	}
}
