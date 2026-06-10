#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARQUIVO_CONTAS "contas.dat"
#define TAM_NOME 50

typedef struct {
    int numero_conta;
    char nome[TAM_NOME];
    double saldo;
    int ativo;
} Cliente;

static const size_t TAM_REGISTRO = sizeof(Cliente);

static void limpar_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
    }
}

static void ler_linha(char *destino, size_t tamanho) {
    if (fgets(destino, (int)tamanho, stdin) == NULL) {
        destino[0] = '\0';
        return;
    }

    size_t len = strlen(destino);
    if (len > 0 && destino[len - 1] == '\n') {
        destino[len - 1] = '\0';
    }
}

static FILE *abrir_arquivo(const char *modo) {
    FILE *arquivo = fopen(ARQUIVO_CONTAS, modo);
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo \"%s\".\n", ARQUIVO_CONTAS);
    }
    return arquivo;
}

static long contar_registros(FILE *arquivo) {
    long posicao_atual;
    long total;

    posicao_atual = ftell(arquivo);
    if (posicao_atual == -1L) {
        return 0;
    }

    if (fseek(arquivo, 0, SEEK_END) != 0) {
        fseek(arquivo, posicao_atual, SEEK_SET);
        return 0;
    }

    total = ftell(arquivo) / (long)TAM_REGISTRO;

    fseek(arquivo, posicao_atual, SEEK_SET);
    return total;
}

static int ler_registro_posicao(FILE *arquivo, long posicao, Cliente *cliente) {
    if (fseek(arquivo, posicao * (long)TAM_REGISTRO, SEEK_SET) != 0) {
        return 0;
    }

    return fread(cliente, TAM_REGISTRO, 1, arquivo) == 1;
}

static int escrever_registro_posicao(FILE *arquivo, long posicao, const Cliente *cliente) {
    if (fseek(arquivo, posicao * (long)TAM_REGISTRO, SEEK_SET) != 0) {
        return 0;
    }

    return fwrite(cliente, TAM_REGISTRO, 1, arquivo) == 1;
}

static void exibir_cliente(const Cliente *cliente, long posicao) {
    printf("Posicao: %ld\n", posicao);
    printf("Numero da conta: %d\n", cliente->numero_conta);
    printf("Nome: %s\n", cliente->nome);
    printf("Saldo: R$ %.2f\n", cliente->saldo);
    printf("Status: %s\n", cliente->ativo ? "Ativa" : "Encerrada");
    printf("-----------------------------\n");
}

static void cadastrar_cliente(void) {
    FILE *arquivo;
    Cliente cliente;
    Cliente existente;
    long posicao;
    long total_registros;
    char nome[TAM_NOME];

    arquivo = fopen(ARQUIVO_CONTAS, "rb+");
    if (arquivo == NULL) {
        arquivo = fopen(ARQUIVO_CONTAS, "wb+");
        if (arquivo == NULL) {
            printf("Erro ao criar o arquivo \"%s\".\n", ARQUIVO_CONTAS);
            return;
        }
    }

    total_registros = contar_registros(arquivo);

    printf("Informe a posicao do registro (0 a %ld): ", total_registros);
    if (scanf("%ld", &posicao) != 1 || posicao < 0) {
        printf("Posicao invalida.\n");
        limpar_buffer();
        fclose(arquivo);
        return;
    }
    limpar_buffer();

    if (posicao > total_registros) {
        printf("Posicao fora do intervalo permitido. Use uma posicao entre 0 e %ld.\n",
               total_registros);
        fclose(arquivo);
        return;
    }

    if (posicao < total_registros &&
        ler_registro_posicao(arquivo, posicao, &existente) &&
        existente.ativo) {
        printf("Ja existe um cliente ativo na posicao %ld.\n", posicao);
        fclose(arquivo);
        return;
    }

    memset(&cliente, 0, sizeof(Cliente));

    printf("Numero da conta: ");
    if (scanf("%d", &cliente.numero_conta) != 1 || cliente.numero_conta <= 0) {
        printf("Numero da conta invalido.\n");
        limpar_buffer();
        fclose(arquivo);
        return;
    }
    limpar_buffer();

    printf("Nome do cliente: ");
    ler_linha(nome, sizeof(nome));
    if (strlen(nome) == 0) {
        printf("Nome invalido.\n");
        fclose(arquivo);
        return;
    }
    strncpy(cliente.nome, nome, TAM_NOME - 1);

    printf("Saldo inicial: ");
    if (scanf("%lf", &cliente.saldo) != 1) {
        printf("Saldo invalido.\n");
        limpar_buffer();
        fclose(arquivo);
        return;
    }
    limpar_buffer();

    cliente.ativo = 1;

    if (!escrever_registro_posicao(arquivo, posicao, &cliente)) {
        printf("Erro ao gravar o cliente na posicao %ld.\n", posicao);
        fclose(arquivo);
        return;
    }

    fflush(arquivo);
    printf("Cliente cadastrado com sucesso na posicao %ld.\n", posicao);
    fclose(arquivo);
}

