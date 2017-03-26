#include <stdio.h>
#include <memory.h>
int main( void )
{
    int i = 0, j = 0, n = 0, tag = 0;
    char c;
    char cmd[328];
    char scmd[10][40];
    memset(cmd, 0, sizeof(cmd));
    memset(scmd, 0, sizeof(scmd));
    while ((c = getchar()) != '\n' && i < 328) {
            if (c == 32 || c == 9) {
                if(tag)
                    continue;
                cmd[i++] = '\0';
                tag = 1;
            }
            else {
                cmd[i++] = c;
                tag = 0;
            }
        }
        i=0;
        char* p;
        while(i < 328) {
            p = scmd[j];    
            while(cmd[i++] != '\0') {
                memcpy(p++, &cmd[i-1], 1);
            }
            if(p == scmd[j])
                break;
            j++;
        }
        printf("%d\n", j);
        for(i=0; i < j; i++) {
            printf("%s\n", scmd[i]);
        }
        printf("'%s'\n", scmd[1]);
}
