#include "RSHash.h"
#include "Blake512.h"
#include "SHA256.h"
#include <stdint.h>
#include <iostream>
using std::cout;
using std::endl;

#define PHI 0x9e3779b9
#define BLOCKHASH_1_PADSIZE (1024*1024*4)

typedef unsigned int uint32;
typedef unsigned long long int uint64;

static uint32 BlockHash_1_Q[4096],BlockHash_1_c,BlockHash_1_i;
unsigned char *BlockHash_1_MemoryPAD8;
uint32 *BlockHash_1_MemoryPAD32_to_8;
uint32 *BlockHash_1_MemoryPAD32;

uint32 BlockHash_1_rand(void)
{
	uint32& pos = BlockHash_1_Q[BlockHash_1_i&0xFFF];
    uint64 t = 0x495ELL * pos + BlockHash_1_c;
    BlockHash_1_c = (t >> 32);
    uint32 x = (t + BlockHash_1_c)&0xFFFFFFFF;
    if (x < BlockHash_1_c)
    {
        x++;
        BlockHash_1_c++;
    }
    ++BlockHash_1_i;
    return (pos = 0xffffffff + (~x));
}


#include <cstdio>
#include <iomanip>

void BlockHash_Init()
{
	try 	
	{ 	
		static unsigned char SomeArrogantText1[]="Back when I was born the world was different. As a kid I could run around the streets, build things in the forest, go to the beach and generally live a care free life. Sure I had video games and played them a fair amount but they didn't get in the way of living an adventurous life. The games back then were different too. They didn't require 40 hours of your life to finish. Oh the good old days, will you ever come back?";
		static unsigned char SomeArrogantText2[]="Why do most humans not understand their shortcomings? The funny thing with the human brain is it makes everyone arrogant at their core. Sure some may fight it more than others but in every brain there is something telling them, HEY YOU ARE THE MOST IMPORTANT PERSON IN THE WORLD. THE CENTER OF THE UNIVERSE. But we can't all be that, can we? Well perhaps we can, introducing GODria, take 2 pills of this daily and you can be like RealSolid, lord of the universe.";
		static unsigned char SomeArrogantText3[]="What's up with kids like artforz that think it's good to attack other's work? He spent a year in the bitcoin scene riding on the fact he took some other guys SHA256 opencl code and made a miner out of it. Bravo artforz, meanwhile all the false praise goes to his head and he thinks he actually is a programmer. Real programmers innovate and create new work, they win through being better coders with better ideas. You're not real artforz, and I hear you like furries? What's up with that? You shouldn't go on IRC when you're drunk, people remember the weird stuff.";
		BlockHash_1_MemoryPAD8 = new unsigned char[BLOCKHASH_1_PADSIZE+8];  //need the +8 for memory overwrites
		BlockHash_1_MemoryPAD32_to_8 = (uint32*)BlockHash_1_MemoryPAD8;

		BlockHash_1_Q[0] = 0x6970F271;
		BlockHash_1_Q[1] = uint32(0x6970F271ULL + PHI);
		BlockHash_1_Q[2] = uint32(0x6970F271ULL + PHI + PHI);
		for (int i = 3; i < 4096; i++)  BlockHash_1_Q[i] = BlockHash_1_Q[i - 3] ^ BlockHash_1_Q[i - 2] ^ PHI ^ i;
		BlockHash_1_c=362436;
		BlockHash_1_i=0;

		int count1=0,count2=0,count3=0;
		for(int x=0;x<(BLOCKHASH_1_PADSIZE/4)+2;x++)  BlockHash_1_MemoryPAD32_to_8[x] = BlockHash_1_rand();
		for(int x=0;x<BLOCKHASH_1_PADSIZE+8;x++)
		{
			switch(BlockHash_1_MemoryPAD8[x]&3)
			{
				case 0: BlockHash_1_MemoryPAD8[x] ^= SomeArrogantText1[count1++]; if(count1>=sizeof(SomeArrogantText1)) count1=0; break;
				case 1: BlockHash_1_MemoryPAD8[x] ^= SomeArrogantText2[count2++]; if(count2>=sizeof(SomeArrogantText2)) count2=0; break;
				case 2: BlockHash_1_MemoryPAD8[x] ^= SomeArrogantText3[count3++]; if(count3>=sizeof(SomeArrogantText3)) count3=0; break;
				case 3: BlockHash_1_MemoryPAD8[x] ^= 0xAA; break;
			}
		}
		BlockHash_1_MemoryPAD32 = new uint32[BLOCKHASH_1_PADSIZE];
		for(uint32 i=0; i<BLOCKHASH_1_PADSIZE; ++i)
			BlockHash_1_MemoryPAD32[i] = *(uint32*)(BlockHash_1_MemoryPAD8+i);
	} 	
	catch(std::exception s) 	
	{ 		
		cout << "(3) Error: " << s.what() << endl; 	
	}
}

void BlockHash_DeInit()
{
    delete[] BlockHash_1_MemoryPAD8;
	delete[] BlockHash_1_MemoryPAD32;
}

const uint32 PAD_MASK = BLOCKHASH_1_PADSIZE-1;
typedef unsigned char uchar;

#define READ_PAD8(offset) BlockHash_1_MemoryPAD8[(offset)&PAD_MASK]
#define READ_PAD32(offset) (*((uint32*)&BlockHash_1_MemoryPAD8[(offset)&PAD_MASK]))


typedef unsigned int uint;

void BlockHash_1(unsigned char *p512bytes, unsigned char* final_hash)
{
    //0->127   is the block header      (128)
    //128->191 is blake(blockheader)    (64)
    //192->511 is scratch work area     (320)

    unsigned char *work1 = p512bytes;
    unsigned char *work2=work1+128;
    unsigned char *work3=work1+192;

    blake512_hash(work2,work1);

    //setup the 320 scratch with some base values
#define WORKINIT(a,b,c)   work3[a] ^= work2[c]; \
        if(work3[a]&0x80) work3[b]=work1[(b+work3[a])&127]; \
        else              work3[b]=work2[(b+work3[a])&63];

	work3[0] = work2[15];
    for(int x=1;x<320;x++)
    {
		WORKINIT(x-1, x, x&63);
    }
	
	uint64 qCount = *((uint64*)&work3[310]);
    int nExtra=(READ_PAD8(qCount+work3[300])>>3)+512;
    for(int x=1;x<nExtra;x++)
    {
        qCount+= READ_PAD32( qCount );
        if(qCount&0x87878700)        work3[qCount%320]++;

        qCount-= READ_PAD8( qCount+work3[qCount%160] );
        if(qCount&0x80000000)   { qCount+= READ_PAD8( qCount&0x8080FFFF ); }
        else                    { qCount+= READ_PAD32( qCount&0x7F60FAFB ); }

        qCount+= READ_PAD32( qCount+work3[qCount%160] );
        if(qCount&0xF0000000)        work3[qCount%320]++;

        qCount+= READ_PAD32( *((uint32*)&work3[qCount&0xFF]) );
		work3[x%320]=work2[x&63]^uchar(qCount);

        qCount+= READ_PAD32( (qCount>>32)+work3[x%200] );
        *((uint32*)&work3[qCount%316]) ^= (qCount>>24)&0xFFFFFFFF;
        if((qCount&0x07)==0x03) x++;
        qCount-= READ_PAD8( (x*x) );
        if((qCount&0x07)==0x01) x++;
	}

	Sha256(work1, final_hash);
}
