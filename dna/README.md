You are the BioTech Savior
==========================

We're going to pretend that you're working at a BioTech research firm.
And somehow they've come to exist and flourish without some pretty critical
tools. But now it's come time that they need to build some fundamental pieces
of functionality for their biological researchers to be able to do their jobs.

So far, they've managed to map the genome from hundreds of thousands of people,
and store those samples anonymously. The anonymous profiles are still full of
private data though: Health Conditions, Test results, academic performance,
ethnic and family history, etc, etc.

So they are sitting on top of a goldmine of data for research that has
the potential to uncover all kinds of useful genetic information. But
they haven't even started this process. In fact, they don't really even know
what this process is. They know that they're going to keep collecting more and
more genetic samples. And Elon Musk invested a few billion dollars in the
company so they have some runway. But they're still going to need to figure
out what their business is going to be if they ever hope to secure any additional
funding from a sane investor. And this genetic sampling process is not cheap.

Your job isn't going to be to tell them what to do with all of this valuable
data. You will simply be enabling them and their scientists to explore the
data to discover trends in the data, along with ways that they can use it.

# What's already there

Fortunately, this BioTech firm is relatively new. So they were able to build
their platform on top of AWS after it was officially certified for HIPAA and
state secrets.

As a result, they've built a lot of functionality already for getting data
to and from the cloud using Amazon's functionality. And they've built out
APIs for this kind of data streaming. They just don't have good ways to
analyze the data in these streams in bulk.

A base pair is represented as a single helix in a stream of data. For a refresher:

## DNA

DNA is made up of molecules called nucleotides. Each nucleotide contains a
phosphate group, a sugar group and a nitrogen base. The four types of
nitrogen bases are adenine (A), thymine (T), guanine (G) and cytosine (C).
The order of these bases is what determines DNA's instructions, or genetic
code. Human DNA has around 3 billion bases, and more than 99 percent of
those bases are the same in all people, according to the U.S. National
Library of Medicine (NLM).

-plagiarized from https://www.livescience.com/37247-dna.html

Each nitrogen base has a complimentary base that it binds to to form a full
rung on the double helix. Adenine binds with thymine (in the case of DNA) and
guanine binds with cytosine. So we can pack one side of the helix into a stream
of data and ascertain the other half of the rung.

## Storage

The data is stored with 4 bases packed into each byte. But the company already
has code to unpack these and process them. The sequence buffer handles the
relationship between bytes and bases. A helix sits on top of a data stream
to provide these buffers. As such, all of your code will operate on helices.

# The task at hand

The first thing that the biodata scientists will need to do is to find interesting
sections of DNA. The way that they're going to do this is to compare sequences
to each other and extract interesting portions.

The samples of data that our BioTech company has in storage
are the raw outputs of their sequencing systems. The data hasn't been cleaned
up or processed. As a result, the system is going to need to tolerate some level
of inconsistency.

## The data

Let's discuss the quality of the data that the company has

### The type of data

All of the sequences that the company is sitting on are of the same species:
human beings. All human beings share about 99.9% of our effective DNA. In other
words, every sample should have about 99.9% of the base pairs in common with any
other sample.

### Telomeres

DNA has a special "padding" sequence on both ends to protect the integrity of
the sequence. During replication, some portion of the DNA sequence gets lost.
As a result, humans have an average of 2500 special sequences at the end of
our chromosomes. These special padding sequences are known as telomeres.

Telomeres are composed of repeated `TTAGGG` sequences. The number of these can
vary significantly between people. On average, at birth, we have about 11 kilobases
of telomere. And as we age, this can drop to below 4 kilobases. This is interesting
unto itself. But it's relevant here because our DNA samples contain these telomeres.
When we're comparing two sequences, the telomeres should not be considered.

### Equipment data loss

Our company has spent many years perfecting the accuracy and reliability of the machines
that read sequences from DNA samples. But so far, they've only been able to ensure
the accuracy of their readings from the point where the DNA is read until the end of
where it reads. So given a sequence, that sequence has no holes and no mis-read data.

What we can't guarantee though, is that our sequencer read from the absolute beginning of
the strand until the end of the strand. The biological scientists at the company have
decided that this is good enough. For 99.9% of our samples, that lost data is in the
telomeres anyway. But occasionally, we lose some of the real data too.

As a result, for *all* data, there's no guarantee that you'll have full telomere
sequences. The machine may have lost the `TTAG` and the sequence may start with
`GGTTAGGGTTAGGGTTAGGG` and in that case, we've most likely just chopped off some
number of telomeres and the first 4 bases of the next telomere.

This same principle applies to the end of the sequence too. Telomeres may chop off halfway
through at the end and may also have been lost completely.

But occasionally, we'll have lost all of the telomere sequences. So if we're comparing
two sequences, we'll need to line them up and start comparing where we can.

### Format

