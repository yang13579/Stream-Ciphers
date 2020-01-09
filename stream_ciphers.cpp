/*
 * cipher.cpp
 *
 *  Created on: Nov 2, 2019
 *      Author: Yutong Yang
 */
#include <iostream>
#include <cctype>
#include <cmath>
#include <cassert>

#ifndef MARMOSET_TESTING
int main();
#endif
char *encode(char *plaintext, unsigned long key);
char *decode(char *ciphertext, unsigned long key);
bool is_valid_ciphertext(char *str);
bool is_valid_plaintext(char *str);


#ifndef MARMOSET_TESTING
int main(){
	char str[] = "Hello world!";
	char str1[] {"A Elbereth Gilthoniel\nsilivren penna miriel\n"
			"o menel aglar elenath!\nNa-chaered palan-diriel\n"
			"o galadhremmin ennorath,\nFanuilos, le linnathon\n"
			"nef aear, si nef aearon!"};
	char str2[] {"4m + 2"};
	char str3[] {"study"};
	std::cout << str1 << std::endl;
	char *ciphertext = encode(str1, 51323);
	char *decode_text = decode(ciphertext,51323);
	std::cout << std::endl;
	std::cout << ciphertext << std::endl;
	std::cout << std::endl;
	std::cout << decode_text << std::endl;

	delete[] ciphertext;
	ciphertext = nullptr;
	delete[] decode_text;
	decode_text = nullptr;

	return 0;
}
#endif


char *encode(char *plaintext, unsigned long key){
	unsigned int null_index = 0;
	for(int x = 0; plaintext[x] != '\0'; x++){
		null_index++;
	}

	// put the plaintext into a new char array and add \0 to the end if the size of the plaintext is not a multiple of 4
	int size = std::ceil(null_index/4.0)*4;
	char input[size];
	for(int x = 0; x < null_index; x++){
		input[x] = plaintext[x];
	}
	if(null_index % 4 != 0){
		for(int x = 0; x < (4-null_index % 4); x++){
			input[null_index+x] = '\0';
		}
	}

	// create the state array s with size of 256 and assign the initial value in each index
	unsigned char s[256];
	for(std::size_t x = 0; x < 256; x++){
		s[x] = x;
	}


	// Randomize the entries in the array using the given key
	int i = 0;
	int j = 0;

	for(int x = 0; x < 256; x++){
		unsigned int k = i % 64;
		//j = (j + s[i] + (key >> k)&1) % 256;
		unsigned long digit = 1;
		digit <<= k;

		if((digit & key) > 0){
			j = (j + s[i] + 1) % 256;
		} else{
			j = (j + s[i]) % 256;
		}

		// swap Si and Sj
		unsigned char temp = s[i];
		s[i] = s[j];
		s[j] = temp;

		i = (i + 1) % 256;
	}


	unsigned int count = 0;
	unsigned char R = '\0';
	// calculate value of R for every byte in plaintext
	//for(std::size_t x = 0; plaintext[x] != '\0'; x++){
	for(std::size_t x = 0; x < size; x++){
		i = (i+1) % 256;
		j = (j + s[i]) % 256;

		// Swap Si and Sj
		unsigned char temp = s[i];
		s[i] = s[j];
		s[j] = temp;

		int r = (s[i] + s[j]) % 256;
		R = s[r];

		// Exclusive-or R with each byte in the plaintext
		input[x] ^= R;

		count++;
	}

	int integer_number = count / 4;
	int capacity = std::ceil(count / 4.0)*5+1;
	char *encode_array = new char[capacity];
	int count_four_char_integer = 0;


	// Ascii armour the plaintext
	for(int m = 0; m < integer_number * 4; m = m+4){
		// get every bit in every 4 bytes of chars
		unsigned char fourth = static_cast<unsigned char>(input[m+3]);
		unsigned char third = static_cast<unsigned char>(input[m+2]);
		unsigned char second = static_cast<unsigned char>(input[m+1]);
		unsigned char first = static_cast<unsigned char>(input[m]);

		// convert 4 bytes chars into one int
		unsigned int four_char_integer = fourth + (third << 8) + (second << 16) + (first << 24);

		int base_eighty_fives[5];
		for(int x = 4; x >= 0; x--){
			base_eighty_fives[x] = four_char_integer % 85;
			four_char_integer /= 85;
		}
		// convert each digit of the base 85 number into printable characters
		for(std::size_t y = 0; y < 5; y++){
			char encoded_char = base_eighty_fives[y] + '!';
			encode_array[count_four_char_integer*5 + y] = encoded_char;
		}
		count_four_char_integer++;
	}

	encode_array[capacity-1] = '\0';

	assert(is_valid_ciphertext(encode_array));
	return encode_array;
}



