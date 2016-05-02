/* testSPN.cpp
 *
 * Testing of a simple substitution-permutation network.
 *
 * Created by Khoa Nguyen on 03/16/2016.
 *
 */

#include <fstream>
#include "SPN-1-0.h"
#include "SPN-1-0-debug.h"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/types_c.h"

#define NUM_CHANNELS 3 // number of color channels in each pixel of image

using namespace std;
using namespace cv;

void generate_data();
void testSPN_string();
void testSPN_image();

int main() {
	generate_data();
	testSPN_image();
    testSPN_string();
	return 0;		 
}

void generate_data() {
	srand(time(NULL));
	ofstream output;
	output.open("data.csv");
	int numDataPts = 64;
	unsigned char in[numDataPts], *out;
	for (int i = 0; i < numDataPts; i++) {
		in[i] = (unsigned char) (rand() % 256);
	}

	SPN tmp(8);
	out = tmp.encrypt_ECB_mode(in, numDataPts);

	output << "Plain ," << "Cipher" << endl;
	for (int i = 0; i < numDataPts; i++) {
		output << (int) in[i] << "," << (int) out[i] << endl;
	}

	delete [] out;
	output.close();
}

void testSPN_string() {
    SPN_Debug tmp(8);
    string cont = "y";
    string plaintext;
    
    cout << "Enter plaintext: ";
    getline(cin, plaintext);
    
    while (true) {
        cout << endl;
        
        unsigned char p[plaintext.length()];
        for (int i = 0; i < plaintext.length(); i++) {
            p[i] = (unsigned char) plaintext[i];
        }
        
        cout << "--------------------------------------------------" << endl;
        cout << "* ENCRYPTION *************************************" << endl;
        cout << "--------------------------------------------------" << endl;
        unsigned char* cipher = tmp.encrypt_ECB_mode(p, plaintext.length());
        cout << endl;
        
        cout << "--------------------------------------------------" << endl;
        cout << "* DECRYPTION *************************************" << endl;
        cout << "--------------------------------------------------" << endl;
        int cipherLen = plaintext.length();
        if (plaintext.length() % BLOCK_LEN != 0) {
            cipherLen = cipherLen - (plaintext.length() % BLOCK_LEN) + BLOCK_LEN;
        }
        tmp.decrypt_ECB_mode(cipher, cipherLen);
        delete [] cipher;
        cout << endl;
        
        cout << "Continue? (y/n)" << endl;
        cin >> cont;
        if (cont != "y" && cont != "Y") break;
        cout << "Enter plaintext: ";
        cin.get();
        getline(cin, plaintext);
    }
}


void testSPN_image() {
	SPN newSPN(8);
	string filename;
	cout << "Enter an image file's name: " << endl;
	getline(cin, filename);
	Mat plain = imread(filename, CV_LOAD_IMAGE_COLOR);

	if (!plain.data) {
		cout << "ERROR: Can't load image." << endl;
		return;
	}

	int len = plain.rows * plain.cols;
	unsigned char **tmp_plain = new unsigned char*[NUM_CHANNELS];
	for (int i = 0; i < NUM_CHANNELS; i++) {
		tmp_plain[i] = new unsigned char[len];
	}
	unsigned char **tmp_encrypted = new unsigned char*[NUM_CHANNELS];
	int currIndex = 0;
	
	for (int i = 0; i < plain.rows; i++) {
		for (int j = 0; j < plain.cols; j++) {
			for (int k = 0; k < NUM_CHANNELS; k++) {
				tmp_plain[k][currIndex] = plain.at<Vec3b>(i, j)[k];
			}
			currIndex++;
		}
	}

	for (int i = 0; i < NUM_CHANNELS; i++) {
		tmp_encrypted[i] = newSPN.encrypt_ECB_mode(tmp_plain[i], len);
	}
	
	Mat encrypted;
	encrypted.create(plain.rows + 1, plain.cols, plain.type());

	int row = 0, cipherLen = len;
	currIndex = 0;
	if (len % BLOCK_LEN != 0) {
		cipherLen = cipherLen - (len % BLOCK_LEN) + BLOCK_LEN;
	}
	
	for (int i = 0; i < cipherLen; i++) {
		if (i % plain.cols == 0) { currIndex = 0; }
		else { currIndex++;	}
		row = (int) i / plain.cols;
		for (int k = 0; k < NUM_CHANNELS; k++) {
			encrypted.at<Vec3b>(row, currIndex)[k] = tmp_encrypted[k][i];
		}
	}

	namedWindow("orig");
	imshow("orig", plain);
    namedWindow("encrypted");
    imshow("encrypted", encrypted);

	string resultFile(filename);
	resultFile += "_result.jpg";
	imwrite(resultFile, encrypted);

	for (int i = 0; i < NUM_CHANNELS; i++) {
		delete [] tmp_encrypted[i];
	}

	for (int i = 0; i < NUM_CHANNELS; i++) {
		delete [] tmp_plain[i];
	}

	delete [] tmp_encrypted;
	delete [] tmp_plain;
}
