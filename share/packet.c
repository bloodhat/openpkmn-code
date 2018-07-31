#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "packet.h"
#include "db_iface.h"
#include "encryption.h"
#include "netutil.h"

int send_package(int sock, char * user_name, void * payload, int payload_len)
{
  Header header;

  void * encrypted_payload;
  int encrypted_len;
  int key_type;
  unsigned char * key_data;
  int key_len;

  /* get user key data */

  if(get_user_key_type(user_name, &key_type) < 0)
    {
      fprintf(stderr, "could not get key type\n");
      return DATABASE_ERROR;
    }

  key_len = get_key_len(key_type);

  /* if we are dealing with a nonzero length key
     look it up */
  if(key_len > 0)
    {
      fprintf(stderr, "encryption is required\n");
      key_data = malloc(key_len);
      if(key_data == NULL)
	{
	  fprintf(stderr, "could not allocated key data\n");
	  return -1;
	}
      else
	{
	  if(get_user_key_data(user_name, key_data) < 0)
	    {
	      fprintf(stderr, "could not get key data\n");
	      return DATABASE_ERROR;
	    }
	}
    }
  /* get maximum encrypted length from key type */
  encrypted_len = calc_max_encrypted_len(payload_len, key_type);
  fprintf(stderr, "making encrypted data buffer of %i\n", encrypted_len);
  if((encrypted_payload = malloc(encrypted_len)) == NULL)
    {
      fprintf(stderr, "could not allocated encrypted buffer\n");
      return -1;
    }
  else
    {
      /* put encrypted data into this buffer */
      encrypt(payload, payload_len, key_type, key_len, key_data,
	      encrypted_payload, encrypted_len);
      fprintf(stderr, "encryption successful\n");
      /* set up header */
      strcpy(header.user_name, user_name);
      header.encryption_type = htonl(key_type);
      header.packet_type = ((Packet *)payload)->packet_type;
      header.packet_len = htonl(encrypted_len);
      /* send header */ 
      if(send_all(sock, &header, sizeof(header), 0) < 0)
	{
	  perror("send_all");
	  return SEND_ERROR;
	}
      /* send out remainder of packet */
      if(send_all(sock, encrypted_payload, encrypted_len, 0) < 0)
	{
	  perror("send_all");
	  return SEND_ERROR;
	}
  
      /* free unneeded packet data */
      if(key_len > 0)
	{
	  free(key_data);
	}
      free(encrypted_payload);
    }
  return 1;
}

int receive_package_unverified(int sock, char * user_name, void ** payload,
			       int * payload_len, int * packet_type)
{
  Header header;
  void * encrypted_payload;
  int encrypted_len;

  /* receive header */
  if(recv_all(sock, &header, sizeof(header), 0) < 0)
    {
      perror("recv_all");
      return RECEIVE_ERROR;
    }
  /* get encrypted length from header, ready buffer */
  encrypted_len = ntohl(header.packet_len);
  if((encrypted_payload = malloc(encrypted_len)) == NULL)
    {
      fprintf(stderr, "could not allocate encrypted payload\n");
      return -1;
    }
  else
    {
      /* receive remainder of packet into buffer */
      if(recv_all(sock, encrypted_payload, encrypted_len, 0) < 0)
	{
	  perror("recv_all");
	  return RECEIVE_ERROR;
	}
      
      *payload = encrypted_payload;
      *payload_len = encrypted_len;
      
      strncpy(user_name, header.user_name, 32);
      *packet_type = htonl(header.packet_type);
    }
  return 1;
}

int verify_package(char * user_name, void ** payload, int * payload_len,
		   void * encrypted_payload, int encrypted_len, int packet_type)
{
  void * decrypted_payload;
  int decrypted_len;
  
  int key_type;
  int key_len;
  unsigned char * key_data;
  /* get key out of database */
  
  if(get_user_key_type(user_name, &key_type) < 0)
    {
      return DATABASE_ERROR;
    }
  
  key_len = get_key_len(key_type);

  if(key_len > 0)
    {
      key_data = malloc(key_len);
      
      if(key_data == NULL)
	{
	  fprintf(stderr, "could not allocate key data\n");
	  return -1;
	}
      else
	{
	  if(get_user_key_data(user_name, key_data) < 0)
	    {
	      return DATABASE_ERROR;
	    }
	}
    }

  /* get maximal decrypted size to ready buffer */
  decrypted_len = calc_max_decrypted_len(encrypted_len, key_type);
  if(decrypted_len == -1)
    {
      return RECEIVE_ERROR;
    }
  
  if((decrypted_payload = malloc(decrypted_len)) == NULL)
    {
      fprintf(stderr, "could not allocate decrypetd payload");
      return -1;
    }
  else
    {
      /* decrypt, taking into account actual size */
      decrypt(decrypted_payload, &decrypted_len, key_type, key_len, key_data,
	      encrypted_payload, encrypted_len);
      /* we don't need the encrypted payload anymore */
      
      if(decrypted_len == -1)
	{
	  return RECEIVE_ERROR;
	}
      
      if(!verify_payload(packet_type, (Packet *)decrypted_payload))
	{
	  fprintf(stderr, "packet failed verification: header: %i\n",
		  packet_type);
	  
	  return RECEIVE_ERROR;
	}
      
      *payload = decrypted_payload;
      *payload_len = decrypted_len;
      if(key_len > 0)
	{
	  free(key_data);
	}
    }
  return 1;
}

int receive_package(int sock, char * user_name, void ** payload,
		    int * payload_len)
{
  void * encrypted_payload;
  void * decrypted_payload;
  int encrypted_len;
  int decrypted_len;
  int packet_type;
  
  if(receive_package_unverified(sock, user_name, &encrypted_payload,
				&encrypted_len,	&packet_type) < 0)
    {
      return -1;
    }

  if(verify_package(user_name, &decrypted_payload, &decrypted_len,
		    encrypted_payload, encrypted_len, packet_type) < 0)
    {
      return -1;
    }
  
  free(encrypted_payload);
  *payload = decrypted_payload;
  *payload_len = decrypted_len;
  return 1;
 }



int send_error(int sock, char * user_name)
{
  Server_response_packet packet;
  packet.packet_type = htonl(SERVER_RESPONSE_PACKET);
  packet.response_code =  htonl(ERR_CODE); 

  if(send_package(sock, user_name, &packet, sizeof(packet)) < 0)
    {
      return SEND_ERROR;
    }
  return 1;
}

int send_ack(int sock, char * user_name)
{
  Server_response_packet packet;
  packet.packet_type = htonl(SERVER_RESPONSE_PACKET);
  packet.response_code = htonl(ACK_CODE);
  
  if(send_package(sock, user_name, &packet, sizeof(packet)) < 0)
    {
      return SEND_ERROR;
    }
  return 1;
}

int send_engine_params(int sock, char * opponent_name, int port,
		       char * user_name)
{
  Engine_params_pkt packet;
  packet.packet_type = htonl(ENGINE_PARAMS_PACKET);
  packet.port = htonl(port);
  strncpy(packet.opponent_name, opponent_name, 32);

  if(send_package(sock, user_name, &packet, sizeof(packet)) < 0)
    {
      perror("send_ack");
      return SEND_ERROR;
    }
  fprintf(stderr, "ack sent\n");
  return 1;
}
