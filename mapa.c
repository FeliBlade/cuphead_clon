#include <stdio.h>

#define LARGO 5
#define ANCHO 4

int main()
{
   int i, j, mapa[ANCHO][LARGO]={0};

   /*i=20;
   for(j=10;j<14;j++)
   {
      mapa[i][j]=1;
   }*/

   for(i=0;i<ANCHO;i++)
   {
      for(j=0;j<LARGO;j++)
      {
         printf("%d ",mapa[i][j]);
      }
      printf("\n\n");
   }

   return 0;
}
