#pragma once
#include <vector>
#include <atomic>
#include <string_view>
#include "sequence_buffer.hpp"

namespace dna
{

	class HelixStream
	{
		std::vector<std::byte> data_;
		std::size_t chunksize_;
		std::atomic<long> offset_;
	public:
		using byte_view = std::basic_string_view<std::byte/*, detail::binary_traits*/>;

		HelixStream();
		HelixStream(const HelixStream& other);
		HelixStream(HelixStream&& other) noexcept;
		HelixStream(std::vector<std::byte> data, std::size_t chunksize);

		HelixStream& operator=(const HelixStream& other);
		HelixStream& operator=(HelixStream&& other) noexcept;

		void seek(long offset);
		long size() const;
		dna::sequence_buffer<byte_view> read(std::size_t chunkSize);
		dna::sequence_buffer<byte_view> read();
	};

	HelixStream::HelixStream() :
		data_(0),
		chunksize_(1),
		offset_(0)
	{ }

	HelixStream::HelixStream(const HelixStream& other) :
		data_(other.data_),
		chunksize_(other.chunksize_),
		offset_(other.offset_.load())
	{ }

	HelixStream::HelixStream(HelixStream&& other) noexcept :
		data_(std::move(other.data_)),
		chunksize_(other.chunksize_),
		offset_(other.offset_.exchange(0))
	{ }

	HelixStream::HelixStream(std::vector<std::byte> data, std::size_t chunksize) :
		data_(std::move(data)),
		chunksize_(chunksize),
		offset_(0)
	{ }

	HelixStream& HelixStream::operator=(const HelixStream& other)
	{
		chunksize_ = other.chunksize_;
		data_ = other.data_;
		offset_ = other.offset_.load();

		return *this;
	}

	HelixStream& HelixStream::operator=(HelixStream&& other) noexcept
	{
		chunksize_ = other.chunksize_;
		data_ = std::move(other.data_);
		offset_ = other.offset_.exchange(0);

		return *this;
	}

	long HelixStream::size() const
	{
		return (long)data_.size();
	}


	void HelixStream::seek(long offset)
	{
		offset_.store(std::min(std::max(offset, 0L), static_cast<long>(data_.size())));
	}

	sequence_buffer<HelixStream::byte_view> HelixStream::read(std::size_t chunkSize)
	{
		auto offset = offset_.load(std::memory_order_consume);
		while (true)
		{
			auto len = std::min(chunkSize, data_.size() - offset_);
			if (len == 0)
				return byte_view(nullptr, 0);

			if (offset_.compare_exchange_weak(offset, offset + len, std::memory_order_release))
				return byte_view(data_.data() + offset, len);
		}
	}

	sequence_buffer<HelixStream::byte_view> HelixStream::read()
	{
		return HelixStream::read(chunksize_);
	}


	class Person
	{
		std::array<HelixStream, 23> chroms_;
		std::size_t chunksize_;
	public:
		template<typename T>
		Person(const T& chromosome_data, std::size_t chunk_size = 512)
		{
			chunksize_ = chunk_size;

			if (chromosome_data.size() != chroms_.size())
				throw std::invalid_argument("chromosome data does not match expected size");

			std::size_t index = 0;
			auto it = chromosome_data.begin();
			for (; index < chromosome_data.size() && it != chromosome_data.end(); ++index, ++it)
				chroms_[index] = HelixStream(*it, chunk_size);
		}

		const HelixStream& chromosome(std::size_t chromosome_index) const
		{
			if (chromosome_index >= chroms_.size())
				throw std::invalid_argument("index is out of range for the number of chromosomes available");

			return chroms_[chromosome_index];
		}

		constexpr std::size_t chromosomes() const
		{
			return chroms_.size();
		}

		



		/*****************************************************
		*    Below are the code I wrote for this assignment
		* ****************************************************
		*/



		/*
		* if index = 159 and count = 4, check if the 4 bases (starting at index 159 of the seq)
		* match the first 4 bases of a tolemere
		*/
		static bool matchTelomereStart(sequence_buffer<HelixStream::byte_view>& seq, size_t index, int count)
		{
			for (int i = 0; i < count; i++)
			{
				if (seq[index + i] != telo[i])
					return false;
			}
			return true;
		}

		static bool matchTelomereEnd(sequence_buffer<HelixStream::byte_view>& seq, size_t index, int count)
		{
			for (int i = 0; i < count; i++)
			{
				if (seq[index + i] != telo[6-count+i])
					return false;
			}
			return true;
		}

		/*
		* Find the index of the beginning of the very first telomere
		*/
		static int checkFirstTelomereStart(sequence_buffer<HelixStream::byte_view>& seq)
		{
			for (int i = 0; i < 6; i++)
			{
				if (matchTelomereStart(seq, i, 6))
					return i;
			}
		}

