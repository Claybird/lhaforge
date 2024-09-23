#pragma once
#include "archive.h"

// this code is based on xacrett: http://www.kmonos.net/lib/xacrett.ja.html
// subset of LzhDecoder2.h/LzhDecoder2.cpp

#define MAXMATCH	256
#define THRESHOLD	3

#define NT      (16+3)
#define TBIT 	5
#define CBIT	9
#define NC 	    (255+MAXMATCH+2-THRESHOLD)
#define NPT 	0x80

#define N_CHAR      (256+60-THRESHOLD+1)
#define TREESIZE_C  (N_CHAR*2)
#define TREESIZE_P  (128*2)
#define TREESIZE    (TREESIZE_C+TREESIZE_P)
#define ROOT_C      0
#define ROOT_P      TREESIZE_C

enum lzh_method { LH0, ARJ, UNKNOWN };

class CLzhDecoder2
{
	//
	// original source is
	//
	// LHa for UNIX 1.14d
	//   http://www2m.biglobe.ne.jp/~dolphin/lha/lha-unix.htm
	// Zoo 2.10 ( 88/01/27 )
	//   http://www.vector.co.jp/soft/dos/util/se010838.html
	//
public:
	virtual ~CLzhDecoder2() {}
	struct INTERNAL {
		INTERNAL(FILE* in): _in(in){}
		virtual ~INTERNAL() {}
		FILE* _in;
		virtual void decode(std::function<void(const void* buffer, size_t/*data size*/)>) = 0;
	};
	void initDecoder(lzh_method mhd, FILE* infile, DWORD insize, DWORD outsize);
	void decode(std::function<void(const void* buffer, size_t/*data size*/)>);
protected:
	std::shared_ptr<INTERNAL> _decoder;
};

