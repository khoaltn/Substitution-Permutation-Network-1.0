I wrote this simple substitution-permutation network (SPN) as part of my Junior Independent Study project on block ciphers. Most modern block ciphers, notably Advanced Encryption Standard (AES), are designed as a substitution-permutation network. This implementation serves to illustrate the components of an SPN without confusing beginners in cryptography and theoretical mathematics.
 
* Instruction: In the terminal, go to the folder containing all the source code:
                To compile the source code, type:
                      $ g++ -std=c++11 -o SPN SPN.cpp testSPN.cpp
                To run the binary file, type:
                      $ ./SPN
Follow the documentation and example usage in testSPN.cpp on how to use the SPN class. 

* Future Improvements:
- More secure key schedule, S-boxes, and permutation tables using Galois Fields theory.
- More efficient implementation of calculations.