char *decode(char *ciphertext, unsigned long key){
	// count the capacity of ciphertext
	int count = 0;
	for(int x = 0; ciphertext[x] != '\0'; x++){
		count++;
	}

	int first_digit = 0;
	int second_digit = 0;
	int third_digit = 0;
	int fourth_digit = 0;
	int fifth_digit = 0;
	int ints_capacity = std::ceil(count / 5.0);
	int place_in_decode_array = 0;
	unsigned int decode_ints[ints_capacity];
	for(int x = 0; ciphertext[x] != '\0'; x += 5){
		unsigned int save_base_eighty_five = 0;

		// convert the first char into base 85 integer.
		first_digit = ciphertext[x] - '!';

		// convert the second char
		if(ciphertext[x+1] != '\0')
			second_digit = ciphertext[x+1] - '!';
		else{
			save_base_eighty_five = first_digit*85*85*85*85;
			decode_ints[place_in_decode_array] = save_base_eighty_five;
			break;
		}

		// convert the third char
		if(ciphertext[x+2] != '\0')
			third_digit = ciphertext[x+2] - '!';
		else{
			save_base_eighty_five = first_digit*85*85*85*85 + second_digit*85*85*85;
			decode_ints[place_in_decode_array] = save_base_eighty_five;
			break;
		}

		// convert the fourth char
		if(ciphertext[x+3] != '\0')
			fourth_digit = ciphertext[x+3] - '!';
		else{
			save_base_eighty_five = first_digit*85*85*85*85 + second_digit*85*85*85 + third_digit*85*85;
			decode_ints[place_in_decode_array] = save_base_eighty_five;
			break;
		}

		// convert the fifth char
		if(ciphertext[x+4] != '\0')
			fifth_digit = ciphertext[x+4] - '!';
		else{
			save_base_eighty_five = first_digit*85*85*85*85 + second_digit*85*85*85 + third_digit*85*85 + fourth_digit*85;
			decode_ints[place_in_decode_array] = save_base_eighty_five;
			break;
		}

		// convert the five digits base-85 number into integer
		save_base_eighty_five = fifth_digit + fourth_digit*85 + third_digit*85*85
				                             + second_digit*85*85*85 + first_digit*85*85*85*85;
		decode_ints[place_in_decode_array] = save_base_eighty_five;
		place_in_decode_array++;
	}

	// Create the decode array which store each byte of the integer as characters
	int decode_capacity = ints_capacity * 4;
	char *decode_array = new char[decode_capacity];
	for(int x = 0; x < ints_capacity; x++){
		char  first_char = (decode_ints[x] & 0b11111111000000000000000000000000) >> 24;
		char second_char = (decode_ints[x] & 0b00000000111111110000000000000000) >> 16;
		char  third_char = (decode_ints[x] & 0b00000000000000001111111100000000) >> 8;
		char fourth_char = decode_ints[x] & 0b00000000000000000000000011111111;
		decode_array[4*x] = first_char;
		decode_array[4*x+1] = second_char;
		decode_array[4*x+2] = third_char;
		decode_array[4*x+3] = fourth_char;
	}



	// create the state array s with size of 256 and assign the initial value in each index
	unsigned char s[256];
	for(std::size_t x = 0; x < 256; x++){
		s[x] = x;
	}

	// Randomize the entries in the array using the given key
	int i = 0;
	int j = 0;

	for(int x = 0; x < 256; x++){
		unsigned int k = i % 64;
		//j = (j + s[i] + (key >> k)&1) % 256;
		unsigned long digit = 1;
		digit <<= k;

		if((digit & key) > 0){
			j = (j + s[i] + 1) % 256;
		} else{
			j = (j + s[i]) % 256;
		}

		// swap Si and Sj
		unsigned char temp = s[i];
		s[i] = s[j];
		s[j] = temp;

		i = (i + 1) % 256;
	}

	unsigned char R = '\0';
	// calculate value of R for every byte in plaintext
	for(std::size_t x = 0; x < decode_capacity; x++){
		i = (i+1) % 256;
		j = (j + s[i]) % 256;

		// Swap Si and Sj
		unsigned char temp = s[i];
		s[i] = s[j];
		s[j] = temp;

		int r = (s[i] + s[j]) % 256;
		R = s[r];

		// Exclusive-or R with each byte in the plaintext
		decode_array[x] ^= R;
	}

	decode_array[decode_capacity] = '\0';
	assert(is_valid_plaintext(decode_array));
	return decode_array;
}

bool is_valid_ciphertext(char *str){
	int x = 0;
	while(str[x] != '\0'){
		if(str[x] < '!' || str[x] > 'u')
			return false;

		x++;
	}
	if(x % 5 != 0)
		return false;

	return true;
}

bool is_valid_plaintext(char *str){
	for(int x = 0; str[x] != '\0'; x++){
		if(!std::isprint(str[x]) && !std::isspace(str[x]))
			return false;
	}
	return true;
}














