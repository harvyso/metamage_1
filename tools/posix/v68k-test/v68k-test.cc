/*
	v68k-test.cc
	------------
*/

// Standard C
#include <stdio.h>
#include <string.h>

// POSIX
#include <unistd.h>

// v68k
#include "v68k/endian.hh"
#include "v68k/emulator.hh"


#pragma exceptions off


using v68k::big_word;
using v68k::big_longword;


static void dump( const v68k::emulator& emu )
{
	const v68k::registers& regs = emu.regs;
	
	printf( "\n" );
	
	for ( int i = 0;  i < 8;  ++i )
	{
		printf( "D%d: %.8x   A%d: %.8x\n", i, regs.d[i], i, regs.a[i] );
	}
	
	printf( "\n" );
	
	printf( "Alt SP:  %.8x\n", regs.alt_sp  );
	printf( "Alt SSP: %.8x\n", regs.alt_ssp );
	
	printf( "\n" );
	
	printf( "PC: %.8x\n", regs.pc );
	printf( "SR: %.4x\n", emu.get_SR() );
}


const uint32_t initial_SSP  = 4096;
const uint32_t initial_USP  = 3072;
const uint32_t code_address = 2048;
const uint32_t os_address   = 1024;

static const uint16_t os[] =
{
	0x6006,  // BRA.S  *+8
	
	0x484F,  // BKPT  #7
	
	0x4E72,  // STOP #FFFF  ; finish
	0xFFFF,
	
	0x027C,  // ANDI #DFFF,SR  ; clear S
	0xDFFF,
	
	0x4FF8,  // LEA  (3072).W,A7
	initial_USP,
	
	0x4EB8,  // JSR  0x0800  ; 2048
	0x0800,
	
	0x4e4F   // TRAP  #15
};

const uint32_t bkpt_7_addr = os_address + 2;
const uint32_t finish_addr = os_address + 4;

static const uint16_t program[] =
{
	0x41F8,  // LEA  (???).W,A0
	0xFFFF,
	
	0x7000,  // MOVEQ  #00,D0
	
	0x6002,  // BRA.S *+4
	
	0x4848,  // BKPT  #0
	
	0x1018,  // MOVE.B  (A0)+,D0
	
	0x66FA,  // BNE.S *-4
	
	0x4E75   // RTS
};

static void load_vectors( uint8_t* mem )
{
	uint32_t* vectors = (uint32_t*) mem;
	
	memset( vectors, 0xFF, 1024 );
	
	vectors[0] = big_longword( initial_SSP );  // isp
	vectors[1] = big_longword( os_address  );  // pc
	
	vectors[4] = big_longword( bkpt_7_addr );  // Illegal Instruction
	vectors[8] = big_longword( bkpt_7_addr );  // Privilege Violation
	
	vectors[47] = big_longword( finish_addr );  // Trap 15
}

static void load_code( uint16_t*        dest,
                       const uint16_t*  begin,
                       const uint16_t*  end )
{
	while ( begin < end )
	{
		*dest++ = big_word( *begin++ );
	}
}

static inline void load_n_words( uint8_t*         mem,
                                 uint32_t         offset,
                                 const uint16_t*  begin,
                                 size_t           n_words )
{
	uint16_t* dest = (uint16_t*) (mem + offset);
	
	load_code( dest, begin, begin + n_words );
}

static void load_data( uint8_t* mem )
{
	uint16_t* code = (uint16_t*) (mem + code_address);
	
	uint8_t* data = (uint8_t*) code + sizeof program;
	
	strcpy( (char*) data, "Hello world\n" );
	
	code[1] = big_word( data - mem );
}

static void emulator_test()
{
	uint8_t mem[ 4096 ];
	
	load_vectors( mem );
	load_n_words( mem, code_address, program, sizeof program / 2 );
	load_n_words( mem, os_address,   os,      sizeof os      / 2 );
	load_data   ( mem );
	
	v68k::emulator emu( v68k::mc68000, mem, sizeof mem );
	
	emu.reset();
	
step_loop:
	
	while ( emu.step() )
	{
		continue;
	}
	
	if ( emu.condition == v68k::bkpt_0 )
	{
		const uint32_t c = emu.regs.d[0];
		
		if ( c <= 0x7F )
		{
			putchar( c );
			
			emu.acknowledge_breakpoint( 0x4E71 );  // NOP
		}
		
		goto step_loop;
	}
	
	if ( emu.condition == v68k::finished )
	{
		return;
	}
	
	putchar( '\n' );
	
	const char* condition;
	
	switch ( emu.condition )
	{
		using namespace v68k;
		
		case halted:
			condition = "halted";
			break;
		
		case stopped:
			condition = "stopped";
			break;
		
		case bkpt_0:
		case bkpt_1:
		case bkpt_2:
		case bkpt_3:
		case bkpt_4:
		case bkpt_5:
		case bkpt_6:
		case bkpt_7:
			condition = NULL;
			
			printf( "Breakpoint %d\n", emu.condition - bkpt_0 );
			break;
		
		default:
			condition = "???";
			break;
	}
	
	if ( condition )
	{
		printf( "Processor %s\n", condition );
	}
	
	dump( emu );
}

int main( int argc, char** argv )
{
	emulator_test();
	
	return 0;
}

