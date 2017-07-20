/* Copyright (c) 2013-2016 Jeffrey Pfau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include <mgba-util/common.h>
#include <mgba/core/version.h>

#include <3ds.h>
#include <string.h>

int main() {
	gfxInitDefault();
	gspWaitForVBlank();

	uint64_t pid;
	char path[0x200] = { 0 };

	APT_GetProgramID(&pid);
	snprintf(path, sizeof(path) - 1, "/%s/.%016llx.bin", projectName, pid);

	u8 hmac[0x20];
	memset(hmac, 0, sizeof(hmac));

	romfsMount(NULL);
	int infd = open("romfs:/rom.bin", O_RDONLY);
	int outfd = open(path, O_WRONLY | O_CREAT | O_TRUNC);

	char buffer[8192];
	ssize_t size;
	while ((size = read(infd, buffer, sizeof(buffer))) > 0) {
		write(outfd, buffer, size);
	}
	gspWaitForVBlank();
	close(outfd);
	close(infd);
	romfsExit();
	gfxExit();

	APT_PrepareToDoApplicationJump(0, 0x00040000001A1E00ULL, MEDIATYPE_SD);
	APT_DoApplicationJump(path, sizeof(path), hmac);

	while (aptMainLoop()) {
		svcSleepThread(10000);
	}
	return 0;
}
