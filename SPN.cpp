/* SPN.cpp
 *
 * Implementation of a simple substitution-permutation network.
 *
 * Created by Khoa Nguyen on 03/16/2016.
 */

#include "SPN.h"

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

// Key schedule: A simple function for the key schedule is that for subkey of round r K_r, K_r is a copy of the original key starting from bit 3i + 1, wrapped around if necessary. This is not a secure way to generate key in practice. It's good to demonstrate linear cryptanalysis, however.
void SPN::generate_subkeys() {
	subkeys = new unsigned char*[numRounds + 1];
	for (int i = 0; i < numRounds + 1; ++i) {
		subkeys[i] = new unsigned char[KEY_LEN];
		for (int j = 0; j < KEY_LEN; j++) {
			subkeys[i][j] = key[(j + (3 * i + 1)) % KEY_LEN];
		}
		printArray(subkeys[i], KEY_LEN);
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

/* ENCRYPTION
 *
 * Encrypt a string plaintext
*/
unsigned char* SPN::encrypt(const string plaintext){
	int currIndex = 0;
	
	// Prepare input string
	int n = plaintext.length();
	int numSubInput = (int) (n / BLOCK_LEN); 
	if (n % BLOCK_LEN != 0) { numSubInput++; }

	unsigned char input[numSubInput][BLOCK_LEN];	
	prepare_string_ECB_mode(plaintext, input, numSubInput);
	unsigned char* ciphertext = new unsigned char[numSubInput * BLOCK_LEN];
	
	
	// SPN MAIN ALGORITHM
	unsigned char *XORed, *substituted, *permuted;
	XORed = new unsigned char[BLOCK_LEN];
	substituted = new unsigned char[BLOCK_LEN];
	permuted = new unsigned char[BLOCK_LEN];

	for (int s = 0; s < numSubInput; s++) {
		cout << "\n => Encrypting subinput number " << s << ": \n" << endl;
		// copy subinput input[s] to permuted as pre-round
		for (int i = 0; i < BLOCK_LEN; i++) {
			permuted[i] = input[s][i];
		}

		// run through the encryption rounds
		for (int r = 0; r < numRounds - 1; r++) {
			// XOR result of last round with corresponding subkey of current round
			cout << "XORed_" << r << ": ";
			operation_XOR(permuted, XORed, r);
			printArray(XORed, BLOCK_LEN);
			cout << endl;

			// Substitution Pi_S()
			cout << "subs_" << r << ": ";
			pi_S(XORed, substituted);
			printArray(substituted, BLOCK_LEN);
			cout << endl;
			
			// Permutation Pi_P()
			cout << "perm_" << r << ": ";
			pi_P(substituted, permuted, true); // bool encrypt for pi_P is true
			printArray(permuted, BLOCK_LEN);
			cout << endl;
			
			cout << "--------------------------------------------------" << endl;
		}
		// the last round does not permute the result, only XOR and pi_S()
		cout << "XORed_" << numRounds - 1 << ": ";
		operation_XOR(permuted, XORed, numRounds - 1);
		printArray(XORed, BLOCK_LEN);
		cout << endl;

		cout << "subs_" << numRounds - 1 << ": ";
		pi_S(XORed, substituted);
		printArray(substituted, BLOCK_LEN);
		cout << endl;

		// Output whitening using the last subkey. Recall that we produce
		// (numRounds + 1) subkeys. The first (numRounds) subkeys have been used.
		cout << "Whitening: ";
		operation_XOR(substituted, XORed, numRounds);
		printArray(XORed, BLOCK_LEN);
		cout << endl;
		cout << "--------------------------------------------------" << endl;
		// Convert the result back to type string and copy to ciphertext string
		for (int i = 0; i < BLOCK_LEN; i++) {
			ciphertext[currIndex] =  XORed[i];
			currIndex++;
		}
	}

	// destroy intermediate step's materials
	delete [] XORed;
	delete [] substituted;
	delete [] permuted;

	cout << "----------------- PLAINTEXT  ---------------------" << endl;
	for (int i = 0; i < numSubInput; i++) {
		printArray(input[i], BLOCK_LEN);
	}
	cout << "= " << plaintext << endl;

	cout << "----------------- CIPHERTEXT ---------------------" << endl;
	printArray(ciphertext, numSubInput * BLOCK_LEN);
	cout << endl;

	return ciphertext;
}

/* DECRYPTION
 *
 * Decrypt an array of encrypted ciphertext characters in hexa form
 */
string SPN::decrypt(const unsigned char ciphertext[], const int len) {
	int numSubInput = (int) len / BLOCK_LEN;
	unsigned char input[numSubInput][BLOCK_LEN];
	int currIndex = 0, row = 0;
	static string plaintext;
	plaintext.resize(len);

	// Divide ciphertext array into <numSubInput> subinputs
	for (int i = 0; i < len; i++) {
		if (i % BLOCK_LEN == 0) { currIndex = 0; }
		else { currIndex++;	}
		row = (int) i / BLOCK_LEN;
		input[row][currIndex] = ciphertext[i];
	}

	// Decryption
	currIndex = 0;
	unsigned char *XORed, *substituted, *permuted;
	XORed = new unsigned char[BLOCK_LEN];
	substituted = new unsigned char[BLOCK_LEN];
	permuted = new unsigned char[BLOCK_LEN];

	for (int s = 0; s < numSubInput; s++) {
		cout << "\n => Decrypting subinput number " << s << ": \n" << endl;
		// copy subinput input[s] to permuted as pre-round
		for (int i = 0; i < BLOCK_LEN; i++) {
			permuted[i] = input[s][i];
		}

		// De-whitening
		cout << "--------------------------------------------------" << endl;
		cout << "De-Whitening: ";
		operation_XOR(permuted, XORed, numRounds);
		printArray(XORed, BLOCK_LEN);
		cout << endl;

		// Unwind the last pi_S() and XOR
		cout << "subs_" << numRounds - 1 << ": ";
		pi_S(XORed, substituted);
		printArray(substituted, BLOCK_LEN);
		cout << endl;

		cout << "XORed_" << numRounds - 1 << ": ";
		operation_XOR(substituted, XORed, numRounds - 1);
		printArray(XORed, BLOCK_LEN);
		cout << endl;

		// run through the decryption rounds
		for (int r = numRounds - 2; r > -1; r--) {
			cout << "--------------------------------------------------" << endl;

			// Unwind Permutation Pi_P()
			cout << "perm_" << r << ": ";
			pi_P(XORed, permuted, false); // bool encrypt is false
			printArray(permuted, BLOCK_LEN);
			cout << endl;

			// Unwind Substitution Pi_S()
			cout << "subs_" << r << ": ";
			pi_S(permuted, substituted);
			printArray(substituted, BLOCK_LEN);
			cout << endl;
			
			// Unwind XOR of last round with corresponding subkey of current round
			cout << "XORed_" << r << ": ";
			operation_XOR(substituted, XORed, r);
			printArray(XORed, BLOCK_LEN);
			cout << endl;
		}

		// Convert the result back to type string and copy to ciphertext string
		for (int i = 0; i < BLOCK_LEN; i++) {
			plaintext[currIndex] =  (char) ((int) XORed[i]);
			currIndex++;
		}
	}

	// destroy intermediate step's materials
	delete [] XORed;
	delete [] substituted;
	delete [] permuted;

	cout << "----------------- CIPHERTEXT ---------------------" << endl;
	printArray(ciphertext, len);
	cout << endl;

	cout << "----------------- PLAINTEXT  ---------------------" << endl;
	for (int i = 0; i < plaintext.length(); i++) {
		cout << hex << (int) plaintext[i] << " ";
	}
	cout << "= " << plaintext << endl;


	return plaintext;
}	


// Permutation matrix generator for pi_P()
void SPN::generate_permutation_matrix() {
	srand(time(NULL));
	int row = 0;
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
		while (flag[row] || row == i) { 
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

// Input processor: Turn string input into a 2D array of BLOCK_LEN substrings
void SPN::prepare_string_ECB_mode(string input,
							   unsigned char in[][BLOCK_LEN], int numSubInput) {
	int n = input.length();
	int row = 0, currIndex = 0;

	// Process string input into unsigned char array for encryption; 

	for (int i = 0; i < n; i++) {
		if (i % BLOCK_LEN == 0) { currIndex = 0; }
		else { currIndex++;	}
		row = (int) i / BLOCK_LEN;
		in[row][currIndex] = (unsigned char) input[i];
	}
	// Pad the last substring with 0's if needed
	if (n % BLOCK_LEN != 0) {
		for (int i = n % BLOCK_LEN; i < BLOCK_LEN; i++) { 
			in[numSubInput - 1][i] = (unsigned char) 0;
		}
	}
}

// print an unsigned char array as hexadecimal values
void SPN::printArray(const unsigned char in[], int len) {
	for (int i = 0; i < len; i++) {
		cout << hex << (int) in[i] << " ";
	}
}