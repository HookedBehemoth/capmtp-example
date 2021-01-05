#include <cstdio>
#include <switch.h>

#include <memory>

int main(int argc, char *argv[])
{
    consoleInit(NULL);

    padConfigureInput(8, HidNpadStyleSet_NpadStandard);

    PadState pad;
    padInitializeDefault(&pad);

    {
        auto raw = new (std::align_val_t(0x1000)) u8[0x1000000];
        auto mem = std::unique_ptr<u8[]>(raw);
        printf("0x%x capmtpInitialize\n", capmtpInitialize(mem.get(), 0x1000000, 101, 11000, 1100, "Applets/Other"));

        auto connect_event = capmtpGetConnectionEvent();
        auto scan_error_event = capmtpGetScanErrorEvent();

        printf("0x%x capmtpStartCommandHandler\n", capmtpStartCommandHandler());
        consoleUpdate(nullptr);

        while (appletMainLoop())
        {
            s32 idx = 0;
            Result rc = waitMulti(&idx, 33'000'000, waiterForEvent(connect_event), waiterForEvent(scan_error_event));
            if (R_SUCCEEDED(rc))
            {
                if (idx == 0)
                {
                    eventClear(connect_event);
                    printf("Device %sconnected\n", capmtpIsConnected() ? "" : "dis");
                }
                else
                {
                    eventClear(scan_error_event);
                    printf("Scan error occurred: 0x%x\n", capmtpGetScanError());
                }

                consoleUpdate(nullptr);
            }

            padUpdate(&pad);
            u64 kDown = padGetButtonsDown(&pad);

            if (kDown & HidNpadButton_Plus)
                break;
        }

        printf("0x%x capmtpStopCommandHandler\n", capmtpStopCommandHandler());
        capmtpExit();
    }

    consoleExit(NULL);

    return EXIT_SUCCESS;
}