		static int getTelomereBasesCountBegin(HelixStream& hs)
		{
			auto seq = hs.read(3); // read 3 bytes = 12 bases, should have at least 1 complete telomere

			int firstTelomereStart = checkFirstTelomereStart(seq);  // index of the start of the very first telomere
			if (firstTelomereStart == -1)
				return -1;

			int incompleteTeloBasesAtBeginning = firstTelomereStart; // the first a few bases that don't form a *complete* telomere, but should match the end of a telomere

			hs.seek(0);
			int readBytes = 512;			
			int seqLength = readBytes * 4;
			int checkedBases = 0;

			while (true)
			{
				seq = hs.read(readBytes);

				int potentialCompleteTelo = (seqLength - incompleteTeloBasesAtBeginning) / 6;
				int incompleteTeloBasesAtTail = (seqLength - incompleteTeloBasesAtBeginning - potentialCompleteTelo * 6);
				
				if (matchTelomereEnd(seq, 0, incompleteTeloBasesAtBeginning))
				{
					checkedBases += incompleteTeloBasesAtBeginning;
				}
				else
				{
					/*
					* What should I do? Should I declare this gene is "damaged" just because the first a few bases
					* don't match the end of the telomere? But I may be able to find hundreds of complete telomeres
					* right after that first a few.
					*/
				}


				int start = 0, end = potentialCompleteTelo - 1;
				while (start < end)
				{
					int mid = start + (end - start) / 2;
					int midIndex = incompleteTeloBasesAtBeginning + mid * 6;
					if (matchTelomereStart(seq, midIndex, 6))
						start = mid + 1;
					else
						end = mid - 1;
				}

				if (matchTelomereStart(seq, incompleteTeloBasesAtBeginning + start * 6, 6))
				{
					if (start == potentialCompleteTelo - 1)
					{
						// every potentail complete telomere is valid
						if (matchTelomereStart(seq, seqLength - incompleteTeloBasesAtTail, incompleteTeloBasesAtTail))
						{
							checkedBases += (potentialCompleteTelo*6 + incompleteTeloBasesAtTail);
							incompleteTeloBasesAtBeginning = 6 - incompleteTeloBasesAtTail;
						}
						else
						{
							checkedBases += (potentialCompleteTelo * 6);
							return checkedBases;
						}
					}
					else
					{
						checkedBases += ((start+1) * 6);
						return checkedBases;
					}
				}
				else
				{
					checkedBases += start * 6;
					return checkedBases;
				}
			}
		}


		/*
		*  0  1  2  3  4  5  6  7  8  9  10  11
		*     |              |
		*     |              if the last telomere starts here, then there is no incomplete telomere
		*     if the last telomere starts here, then there are 5 extra bases that should match the beginning of a telomere
		*/
		static int checkLastTelomereStart(sequence_buffer<HelixStream::byte_view>& seq)
		{
			for (int i = 1; i <= 6; i++)
			{
				if (matchTelomereStart(seq, i, 6))
					return i;
			}
		}

		static int getTelomereBasesCountEnd(HelixStream& hs)
		{
			long totalBytes = hs.size();
			hs.seek(totalBytes - 3);
			auto seq = hs.read(3);

			int lastTelomereStart = checkLastTelomereStart(seq);
			if (lastTelomereStart == -1)
				return -1;

			int incompleteTeloBasesAtEnd = 12 - (lastTelomereStart + 6);

			int readBytes = 512;
			int seqLength = readBytes * 4;
			int checkedBases = 0;
			int seekOffsetMultiple = 1;

			while (true)
			{
				hs.seek(totalBytes - seekOffsetMultiple * 512);
				seq = hs.read(readBytes);

				int potentialCompleteTelo = (seqLength - incompleteTeloBasesAtEnd) / 6;
				int incompleteTeloBasesAtBegin = (seqLength - incompleteTeloBasesAtEnd - potentialCompleteTelo * 6);

				if (matchTelomereStart(seq, seqLength - incompleteTeloBasesAtEnd, incompleteTeloBasesAtEnd))
				{
					checkedBases += incompleteTeloBasesAtEnd;
				}
				else
				{

				}

				int start = 0, end = potentialCompleteTelo - 1;
				while (start < end)
				{
					int mid = start + (end - start) / 2;
					int midIndex = seqLength - ((mid+1) * 6 + incompleteTeloBasesAtEnd);
					if (matchTelomereStart(seq, midIndex, 6))
						start = mid + 1;
					else
						end = mid - 1;
				}

				if (matchTelomereStart(seq, seqLength - ((start+1) * 6 + incompleteTeloBasesAtEnd), 6))
				{
					if (start == potentialCompleteTelo - 1)
					{
						if (matchTelomereEnd(seq, 0, incompleteTeloBasesAtBegin))
						{
							checkedBases += (potentialCompleteTelo * 6 + incompleteTeloBasesAtBegin);
							incompleteTeloBasesAtEnd = 6 - incompleteTeloBasesAtBegin;
						}
						else
						{
							checkedBases += (potentialCompleteTelo * 6);
							return checkedBases;
						}
					}
					else
					{
						checkedBases += ((start + 1) * 6);
						return checkedBases;
					}
				}
				else
				{
					checkedBases += start * 6;
					return checkedBases;
				}

				seekOffsetMultiple++;
			}
		}
	};
}

