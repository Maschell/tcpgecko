#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <fcntl.h>
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/fs_functions.h"
#include "dynamic_libs/sys_functions.h"
#include "dynamic_libs/vpad_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "kernel/kernel_functions.h"
#include "system/memory.h"
#include "common/common.h"
#include "main.h"
#include "utils/logger.h"
#include "utils/function_patcher.h"
#include "patcher/function_patcher_gx2.h"
#include "patcher/function_patcher_coreinit.h"

#include "tcp_gecko_engine/tcp_gecko.h"

#include "fs/fs_utils.h"
#include "fs/sd_fat_devoptab.h"

bool sdCheatsEnabled __attribute__((section(".data"))) = 0;

typedef enum {
	EXIT,
	TCP_GECKO
} LaunchMethod;

void applyFunctionPatches() {
	PatchInvidualMethodHooks(method_hooks_gx2, method_hooks_size_gx2, method_calls_gx2);
	//PatchInvidualMethodHooks(method_hooks_coreinit, method_hooks_size_coreinit, method_calls_coreinit);
}

void restoreFunctionPatches() {
    RestoreInvidualInstructions(method_hooks_gx2,               method_hooks_size_gx2);
    //RestoreInvidualInstructions(method_hooks_coreinit,          method_hooks_size_coreinit);
    KernelRestoreInstructions();
}

void deInit(){
    restoreFunctionPatches();
}

unsigned char *screenBuffer;

#define PRINT_TEXT(x, y, ...) { snprintf(messageBuffer, 80, __VA_ARGS__); OSScreenPutFontEx(0, x, y, messageBuffer); OSScreenPutFontEx(1, x, y, messageBuffer); }

void initializeScreen() {
	// Init screen and screen buffers
	OSScreenInit();
	unsigned int screenBuffer0Size = OSScreenGetBufferSizeEx(0);
	unsigned int screenBuffer1Size = OSScreenGetBufferSizeEx(1);

	screenBuffer = (unsigned char *) MEM1_alloc(screenBuffer0Size + screenBuffer1Size, 0x40);

	OSScreenSetBufferEx(0, screenBuffer);
	OSScreenSetBufferEx(1, (screenBuffer + screenBuffer0Size));

	OSScreenEnableEx(0, 1);
	OSScreenEnableEx(1, 1);
}

void install() {
	TCPGecko::installCodeHandler();
	log_print("Patching functions\n");
	applyFunctionPatches();
}

int mountSDCard(){
    log_print("Mounting...\n");
    int result = mount_sd_fat(SD_PATH);

    if (result < 0) {
        log_printf("Mounting error: %i\n", result);
        return -1;
    } else {
        log_print("Mounted!\n");
    }
    return 0;
}

int unmountSDCard(){
    log_print("Unmounting...\n");
    s32 result = unmount_sd_fat(SD_PATH);

    if (result < 0) {
        log_printf("Unmounting error: %i\n", result);
        return -1;
    } else {
        log_print("Unmouted!\n");
    }
    return 0;
}

