#ifndef __OPENPKMN_SHARE_ENCRYPTION__
#define __OPENPKMN_SHARE_ENCRYPTION__ 1

void encrypt(void * payload, int payload_len,
	     int key_type, int key_len, void * key_data,
	     void * encrypted_payload, int encrypted_len);

void decrypt(void * decrypted_payload, int * decrypted_len,
	     int key_type, int key_len, void * key_data,
	     void * encrypted_payload, int encrypted_len);
  
int verify_payload(int packet_type, void * payload);

int calc_max_encrypted_len(int payload_len,
			   int encryption_type);

int calc_max_decrypted_len(int payload_len,
			   int encryption_type);

int get_key_len(int key_type);

void md5_str_to_bytes(unsigned char * bytes, char * str);

#endif
