#!/bin/sh

gcc -g -o  openpkmn_engine $(mysql_config --cflags) -Wall  -pedantic events.c server.c engine.c mechanics.c  environment.c packet.c eutil.c datatypes.c db_iface.c ../share/entropy.c ../share/netutil.c ../share/packet.c ../share/db_iface.c ../share/encryption.c ../share/rijndael.c $(mysql_config --libs)
