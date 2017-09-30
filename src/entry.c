#include "main.h"
#include "dynamic_libs/socket_functions.h"
#include "dynamic_libs/gx2_functions.h"
#include "utils/logger.h"
#include "common/common.h"

int entry() {
	//! *******************************************************************
	//! *                 Jump to our application                    *
	//! *******************************************************************
	return Menu_Main();
}

int __entry_menu(int argc, char **argv) {
	return entry();
}
