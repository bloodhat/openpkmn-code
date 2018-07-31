#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "encryption.h"
#include "packet.h"
#include "rijndael.h"

#define NO_ENCRYPTION 0
#define AES 1
#define AES_EXTERNAL_AUTH 2
#define KEYBITS 128

void aes_encrypt(unsigned char * clear_payload, int clear_len,
		 char * key_data, unsigned char * encrypted_payload,
		 int encrypted_len)
{
  unsigned long rk[RKLENGTH(KEYBITS)];
  unsigned char key[KEYLENGTH(KEYBITS)];
  int block_num;
  int byte_in_block;
  int leftover;
  int nrounds;
  unsigned char clear_buffer[16];
  unsigned char encrypted_buffer[16];

  memcpy(key, key_data, KEYLENGTH(KEYBITS));

  nrounds = rijndaelSetupEncrypt(rk, key, KEYBITS);
  
  block_num = 0;
  /* go through our clear data */
  while(encrypted_len > 0)
    {
      /* break down into 16 byte chunks */
      byte_in_block = 0;
      /* we have chunk space and data remaining */
      leftover = 16; /* 16 bytes in a block */
      while(byte_in_block < 16 && clear_len > 0)
	{
	  clear_buffer[byte_in_block] =
	    clear_payload[byte_in_block + (block_num * 16)];
	  
	  clear_len--;
	  byte_in_block++;
	  leftover--;
	}
      /* clear data underflow, pad with remaining space and pray */
      while(byte_in_block < 16)
	{
	  clear_buffer[byte_in_block] = leftover;
	  byte_in_block++;
	}
      /* encrypt */
      rijndaelEncrypt(rk, nrounds, clear_buffer, encrypted_buffer);

      /* write out */
      memcpy(encrypted_payload + (block_num  * 16), encrypted_buffer, 16);
      encrypted_len += -16;
      
      /* procede to the next block */
      block_num++;
    }
}

void encrypt(void * clear_payload, int clear_len,
	     int key_type, int key_len, void * key_data,
	     void * encrypted_payload, int encrypted_len)
{
  if(key_type == 0)
    {
      memcpy(encrypted_payload, clear_payload, clear_len);
    }
  else if(key_type == AES || key_type == AES_EXTERNAL_AUTH)
    {
      aes_encrypt(clear_payload, clear_len,
		  key_data, encrypted_payload, encrypted_len);
    }
  else
    {
      fprintf(stderr, "encrypt: encryption scheme %i not implemented\n",
	      key_type);
    }
}

void aes_decrypt(unsigned char * decrypted_payload, int * decrypted_len,
		 void * key_data, unsigned char * encrypted_payload,
		 int encrypted_len)
{
  unsigned long rk[RKLENGTH(KEYBITS)];
  unsigned char key[KEYLENGTH(KEYBITS)];
  int block_num;
  int nrounds;
  unsigned char encrypted_buffer[16];
  unsigned char decrypted_buffer[16];

  memcpy(key, key_data, KEYLENGTH(KEYBITS));

  nrounds = rijndaelSetupDecrypt(rk, key, KEYBITS);
  
  *decrypted_len = 0;
  block_num = 0;
  /* go through our encrypted data */
  while(encrypted_len > 0)
    {
      /* break down into 16 byte chunks */
      /* get a chunk */
      memcpy(encrypted_buffer, encrypted_payload + (16 * block_num), 16);
      encrypted_len += -16;
      
      rijndaelDecrypt(rk, nrounds, encrypted_buffer, decrypted_buffer);

      /* write out */
      memcpy(decrypted_payload + (16 * block_num), decrypted_buffer, 16);
      *decrypted_len += 16;
      
      /* procede to the next block */
      block_num++;
    }
}

void decrypt(void * decrypted_payload, int * decrypted_len,
	     int key_type, int key_len, void * key_data,
	     void * encrypted_payload, int encrypted_len)
{
  if(key_type == 0)
    {
      *decrypted_len = encrypted_len;
      memcpy(decrypted_payload, encrypted_payload, *decrypted_len);
    }
  else if(key_type == AES || key_type == AES_EXTERNAL_AUTH)
    {
      aes_decrypt(decrypted_payload, decrypted_len,
		  key_data, encrypted_payload, encrypted_len);
    }
  else
    {
      fprintf(stderr, "decrypt: encryption scheme %i not implemented\n",
	      key_type);
      *decrypted_len = -1;
    } 
}

int verify_payload(int packet_type, void * payload)
{
  Packet * packet = payload;
  int internal_packet_type = ntohl(packet->packet_type);
  fprintf(stderr, "verify_payload: comparing %i to %i\n",
	  internal_packet_type, packet_type);
  return internal_packet_type == packet_type;
}

int calc_max_encrypted_len(int payload_len,
			   int encryption_type)
{
  if(encryption_type == 0)
    {
      return payload_len;
    }
  else if(encryption_type == AES || encryption_type == AES_EXTERNAL_AUTH)
    {
      /* note: initially we would detect if we were a
	 16 byte multiple already and if not just return
	 that.  that's apparantly not the case with PKCS5! */
      return ((payload_len / 16) + 1) * 16;
    }
  else
    {
      fprintf(stderr, "calc_max_encrypted_len: encryption scheme %i not "
	      "implemented\n", encryption_type);
      return -1;
    }
}

int calc_max_decrypted_len(int payload_len,
			   int encryption_type)
{
  if(encryption_type == 0)
    {
      return payload_len;
    }
  else if(encryption_type == AES || encryption_type == AES_EXTERNAL_AUTH)
    {
      return payload_len;
    }
  else
    {
      fprintf(stderr, "calc_max_encrypted_len: encryption scheme %i not implemented\n",
	      encryption_type);
      return -1;
    }
}

int get_key_len(int key_type)
{
  if(key_type == 0)
    {
      return 0;
    }
  else if(key_type == AES || key_type == AES_EXTERNAL_AUTH)
    {
      return 16;
    }
  return -1;
}

void md5_str_to_bytes(unsigned char * bytes, char * str)
{
  int i;
  int str_start;
  int addition;
  for(i = 0; i < 32; i++)
    {
      /* fprintf(stderr, "%c", str[i]); */
    }
  fprintf(stderr, "\n");
  for(i = 0; i < 16; i++)
    {
      str_start = i*2;
      bytes[i] = 0;
      
      if(isalpha(str[str_start + 1]))
	{
	  addition = 10 + toupper(str[str_start+1]) - 'A';
	  /* fprintf(stderr, "addition is %i\n", addition); */
	  bytes[i] += addition;
	}
      else
	{
	  addition = toupper(str[str_start+1]) - '0';
	  /* fprintf(stderr, "addition is %i\n", addition); */
	  bytes[i] += addition;
	}
      if(isalpha(str[str_start]))
	{
	  addition = (10 + toupper(str[str_start]) - 'A');
	  /* fprintf(stderr, "addition is %i * 16\n", addition); */
	  bytes[i] += (addition * 16);
	}
      else
	{
	  addition = toupper(str[str_start]) - '0';
	  /* fprintf(stderr, "addition is %i * 16\n", addition); */
	  bytes[i] += (addition * 16);
	}
      /* fprintf(stderr, "%i: %c%c == %x\n", i, str[str_start],
	 str[str_start+1], bytes[i]); */
    }
}
