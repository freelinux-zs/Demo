

#ifndef __FLASH_ID__
#define __FLASH_ID__

typedef struct {
	int flash_id;
	const char *flash_name;
}FLASH_DRV;

FLASH_DRV hynix = {
	.flash_name = "hynix",
	.flash_id = 0x1122,
};

FLASH_DRV jst = {
	.flash_name = "jst",
	.flash_id = 0x2233,
};

FLASH_DRV *flash_list[] = {
	&hynix,
	&jst,
};




#endif