static long buscar_posicao_por_conta(FILE *arquivo, int numero_conta) {
    Cliente cliente;
    long posicao;
    long total_registros;

    total_registros = contar_registros(arquivo);

    for (posicao = 0; posicao < total_registros; posicao++) {
        if (!ler_registro_posicao(arquivo, posicao, &cliente)) {
            continue;
        }

        if (cliente.ativo && cliente.numero_conta == numero_conta) {
            return posicao;
        }
    }

    return -1;
}

static void consultar_cliente(void) {
    FILE *arquivo;
    Cliente cliente;
    int numero_conta;
    long posicao;

    arquivo = abrir_arquivo("rb");
    if (arquivo == NULL) {
        return;
    }

    printf("Informe o numero da conta: ");
    if (scanf("%d", &numero_conta) != 1) {
        printf("Numero da conta invalido.\n");
        limpar_buffer();
        fclose(arquivo);
        return;
    }
    limpar_buffer();

    posicao = buscar_posicao_por_conta(arquivo, numero_conta);
    if (posicao == -1) {
        printf("Cliente com conta %d nao encontrado.\n", numero_conta);
        fclose(arquivo);
        return;
    }

    ler_registro_posicao(arquivo, posicao, &cliente);
    exibir_cliente(&cliente, posicao);
    fclose(arquivo);
}

static void atualizar_saldo(void) {
    FILE *arquivo;
    Cliente cliente;
    int numero_conta;
    long posicao;
    double novo_saldo;

    arquivo = abrir_arquivo("rb+");
    if (arquivo == NULL) {
        return;
    }

    printf("Informe o numero da conta: ");
    if (scanf("%d", &numero_conta) != 1) {
        printf("Numero da conta invalido.\n");
        limpar_buffer();
        fclose(arquivo);
        return;
    }
    limpar_buffer();

    posicao = buscar_posicao_por_conta(arquivo, numero_conta);
    if (posicao == -1) {
        printf("Cliente com conta %d nao encontrado.\n", numero_conta);
        fclose(arquivo);
        return;
    }

    ler_registro_posicao(arquivo, posicao, &cliente);

    printf("Saldo atual: R$ %.2f\n", cliente.saldo);
    printf("Informe o novo saldo: ");
    if (scanf("%lf", &novo_saldo) != 1) {
        printf("Saldo invalido.\n");
        limpar_buffer();
        fclose(arquivo);
        return;
    }
    limpar_buffer();

    cliente.saldo = novo_saldo;

    if (!escrever_registro_posicao(arquivo, posicao, &cliente)) {
        printf("Erro ao atualizar o saldo.\n");
        fclose(arquivo);
        return;
    }

    fflush(arquivo);
    printf("Saldo da conta %d atualizado com sucesso.\n", numero_conta);
    fclose(arquivo);
}

