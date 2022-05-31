#include "stdafx.h"
#include "mini_lzh_decoder.h"

struct UnstoreInternal :CLzhDecoder2::INTERNAL {
	DWORD _cmpsize;
	std::array<char, 65536> _buf;
	UnstoreInternal(FILE* in, DWORD cmpsize) :INTERNAL(in), _cmpsize(cmpsize) {}
	virtual ~UnstoreInternal() {}
	virtual LF_BUFFER_INFO decode()override {
		if (_cmpsize == -1) {
			LF_BUFFER_INFO info;
			info.make_eof();
			int how_much = fread(&_buf[0], 1, _buf.size(), _in);
			if (0 != how_much) {
				info.buffer = &_buf[0];
				info.size = how_much;
				info.offset = _file_offset;
				_file_offset += info.size;
			}
			return info;
		} else {
			LF_BUFFER_INFO info;
			info.make_eof();
			if (_cmpsize > 0) {
				int how_much = std::min(_cmpsize, (DWORD)_buf.size());
				how_much = fread(&_buf[0], 1, how_much, _in);
				if (0 == how_much) {
					return info;
				} else {
					info.buffer = &_buf[0];
					info.size = how_much;
					info.offset = _file_offset;
					_file_offset += info.size;
					_cmpsize -= how_much;
				}
			}
			return info;
		}
	}
};

struct ArjDecodeInternal :CLzhDecoder2::INTERNAL {
	DWORD _count;
	WORD blocksize;
	const int pbit, np, offset;
	enum {
		dicsiz = 26624
	};
	std::array<char, dicsiz> _buf;
	DWORD _loc;
	DWORD dicbit;
	DWORD _cmpsize, _orisize;
	int _i, _j, _k;

	ArjDecodeInternal(FILE* in, DWORD cmpsize, DWORD orisize) :INTERNAL(in),
		_cmpsize(cmpsize),
		_orisize(orisize),
		np(17), pbit(5), offset(0x0100 - 3)
	{
		init_getbits();
		blocksize = 0;
		_count = 0;
		_loc = 0;
		_i = 0;
		_j = 0;
		_k = -1;
	}
	virtual ~ArjDecodeInternal() {}
	virtual LF_BUFFER_INFO decode()override {
		LF_BUFFER_INFO info;
		info.make_eof();
		while(_count < _orisize) {
			if (_k == -1) {
				int c = Decode_C();

				if (c <= 255) {
					_buf[_loc++] = c;
					_count++;
					if (_loc == dicsiz) {
						info.buffer = &_buf[0];
						info.size = dicsiz;
						info.offset = _file_offset;
						_file_offset += info.size;
						_loc = 0;
						return info;
					}
				} else {
					_j = c - offset;
					_i = Decode_P();
					if ((_i = _loc - _i - 1) < 0) {
						_i += dicsiz;
					}
					_k = 0;	//go into decoder loop
				}
			}
			if (_k != -1) {
				//decoder loop
				for (; _k < _j; _k++) {
					_buf[_loc++] = _buf[_i];
					_count++;
					_i = (_i + 1) % dicsiz;
					if (_loc >= dicsiz) {
						info.buffer = &_buf[0];
						info.size = _loc;
						info.offset = _file_offset;
						_file_offset += info.size;
						_loc = 0;
						return info;
					}
				}
				_k = -1;	//get out of decoder loop
			}
		}

		if (_loc != 0) {
			info.buffer = &_buf[0];
			info.size = _loc;
			info.offset = _file_offset;
			_file_offset += info.size;
			_loc = 0;
			_k = -1;
		}
		return info;
	}

	BYTE c_len[NC], pt_len[NPT];
	WORD left[2 * NC - 1], right[2 * NC - 1];
	WORD c_table[4096], pt_table[256];
	WORD Decode_C() {
		WORD j, mask;

		if (blocksize == 0) {
			blocksize = getbits(16);
			read_pt_len(NT, TBIT, 3);
			read_c_len();
			read_pt_len(np, pbit, -1);
		}
		blocksize--;
		j = c_table[bitbuf >> 4];
		if (j < NC)
			fillbuf(c_len[j]);
		else {
			fillbuf(12);
			mask = 1 << (16 - 1);
			do {
				if (bitbuf & mask)	j = right[j];
				else				j = left[j];
				mask >>= 1;
			} while (j >= NC);
			fillbuf(c_len[j] - 12);
		}
		return j;
	}
	WORD Decode_P() {
		WORD j, mask;

		j = pt_table[bitbuf >> (16 - 8)];
		if (j < np)
			fillbuf(pt_len[j]);
		else {
			fillbuf(8);
			mask = 1 << (16 - 1);
			do {
				if (bitbuf & mask)	j = right[j];
				else				j = left[j];
				mask >>= 1;
			} while (j >= np);
			fillbuf(pt_len[j] - 8);
		}
		if (j != 0)
			j = (1 << (j - 1)) + getbits(j - 1);
		return j;
	}

