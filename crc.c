/* CRC 0.1, Public domain               */
/* Radim Kolar 2:423/66.111@FidoNet.Org */

#include <stdio.h>
#include <string.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#if defined __EMX__
#include <stdlib.h>
#endif

unsigned long crc32tab[256];
unsigned int  crc16tab[256];
unsigned int  crc16tabnew[256];
   

void crctabinit(void)
{
  int i,n;
  unsigned long CRC32;
  unsigned int  CRC16;
    for (i=0;i<256;i++)
        {
        CRC32=i;
        for (n=1;n<9;n++)
            {
            if (CRC32 & 1)
                CRC32 = (CRC32 >> 1) ^ /* XOR */ 0xedb88320;
            else
                CRC32 = CRC32 >> 1;
            }
        crc32tab[i] = CRC32;
        }

    for (i=0;i<256;i++)
            {
            CRC16=i;
            for (n=1;n<9;n++)
                {
                if (CRC16 & 1)
                    CRC16=(CRC16 >> 1) ^ 0xA001;
                else
                    CRC16=CRC16 >> 1;
                }
            crc16tab[i]=CRC16;
            }

    for (i=0;i<256;i++)
            {
            CRC16=i;
            for (n=1;n<9;n++)
                {
                if (CRC16 & 1)
                    CRC16=(CRC16 >> 1) ^ 0x8408;
                else
                    CRC16=CRC16 >> 1;
                }
            crc16tabnew[i]=CRC16;
            }
        
        
}        

/* CCITT CRC-32 */
unsigned long calc32crc(FILE *f)
{
 unsigned long CRC=0xffffffffL;
 unsigned int C;
 rewind(f);
 while((C=fgetc(f))!=EOF)
   CRC=crc32tab[(CRC ^ C) & 0xFF] ^ ((CRC>>8) & 0x00ffffffL);
 return CRC ^ 0xffffffffL;
}

unsigned int calc16crc(FILE *f,int mode)
/* mode =0 CCITT CRC-16, AX.25 Europan    */
/*       1 XModem/Zmodem/Arc/Hpack/LZH    */

{
 unsigned int  CRC=0xffff;
 unsigned int C;
 if (mode==1) CRC=0; 
 rewind(f);
 while((C=fgetc(f))!=EOF)
    if (mode==1) CRC=((CRC >> 8) & 0x00ff) ^ crc16tab[(CRC ^ C) & 0x00ff];
       else
    CRC=((CRC >> 8) & 0x00ff) ^ crc16tabnew[(CRC ^ C) & 0x00ff];
 if (mode==1) return CRC;
 return CRC ^ 0xffff;
}


int main(int argc,char *argv[])
{
  FILE *f; 
  int i,mode=0,verb=0,fixed=0;
  struct stat s;
  char *tmp;
#if defined __EMX__
 _response(&argc,&argv);
 _wildcard(&argc,&argv);
#endif
 if (argc==1) {
                printf("CRC 0.1, Cyclic Redundancy Check. Radim Kolar 2:423/66.111@FidoNet.Org.");
#ifdef __EMX__                
                printf("\nCompiled "__DATE__" by GCC "__VERSION__".  Public domain.");
#endif                
                printf("\n\nCRC [ -crctype ] [ -options ] <files...> <@listfile...>");
                printf("\nCRC Type: one of CRC32, CRC16, CCITT16, ALL.");
                printf("\nOptions: V - print filedate/size");
                printf("\n         F - fixed filename length (25 chars), size (7 chars)");
                return 0; /* no deal */
              }  
 crctabinit();
 for (i=1;i<argc;i++)
 {
   if((argv[i][0]=='/')||(argv[i][0]=='-'))
                      { if(!stricmp(argv[i]+1,"CRC32")) { mode=0;continue;}
                        if(!stricmp(argv[i]+1,"CRC16")) { mode=1;continue;}
                        if(!stricmp(argv[i]+1,"CCITT16")) { mode=2;continue;}
                        if(!stricmp(argv[i]+1,"ALL")) { mode=3;continue;}
                        if(!stricmp(argv[i]+1,"V")) { verb=1;continue;}
                        if(!stricmp(argv[i]+1,"F")) { fixed=1;continue;}
                        fprintf(stderr, "Unknown switch %s. RTFM !\n",argv[i]);
                        continue;
                      }  
   f=fopen(argv[i],"rb");
   if (f==NULL) {
                  fprintf(stderr,"Error opening %s\n",argv[i]);
                  continue;
                }

   if (verb==1) if (stat(argv[i],&s)) { fclose(f);
                                        fprintf(stderr,"Error getting file info.\n");
                                        continue;
                                      }
                                
   if ((fixed==1)&&(strlen(argv[i])>25)) argv[i][25]='\x0';             
   if (fixed==1) printf("%-25s ",argv[i]);
      else
                 printf("%s ",argv[i]);             
                 
   if (verb==1)          
                    {                    
                    tmp=ctime(&s.st_mtime);
                    tmp[24]='\0';
                    if (fixed==1) printf("%-9ld %s ",s.st_size,tmp);
                       else
                                  printf("%ld %s ",s.st_size,tmp);
                    };
   switch(mode)
   {             
   case 0: printf("%08lX\n",calc32crc(f));break;
   case 1: printf("%04X\n",calc16crc(f,1));break;
   case 2: printf("%04X\n",calc16crc(f,0));break;
   case 3: printf("%08lX %04X %04X\n",calc32crc(f),calc16crc(f,1),calc16crc(f,0));
   }
   fclose(f);               
 }
 return 0;
}