Each person's data consists of 23 helixes as noted above. They have specific identifiers
1-23. The 23rd chromosome is the famous X/Y chromosome. For now, we are only interested in
Comparing X chromosomes to X chromosomes and Y to Y. So given two samples, if they are
different genetic genders, we will only look at the first 22 chromosomes. Of course,
it's not immediately obvious what we're looking at until we inspect that 23rd chromosome.

For this, it is sufficient to just look at the size of the chromosome. An X chromosome
has about 156 million base pairs. A Y chromosome is closer to 57 million. There's enough
wiggle room there to decidedly recognize when you have a mismatch on chromosome 23.

Each chromosome is of a different length ranging from about 46 million base pairs to about
249 million. For reference see [Wikipedia](https://en.wikipedia.org/wiki/Human_genome#Molecular_organization_and_gene_content).
It won't necessarily be important to know this because we can guarantee that our data
has each chromosome accurately labeled 1-23. And a helix has a length attached to it.

# Requirements

We're going to use this code in jobs that other teams are going to write. They will want
to do large comparisons where they hone in on specific sections of DNA.

## One more important consideration

These genomes are big. Each person will have over 3 billion base pairs. Comparing two of
these will obviously be slow. The company was only interested before in getting data to S3
and back quickly. They didn't think about horizontal scaling for this.

So we'd also like the ability to be able to parallelize these work loads. In theory, with
sufficient resources, we would like to be able to compare people in sub-second time. This
would potentially require thousands of machines running a sort of map reduce job. You
absolutely need *not* build any kind of work distribution system or anything like that.

We simply want contracts that are capable of being used in these map reduce jobs to
distribute the workload later on top of some other system like hadoop. For now, we will
only distribute the work to multiple threads. So as long as we can start a subsection of
the below and distribute it between threads, that's sufficient.

## What it needs to do

We need to be able to compare 2 full `Person` instances. `Person` is a concept that exposes
`HelixStream` instances on a per chromosome basis. It also provides the number of chromosomes
which will always be 23 for the available data. This should only fail if the data doesn't
conform to the specification above. The output should be a collection highlighting the
interesting distinctions between the two people, considering all of the details above.

The other thing we need to be able to do is to extract *one* of the sub-sequences found 
in the above process from either of the two people (that'll be the user's responsibility 
to go back and read the actual sequence as long as you provide a range for them to do
so from), and compare just that section with other people. This endpoint is allowed to 
fail for any reason. The obvious example is when the chromosome for the person in question 
has an insufficient number of telomere sequences to establish where the "start" is.
Scientists can fall back on the full Person to Person comparison for these samples. 
It should also return a collection of areas with differences, just like the first API.

This will be compiled with gcc 8 on Linux with `-fconcepts`. Ideally, it only relies on
the C++ standard library but header only libraries that are included in-source 
will still be accepted (where licensing permits, it's not proprietary per se but 
closed source on account of submissions not being published).

## How to submit

Check out the git repo and add tests for your code in the test directory. The tests
use [Catch2](https://github.com/catchorg/Catch2) as the testing framework. You are
free to add as many test support files (including implementation of concepts) as needed.

Keep the actual tests simple (fixtures and support code doesn't need to be as simple though).
We will create additional tests using your tests as a basis that run the process on real
genomes that were manipulated for edge cases.

### Rubric

#### 1. Does it work for all of our additional tests

As noted above, we will run the library on a larger data set crafted to expose edge cases

#### 2. Are there tests that cover all the possible edge cases.

We'll be looking for tests for the obvious happy path as well as the edge cases described
here along with others. Of course, we make certain guarantees and don't require your
submission to defensively check for those. We're only looking for edge cases that exist
within the scope of the assignment.

Additionally, if one of our tests fails in spite of there being a submitted test for the
edge case that just wasn't sufficient or had a mistake, partial credit will still be applied.

#### 3. How gracefully can it horizontally scale

Remember that we said we want to have the *option* to distribute these jobs over a cluster
to get immediate feedback. This would probably be implemented on a higher level map reduce
framework. So we'll just be looking at the theoretical ease of that implementation on top
of the library.

#### 4. How gracefully can it scale in terms of future functionality

This is more about design. We have a company that still doesn't have a clue what their
business is going to be other than having something to do with all of this genetic data.
We might need to pivot or evolve this library to do other similar things. And it might be
someone else that takes ownership of that.

So this will be more about decomposition and readability.

#### 5. Performance

We'll run our tests with the submission in a benchmark. This will be compared to a reference
implementation. It's not required that the submission outperform the reference. Only that
it's in the same order of magnitude or better (which shouldn't be too hard as long as
performance is a consideration while the work is being done. The reference implementation
won't be particularly tuned).

#### Disclaimer

This whole example is totally contrived. Cognitiv is not taking DNA sequences. There are
standard industry tools and file format standards for handling genome data that make all of
the difficult coding problems presented in this challenge non-existent. We're just
ignoring those in the name of a coding exercise.

If you are interested in sample data and more information about how the industry
really handles this stuff (which looks nothing like this exercise) check out
[The 1000 Genome Project](http://www.internationalgenome.org/)



