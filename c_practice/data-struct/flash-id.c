#include <stdio.h>
#include "flash-id.h"



int main(void)
{
	int id;
	int i;
	FLASH_DRV *flash_drv = NULL;
	unsigned int flash_count = sizeof(flash_list) / sizeof(FLASH_DRV *);
	
	if(flash_count == 0)
		printf("none flash check\n");
	
	printf("flash_count =  %d\n",flash_count);
	printf("please input flash_id\n");
	scanf("%x",&id);
		
	for(i = 0; i < flash_count; i++){
		flash_drv = flash_list[i];
		if (flash_drv->flash_id == id){
			printf("id ok: %s\n",flash_drv->flash_name);
		
		}
	}
	
	#if 0
	while(1){
		printf("please input flash_id\n");
		scanf("%x",&flash_id);
	
		printf("flash_id :%x\n",flash_id);
	}
	#endif
	
	
	
	return 0;
}

