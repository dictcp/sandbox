#include <stdio.h>
#include <string.h>
#include "KeccakHash.h"

/*
 * sha3sum (based on offical library)
 * By Dick Tang (dick.tang@ymail.com, tangcp@cse.cuhk.edu.hk, dictcp@gmail.com)
 *
 * Need Keccak Code Package from offical site http://keccak.noekeon.org/.
 *
 * To compile:
 * gcc -O3 -o sha3sum -std=gnu99 -I Modes -I Common/ -I KeccakF-1600/ -I KeccakF-1600/Optimized -I Constructions/ sha3sum.c Modes/KeccakHash.c Constructions/KeccakSponge.c KeccakF-1600/Optimized/KeccakF-1600-opt64.c
 *
 * Notice:
 * (i)  The standard is not still in drafting state.
 * (ii) The code is not well tested.
 */

static int bin2hex(const unsigned char* in, unsigned char *out, int size)
{
  int i;

  for(i = 0; i < size; ++i)
  {
    char tmp[3]={0,0,0};
    sprintf(tmp, "%02x", in[i]);
    strcat((char*)out, (char*)tmp);
  }
  return 0;
}


int main (int argc, char**argv) {

  for (int i=0;i<argc;i++) {
    FILE* fp;
    char *filename="-";

    if (argc==1) {
      fp=stdin;
    }
    else if (argc>1&&i==0) {
      // skip dummy code if argc>1
      continue;
    }
    else if (argc==2&&strcmp(argv[1],"-")==0) {
      fp=stdin;
    }
    else {
      filename = argv[i];
      fp=fopen(filename,"rb");
    }

    if (fp==NULL) {
      fprintf(stderr, "Fail to calculate hash for %s\n", filename);
      continue;
    }

    unsigned char md[512/8];

    Keccak_HashInstance ctx;
    Keccak_HashInitialize(&ctx, 1088, 512, 512, 0x37); // from Tests

    while (!feof(fp)) {
      int len=1600;
      unsigned char data[len];
      len = fread(data, 1, len, fp);
      if (len==0)
        break;
      Keccak_HashUpdate(&ctx, data, len);
    }
    if(ferror(fp)) {
      fprintf(stderr, "Fail to calculate hash for %s\n", filename);
      fclose(fp);
      continue;
    }
    Keccak_HashFinal(&ctx, (unsigned char *)&md);

    char out_string[512/8*2+1]="";
    bin2hex(md, out_string, 512/8);
    printf("%s\t%s\n", out_string, filename);

    fclose(fp);
  }
  return 0;
}
