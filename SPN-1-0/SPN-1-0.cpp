/* SPN.cpp
 *
 * Implementation of a simple substitution-permutation network.
 *
 * Created by Khoa Nguyen on 03/16/2016.
 */

#include "SPN-1-0.h"
#include <iomanip>

using namespace std;


// Default constructor: Random key, min# of rounds = 4
SPN::SPN(int nr) {
    // Make sure number of rounds >= 4
	if (nr < 4) {
		numRounds = 4;
	}
	else {
		numRounds = nr;
	}

    // Random key generated
	cout << "--------------- RANDOM KEY: ----------------------" << endl;
	srand (time(NULL));
	key = new unsigned char[KEY_LEN];
	
	for (int i = 0; i < KEY_LEN; i++) {
		key[i] = (unsigned char) rand() % KEY_RANGE;
	}
	printArray(key, KEY_LEN);
	cout << endl;

    // Generate Subkeys
	cout << "--------------- GENERATED SUBKEYS: ---------------" << endl;
	generate_subkeys();
		
    // Generate Permutation Matrix
	cout << "--------------- GENERATED PERMUTATION: -----------" << endl;
	generate_permutation_matrix();

	cout << "--------------------------------------------------" << endl;
}

/*
// Destructor
// Delete all data members that are dynamically allocated of the SPN object

*/
SPN::~SPN() {
// Destroy key
	delete [] key;
	
// Destroy subkeys	
	for (int i = 0; i < numRounds; ++i) {
		delete [] subkeys[i];
	}
	delete [] subkeys;
}

// Key schedule: A simple function for the key schedule is that for subkey of round r, subkey K_r is a copy of the original key starting from byte 3i + 1, wrapped around if necessary. This is not a secure way to generate key in practice. It's good to demonstrate linear cryptanalysis, however.
void SPN::generate_subkeys() {
	subkeys = new unsigned char*[numRounds + 1];
	for (int i = 0; i < numRounds + 1; ++i) {
		subkeys[i] = new unsigned char[BLOCK_LEN];
		for (int j = 0; j < BLOCK_LEN; j++) {
			subkeys[i][j] = key[(j + (3 * i + 1)) % KEY_LEN];
		}
		printArray(subkeys[i], BLOCK_LEN);
		cout << endl;
	}
}

/*
 * Substitution box pi_S()
 * Pre: a block of input characters of length BLOCK_LEN
 * Post: a block of output characters of length BLOCK_LEN, each of which is the result of mapping the corresponding input character through the substitution function
 * Note: Substitution pi_S(): A simple function for substitution is to use the bit-flipped version of each input[i]. For example,  0000 0001 (0x01) becomes 1111 1110 (FE) (bit flipped). This can be improved greatly by using GF(2^8) and maximum-distance-separable (MDS) matrix.
 */
void SPN::pi_S(const unsigned char* input, unsigned char substituted[]) {
	for (int i = 0; i < BLOCK_LEN; i++) {
		substituted[i] = ~input[i];
	}
}
// TODO: improve S-boxes

/* Permutation pi_P(): "Mixing up" the positions of the characters in input.
 * Pre: a block of input characters of length BLOCK_LEN
 * Post: the characters in input have changed places with each other per the permutation function (which is to consider the input as a vector of length BLOCK_LEN, and then multiply it with a square matrix whose columns are the standard basis vectors e_1, e_2,..., e_{BLOCK_LEN} in some permuted order). 
 *
 */
void SPN::pi_P(const unsigned char* input, unsigned char permuted[], bool encrypt) {
	// Do the permutation as a matrix-vector multiplication 
	int sum;
	for (int i = 0; i < BLOCK_LEN; i++) {
		sum = 0;
		if (encrypt == true) {
			for (int j = 0; j < BLOCK_LEN; j++) {
				sum = sum + ((int) input[j]) * pMatrix[i][j]; 
			}
		}
		else {
			for (int j = 0; j < BLOCK_LEN; j++) {
				sum = sum + ((int) input[j]) * pMatrixInverse[i][j];
			}
		}
		permuted[i] = (unsigned char) sum;
	}
}

// XOR operation
void SPN::operation_XOR(const unsigned char* input, unsigned char XORed[],
						int numSubkey) {
	for (int i = 0; i < BLOCK_LEN; i++) {
		XORed[i] = input[i] ^ subkeys[numSubkey][i];
	}
}

/***************************************************
 * ENCRYPTION
 ***************************************************
 * Encrypt a string plaintext
 */
