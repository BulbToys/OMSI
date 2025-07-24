#include "version.h"
#include "git.h"

// Make sure to rebuild "version.obj" every time you compile your project!
const char* BulbToys::GetBuildDateTime()
{
	return __DATE__ " " __TIME__;
}

int BulbToys::GetBuildNumber()
{
	return GIT_REV_COUNT + 1;
}