
#include <iostream>
#include <cstdio>
#include <string.h>

using namespace std;

int read_ts_file(char*);
void process_packet(unsigned char*);
void process_pid(unsigned short int , unsigned char *, bool);
unsigned short int read_2_bytes(unsigned char *);
void read_pmt(unsigned char *, bool);
void read_pat(unsigned char *, bool);

