#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //read
#include <fcntl.h> //--O_RDONLY,open
#include <media/lirc.h>
#include <sys/ioctl.h> //ioctl
#include <stdint.h>
#include <string.h>
#include <errno.h>

#define LIRC_DAT_PAUSE 0xc2
#define LIRC_DAT_NEXT 0x02
#define LIRC_DAT_PREV  0x22
#define LIRC_DAT_PLUS 0xa8
#define LIRC_DAT_MINUS 0xe0
#define LIRC_DAT_CH 0x62
#define LIRC_DAT_CH_MINUS 0xa2
#define LIRC_DAT_CH_PLUS 0xe2

/*-------------------------------------------------
  write commands to mplayer slave fifo
-------------------------------------------------*/
static int write_to_fifo(int fd_slave,const char *str_cmd)
{
   int nwrite,nread;
   static char str_buf[100]={0};
   int len=strlen(str_cmd);
   strcpy(str_buf,str_cmd);

   //----- read out all commands first to avoid fifo jam ----
   nread = read(fd_slave,str_buf,20);
   if(nread>0)
   {
//   	str_buf[nread]='\r'; //--put and end to command string
   	str_buf[nread+1]='\n'; // -- for mplayer to seperate commands
   	printf("read out jammed commands: %s\n",str_buf);
   }

   //----- write commands to mplayer slave fifo ------
   str_buf[len]='\r'; //--put and end to command string
   str_buf[len+1]='\n';
   if( (nwrite = write(fd_slave,str_buf,len+2)) <= 0 )
   {
        if(errno == EAGAIN)
        {
            printf("The FIFO is busy or has not been read out yet, Please try write later!\n");
            return nwrite;
        }
        else
        {
            perror("Write to mplayer slave FIFO");
            return nwrite;
        }
   }
   else
   {
        printf("Finish wirting %s to FIFO with nwirte=%d\n",str_cmd,nwrite);
        return nwrite;

    }

}


uint8_t getLircData(const uint32_t* lirc_buf)
{
	int i;
	uint32_t lirc_codes=0;//--decoded lirc code  [userID][userID'][DATA][~DATA]
	uint8_t lirc_data=0;  //--- received lirc DATA =lirce_codes[15:8]

	for(i=0;i<32;i++) //--read [8bit_UserID][8bit_UserID2][8bit_DATA][8bit_~DATA]
	{
		if( *(lirc_buf+2*i+3+1) > 1200 )
			lirc_codes+=(1<<(31-i)); //--check if it's 1.
	}
	printf("----- LIRC CODEs: 0x%08x -----\n",lirc_codes);

	//--extract LIRC DATA ---
	lirc_data=(lirc_codes&(0xff00))>>8;
	if( lirc_data+(lirc_codes&(0xff)) == 0xFF)//---complement check  
		printf("----- LIRC DATA: 0x%02x -----\n",lirc_data);
	else
	{
		printf("----- LIRC DATA: complement check fails! -----\n");
		lirc_data=0;
	}

	return lirc_data;
}


void setVolume(int vol)
{
    static char strCMD[50];
    sprintf(strCMD,"amixer set 'Line Out' %d", vol);
    system(strCMD);
}


void main(void)
{
  //--------- for LIRC dev ---
  char *dev_lirc="/dev/lirc0";
  int fd_lirc;// file handle to lirc dev.
  int vol=15; //0-31 volume
  uint32_t lirc_buf[100]={0}; //--4bytesx68=272bytes,  for raw lirc data from hardware IR receiver
  uint8_t  lirc_data; //--- received lirc DATA =lirce_codes[15:8]


  //-------- for mplayer slave FIFO
  const char* str_fifo="/home/fa/slave"; // slave fifo for mplayer
  char str_buf[100]={0};//command string buf
  int fd_slave; //file handle to slave FIFO
  int nread,nwrite; 


  int ret;
  int param;
  int i,j;


   //--------open lirc device ------
  if((fd_lirc=open(dev_lirc, O_RDWR)) <0 )
  {
	perror("Open LIRC dev");
	close(fd_lirc);
	exit(-1);
   }
  else
	printf("Open %s successfully!\n",dev_lirc);

  //--------- open mplayer slave FIFO --------
  if( (fd_slave=open(str_fifo,O_RDWR | O_NONBLOCK))<0)
  {
        perror("Open mplayer slave FIFO");
        close(fd_slave);
        exit(-1);
   }
   else
	printf("Open %s successfully!\n",str_fifo);


   //------  read some defaults ------
  ioctl(fd_lirc,LIRC_GET_FEATURES,&param);
  printf("LIRC_GET_FEATURES: 0x%08x\n",param);

  ioctl(fd_lirc,LIRC_GET_LENGTH,&param);
  printf("LIRC_GET_LENGTH: 0x%08x\n",param);

  ioctl(fd_lirc,LIRC_GET_MIN_TIMEOUT,&param);
  printf("LIRC_GET_MIN_TIMEOUT: %d ms\n",param);

  ioctl(fd_lirc,LIRC_GET_MAX_TIMEOUT,&param);
  printf("LIRC_GET_MAX_TIMEOUT: %d ms\n",param);

  //---------------- loop reading LIRC data  -------------------
  while(1)
  {
    ret=read(fd_lirc,lirc_buf,sizeof(lirc_buf));
    printf("ret=%d\n",ret);


    if(ret>270 && lirc_buf[0]>0x20000) //if receive complete signal && preamble signal long enough
    {
	//---preamble---
	printf("---preamble: 0x%08x\n",lirc_buf[0]);

	lirc_data=getLircData(lirc_buf);

	switch(lirc_data)
	{
		case LIRC_DAT_CH_PLUS:  //----change channel
			setVolume(vol);
			system("killall mplayer");
			printf("screen -dmS MPLAY_LIST /home/fa/mplaylist\n");
			system("screen -dmS MPLAY_LIST /home/fa/mplaylist");
			break;
		case LIRC_DAT_CH_MINUS:  //----change channel
			setVolume(vol);
			system("killall mplayer");
			printf("screen -dmS MPLAY_FOX /home/fa/mplayfox\n");
			system("screen -dmS MPLAY_FOX /home/fa/mplayfox");
			break;
		case LIRC_DAT_NEXT:
			printf("/home/fa/mnext\n");
			//system("/home/fa/mnext");
			write_to_fifo(fd_slave,"pt_step 1");
			break;
		case LIRC_DAT_PREV:
			printf("/home/fa/mprev\n");
			write_to_fifo(fd_slave,"pt_step -1");
			//system("/home/fa/mprev");
			break;
		case LIRC_DAT_PLUS:
			printf("Volume Up\n");
			if(vol<30)
				vol+=3;
			setVolume(vol);
			break;
		case LIRC_DAT_MINUS:
			printf("Volume Down\n");
			if(vol>3)
				vol-=3;
			setVolume(vol);
			break;
		case LIRC_DAT_PAUSE:
			printf("pause mplayer\n");
			//system("/home/fa/mpause");
			write_to_fifo(fd_slave,"pause");
			break;

	}

    }

    else
	    usleep(200000);

   }

 close(fd_slave);
 close(fd_lirc);

}
 



