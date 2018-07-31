#ifndef __OPENPKMN_ERRORS__
#define __OPENPKMN_ERRORS__ 1

/* In some cases, it is possible that an error
   error is not severe enough such the the
   client session must not be terminated. */
#define RECOVERABLE_ERROR 0

/* Other errors indicated that the communication
   with the client must stop.  The reasons typically
   are a seeming loss of connectivity or an inability
   to access the database (which would make it
   impossible to properly form packets */
#define SEND_ERROR -1
#define RECEIVE_ERROR -2
#define INVALID_CREDENTIAL_ERROR -4
#define BAD_USER_ID_ERROR -5

#endif
