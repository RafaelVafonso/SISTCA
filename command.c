#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


int main(int argc, char* const argv[]) {

    FILE *serial = fopen("/dev/ttyUSB0", "w");
    if(serial == NULL) {
        perror("Erro ao abrir porta");
        return 1;
    }


    printf("Aguardar...\n");

    uint16_t time = 0;
    char *out = NULL;
    char *state = NULL;

    if (argc == 5 && strcmp(argv[1], "-s") == 0) {
        out = argv[2];
        state = argv[4];
    }
    else if (argc == 5 && strcmp(argv[1], "-t") == 0 && strcmp(argv[3], "-s") == 0) {
        time = atoi(argv[2]);
        out = argv[4];
        state = "x"; /* prevenir segmentantion fault ao dar valor a este ponteiro*/
    }
    else {
        printf("Uso:\n");
        printf("%s -s saida -m estado (on/off)\n", argv[0]);
        printf("%s -t tempo -s saida \n", argv[0]);
        return 1;
    }

    if(strcmp(out, "led") == 0 && strcmp(state, "on") == 0) {
        fprintf(serial,"1\n");
    }
    else if(strcmp(out, "led") == 0 && strcmp(state, "off") == 0) {
        fprintf(serial,"0\n");
    }
    else if(strcmp(out, "led") == 0 && strcmp(argv[1], "-t") == 0) {
        fprintf(serial, "2 %d\n", time); 
    }
    else {
        printf("Saida desconhecida\n");
    }

    fclose(serial);
    return 0;
}