	void read_pt_len(short nn, short nbit, short i_special){
		short i, c, n = getbits((BYTE)nbit);

		if (n == 0) {
			c = getbits((BYTE)nbit);
			for (i = 0; i < nn; i++)	pt_len[i] = 0;
			for (i = 0; i < 256; i++)	pt_table[i] = c;
		} else {
			short i = 0;
			while (i < n) {
				c = bitbuf >> (16 - 3);
				if (c == 7) {
					WORD mask = 1 << (16 - 4);
					while (mask & bitbuf) {
						mask >>= 1;
						c++;
					}
				}
				fillbuf((c < 7) ? 3 : c - 3);
				pt_len[i++] = (BYTE)c;
				if (i == i_special) {
					c = getbits(2);
					while (--c >= 0)
						pt_len[i++] = 0;
				}
			}
			while (i < nn)
				pt_len[i++] = 0;
			make_table(nn, pt_len, 8, pt_table);
		}
	}

	void read_c_len(){
		short i, c, n = getbits(CBIT);

		if (n == 0) {
			c = getbits(CBIT);
			for (i = 0; i < NC; i++)	c_len[i] = 0;
			for (i = 0; i < 4096; i++)	c_table[i] = c;
		} else {
			short i = 0;
			while (i < n) {
				c = pt_table[bitbuf >> (16 - 8)];
				if (c >= NT) {
					WORD mask = 1 << (16 - 9);
					do {
						if (bitbuf & mask)c = right[c];
						else			 c = left[c];
						mask >>= 1;
					} while (c >= NT);
				}
				fillbuf(pt_len[c]);
				if (c <= 2) {
					if (c == 0)		c = 1;
					else if (c == 1)	c = getbits(4) + 3;
					else			c = getbits(CBIT) + 20;
					while (--c >= 0)
						c_len[i++] = 0;
				} else
					c_len[i++] = c - 2;
			}
			while (i < NC)
				c_len[i++] = 0;
			make_table(NC, c_len, 12, c_table);
		}
	}

	void make_table(WORD nchar, BYTE* bitlen, WORD tablebits, WORD* table) {
		WORD count[17], weight[17], start[17], total, * p;
		unsigned int i;
		int j, k, l, m, n, avail;

		avail = nchar;

		for (i = 1; i <= 16; i++) {
			count[i] = 0;
			weight[i] = 1 << (16 - i);
		}

		for (i = 0; i < nchar; i++)
			count[bitlen[i]]++;

		total = 0;
		for (i = 1; i <= 16; i++) {
			start[i] = total;
			total += weight[i] * count[i];
		}
		if ((total & 0xffff) != 0)
			return;//error("Bad table (5)\n");

		m = 16 - tablebits;
		for (i = 1; i <= tablebits; i++) {
			start[i] >>= m;
			weight[i] >>= m;
		}

		j = start[tablebits + 1] >> m;
		k = 1 << tablebits;
		if (j != 0)
			for (i = j; i < (unsigned)k; i++)
				table[i] = 0;

		for (j = 0; j < nchar; j++) {
			k = bitlen[j];
			if (k == 0)
				continue;
			l = start[k] + weight[k];
			if (k <= tablebits) {
				for (i = start[k]; i < (unsigned)l; i++)
					table[i] = j;
			} else {
				p = &table[(i = start[k]) >> m];
				i <<= tablebits;
				n = k - tablebits;
				while (--n >= 0) {
					if (*p == 0) {
						right[avail] = left[avail] = 0;
						*p = avail++;
					}
					if (i & 0x8000)
						p = &right[*p];
					else
						p = &left[*p];
					i <<= 1;
				}
				*p = j;
			}
			start[k] = l;
		}
	}

	// bitwise read&write
	WORD bitbuf; BYTE subbitbuf, bitcount;
	void init_getbits() {
		bitbuf = 0;
		subbitbuf = 0;
		bitcount = 0;
		fillbuf(16);
	}
	void fillbuf(BYTE n) {
		while (n > bitcount) {
			n -= bitcount;
			bitbuf = (bitbuf << bitcount) + (subbitbuf >> (8 - bitcount));
			if (_cmpsize != 0) {
				_cmpsize--;
				subbitbuf = (BYTE)getc(_in);
			} else {
				subbitbuf = 0;
			}
			bitcount = 8;
		}
		bitcount -= n;
		bitbuf = (bitbuf << n) + (subbitbuf >> (8 - n));
		subbitbuf <<= n;
	}
	WORD getbits(BYTE n) {
		WORD x = bitbuf >> (16 - n);
		fillbuf(n);
		return x;
	}
};

void CLzhDecoder2::initDecoder(lzh_method mhd, FILE* infile, DWORD insize, DWORD outsize)
{
	switch (mhd) {
	case ARJ:
		_decoder = std::make_shared<ArjDecodeInternal>(infile, insize, outsize);
		break;
	case LH0:
	default:
		_decoder = std::make_shared<UnstoreInternal>(infile, insize);
		break;
	}
}

LF_BUFFER_INFO CLzhDecoder2::decode()
{
	if (_decoder) {
		return _decoder->decode();
	} else {
		LF_BUFFER_INFO info;
		info.make_eof();
		return info;
	}
}