/* Entry point */
int Menu_Main(void) {
	//!*******************************************************************
	//!                   Initialize function pointers                   *
	//!*******************************************************************
	//! do OS (for acquire) and sockets first so we got logging
	InitOSFunctionPointers();
	InitSocketFunctionPointers();
	InitFSFunctionPointers();
	InitVPadFunctionPointers();
	InitSysFunctionPointers();

	log_init();
    DEBUG_FUNCTION_LINE("Starting TCPGecko\n");

	SetupKernelCallback();
    int launchMethod = EXIT;

	if(isInMiiMakerHBL()){

        memoryInitialize();
        VPADInit();
        initializeScreen();

        char messageBuffer[80];

        int shouldUpdateScreen = 1;
        s32 vpadError = -1;
        VPADData vpad_data;

        while (true) {
            VPADRead(0, &vpad_data, 1, &vpadError);

            if (shouldUpdateScreen) {
                OSScreenClearBufferEx(0, 0);
                OSScreenClearBufferEx(1, 0);

                InitSocketFunctionPointers();

                // Build the IP address message
                char ipAddressMessageBuffer[64];
                __os_snprintf(ipAddressMessageBuffer, 64, "Your Wii U's IP address: %i.%i.%i.%i",
                              (hostIpAddress >> 24) & 0xFF, (hostIpAddress >> 16) & 0xFF, (hostIpAddress >> 8) & 0xFF,
                              hostIpAddress & 0xFF);

                PRINT_TEXT(14, 1, "-- TCP Gecko Installer --")
                PRINT_TEXT(7, 2, ipAddressMessageBuffer)
                PRINT_TEXT(0, 5, "Press A to install TCP Gecko (with built-in code handler)...")
                PRINT_TEXT(0, 6, "Press X to install TCP Gecko (with code handler and SD cheats)...")

                PRINT_TEXT(0, 8, "Note:")
                PRINT_TEXT(0, 9, "* You can enable loading SD cheats with Mocha SD access")
                PRINT_TEXT(0, 10, "* Generate and store GCTUs to your SD card with JGecko U")

                // testMount();
                /*if (isSDAccessEnabled()) {
                    PRINT_TEXT2(0, 8, "SD card access: SD cheats will be applied automatically when titles are loaded!")
                } else {
                    PRINT_TEXT2(0, 8, "No SD card access: Please run Mocha SD Access by maschell for SD cheat support...")
                }*/

                PRINT_TEXT(0, 17, "Press Home to exit...")

                OSScreenFlipBuffersEx(0);
                OSScreenFlipBuffersEx(1);
            }

            u32 pressedButtons = vpad_data.btns_d | vpad_data.btns_h;

            // Home Button
            if (pressedButtons & VPAD_BUTTON_HOME) {
                launchMethod = EXIT;

                break;
            } else if (pressedButtons & VPAD_BUTTON_A) {
                install();
                launchMethod = TCP_GECKO;

                break;
            } else if (pressedButtons & VPAD_BUTTON_X) {
                install();
                launchMethod = TCP_GECKO;
                sdCheatsEnabled = true;

                break;
            }

            // Button pressed?
            shouldUpdateScreen = (pressedButtons &
                                  (VPAD_BUTTON_LEFT | VPAD_BUTTON_RIGHT | VPAD_BUTTON_UP | VPAD_BUTTON_DOWN)) ? 1 : 0;
            os_usleep(20 * 1000);
        }

        MEM1_free(screenBuffer);

        memoryRelease();
	}else if (!isInMiiMakerHBL()){ // Boot Application
        if (TCPGecko::isRunningAllowedTitleID()) {
            InitOSFunctionPointers();
            InitSocketFunctionPointers();
            InitGX2FunctionPointers();

            log_init();
            DEBUG_FUNCTION_LINE("OSGetTitleID checks passed...\n");

            if (sdCheatsEnabled) {
                if(mountSDCard() == 0){
                    const char * basePath = "sd:";
                    u64 currentTitleID = OSGetTitleID();
                    char filePath[FS_MAX_ENTNAME_SIZE];
                    memset(filePath, '0', sizeof(filePath));
                    snprintf(filePath,FS_MAX_ENTNAME_SIZE,"%s/%s/%llX.%s",basePath,TCPGECKO_CODE_FOLDER,currentTitleID,TCPGECKO_CODEFILE_EXTENSION);
                    DEBUG_FUNCTION_LINE("Title ID: %llX\n", currentTitleID);
                    DEBUG_FUNCTION_LINE("File Path: %s\n", filePath);

                    if(TCPGecko::applySDCheats(filePath)){
                        //SUCCESS
                    }else{
                        //FAILED
                    }
                    unmountSDCard();
                }
            }

            TCPGecko::startTCPGecko();
        }

        DEBUG_FUNCTION_LINE("Menu_Main(line %d): Patching functions\n");
        applyFunctionPatches();

        return EXIT_RELAUNCH_ON_LOAD;
    }

    if (launchMethod == TCP_GECKO) {
        DEBUG_FUNCTION_LINE("Loading the System Menu\n");
        SYSLaunchMenu();
        return EXIT_RELAUNCH_ON_LOAD;
	}


    DEBUG_FUNCTION_LINE("Return to Homebrew Launcher\n");

    deInit();
    return EXIT_SUCCESS;
}

s32 isInMiiMakerHBL(){
    if (OSGetTitleID != 0 && (
            OSGetTitleID() == 0x000500101004A200 || // mii maker eur
            OSGetTitleID() == 0x000500101004A100 || // mii maker usa
            OSGetTitleID() == 0x000500101004A000 ||// mii maker jpn
            OSGetTitleID() == 0x0005000013374842))
        {
            return 1;
    }
    return 0;
}
