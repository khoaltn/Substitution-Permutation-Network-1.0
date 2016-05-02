/* SPN-1-0-debug.h
 *
 * Header file of a simple substitution-permutation network. DEBUG version that dumps out intermediate values.
 *
 * Created by Khoa Nguyen on 03/16/2016.
 */

#ifndef __SPN-DEBUG__
#define __SPN-DEBUG__

#include <iostream>
#include <string>

using namespace std;

#define KEY_LEN 16
#define KEY_RANGE 256
#define BLOCK_LEN 8 // 8 bytes = 64 bits, the usual block length of modern block ciphers.
#define PERMUTATION_ENCRYPT_MODE true
#define PERMUTATION_DECRYPT_MODE false

class SPN_Debug {

public:

	// Default constructor: Random key, min# of rounds = 4
	SPN_Debug(int nr = 4);
	
	// Destructor
	~SPN_Debug();

	// Encryption for a string input
	unsigned char* encrypt_ECB_mode(const unsigned char plaintext[], int len);

	// Decryption for an array of ciphertext characters
	unsigned char* decrypt_ECB_mode(const unsigned char ciphertext[], int len);

	// print an unsigned char array as hexadecimal values
	void printArray(const unsigned char in[], int len);

	// Input processor: Turn string input into a 2D array of BLOCK_LEN substrings
	void prepare_string_ECB_mode(const unsigned char input[],
						unsigned char in[][BLOCK_LEN], int len);

private:
	
	int numRounds;	
	unsigned char* key; // default length = KEY_LEN
	unsigned char** subkeys; // there are (numRounds + 1) subkeys of length KEY_LEN
	int pMatrix[BLOCK_LEN][BLOCK_LEN]; // matrix for pi_P()
	int pMatrixInverse[BLOCK_LEN][BLOCK_LEN]; // inverse matrix of pi_P()
	
	// Key schedule: populate 2-D array subkeys from key
	void generate_subkeys();

	// XOR operation with key materials
	void operation_XOR(const unsigned char* input, unsigned char XORed[],
		int numSubkey);
	
	// Substitution pi_S()
	void pi_S(const unsigned char* input, unsigned char substituted[]);

	// Permutation pi_P()
	void pi_P(const unsigned char* input, unsigned char permuted[], bool encrypt);

	// Permutation matrix generator for pi_P()
	void generate_permutation_matrix();

	// Encrypt Algorithm
	unsigned char* SPN_encrypt(const unsigned char in[BLOCK_LEN]);

	// Decrypt Algorithm
	unsigned char* SPN_decrypt(const unsigned char in[BLOCK_LEN]);
};

#endif
