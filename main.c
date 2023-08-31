#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INSERIR 1
#define REMOVER 2
#define COMPACTAR 3

typedef struct {
    int primeiro_dispo;
} dispo;

typedef struct {
    int proximo_reg;
    char buffer[139]; 
} registro;

char astesrisco = '*';

void intToChar3(int number, char *result) {
    if (number >= 0 && number <= 999) {
        sprintf(result, "%03d", number);
    } else {
        strcpy(result, "ERR");
    }
}

void char3ToInt(int *number, char *string) {
    if (strlen(string) == 3) {
        *number = atoi(string);
    } else {
        *number = -1;
    }
}

registro * ler_registro(FILE * fp, int byte_offset) {
    fseek(fp, byte_offset, SEEK_SET);
    registro * reg = (registro *) malloc(sizeof(registro));

    int tam_buffer;
    fread(&tam_buffer, sizeof(int), 1, fp);

    fread(reg->buffer, sizeof(char), tam_buffer, fp);
    if (reg->buffer[0] == '*') {
        // printf("Registro removido.\n");
        return NULL;
    }

    return reg;
}

int busca_espaco_livre_na_lista(FILE * fp, int tamanho) {
    int posicao_inicial = ftell(fp);

    rewind(fp);

    int posicao_atual;
    fread(&posicao_atual, sizeof(int), 1, fp);

    while(1) {
        printf("%d\n", posicao_atual);
        if (posicao_atual == -1) {
            fseek(fp, posicao_inicial, SEEK_SET);
            return -1;
        }
        fseek(fp, posicao_atual, SEEK_SET);
        int tam_buffer;
        fread(&tam_buffer, sizeof(int), 1, fp);

        // (INT)*(PROX)

        if(tam_buffer >= tamanho + 2*sizeof(int) + sizeof(char)) {
            fseek(fp, posicao_inicial, SEEK_SET);
            return posicao_atual;
        } else if (tam_buffer == tamanho) {
            fseek(fp, posicao_inicial, SEEK_SET);
            return posicao_atual;
        } else {
            fseek(fp, sizeof(char), SEEK_CUR);
            fread(&posicao_atual, sizeof(int), 1, fp); // i++
        }
    }
}

int busca_espaco_livre(FILE * fp, int size) {
    int byte_offset = busca_espaco_livre_na_lista(fp, size);

    if (byte_offset != -1) {
        return byte_offset;
    }

    fseek(fp, 0, SEEK_END);
    return ftell(fp);
}

void reorganizar_lista(FILE *fp, int removido) {
    int posicao_inicial = ftell(fp);

    rewind(fp);

    int inicio_lista;
    fread(&inicio_lista, sizeof(int), 1, fp);

    if (inicio_lista == removido) {
        printf("inicio da lista\n");
        fseek(fp, inicio_lista + sizeof(int) + sizeof(char), SEEK_SET);
        int prox;
        fread(&prox, sizeof(int), 1, fp);
        rewind(fp);
        fwrite(&prox, sizeof(int), 1, fp);

        fseek(fp, posicao_inicial, SEEK_SET);
        return;
    }

    int posicao_atual = inicio_lista;
    while (1) {
        fseek(fp, sizeof(char), SEEK_CUR);
        int prox;
        fread(&prox, sizeof(int), 1, fp);

        if (prox == removido) {
            fseek(fp, prox + sizeof(char), SEEK_SET);
            int prox_prox;
            fread(&prox_prox, sizeof(int), 1, fp);
            fseek(fp, posicao_atual + sizeof(char), SEEK_SET);
            fwrite(&prox_prox, sizeof(int), 1, fp);

            fseek(fp, posicao_inicial, SEEK_SET);
            return;
        }

        posicao_atual = prox; // i++
    }
}

