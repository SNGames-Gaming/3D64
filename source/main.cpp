
#include <3ds.h>
#include <cstdio>
#include <stdlib.h>
#include <unistd.h>
#include "debugger.h"
#include "mips.h"
#include "disasm.h"

#define STAGE_IF 1
#define STAGE_DC 2
#define STAGE_EO 4
#define STAGE_MA 8
#define STAGE_WB 16

int main(int argc, char **argv)
{

    if (argc) chdir(argv[0]);

    gfxInitDefault();
    hidInit(NULL);
    consoleInit(GFX_BOTTOM, NULL);

    MIPS_R3000 Cpu;

    FILE *f = fopen("psx_bios.bin", "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    u8 *BiosBuffer = (u8 *)linearAlloc(fsize + 1);
    fread(BiosBuffer, fsize, 1, f);
    fclose(f);


    for (int i = 0; i < fsize; ++i)
    {
        WriteMemByte(&Cpu, RESET_VECTOR + i, BiosBuffer[i]);
    }

    opcode OpCodes[5];
    int Stages[5];

    for (int i = 4; i >= 0; --i)
    {
        Stages[i] = -i;
    }

    u32 MachineCode = 0;
#ifdef ENABLE_DEBUGGER
    if (DebuggerOpen())
    {
        printf("Could not start debugger client!\n");
    }
    else
    {
        printf("Started debugger\n");
    }

#endif

    bool Step = false;

    while (aptMainLoop())
    {
        hidScanInput();

        if (keysDown() & KEY_START)
            break;

        Step = false;
        if (keysUp() & KEY_A)
            Step = true;

        if (Step)
        {
            for (int i  = 0; i < 5; ++i)
            {
                if (Stages[i] < 1)
                {
                    ++Stages[i];
                }

                if (Stages[i] == STAGE_IF)
                {
                    InstructionFetch(&Cpu, &MachineCode);
                    Stages[i] = STAGE_DC;
                    continue;
                }
                if (Stages[i] == STAGE_DC)
                {
                    DecodeOpcode(&Cpu, &OpCodes[i], MachineCode, Cpu.pc - 4);
                    Stages[i] = STAGE_EO;
                    continue;
                }
                if (Stages[i] == STAGE_EO)
                {
                    ExecuteOpCode(&Cpu, &OpCodes[i]);
                    Stages[i] = STAGE_MA;
                    continue;
                }
                if (Stages[i] == STAGE_MA)
                {
                    MemoryAccess(&Cpu, &OpCodes[i]);
                    Stages[i] = STAGE_WB;
                    continue;
                }
                if (Stages[i] == STAGE_WB)
                {
                    WriteBack(&Cpu, &OpCodes[i]);
                    Stages[i] = STAGE_IF;
                    continue;
                }
            }
        }

        printf("\x1b[0;0H");
        DisassemblerPrintRange(&Cpu, Cpu.pc - (5 * 4), 29);

        gfxFlushBuffers();
        gfxSwapBuffersGpu();
        gspWaitForVBlank();
    }
#ifdef ENABLE_DEBUGGER
    DebuggerClose();
#endif

    // Exit services
    gfxExit();
    hidExit();
    return 0;
}



