#include "../utils/function_patcher.h"
#include "../utils/logger.h"
#include "function_patcher_coreinit.h"

DECL(int, FSOpenFile, void *pClient, void *pCmd, const char *path, const char *mode, int *fd, int errHandling) {
	log_printf("FSOpenFile(): %s\n", path);
	return real_FSOpenFile(pClient, pCmd, path, mode, fd, errHandling);
}

DECL(int, FSOpenFileAsync, void *pClient, void *pCmd, const char *path, const char *mode, int *handle, int error, const void *asyncParams) {
	log_printf("FSOpenFileAsync(): %s\n", path);
	return real_FSOpenFileAsync(pClient, pCmd, path, mode, handle, error, asyncParams);
}

DECL(int, FSOpenDir, void *pClient, void *pCmd, const char *path, s32 *dh, int errHandling) {
	log_printf("FSOpenDir(): %s\n", path);
	return real_FSOpenDir(pClient, pCmd, path, dh, errHandling);
}

DECL(int, FSOpenDirAsync, void *pClient, void *pCmd, const char *path, int *handle, int error, void *asyncParams) {
	log_printf("FSOpenDirAsync(): %s\n", path);
	return real_FSOpenDirAsync(pClient, pCmd, path, handle, error, asyncParams);
}

DECL(int, FSGetStat, void *pClient, void *pCmd, const char *path, FSStat *stats, int errHandling) {
	log_printf("FSGetStat(): %s\n", path);
	return real_FSGetStat(pClient, pCmd, path, stats, errHandling);
}

DECL(int, FSGetStatAsync, void *pClient, void *pCmd, const char *path, FSStat *stats, int errHandling, void *asyncParams) {
	log_printf("FSGetStatAsync(): %s\n", path);
	return real_FSGetStatAsync(pClient, pCmd, path, stats, errHandling, asyncParams);
}

hooks_magic_t method_hooks_coreinit[] __attribute__((section(".data"))) = {
		MAKE_MAGIC(FSOpenFile, LIB_CORE_INIT, STATIC_FUNCTION),
		MAKE_MAGIC(FSOpenFileAsync, LIB_CORE_INIT, STATIC_FUNCTION),

		MAKE_MAGIC(FSOpenDir, LIB_CORE_INIT, STATIC_FUNCTION),
		MAKE_MAGIC(FSOpenDirAsync, LIB_CORE_INIT, STATIC_FUNCTION),

		MAKE_MAGIC(FSGetStat, LIB_CORE_INIT, STATIC_FUNCTION),
		MAKE_MAGIC(FSGetStatAsync, LIB_CORE_INIT, STATIC_FUNCTION),
};

u32 method_hooks_size_coreinit __attribute__((section(".data"))) = sizeof(method_hooks_coreinit) / sizeof(hooks_magic_t);

volatile u32 method_calls_coreinit[sizeof(method_hooks_coreinit) / sizeof(hooks_magic_t) * FUNCTION_PATCHER_METHOD_STORE_SIZE] __attribute__((section(".data")));
