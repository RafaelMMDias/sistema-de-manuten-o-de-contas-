# Sistema de Manutenção de Contas

Atividade somativa de **PIF** — sistema em C para gerenciar clientes utilizando **arquivo binário** com **registros de tamanho fixo**.

## Objetivo

Implementar um programa com menu interativo que persiste os dados em disco no arquivo `contas.dat`, usando as funções de manipulação de arquivos da biblioteca padrão C:

- `fseek()` — posicionar o ponteiro do arquivo em um registro específico
- `fread()` — ler registros
- `fwrite()` — gravar registros
- `rewind()` — voltar a leitura para o início do arquivo

## Estrutura do registro

Cada cliente é representado pela struct `Cliente`, com tamanho fixo de **72 bytes** (valor dependente do compilador/plataforma):

| Campo          | Tipo        | Descrição                          |
|----------------|-------------|------------------------------------|
| `numero_conta` | `int`       | Identificador único da conta       |
| `nome`         | `char[50]`  | Nome do cliente                    |
| `saldo`        | `double`    | Saldo da conta                     |
| `ativo`        | `int`       | `1` = conta ativa, `0` = encerrada  |

O tamanho de cada registro é calculado com:

```c
static const size_t TAM_REGISTRO = sizeof(Cliente);
```

Como todos os registros têm o mesmo tamanho, a posição *n* no arquivo corresponde ao byte:

```
offset = posicao * TAM_REGISTRO
```

Isso permite acesso direto a qualquer registro com `fseek()`.

## Arquivo binário

Os dados são gravados em `contas.dat` no formato binário. O arquivo **não pode ser lido como texto** em um editor comum — os dados aparecem como caracteres ilegíveis porque são bytes brutos da struct.

Modos de abertura utilizados:

| Modo   | Uso                                              |
|--------|--------------------------------------------------|
| `"rb"` | Leitura (consulta, listagem)                     |
| `"rb+"`| Leitura e escrita (cadastro, atualização)        |
| `"wb+"`| Criação do arquivo quando ainda não existe       |

## Menu e implementação

### 1. Cadastrar cliente em posição específica

O usuário informa a **posição** do registro (0 = primeiro, 1 = segundo, etc.) e os dados do cliente.

**Fluxo:**
1. Abre `contas.dat` em `"rb+"` (ou cria com `"wb+"` se não existir)
2. Calcula quantos registros já existem no arquivo
3. Posiciona com `fseek()` na posição informada
4. Grava o struct completo com `fwrite()`

Posições válidas: de `0` até o total atual de registros (inclusive a posição final para adicionar ao fim).

### 2. Consultar cliente pelo número da conta

**Fluxo:**
1. Abre o arquivo em `"rb"`
2. Percorre os registros com `fseek()` + `fread()`
3. Compara o campo `numero_conta` até encontrar a conta ativa desejada
4. Exibe os dados do cliente

### 3. Atualizar saldo

**Fluxo:**
1. Busca a conta pelo número (mesma lógica da consulta)
2. Lê o registro com `fread()`
3. Altera o campo `saldo`
4. Regrava na mesma posição com `fseek()` + `fwrite()`

### 4. Encerrar conta (remover cliente)

Em arquivos de tamanho fixo, o registro **não é apagado fisicamente** — ele é marcado como encerrado:

- `ativo = 0`
- `saldo = 0.0`

A gravação usa `fseek()` + `fwrite()` na posição encontrada. Contas encerradas continuam no arquivo, mas não aparecem em consultas.

### 5. Listar todos os clientes

Exibe **todos** os registros do arquivo (ativos e encerrados), percorrendo cada posição com:

```c
fseek(arquivo, posicao * TAM_REGISTRO, SEEK_SET);
fread(cliente, TAM_REGISTRO, 1, arquivo);
```

O status de cada conta é mostrado como `Ativa` ou `Encerrada`.

### 6. Restaurar leitura com `rewind()` e listar novamente

Demonstra o uso de `rewind()` de forma didática:

1. **Primeira leitura** — percorre o arquivo do início ao fim com `fread()` sequencial
2. Mostra a posição final do ponteiro com `ftell()`
3. Chama **`rewind(arquivo)`** para voltar ao byte 0
4. **Segunda leitura** — repete a listagem completa a partir do início

Sem o `rewind()`, a segunda leitura não retornaria dados, pois o ponteiro estaria no final do arquivo.

### 7. Encerrar

Finaliza o programa.

## Funções auxiliares principais

| Função                      | Descrição                                              |
|-----------------------------|--------------------------------------------------------|
| `ler_registro_posicao()`    | `fseek()` + `fread()` em uma posição lógica            |
| `escrever_registro_posicao()`| `fseek()` + `fwrite()` em uma posição lógica          |
| `contar_registros()`        | `fseek(SEEK_END)` + `ftell()` ÷ tamanho do registro    |
| `buscar_posicao_por_conta()`| Varre o arquivo buscando pelo `numero_conta`           |
| `listar_clientes_sequencial()` | Leitura contínua com `fread()` até o fim do arquivo |

## Compilação

```bash
gcc sistema_contas.c -o sistema_contas.exe -Wall -Wextra
```

> **Atenção:** feche o programa (opção 7 ou `Ctrl+C`) antes de recompilar. Se o `.exe` estiver em execução, o Windows bloqueia a sobrescrita e ocorre erro *Permission denied*.

## Execução

No terminal, entre na pasta do projeto e execute:

```powershell
cd "c:\Users\rafam\Documents\Cesar school\2 período\PIF\atividade somativa\sistema-de-manuten-o-de-contas-"
.\sistema_contas.exe
```

Digite o número da opção desejada (1 a 7) e pressione Enter.

## Arquivos do projeto

| Arquivo            | Descrição                              |
|--------------------|----------------------------------------|
| `sistema_contas.c` | Código-fonte do sistema                |
| `contas.dat`       | Arquivo binário gerado automaticamente |
| `sistema_contas.exe` | Executável compilado (Windows)       |

## Exemplo de uso

```
1 → Cadastrar cliente na posição 0 (conta 1001, João, R$ 1500.00)
1 → Cadastrar cliente na posição 1 (conta 1002, Maria, R$ 800.00)
5 → Listar todos os clientes
2 → Consultar conta 1001
3 → Atualizar saldo da conta 1001
4 → Encerrar conta 1002
6 → Listar com rewind() (duas leituras seguidas)
7 → Sair
```