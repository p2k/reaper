typedef uchar u8;
typedef ushort u16;
typedef uint u32;
typedef ulong u64;

//u8 that might be uchar or uchar2 depending on the kernel
typedef uchar2 u8_v;

#define U8TO32(p) \
  (((u32)((p)[0].x) << 24) | ((u32)((p)[1].x) << 16) | \
   ((u32)((p)[2].x) <<  8) | ((u32)((p)[3].x)      ))
#define U8TO64(p) \
  (((u64)U8TO32(p) << 32) | (u64)U8TO32((p) + 4))
#define U32TO8(p, v) \
    (p)[0].x = (u8)((v) >> 24); (p)[1].x = (u8)((v) >> 16); \
    (p)[2].x = (u8)((v) >>  8); (p)[3].x = (u8)((v)      );
#define U64TO8(p, v) \
    U32TO8((p),     (u32)((v) >> 32));	\
    U32TO8((p) + 4, (u32)((v)      ));

//#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable

__constant u8 sigma[256] =
{
     0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15 ,
    14,10, 4, 8, 9,15,13, 6, 1,12, 0, 2,11, 7, 5, 3 ,
    11, 8,12, 0, 5, 2,15,13,10,14, 3, 6, 7, 1, 9, 4 ,
     7, 9, 3, 1,13,12,11,14, 2, 6, 5,10, 4, 0,15, 8 ,
     9, 0, 5, 7, 2, 4,10,15,14, 1,11,12, 6, 8, 3,13 ,
     2,12, 6,10, 0,11, 8, 3, 4,13, 7, 5,15,14, 1, 9 ,
    12, 5, 1,15,14,13, 4,10, 0, 7, 6, 3, 9, 2, 8,11 ,
    13,11, 7,14,12, 1, 3, 9, 5, 0,15, 4, 8, 6, 2,10 ,
     6,15,14, 9,11, 3, 0, 8,12, 2,13, 7, 1, 4,10, 5 ,
    10, 2, 8, 4, 7, 6, 1, 5,15,11, 9,14, 3,12,13 ,0 ,
     0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15 ,
    14,10, 4, 8, 9,15,13, 6, 1,12, 0, 2,11, 7, 5, 3 ,
    11, 8,12, 0, 5, 2,15,13,10,14, 3, 6, 7, 1, 9, 4 ,
     7, 9, 3, 1,13,12,11,14, 2, 6, 5,10, 4, 0,15, 8 ,
     9, 0, 5, 7, 2, 4,10,15,14, 1,11,12, 6, 8, 3,13 ,
     2,12, 6,10, 0,11, 8, 3, 4,13, 7, 5,15,14, 1, 9
};

__constant u64 cst[16] =
{
  0x243F6A8885A308D3UL,0x13198A2E03707344UL,0xA4093822299F31D0UL,0x082EFA98EC4E6C89UL,
  0x452821E638D01377UL,0xBE5466CF34E90C6CUL,0xC0AC29B7C97C50DDUL,0x3F84D5B5B5470917UL,
  0x9216D5D98979FB1BUL,0xD1310BA698DFB5ACUL,0x2FFD72DBD01ADFB7UL,0xB8E1AFED6A267E96UL,
  0xBA7C9045F12C7F99UL,0x24A19947B3916CF7UL,0x0801F2E2858EFC16UL,0x636920D871574E69UL
};

