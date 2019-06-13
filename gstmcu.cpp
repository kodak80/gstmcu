#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include "Vgstmcu.h"
#include "verilated.h"
#include "verilated_vcd_c.h"

static Vgstmcu *tb;
static VerilatedVcdC *trace;
static int tickcount;

void tick(int c) {
	tb->clk32 = c;
	tb->eval();
	trace->dump(tickcount++);
}

void print(bool rise) {
    std::cout << (rise ? "rise" : "fall");
    std::cout << "- hsc : " << std::setw(3) << (int)tb->hsc;
    std::cout << " hsync " << bool(tb->HSYNC_N);
    std::cout << " vsync " << bool(tb->VSYNC_N);
    std::cout << std::endl;
}

void write_reg(int addr, int data)
{
    tb->AS_N = 0;
    tb->UDS_N = 0;
    tb->LDS_N = 0;
    tb->A = addr >> 1;
    tb->DIN = data;
    tb->RW = 0;
    while (tb->DTACK_N) {
	tick(0);
	tick(1);
    }
    tb->RW=1;
    tb->AS_N=1;
    tb->UDS_N=1;
    tb->LDS_N=1;
    tick(0);
    tick(1);

}

void dump(bool ntsc, bool mde0, bool mde1) {
	int steps = 128*2048*8;
	bool disp;

	write_reg(0xff820a, ntsc ? 0 : 0x200);
	write_reg(0xff8260, (mde1 ? 0x200 : 0) | (mde0 ? 0x100 : 0));
	std::cout << "=========================" << std::endl;
	std::cout << "NTSC : " << ntsc << " mde0 : " << mde0 << " mde1: " << mde1 << std::endl;
	std::cout << "=========================" << std::endl;

	disp = false;
	while(steps--) {
	    tick(0);
	    if (tb->hsc == 127) disp = true;
	    if (disp) print(true);
	    tick(1);
	    if (disp) print(false);
	}

}

int main(int argc, char **argv) {

	char steps = 250;
	// Initialize Verilators variables
	Verilated::commandArgs(argc, argv);
//	Verilated::debug(1);
	Verilated::traceEverOn(true);
	trace = new VerilatedVcdC;
	tickcount = 0;

	// Create an instance of our module under test
	tb = new Vgstmcu;
	tb->trace(trace, 99);
	trace->open("gstmcu.vcd");

	tb->interlace = 0;
	tb->AS_N = 1;
	tb->UDS_N = 1;
	tb->LDS_N = 1;
	tb->RW = 1;
	tb->VMA_N = 1;
	tb->MFPINT_N = 1;

	tb->FC0 = 0;
	tb->FC1 = 1;
	tb->FC2 = 1;
	tb->resb = 1;
	tb->porb = 1;
	tick(0);
	tick(1);
	tb->resb = 0;
	tick(0);
	tick(1);
	tb->resb = 1;
	tb->SREQ = 1;
	dump(false,false,false);
	dump(true,false,true);
	dump(true,true,false);

	trace->close();
}
