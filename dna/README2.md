##What I did##

I wrote some code for 2 functionalities.
(1). find the number of bases that form the padding sequence at the beginning of a helix stream.
(2). find the number of bases that form the padding sequence at the end of a helix stream.

If a helix stream has these bases:
A G G G   T T A G G G  T T A G G G  (some real gene sequences)  T T A G G G   T T A G G G   T T A G G G  T T

The first function is to return 16 because there are 16 bases at the beginning that are the padding.
A telomere is cut between its 2nd and 3rd base so there are 4 left at the very beginning of this stream.
After that there are 2 complete telomeres.

The second function is to return 20 because there are 20 bases at the end that are the padding.
A telomere is cat between its 2nd and 3rd base so there are 2 left at the very end of this stream.
In front of that there are 3 complete telomeres.

These two functionalities are implemented in file person.hpp.
(I'm being lazy here because I put class' implementation in header file, this header file so far is only included in one .cpp file so it kind of works,
but it's bad practice.)


There are 6 TEST_CASEs in the person_test.hpp file.
Each of the first 4 TEST_CASEs has only 1 REQUIRE statement.
Each of the last 2 TEST_CASEs has 600 REQUIRE statement (or say REQUIRE is run 600 times).
There are 1204 test cases in total.


I also had to delete all the C++20 concept and constraints code because I had a hard time passing compilation in Visual Studio even when I set the language standard to C++20.


I could've added a lot more comments in both person.hpp and person_test.cpp. But I have to turn it in the way it is due to time limit. I'm happy to discuss my thoughts if folks at Cognitiv find my code interesting at all.
