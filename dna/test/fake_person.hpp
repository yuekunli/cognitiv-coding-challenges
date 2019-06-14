#pragma once

#include <array>
#include "fake_stream.hpp"

class fake_person
{
	std::array<fake_stream, 23> chroms_;
public:
	template<typename T>
	fake_person(const T& chromosome_data, std::size_t chunk_size = 512)
	{
		if (chromosome_data.size() != chroms_.size())
			throw std::invalid_argument("chromosome data does not match expected size");

		std::size_t index = 0;
		auto it = chromosome_data.begin();
		for (; index < chromosome_data.size() && it != chromosome_data.end(); ++index, ++it)
			chroms_[index] = fake_stream(*it, chunk_size);
	}

	const fake_stream& chromosome(std::size_t chromosome_index) const
	{
		if (chromosome_index >= chroms_.size())
			throw std::invalid_argument("index is out of range for the number of chromosomes available");

		return chroms_[chromosome_index];
	}

	constexpr std::size_t chromosomes() const
	{
		return chroms_.size();
	}
};


