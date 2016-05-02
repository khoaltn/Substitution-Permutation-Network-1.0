I wrote this simple substitution-permutation network (SPN) as part of my Junior Independent Study project on block ciphers. Most modern block ciphers, notably Advanced Encryption Standard (AES), are designed as a substitution-permutation network. This implementation serves to illustrate the components of an SPN without confusing beginners in cryptography and theoretical mathematics.
 
Instructions: 
If you want to run the test on images, you need to install OpenCV at http://opencv.org per the instructions there. Otherwise, comment out the testSPN_image() and all the code to load OpenCV libraries and namespace in SPN-1-0-test.cpp.
In the terminal, go to the folder containing all the source code:
- To compile the source code, type: $ ./build.sh
- To run the binary file, type:     $ ./SPN

Follow the documentation and example usage in SPN-1-0-test.cpp on how to use the SPN class. 

Future Improvements:
- More secure key schedule, S-boxes, and permutation tables using Galois Fields theory.
- More efficient implementation of calculations.