unsigned char* SPN::encrypt_ECB_mode(const unsigned char plaintext[], int len){
	int currIndex = 0;

	// Prepare input string
	int numSubInput = (int) (len / BLOCK_LEN); 
	if (len % BLOCK_LEN != 0) { numSubInput++; }
//	unsigned char input[numSubInput][BLOCK_LEN]; // somehow this doesn't work in testing image. possibly because the image is too big
	unsigned char **input;
	input = new unsigned char*[numSubInput];
	for (int i = 0; i < numSubInput; i++) {
		input[i] = new unsigned char[BLOCK_LEN];
	}

	prepare_string_ECB_mode(plaintext, input, len);
	unsigned char* ciphertext = new unsigned char[numSubInput * BLOCK_LEN];
	
	for (int s = 0; s < numSubInput; s++) {
		unsigned char* tmp = SPN_encrypt(input[s]);
		
		// Convert the result back to type string and copy to ciphertext string
		for (int i = 0; i < BLOCK_LEN; i++) {
			ciphertext[currIndex] =  tmp[i];
			currIndex++;
		}

		delete [] tmp;
	}

	for (int i = 0; i < numSubInput; i++) {
		delete [] input[i];
	}
	delete [] input;

	return ciphertext;
}

// Encrypt Algorithm
unsigned char* SPN::SPN_encrypt(const unsigned char in[BLOCK_LEN]) {
	unsigned char* ciphertext = new unsigned char[BLOCK_LEN];
	
	// SPN MAIN ALGORITHM
	unsigned char *XORed, *substituted, *permuted;
	XORed = new unsigned char[BLOCK_LEN];
	substituted = new unsigned char[BLOCK_LEN];
	permuted = new unsigned char[BLOCK_LEN];

	// copy subinput input[s] to permuted as pre-round
	for (int i = 0; i < BLOCK_LEN; i++) {
		permuted[i] = in[i];
	}

	// run through the encryption rounds
	for (int r = 0; r < numRounds - 1; r++) {
		// XOR result of last round with corresponding subkey of current round
		operation_XOR(permuted, XORed, r);

		// Substitution Pi_S()
		pi_S(XORed, substituted);
			
		// Permutation Pi_P()
		pi_P(substituted, permuted, PERMUTATION_ENCRYPT_MODE);
	}
	// the last round does not permute the result, only XOR and pi_S()
	operation_XOR(permuted, XORed, numRounds - 1);
	pi_S(XORed, substituted);

	// Output whitening using the last subkey. Recall that we produce
	// (numRounds + 1) subkeys. The first (numRounds) subkeys have been used.
	operation_XOR(substituted, ciphertext, numRounds);

	// destroy intermediate step's materials
	delete [] XORed;
	delete [] substituted;
	delete [] permuted;

	return ciphertext;
}

/***************************************************
 * DECRYPTION
 ***************************************************
 * Decrypt an array of encrypted ciphertext characters in hexa form
 */
unsigned char* SPN::decrypt_ECB_mode(const unsigned char ciphertext[], int len) {
	int numSubInput = (int) len / BLOCK_LEN;
	unsigned char* plaintext = new unsigned char[len];

	// Prepare input ciphertext
//	unsigned char input[numSubInput][BLOCK_LEN]; // somehow this doesn't work in testing image. possibly because the image is too big
	unsigned char **input;
	input = new unsigned char*[numSubInput];
	for (int i = 0; i < numSubInput; i++) {
		input[i] = new unsigned char[BLOCK_LEN];
	}

	prepare_string_ECB_mode(ciphertext, input, len);
	
	// Decryption
	int currIndex = 0;

	for (int s = 0; s < numSubInput; s++) {
		unsigned char* tmp = SPN_decrypt(input[s]);
		
		// Convert the result back to type string and copy to ciphertext string
		for (int i = 0; i < BLOCK_LEN; i++) {
			plaintext[currIndex] =  (char) ((int) tmp[i]);
			currIndex++;
		}

		delete [] tmp;
	}

	for (int i = 0; i < numSubInput; i++) {
		delete [] input[i];
	}
	delete [] input;

	return plaintext;
}

