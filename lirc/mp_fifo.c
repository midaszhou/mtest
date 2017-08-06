#include <stdio.h> //printf
#include <fcntl.h> //open() 
#include <unistd.h> //close()
#include <errno.h> //perror
#include <stdlib.h> //exit
#include <string.h> //strlen


static int write_to_fifo(int fd_slave,const char *str_cmd)
{
   int nwrite;
   static char str_buf[100]={0};
   int len=strlen(str_cmd);
   strcpy(str_buf,str_cmd);
   str_buf[len]='\r';
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


int main(int argc, char* argv[])
{
  const char* str_fifo="/home/fa/slave"; // slave fifo for mplayer
  char str_buf[100]={0};
  int fd_slave;
  int nread,nwrite;

  if( (fd_slave=open(str_fifo,O_RDWR | O_NONBLOCK))<0)
  {
  	perror("Open mplayer slave FIFO");
	close(fd_slave);
	exit(-1);
   }

   write_to_fifo(fd_slave,argv[1]);

  close(fd_slave);
  exit(1);
}