__constant u32 K[64] =
{
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

uint rotl(uint x, uint y)
{
	return (x<<y)|(x>>(32-y));
}

#define Ch(x, y, z) (z ^ (x & (y ^ z)))
#define Ma(x, y, z) ((y & z) | (x & (y | z)))

#define Tr(x,a,b,c) (rotl(x,a)^rotl(x,b)^rotl(x,c))
#define Wr(x,a,b,c) (rotl(x,a)^rotl(x,b)^(x>>c))

#define R(x) (work[x] = Wr(work[x-2],15,13,10) + work[x-7] + Wr(work[x-15],25,14,3) + work[x-16])
#define sharound(a,b,c,d,e,f,g,h,x,K) h+=Tr(e,7,21,26)+Ch(e,f,g)+K+x; d+=h; h+=Tr(a,10,19,30)+Ma(a,b,c);
#define sharound_s(a,b,c,d,e,f,g,h,x) h+=Tr(e,7,21,26)+Ch(e,f,g)+x; d+=h; h+=Tr(a,10,19,30)+Ma(a,b,c);

void Sha256_round(u32* s, u8_v* data)
{
	u32 work[64];

	for(u32 i=0; i<16; ++i)
	{
		work[i] = U8TO32(data+i*4);
	}

	u32 A = s[0];
	u32 B = s[1];
	u32 C = s[2];
	u32 D = s[3];
	u32 E = s[4];
	u32 F = s[5];
	u32 G = s[6];
	u32 H = s[7];
#pragma unroll
	for(u32 i=0; i<16; i+=8)
	{
		sharound(A,B,C,D,E,F,G,H,work[i+0],K[i+0]);
		sharound(H,A,B,C,D,E,F,G,work[i+1],K[i+1]);
		sharound(G,H,A,B,C,D,E,F,work[i+2],K[i+2]);
		sharound(F,G,H,A,B,C,D,E,work[i+3],K[i+3]);
		sharound(E,F,G,H,A,B,C,D,work[i+4],K[i+4]);
		sharound(D,E,F,G,H,A,B,C,work[i+5],K[i+5]);
		sharound(C,D,E,F,G,H,A,B,work[i+6],K[i+6]);
		sharound(B,C,D,E,F,G,H,A,work[i+7],K[i+7]);
	}
#pragma unroll
	for(u32 i=16; i<64; i+=8)
	{
		sharound(A,B,C,D,E,F,G,H,R(i+0),K[i+0]);
		sharound(H,A,B,C,D,E,F,G,R(i+1),K[i+1]);
		sharound(G,H,A,B,C,D,E,F,R(i+2),K[i+2]);
		sharound(F,G,H,A,B,C,D,E,R(i+3),K[i+3]);
		sharound(E,F,G,H,A,B,C,D,R(i+4),K[i+4]);
		sharound(D,E,F,G,H,A,B,C,R(i+5),K[i+5]);
		sharound(C,D,E,F,G,H,A,B,R(i+6),K[i+6]);
		sharound(B,C,D,E,F,G,H,A,R(i+7),K[i+7]);
	}
	s[0] += A;
	s[1] += B;
	s[2] +=	C;
	s[3] += D;
	s[4] += E;
	s[5] += F;
	s[6] += G;
	s[7] += H;
}

__constant u32 P[61] =
{
	0xc28a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19c0174,
	0x649b69c1, 0xf9be478a, 0x0fe1edc6, 0x240ca60c, 0x4fe9346f, 0x4d1c84ab, 0x61b94f1e, 0xf6f993db,
	0xe8465162, 0xad13066f, 0xb0214c0d, 0x695a0283, 0xa0323379, 0x2bd376e9, 0xe1d0537c, 0x03a244a0,
	0xfc13a4a5, 0xfafda43e, 0x56bea8bb, 0x445ec9b6, 0x39907315, 0x8c0d4e9f, 0xc832dccc, 0xdaffb65b,
	0x1fed4f61, 0x2f646808, 0x1ff32294, 0x2634ccd7, 0xb0ebdefa, 0xd6fc592b, 0xa63c5c8f, 0xbe9fbab9,
	0x0158082c, 0x68969712, 0x51e1d7e1, 0x5cf12d0d, 0xc4be2155, 0x7d7c8a34, 0x611f2c60, 0x036324af,
	0xa4f08d87, 0x9e3e8435, 0x2c6dae30, 0x11921afc, 0xb76d720e
};

void Sha256_round_padding(u32* s)
{
	u32 A = s[0];
	u32 B = s[1];
	u32 C = s[2];
	u32 D = s[3];
	u32 E = s[4];
	u32 F = s[5];
	u32 G = s[6];
	u32 H = s[7];
#pragma unroll
	for(uint i=0; i<64; i+=8)
	{
		sharound_s(A,B,C,D,E,F,G,H,P[i+0]);
		sharound_s(H,A,B,C,D,E,F,G,P[i+1]);
		sharound_s(G,H,A,B,C,D,E,F,P[i+2]);
		sharound_s(F,G,H,A,B,C,D,E,P[i+3]);
		sharound_s(E,F,G,H,A,B,C,D,P[i+4]);
		sharound_s(D,E,F,G,H,A,B,C,P[i+5]);
		sharound_s(C,D,E,F,G,H,A,B,P[i+6]);
		sharound_s(B,C,D,E,F,G,H,A,P[i+7]);
	}
	s[7] += H;
}

#define ROT(x,n) (((x)<<(64-n))|( (x)>>(n)))

#define G2(arr,val,a,b,c,d,e,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,i)					\
  val[a] += (arr[sigma[i+e]] ^ cst[sigma[i+e+1]]) + val[b];	\
  val[j] += (arr[sigma[i+n]] ^ cst[sigma[i+n+1]]) + val[k];	\
  val[o] += (arr[sigma[i+s]] ^ cst[sigma[i+s+1]]) + val[p];	\
  val[t] += (arr[sigma[i+x]] ^ cst[sigma[i+x+1]]) + val[u];	\
  val[d] = ROT( val[d] ^ val[a],32);				\
  val[m] = ROT( val[m] ^ val[j],32);				\
  val[r] = ROT( val[r] ^ val[o],32);				\
  val[w] = ROT( val[w] ^ val[t],32);				\
  val[c] += val[d];						\
  val[l] += val[m];						\
  val[q] += val[r];						\
  val[v] += val[w];						\
  val[b] = ROT( val[b] ^ val[c],25);				\
  val[k] = ROT( val[k] ^ val[l],25);				\
  val[p] = ROT( val[p] ^ val[q],25);				\
  val[u] = ROT( val[u] ^ val[v],25);				\
  val[a] += (arr[sigma[i+e+1]] ^ cst[sigma[i+e]])+val[b];	\
  val[j] += (arr[sigma[i+n+1]] ^ cst[sigma[i+n]])+val[k];	\
  val[o] += (arr[sigma[i+s+1]] ^ cst[sigma[i+s]])+val[p];	\
  val[t] += (arr[sigma[i+x+1]] ^ cst[sigma[i+x]])+val[u];	\
  val[d] = ROT( val[d] ^ val[a],16);				\
  val[m] = ROT( val[m] ^ val[j],16);				\
  val[r] = ROT( val[r] ^ val[o],16);				\
  val[w] = ROT( val[w] ^ val[t],16);				\
  val[c] += val[d];						\
  val[l] += val[m];						\
  val[q] += val[r];						\
  val[v] += val[w];						\
  val[b] = ROT( val[b] ^ val[c],11);				\
  val[k] = ROT( val[k] ^ val[l],11);				\
  val[p] = ROT( val[p] ^ val[q],11);				\
  val[u] = ROT( val[u] ^ val[v],11);




//assumes input is 512 bytes
__kernel
__attribute__((reqd_work_group_size(128, 1, 1)))
void search(__global uchar* in_param, __global uint* out_param, __global uint* pad32)
{
	u8_v in[512];
//#pragma unroll
	for(uint i=0; i<128; ++i)
		in[i].x = in_param[i];

	uint nonce = get_global_id(0);

	in[108].x = nonce&0xFF;
	in[109].x = (nonce>>8)&0xFF;
	in[110].x = (nonce>>16)&0xFF;
	in[111].x = (nonce>>24)&0xFF;

	u64 h[8];
	h[0]=0x6A09E667F3BCC908UL;
	h[1]=0xBB67AE8584CAA73BUL;
	h[2]=0x3C6EF372FE94F82BUL;
	h[3]=0xA54FF53A5F1D36F1UL;
	h[4]=0x510E527FADE682D1UL;
	h[5]=0x9B05688C2B3E6C1FUL;
	h[6]=0x1F83D9ABFB41BD6BUL;
	h[7]=0x5BE0CD19137E2179UL;

	u64 v[16];
//#pragma unroll
	for(uint i=0; i< 8;++i)  v[i] = h[i];
	v[ 8] = 0x243F6A8885A308D3UL;
	v[ 9] = 0x13198A2E03707344UL;
	v[10] = 0xA4093822299F31D0UL;
	v[11] = 0x082EFA98EC4E6C89UL;
	v[12] = 0x452821E638D01777UL;
	v[13] = 0xBE5466CF34E9086CUL;
	v[14] = 0xC0AC29B7C97C50DDUL;
	v[15] = 0x3F84D5B5B5470917UL;

	u64 m[16];
//#pragma unroll
	for(uint i=0; i<16;++i)  m[i] = U8TO64(in + i*8);
//#pragma unroll
	for(uint i=0; i<256; i+=16)
	{
		G2( m, v, 0, 4, 8,12, 0, 1, 5, 9,13, 2, 2, 6,10,14, 4, 3, 7,11,15, 6, i);
		G2( m, v, 3, 4, 9,14,14, 2, 7, 8,13,12, 0, 5,10,15, 8, 1, 6,11,12,10, i);
	}
//#pragma unroll
	for(uint i=0; i<8;++i)
	{
		h[i] ^= v[i]^v[i+8];
		v[i] = h[i];
	}
	v[8] = 0x243F6A8885A308D3UL;
	v[9] = 0x13198A2E03707344UL;
	v[10] = 0xA4093822299F31D0UL;
	v[11] = 0x082EFA98EC4E6C89UL;
	v[12] = 0x452821E638D01377UL;
	v[13] = 0xBE5466CF34E90C6CUL;
	v[14] = 0xC0AC29B7C97C50DDUL;
	v[15] = 0x3F84D5B5B5470917UL;

	u64 m2[16] = {1UL << 63, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0x400};
//#pragma unroll
	for(uint i=0; i<256; i+=16)
	{
		G2( m2, v, 0, 4, 8,12, 0, 1, 5, 9,13, 2, 2, 6,10,14, 4, 3, 7,11,15, 6, i);
		G2( m2, v, 3, 4, 9,14,14, 2, 7, 8,13,12, 0, 5,10,15, 8, 1, 6,11,12,10, i);
	}

//#pragma unroll
	for(uint i=0; i<8;++i)  h[i] ^= v[i]^v[i+8];

	u8_v* work2 = in+128;

	U64TO8( work2 + 0, h[0]);
	U64TO8( work2 + 8, h[1]);
	U64TO8( work2 +16, h[2]);
	U64TO8( work2 +24, h[3]);
	U64TO8( work2 +32, h[4]);
	U64TO8( work2 +40, h[5]);
	U64TO8( work2 +48, h[6]);
	U64TO8( work2 +56, h[7]);

	u8_v* work3 = work2+64;
//a = x-1, b = x, c = x&63
#define WORKINIT(a,b,c)   work3[a].x ^= work2[c].x; \
		if(work3[a].x&0x80) work3[b].x=in[(b+work3[a].x)&0x7F].x; \
		else              work3[b].x=work2[(b+work3[a].x)&0x3F].x;


	work3[0].x = work2[15].x;
//#pragma unroll
	for(uint x=1;x<320;++x)
	{
		WORKINIT(x-1,x,x&63);
	}

	#define READ_W32(offset) ((u32)work3[offset].x + (((u32)work3[(offset)+1].x)<<8) + (((u32)work3[(offset)+2].x&0x3F)<<16))
	#define PAD_MASK 0x3FFFFF

	u64 qCount =((u64)(work3[310].x))     +
				((u64)(work3[311].x)<<8)  +
				((u64)(work3[312].x)<<16) +
 				((u64)(work3[313].x)<<24) +
                ((u64)(work3[314].x)<<32) +
				((u64)(work3[315].x)<<40) +
				((u64)(work3[316].x)<<48) +
				((u64)(work3[317].x)<<56);


	u32 nExtra=(((u8)pad32[(qCount+work3[300].x)&PAD_MASK])>>3)+512;
	for(u32 x=1;x<nExtra;++x)
	{
		qCount += pad32[qCount&PAD_MASK];

		if(qCount&0x87878700)
			++work3[qCount%320].x;

		qCount -= (u8)pad32[(qCount+work3[qCount%160].x)&PAD_MASK];

		if (qCount&0x80000000)
			qCount += (u8)pad32[qCount&0xFFFF];
		else
			qCount += pad32[qCount&0x20FAFB];

		qCount += pad32[(qCount+work3[qCount%160].x)&PAD_MASK];
		if (qCount&0xF0000000)
			++work3[qCount%320].x;

		qCount += pad32[READ_W32((u8)qCount)];
		work3[x%320].x=work2[x&63].x^((u8)qCount);
		qCount += pad32[((qCount>>32)+work3[x%200].x)&PAD_MASK];
		work3[qCount%316].x     ^= (qCount>>24)&0xFF;
		work3[(qCount%316)+1].x ^= (qCount>>32)&0xFF;
		work3[(qCount%316)+2].x ^= (qCount>>40)&0xFF;
		work3[(qCount%316)+3].x ^= (qCount>>48)&0xFF;
		if ((qCount&7) == 3) ++x;
		qCount -= (u8)pad32[x*x];
		if ((qCount&7) == 1) ++x;
	}

	u32 s[8]= {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
	Sha256_round(s, in);
	Sha256_round(s, in+64);
	Sha256_round(s, in+128);
	Sha256_round(s, in+192);
	Sha256_round(s, in+256);
	Sha256_round(s, in+320);
	Sha256_round(s, in+384);
	Sha256_round(s, in+448);
	Sha256_round_padding(s);

	if ((s[7] & 0x80FFFF) == 0)
	{
		out_param[nonce&0xFF] = nonce;
	}
}
