#!/bin/sh

gcc -g  -o openpkmn_waitingroom $(mysql_config --cflags) -W -Wall -pedantic waitingroom.c ../share/netutil.c ../share/encryption.c ../share/rijndael.c ../share/packet.c ../share/db_iface.c ../share/entropy.c packet.c datatypes.c db_iface.c stats.c -lpthread $(mysql_config --libs)
