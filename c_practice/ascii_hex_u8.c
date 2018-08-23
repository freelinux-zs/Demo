u8 AsciiToHexU8(u8 high,u8 low,u8*number)
{
	u8 err=0,dat;
	dat=high;
	err=AsciiToHex(&dat);if(err)return 1;
	*number=dat<<8;
	dat=low;
	err=AsciiToHex(&dat);if(err)return 1;
	*number|=dat;
	return err;
}
//0 OK 1 FAULT
u8 AsciiToHex(u8*ascii)
{
	if((*ascii>=0x30)&&(*ascii<=0x39)){*ascii -=0x30;return 0;}
	if((*ascii>=0x41)&&(*ascii<=0x46)){*ascii -=0x37;return 0;}
	return 1;
}
//返回ASCII 
u8 HexToAscii(u8 hex)
{
	u8 ascii;
	if(hex<10)ascii=0x30+hex;else ascii=0x37+hex;
	return ascii;
}