int main() {
    FILE * fp = fopen("seguradoras.dad", "r+b");

    if(fp == NULL) {
        printf("Não é possível ler o arquivo.\n");
        fp = fopen("seguradoras.dad", "w+b");
    } else {
        printf("Arquivo aberto com sucesso.\n");
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);

    if (size == 0) {
        int nenhum_vazio = -1;
        fwrite(&nenhum_vazio, sizeof(int), 1, fp);
    } else {
        rewind(fp);
    }

    char nome[50], seguradora[50], tipo_seg[30], buffer_aux[134];
    int codigo;

    // Menu de opções

    // 1 - Inserir
    // 2 - Remover
    // 3 - Compactar

    printf("Escolha a função desejada:\n");
    int funcao;
    scanf("%d", &funcao);

    if (funcao == INSERIR) {
        printf("Código:\n");
        scanf("%d", &codigo);

        printf("Nome do cliente:\n");
        scanf("%s", nome);

        printf("Seguradora:\n");
        scanf("%s", seguradora);

        printf("Tipo do seguro:\n");
        scanf("%s", tipo_seg);

        char cod[4];
        intToChar3(codigo, cod);

        strcpy(buffer_aux, "");

        sprintf(buffer_aux, "%s#%s#%s#%s", cod, nome, seguradora, tipo_seg);

        int tam_buff = strlen(buffer_aux);
        printf("%d\n", tam_buff);

        int byte_offset = busca_espaco_livre(fp, tam_buff);
        printf("byte offset %d\n", byte_offset);

        fseek(fp, 0, SEEK_END);
        // meio
        if (ftell(fp) != byte_offset) {
    	    reorganizar_lista(fp, byte_offset);

            fseek(fp, byte_offset, SEEK_SET);
            printf("byte offset %d\n", byte_offset);

            int tam_dispo;
            fread(&tam_dispo, sizeof(int), 1, fp);
            printf("tam dispo %d\n", tam_dispo);

            int sobra = tam_dispo - tam_buff - sizeof(int);
            printf("sobra %d\n", sobra);

            fseek(fp, byte_offset, SEEK_SET);
            fwrite(&tam_buff, sizeof(int), 1, fp);
            fwrite(buffer_aux, sizeof(char), tam_buff, fp);

            if (sobra > 0) {
                printf("sobra > 0\n");
                fwrite(&sobra, sizeof(int), 1, fp);
                fwrite(&astesrisco, sizeof(char), 1, fp);

                int pos_atual = ftell(fp);
                printf("pos atual %d\n", pos_atual);

                rewind(fp);

                int prox_vazio;
                fread(&prox_vazio, sizeof(int), 1, fp);
                printf("prox vazio %d\n", prox_vazio);

                rewind(fp);
                int inicio_lista = pos_atual - sizeof(int) - 1;
                fwrite(&inicio_lista, sizeof(int), 1, fp);

                fseek(fp, pos_atual, SEEK_SET);
                fwrite(&prox_vazio, sizeof(int), 1, fp);
            } else {
                fseek(fp, byte_offset, SEEK_SET);
                fwrite(&tam_buff, sizeof(int), 1, fp);
                fwrite(buffer_aux, sizeof(char), tam_buff, fp);
            }
        } else {
            fseek(fp, byte_offset, SEEK_SET);
            fwrite(&tam_buff, sizeof(int), 1, fp);
            fwrite(buffer_aux, sizeof(char), tam_buff, fp);
        }
    }

    else if(funcao == REMOVER) {
        printf("Escolha o registro que será retirado:\n");
        int codigo_remover;
        scanf("%d", &codigo_remover);
        char codigo[4];
        intToChar3(codigo_remover, codigo);

        fseek(fp, sizeof(int), SEEK_SET);

        // loop de busca
        while (!feof(fp)) {
            registro * reg = ler_registro(fp, ftell(fp));
            if (reg == NULL) continue;
            printf("%s\n", reg->buffer);

            char codigo_reg[4];
            strncpy(codigo_reg, reg->buffer, 3);

            if (strcmp(codigo_reg, codigo) == 0) {
                printf("Registro encontrado.\n");
                fseek(fp, -1 * strlen(reg->buffer), SEEK_CUR);

                int pos = ftell(fp), prox_vazio;

                rewind(fp);
                fread(&prox_vazio, sizeof(int), 1, fp);

                rewind(fp);
                int posMenosInt = pos - sizeof(int); 
                printf("pos menos int %d\n", posMenosInt);
                fwrite(&posMenosInt, sizeof(int), 1, fp);

                fseek(fp, pos, SEEK_SET);
                fwrite("*", sizeof(char), 1, fp);
                fwrite(&prox_vazio, sizeof(int), 1, fp);

                break;
            }
        }
    }

    else if (funcao == COMPACTAR) {
        FILE *fp_compactado = fopen("seguradoras_compactado.dad", "w+b");

        int nenhum_vazio = -1;
        fwrite(&nenhum_vazio, sizeof(int), 1, fp_compactado);

        fseek(fp, sizeof(int), SEEK_SET);

        while(!feof(fp)) {
            printf("%ld\n", ftell(fp));

            registro * reg = ler_registro(fp, ftell(fp));

            if (reg == NULL) continue;

            printf("%s\n", reg->buffer);

            int tam_buffer = strlen(reg->buffer);
            fwrite(&tam_buffer, sizeof(int), 1, fp_compactado);
            fwrite(reg->buffer, sizeof(char), tam_buffer, fp_compactado);
        }

        // rename file to seguradoras.dad
        remove("seguradoras.dad");
        rename("seguradoras_compactado.dad", "seguradoras.dad");

        fclose(fp_compactado);
    }

    fclose(fp);
}