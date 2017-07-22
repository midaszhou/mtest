/*----------------------------------------------------------
 With reference to:  http://forgotfun.org
 佐须之男的博客
----------------------------------------------------------*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MMAP_PATH "/dev/mem"

#define PIO_REG_BASE (0x01C20800)

#define PG_DAT_OFFSET (0xE8)
#define PG0_CONF_OFFSET (0xD8)  //PG7-0   configure register 0
#define PG1_CONF_OFFSET (0xDC)  //PG13-8  configure register 0
#define PG0_MULT_DRV_OFFSET (0xEC) //PG Multi_driving register 0 n=0~13  defaulx 0x55555555
#define PG0_PULL_OFFSET (0xF4) //PG PULL register 0  n=0~13

uint8_t* gpio_mmap_reg=NULL; 
int gpio_mmap_fd = 0;

static int gpio_mmap(void)
{
	if((gpio_mmap_fd=open(MMAP_PATH,O_RDONLY))<0 )
	{
		fprintf(stderr,"unable to open mmap file");
		return -1;
	}
	else
		printf("open /dev/mem successfully!\n");
	printf("pagesize=%d\n",getpagesize());
	gpio_mmap_reg=(uint8_t *)mmap(NULL,getpagesize(),PROT_READ | PROT_WRITE,MAP_SHARED,gpio_mmap_fd,0); //0x01C20800); //PIO_REG_BASE);
	if(gpio_mmap_reg == MAP_FAILED){
		perror("mmap gpio reg");
		fprintf(stderr,"Failed to mmap\n");
		gpio_mmap_reg=NULL;
		close(gpio_mmap_fd);
		return -1;
	}

 	return 0;
}


uint32_t read_gpio_reg(int gpio_reg_offset)
{
	uint32_t reg_dat=0;
	reg_dat=*(uint32_t *)(gpio_mmap_reg+gpio_reg_offset);

	return reg_dat;
}

void write_gpio_reg(int gpio_reg_offset,uint32_t gpio_reg_data)
{
	*(uint32_t *)(gpio_mmap_reg+gpio_reg_offset)=gpio_reg_data;
}


//===================== MAIN ==========================
void main(void)
{
	if(gpio_mmap() == 0)
		printf("mmap gpio register successfully!\n");
	else
	{
		printf("Fail to mmap gpio register!\n");
		exit(-1);
	}
	printf("gpio_mmap_reg=0x%08x \n",(uint32_t)gpio_mmap_reg);
        //write_gpio_reg(gpio_CTL_REG,0x00000033);
	//printf("gpio_CTL_REG=0x%08x \n", read_gpio_reg(gpio_CTL_REG));
	//printf("gpio_RXCTL_REG=0x%08x \n", read_gpio_reg(gpio_RXCTL_REG));
	printf("PG0_MULT_DRV_OFFSET=0x%08x \n", read_gpio_reg(PG0_MULT_DRV_OFFSET));

 }

