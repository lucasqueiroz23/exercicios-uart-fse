#include <stdio.h>
#include <unistd.h>  //Used for UART
#include <fcntl.h>   //Used for UART
#include <termios.h> //Used for UART
#include <stdlib.h>
#include <string.h>
#include "crc16.h"

// definição de struct auxiliar para guardar os códigos
struct codigos {
    char c_int;
    char c_float;
    char c_str;
};

char endereco = 0x01;

struct codigos codigos_solicitacao = {0xA1, 0xA2, 0xA3};
struct codigos codigos_envio = {0xB1, 0xB2, 0xB3};

char matricula[4] = {1, 7, 0, 3};

// lê os dados na rx
void ler_dado(int *fs, char codigo) {
    // lendo até 256 bytes. 
    // o vetor de bytes abaixo vai guardar os valores lidos serialmente
    unsigned char rx_buffer[256];

    // tentar ler na uart
    int rx_length = read(*fs, (void *)rx_buffer, 255);

    if (rx_length < 0)
    {
        printf("Erro na leitura.\n");
    }
    else if (rx_length == 0)
    {
        printf("Nenhum dado disponível.\n");
    }
    else
    {
        rx_buffer[rx_length] = '\0';
        printf("%i Bytes lidos\n", rx_length);

        if(codigo == codigos_solicitacao.c_int) {
            int dado = 0;
            
            // copiar os bytes do vetor para a variável 'dado'
            memcpy(&dado, rx_buffer, sizeof(dado));
            printf("Dado = %d\n", dado);
        }

        if(codigo == codigos_solicitacao.c_float) {
            float dado = 0.0;

            // copiar os bytes do vetor para a variável 'dado'
            memcpy(&dado, rx_buffer, sizeof(dado));
            printf("Dado = %f\n", dado);
        }

        if(codigo == codigos_solicitacao.c_str) {
            // nesse caso, não precisamos criar uma variável, 
            // pois o valor que queremos ler já é uma string.
            printf("Tamanho = %d\n", rx_buffer[0]);
            printf("Mensagem = %s\n", rx_buffer + 1);
        }

    }
}

void solicitar_dado(int* fs, char codigo) {
    unsigned char tx_buffer[20];
    unsigned char *p_tx_buffer;

    p_tx_buffer = &tx_buffer[0];

    *p_tx_buffer++ = endereco;
    *p_tx_buffer++ = 0x23;
    *p_tx_buffer++ = codigo;

    memcpy(&tx_buffer[3], &matricula, sizeof(matricula));
    p_tx_buffer += sizeof(matricula);
    /*
    short crc = calcula_CRC(unsigned char *commands, sizeof());

    memcpy(&tx_buffer[3 + sizeof(matricula)], &crc, sizeof(crc));
    p_tx_buffer += sizeof(crc);
    */
    *p_tx_buffer++ = 0x5a;
    *p_tx_buffer++ = 0x76;

    printf("\nBuffers de memória criados!\n");
    printf("Escrevendo caracteres na UART ...");
    int count = write(*fs, &tx_buffer, (p_tx_buffer - &tx_buffer[0]));
    if (count <= 0)
    {
        printf("UART TX error\n");
    }
    else
    {
        printf("escrito.\n");
    }

    // sleep para evitar que a leitura ocorra antes da escrita ser completada.
    sleep(1);
    ler_dado(fs, codigo);
}

void menu(int* fs) {
    printf("O que você deseja fazer?\n");
    printf("1 - Solicitar inteiro\n");
    printf("2 - Solicitar float\n");
    printf("3 - Solicitar string\n");
    /*
    printf("4 - Enviar inteiro\n");
    printf("5 - Enviar float\n");
    printf("6 - Enviar string\n");
    */
    printf("7 - Fechar programa\n");

    // receber entrada
    int opt = 0;
    printf("\n>>");
    scanf("%d", &opt);

    int codigo = 0;

    switch(opt) {
        case 1:
            solicitar_dado(fs, codigos_solicitacao.c_int);
            break;
        case 2:
            solicitar_dado(fs, codigos_solicitacao.c_float);
            break;
        case 3:
            solicitar_dado(fs, codigos_solicitacao.c_str);
            break;
        case 4:
        case 5:
        case 6:
            printf("Não implementado\n");
            break;
        case 7:
            close(*fs);
            printf("Fechando aplicação\n");
            exit(0);
        default:
            printf("Escolha uma opção existente\n");
            break;
    }

    printf("\n");
}

int main()
{
    int uart0_filestream = -1;

    // abrir arquivo que aponta para a porta serial 
    uart0_filestream = open("/dev/serial0", O_RDWR | O_NOCTTY | O_NDELAY); // Open in non blocking read/write mode
    if (uart0_filestream == -1)
    {
        printf("Erro - Não foi possível iniciar a UART.\n");
        exit(1);
    }
    else
    {
        printf("UART inicializada!\n");
    }

    // setar options de mensagem
    struct termios options;
    tcgetattr(uart0_filestream, &options);
    options.c_cflag = B115200 | CS8 | CLOCAL | CREAD; //<Set baud rate
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;

    // salvar tudo 
    tcflush(uart0_filestream, TCIFLUSH);
    tcsetattr(uart0_filestream, TCSANOW, &options);

    menu(&uart0_filestream);

    close(uart0_filestream);
    return 0;
}