static void encerrar_conta(void) {
    FILE *arquivo;
    Cliente cliente;
    int numero_conta;
    long posicao;

    arquivo = abrir_arquivo("rb+");
    if (arquivo == NULL) {
        return;
    }

    printf("Informe o numero da conta a encerrar: ");
    if (scanf("%d", &numero_conta) != 1) {
        printf("Numero da conta invalido.\n");
        limpar_buffer();
        fclose(arquivo);
        return;
    }
    limpar_buffer();

    posicao = buscar_posicao_por_conta(arquivo, numero_conta);
    if (posicao == -1) {
        printf("Cliente com conta %d nao encontrado.\n", numero_conta);
        fclose(arquivo);
        return;
    }

    ler_registro_posicao(arquivo, posicao, &cliente);
    cliente.ativo = 0;
    cliente.saldo = 0.0;

    if (!escrever_registro_posicao(arquivo, posicao, &cliente)) {
        printf("Erro ao encerrar a conta.\n");
        fclose(arquivo);
        return;
    }

    fflush(arquivo);
    printf("Conta %d encerrada com sucesso.\n", numero_conta);
    fclose(arquivo);
}

static void listar_clientes(FILE *arquivo) {
    Cliente cliente;
    long posicao;
    long total_registros;

    total_registros = contar_registros(arquivo);

    if (total_registros == 0) {
        printf("Nenhum registro encontrado no arquivo.\n");
        return;
    }

    for (posicao = 0; posicao < total_registros; posicao++) {
        if (!ler_registro_posicao(arquivo, posicao, &cliente)) {
            continue;
        }

        exibir_cliente(&cliente, posicao);
    }
}

static void listar_clientes_sequencial(FILE *arquivo) {
    Cliente cliente;
    long posicao = 0;

    while (fread(&cliente, TAM_REGISTRO, 1, arquivo) == 1) {
        exibir_cliente(&cliente, posicao);
        posicao++;
    }

    if (posicao == 0) {
        printf("Nenhum registro encontrado no arquivo.\n");
    }
}

static void listar_todos_clientes(void) {
    FILE *arquivo;

    arquivo = abrir_arquivo("rb");
    if (arquivo == NULL) {
        return;
    }

    printf("\n--- Listagem de todos os clientes ---\n");
    listar_clientes(arquivo);
    fclose(arquivo);
}

static void restaurar_e_listar(void) {
    FILE *arquivo;
    long posicao_final;

    arquivo = abrir_arquivo("rb");
    if (arquivo == NULL) {
        return;
    }

    printf("\n--- Primeira leitura (fread sequencial) ---\n");
    listar_clientes_sequencial(arquivo);

    posicao_final = ftell(arquivo);
    printf("Fim da primeira leitura. Ponteiro do arquivo em %ld bytes.\n", posicao_final);

    rewind(arquivo);
    printf("rewind() executado: leitura restaurada para o inicio do arquivo.\n");

    printf("\n--- Segunda leitura apos rewind() ---\n");
    listar_clientes_sequencial(arquivo);

    fclose(arquivo);
}

static void exibir_menu(void) {
    printf("\n===== Sistema de Manutencao de Contas =====\n");
    printf("1. Cadastrar um novo cliente em uma posicao especifica\n");
    printf("2. Consultar um cliente pelo numero da conta\n");
    printf("3. Atualizar o saldo de um cliente\n");
    printf("4. Encerrar conta (remover cliente)\n");
    printf("5. Listar todos os clientes\n");
    printf("6. Restaurar leitura do arquivo com rewind() e listar novamente\n");
    printf("7. Encerrar\n");
    printf("Escolha uma opcao: ");
}

int main(void) {
    int opcao;

    do {
        exibir_menu();

        if (scanf("%d", &opcao) != 1) {
            printf("Opcao invalida.\n");
            limpar_buffer();
            continue;
        }
        limpar_buffer();

        switch (opcao) {
            case 1:
                cadastrar_cliente();
                break;
            case 2:
                consultar_cliente();
                break;
            case 3:
                atualizar_saldo();
                break;
            case 4:
                encerrar_conta();
                break;
            case 5:
                listar_todos_clientes();
                break;
            case 6:
                restaurar_e_listar();
                break;
            case 7:
                printf("Programa encerrado.\n");
                break;
            default:
                printf("Opcao invalida. Tente novamente.\n");
                break;
        }
    } while (opcao != 7);

    return 0;
}
