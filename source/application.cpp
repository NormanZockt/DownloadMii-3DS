#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <sys/stat.h>
#include <iostream>
#include <sstream>
#include <3ds.h>
#include "main.h"
#include "application.h"
#include "download.h"
#include "gui.h"
#include "utils.h"
#include "file.h"

using namespace std;

FS_archive sdmcArchive;

Result installApp(Application_s app){
	//ToDo
	print("Installing App..\n");
	Result r;
	char buffer[1024];
	u32 size[2];
	FILE *fp;
	/* MKDIR */
	snprintf(buffer,256, "/%s/%s", HBPATH, app.name.c_str());
	mkdir(buffer, 0777);
	/* Download Files */
	char* file3dsx;
	r = downloadFile((char*)app._3dsx.c_str(), &file3dsx, &size[0]);
	if (r != 0) {
		return -1;
	}
#ifdef  _ //Only used when testing binary output 
	for (u32 i = 0; i <= size[0]; i++)
	{
		print("%x", file3dsx[i]);
		print("\n");
	}
#endif
	//ToDo: ProgressBar
	renderGUI();

	char* filesmdh;
	r = downloadFile((char*)app.smdh.c_str(), &filesmdh, &size[1]);
	if (r != 0) {
		return -1;
	}
	/* Save files to the SD-Card */
	//Start with the elf file
	snprintf(buffer,256, "/%s/%s/%s.3dsx", HBPATH, app.name.c_str(), app.name.c_str());
	fp = fopen(buffer, "w+");
	fwrite(file3dsx, sizeof(file3dsx[0]), size[0], fp);
	fclose(fp);
	//Continue with the smdh file
	snprintf(buffer,256, "/%s/%s/%s.smdh", HBPATH, app.name.c_str(), app.name.c_str());
	fp = fopen(buffer, "w+");
	fwrite(filesmdh, sizeof(filesmdh[0]), size[1], fp);
	fclose(fp);
	print("Done Installing app...\n");
	return 0;
}

Result updateInstalledList(vector<Application_s> *list) {
	list->clear();
	Application_s tempApp;
	//Stolen from HBMenu(modified)
	sdmcArchive = (FS_archive) { 0x00000009, (FS_path) { PATH_EMPTY, 1, (u8*)"" } };
	FSUSER_OpenArchive(NULL, &sdmcArchive);
	Handle dirHandle;
	FS_path dirPath = FS_makePath(PATH_CHAR, "/3ds/");
	FSUSER_OpenDirectory(NULL, &dirHandle, sdmcArchive, dirPath);

	static char fullPath[1024];
	static char tempPath[1024];
	char buffer[256];
	u32 entriesRead;
	do
	{
		static FS_dirent entry;
		memset(&entry, 0, sizeof(FS_dirent));
		entriesRead = 0;
		FSDIR_Read(dirHandle, &entriesRead, 1, &entry);
		if (entriesRead)
		{
			strncpy(fullPath, "/3ds/", 1024);
			int n = strlen(fullPath);
			unicodeToChar(&fullPath[n], entry.name, 1024 - n);
			if (entry.isDirectory) //directories
			{
				snprintf(tempPath,1024, "%s", fullPath);
				if (tempPath[5] != '.') {
					print("%s\n", tempPath);
					char* tp = tempPath;
					tp += 5;
					tempApp.name = tp;
					list->push_back(tempApp);
					snprintf(tempPath, 128, "%s/boot.3dsx", fullPath);
				}
			}
		}
	} while (entriesRead);

	FSDIR_Close(dirHandle);
	return -99; //Not implamentet
}