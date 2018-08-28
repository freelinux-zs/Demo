#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <limits.h>
#include <termios.h>
#include <errno.h>
#include <linux/input.h>
//#include "log.h"
 
//#define TOUCHKEY_TAG             "TouchkeyDemo "
#define TOUCHKEY_FILE            "/dev/input/event15"
 
//#define PL_LOGE(fmt, args...)    printf(fmt, ##args)
//#define PL_LOGI(fmt, args...)    printf(fmt, ##args)

#define TOUCHKEY_TAG  "ctptouch"

int main(int argc, char **argv)
{
	int nFd;
	int nRet;
	struct input_event *sInputEvent;
 
//	PL_LOGI(TOUCHKEY_TAG, " --> Make time: %s %s %s <--\n ", __DATE__, __TIME__, "Tony.Huang");
	printf("111\n");	
	sInputEvent = malloc(sizeof(struct input_event));
	nFd = open(TOUCHKEY_FILE, O_RDWR);
	if(nFd < 0)
	{
		//PL_LOGE(TOUCHKEY_TAG, "open file %s error.", TOUCHKEY_FILE);
		printf("open file errror\n");
		return -1;
	}
	else
	{
		printf("open file success\n");
		//PL_LOGI(TOUCHKEY_TAG, "open file %s success.", TOUCHKEY_FILE);
	}
 
	memset(sInputEvent, 0, sizeof(struct input_event));
	printf("start reed !!\n");
	nRet = read(nFd, sInputEvent, sizeof(struct input_event));
	printf("end read\n");
	if(nRet < 0)
	{
		//PL_LOGE(TOUCHKEY_TAG, "read file %s error.", TOUCHKEY_FILE);
		printf("reead file error\n");
		return nRet;
	}
 
//	PL_LOGI(TOUCHKEY_TAG, "type: 0x%x ", sInputEvent->type);
//	PL_LOGI(TOUCHKEY_TAG, "code: 0x%x ", sInputEvent->code);
//	PL_LOGI(TOUCHKEY_TAG, "value: 0x%x", sInputEvent->value);
	printf("value:%f\n",sInputEvent->value); 
	free(sInputEvent);
	close(nFd);
	return 0;
}