// Decryption Algorithm
unsigned char* SPN::SPN_decrypt(const unsigned char in[BLOCK_LEN]) {
	unsigned char* plaintext = new unsigned char[BLOCK_LEN];
	
	unsigned char *XORed, *substituted, *permuted;
	XORed = new unsigned char[BLOCK_LEN];
	substituted = new unsigned char[BLOCK_LEN];
	permuted = new unsigned char[BLOCK_LEN];

	// copy subinput input[s] to permuted as pre-round
	for (int i = 0; i < BLOCK_LEN; i++) {
		permuted[i] = in[i];
	}

	// De-whitening
	operation_XOR(permuted, XORed, numRounds);

	// Unwind the last pi_S() and XOR
	pi_S(XORed, substituted);
	operation_XOR(substituted, XORed, numRounds - 1);

	// run through the decryption rounds
	for (int r = numRounds - 2; r > -1; r--) {
		cout << "--------------------------------------------------" << endl;

		// Unwind Permutation Pi_P()
		pi_P(XORed, permuted, PERMUTATION_DECRYPT_MODE); // bool encrypt is false

		// Unwind Substitution Pi_S()
		pi_S(permuted, substituted);
			
		// Unwind XOR of last round with corresponding subkey of current round
		operation_XOR(substituted, XORed, r);
	}

	for (int i = 0; i < BLOCK_LEN; i++) {
		plaintext[i] = XORed[i];
	}

	// destroy intermediate step's materials
	delete [] XORed;
	delete [] substituted;
	delete [] permuted;

	return plaintext;
}


//**************************************************
// Permutation matrix generator for pi_P()
//**************************************************
void SPN::generate_permutation_matrix() {
	srand(time(NULL));
	int row = rand() % BLOCK_LEN;
	bool flag[BLOCK_LEN] = {false}; // flag to know what columns already have a 1

	for (int i = 0; i < BLOCK_LEN; i++) {
		for (int j = 0; j < BLOCK_LEN; j++) {
			pMatrix[i][j] = 0;
			pMatrixInverse[i][j] = 0;
		}
	}
	
	// the <row>th entry of each column is set to 1
	// such that it's the only 1 on that row
	for (int i = 0; i < BLOCK_LEN; i++) {
		row = rand() % BLOCK_LEN;
		// if there's already a 1 on the current row, keep running rand()
		while (flag[row] == true) { 
			row = rand() % BLOCK_LEN;
		}
		flag[row] = true;
		pMatrix[row][i] = 1;
		pMatrixInverse[i][row] = 1; // transpose(pMatrix) = inverse(pMatrix)
	}

	cout << "Permutation Matrix (for Encryption): " << endl;
	for (int i = 0; i < BLOCK_LEN; i++) {
		for (int j = 0; j < BLOCK_LEN; j++) {
			cout << pMatrix[i][j] << "  ";
		}
		cout << endl;
	}
	cout << endl;

	cout << "Inverse Permutation Matrix (for Decryption): " << endl;
	for (int i = 0; i < BLOCK_LEN; i++) {
		for (int j = 0; j < BLOCK_LEN; j++) {
			cout << pMatrixInverse[i][j] << "  ";
		}
		cout << endl;
	}
}

//**************************************************
// Input processor: Turn array input into a 2D array of BLOCK_LEN sub-arrays
//**************************************************
void SPN::prepare_string_ECB_mode(const unsigned char input[],
								  unsigned char in[][BLOCK_LEN], int len) {
	int row = 0, currIndex = 0;
	
	// Process string input into unsigned char array for encryption; 
	for (int i = 0; i < len; i++) {
		if (i % BLOCK_LEN == 0) { currIndex = 0; }
		else { currIndex++;	}
		row = (int) i / BLOCK_LEN;
		in[row][currIndex] = input[i];
	}
	// Pad the last substring with 0's if needed
	if (len % BLOCK_LEN != 0) {
		for (int i = len % BLOCK_LEN; i < BLOCK_LEN; i++) { 
			in[(int) len / BLOCK_LEN][i] = (unsigned char) 0;
		}
	}
}
void SPN::prepare_string_ECB_mode(const unsigned char input[],
								  unsigned char **in, int len) {
	int row = 0, currIndex = 0;
	
	// Process string input into unsigned char array for encryption; 
	for (int i = 0; i < len; i++) {
		if (i % BLOCK_LEN == 0) { currIndex = 0; }
		else { currIndex++;	}
		row = (int) i / BLOCK_LEN;
		in[row][currIndex] = input[i];
	}
	// Pad the last substring with 0's if needed
	if (len % BLOCK_LEN != 0) {
		for (int i = len % BLOCK_LEN; i < BLOCK_LEN; i++) { 
			in[(int) len / BLOCK_LEN][i] = (unsigned char) 0;
		}
	}
}

// print an unsigned char array as hexadecimal values
void SPN::printArray(const unsigned char in[], int len) {
	for (int i = 0; i < len; i++) {
		cout << hex << setw(4) << (int) in[i];
	}
}
