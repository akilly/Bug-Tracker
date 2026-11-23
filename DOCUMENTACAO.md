# Documentação técnica completa — Bug Tracker em C++

> **Proposta 4 — Sistema de acompanhamento de defeitos de software**
> Arquivo-fonte: `bugtracker.cpp` · Arquivo de dados: `defeitos.txt`

Este documento explica **todo** o sistema em detalhe: o problema que ele resolve, como compilar, como o código está organizado, o papel de cada constante, variável e função, cada regra de negócio, o formato do arquivo e o passo a passo de cada operação, com exemplos tirados da execução real do programa.

---

## Sumário

1. [Visão geral](#1-visão-geral)
2. [Como compilar e executar](#2-como-compilar-e-executar)
3. [Organização do código-fonte](#3-organização-do-código-fonte)
4. [Bibliotecas utilizadas](#4-bibliotecas-utilizadas)
5. [Constantes](#5-constantes)
6. [Estruturas de dados e variáveis globais](#6-estruturas-de-dados-e-variáveis-globais)
7. [Ciclo de vida de um defeito (máquina de estados)](#7-ciclo-de-vida-de-um-defeito-máquina-de-estados)
8. [Funções utilitárias](#8-funções-utilitárias)
9. [Funcionalidades (requisitos 1 a 11)](#9-funcionalidades-requisitos-1-a-11)
10. [Atividade obrigatória com matriz](#10-atividade-obrigatória-com-matriz)
11. [Atividade obrigatória com leitura de arquivo](#11-atividade-obrigatória-com-leitura-de-arquivo)
12. [Relatório e desafio adicional (proporção de reabertura)](#12-relatório-e-desafio-adicional-proporção-de-reabertura)
13. [Menu principal e função `main()`](#13-menu-principal-e-função-main)
14. [Tratamento de erros e validações de entrada](#14-tratamento-de-erros-e-validações-de-entrada)
15. [Mapa requisito → função](#15-mapa-requisito--função)
16. [Roteiro de teste / demonstração](#16-roteiro-de-teste--demonstração)
17. [Decisões de projeto](#17-decisões-de-projeto)
18. [Limitações conhecidas e possíveis melhorias](#18-limitações-conhecidas-e-possíveis-melhorias)
19. [Glossário](#19-glossário)

---

## 1. Visão geral

### 1.1 Contexto

Durante os testes de uma aplicação, os **analistas de teste** encontram falhas (chamadas de **defeitos** ou *bugs*). Antes que uma versão do software seja entregue, cada defeito precisa passar por estas etapas:

1. **Ser registrado**: o que aconteceu, em qual parte do sistema e qual a gravidade.
2. **Ser atribuído** a um desenvolvedor, que fica responsável pela correção.
3. **Ser corrigido**: o desenvolvedor informa o que foi feito.
4. **Ser retestado**: o analista confere se a correção funcionou.
5. **Ser encerrado** (se a correção funcionou) ou **reaberto** (se não funcionou ou se a falha voltou a acontecer).

### 1.2 Objetivo do programa

O programa é um **bug tracker** de console. Ele mantém em memória a lista de defeitos e controla essas etapas. Ele **impede operações fora de ordem**: por exemplo, não deixa encerrar um defeito que não passou no reteste. Ele também gera **alertas** e **relatórios** para que a equipe saiba a situação dos testes.

### 1.3 O que o sistema oferece

- Cadastro, consulta, listagem e alteração de defeitos.
- Controle completo do ciclo de vida (7 status possíveis).
- Alertas automáticos de defeitos **críticos** ainda abertos.
- Matriz `int defeitos[4][3]` com totais por módulo e por severidade.
- Relatório por módulo, severidade e status.
- Importação de defeitos a partir do arquivo `defeitos.txt`, com validação e resumo.
- Cálculo da proporção de defeitos que precisaram ser reabertos (desafio adicional).

---

## 2. Como compilar e executar

### 2.1 Requisitos

- Um compilador C++ com suporte a **C++11** ou mais recente (g++, clang++, MinGW, MSVC).
- Nenhuma biblioteca externa: só a biblioteca padrão do C++.

### 2.2 Linux / macOS (terminal)

```bash
g++ -std=c++11 -Wall -o bugtracker bugtracker.cpp
./bugtracker
```

- `-std=c++11`: usa o padrão C++11.
- `-Wall`: mostra avisos do compilador (o código compila sem nenhum aviso).
- `-o bugtracker`: nome do executável gerado.

### 2.3 Windows (MinGW / prompt de comando)

```bat
g++ -std=c++11 -Wall -o bugtracker.exe bugtracker.cpp
bugtracker.exe
```

### 2.4 IDEs (Dev-C++, Code::Blocks, VS Code, Visual Studio)

1. Crie um projeto de **aplicação de console** em C++.
2. Adicione o arquivo `bugtracker.cpp` (ou cole o conteúdo no arquivo principal).
3. Se a IDE permitir, ative o padrão **C++11**. No Dev-C++: *Ferramentas → Opções do compilador → Geração de código → Padrão de linguagem: ISO C++11*.
4. Copie o `defeitos.txt` para a **pasta onde o executável roda** (o "diretório de trabalho"). Em muitas IDEs, essa é a pasta do projeto, e não a pasta `bin/` ou `Debug/`.

### 2.5 Sobre o arquivo `defeitos.txt`

O programa procura o arquivo **pelo nome, na pasta atual de execução**. Se não o encontrar, a opção 12 mostra uma mensagem de erro e o programa **continua funcionando** normalmente (ver seção 11.6).

---

## 3. Organização do código-fonte

O arquivo `bugtracker.cpp` está dividido em **5 blocos**, sempre nesta ordem:

| Bloco | Conteúdo | Por que fica nessa posição |
|---|---|---|
| 1. Constantes | Nomes de módulos, severidades e status | Precisam existir antes de qualquer função usá-las |
| 2. Estrutura de dados | `struct Defeito`, vetor `cadastro`, matriz `defeitos` | Todas as funções trabalham sobre esses dados |
| 3. Funções utilitárias | Leitura do teclado, conversões, buscas, exibição | São usadas pelas funcionalidades, então vêm antes delas |
| 4. Funcionalidades | Uma função por requisito (1 a 11) | Usam as utilitárias e são chamadas pelo menu |
| 5. Menu e `main()` | Laço principal | Fica por último porque chama todas as outras |

> **Por que essa ordem importa?** Em C++, uma função só pode ser chamada depois de ser **declarada**. Como o código não usa protótipos separados, cada função aparece antes das funções que a chamam.

### 3.1 Diagrama de dependências (quem chama quem)

```
main()
 ├── atualizarMatriz()
 ├── exibirMenu()
 ├── lerInteiro() ──► lerTexto() ──► trim()
 │                └─► paraInteiro() ──► trim()
 └── (switch) ─┬─ cadastrarDefeito() ──► lerInteiro, buscarDefeito, escolherModulo,
               │                          lerTextoObrigatorio, atualizarMatriz
               ├─ consultarDefeito()  ──► solicitarDefeito, exibirDefeito
               ├─ listarDefeitos()
               ├─ atribuirDefeito()   ──► solicitarDefeito, lerTextoObrigatorio
               ├─ registrarCorrecao() ──► solicitarDefeito, lerTextoObrigatorio
               ├─ executarReteste()   ──► solicitarDefeito, lerInteiro
               ├─ encerrarDefeito()   ──► solicitarDefeito
               ├─ reabrirDefeito()    ──► solicitarDefeito
               ├─ alterarDefeito()    ──► solicitarDefeito, exibirDefeito, escolherModulo,
               │                          lerTextoObrigatorio, lerInteiro, atualizarMatriz
               ├─ verificarCriticos()
               ├─ gerarRelatorioDefeitos() ──► exibirMatriz ──► atualizarMatriz
               └─ importarDefeitosArquivo() ──► trim, minusculo, paraInteiro, buscarDefeito,
                                                indiceModulo, normalizarStatus, atualizarMatriz

solicitarDefeito() ──► lerInteiro, buscarDefeito
atualizarMatriz()  ──► indiceModulo ──► minusculo, trim
```

---

## 4. Bibliotecas utilizadas

| `#include` | Para que é usada no programa |
|---|---|
| `<iostream>` | `cout` (escrever na tela) e `cin` (ler do teclado) |
| `<fstream>` | `ifstream`, para abrir e ler o `defeitos.txt` (exigência do trabalho) |
| `<sstream>` | `stringstream` para quebrar uma linha do arquivo nos `;` e `ostringstream` para montar mensagens de erro |
| `<string>` | O tipo `string` e suas operações (`substr`, `find`, `replace`...) |
| `<vector>` | `vector<Defeito>`, a lista de defeitos que cresce sozinha |
| `<cctype>` | `isspace`, `isdigit`, `tolower`, para analisar caracteres |
| `<cstdlib>` | `atoi`, para converter texto em número inteiro |
| `<iomanip>` | `setw`, `left`, `right`, `fixed`, `setprecision`, para alinhar tabelas e formatar a porcentagem |

A linha `using namespace std;` permite escrever `cout` em vez de `std::cout`, `string` em vez de `std::string` e assim por diante.

---

## 5. Constantes

As constantes ficam no início do arquivo para que **nenhum nome seja digitado duas vezes**. Isso evita erros como escrever `"Em Correcao"` em um lugar e `"Em correcao"` em outro (para o computador, são textos diferentes). Também facilita mudar um nome no futuro: basta alterar em um só lugar.

### 5.1 Tamanhos e severidade crítica

| Constante | Valor | Significado |
|---|---|---|
| `NUM_MODULOS` | `4` | Quantidade de módulos = número de **linhas** da matriz |
| `NUM_SEVERIDADES` | `3` | Quantidade de severidades = número de **colunas** da matriz |
| `SEVERIDADE_CRITICA` | `3` | Valor de severidade considerado crítico (usado nos alertas e na matriz) |
| `NUM_STATUS` | `7` | Quantidade de status possíveis |

### 5.2 Módulos — `MODULOS[4]`

| Índice (linha na matriz) | Nome |
|---|---|
| 0 | `Login` |
| 1 | `Cadastro` |
| 2 | `Pagamento` |
| 3 | `Relatorios` |

O **índice** do módulo no vetor é também a **linha** dele na matriz. Por isso a ordem desse vetor não pode mudar sem pensar no efeito sobre a matriz.

### 5.3 Severidades — `NOMES_SEVERIDADE[3]`

| Severidade (valor guardado) | Índice no vetor / coluna na matriz | Nome exibido |
|---|---|---|
| 1 | 0 | `Baixa` |
| 2 | 1 | `Media` |
| 3 | 2 | `Critica` |

**Regra de conversão:** `coluna = severidade - 1`. Os vetores em C++ começam na posição 0, mas a severidade começa em 1.

### 5.4 Status — constantes `ST_*` e vetor `STATUS_VALIDOS[7]`

| Constante | Texto | Significado |
|---|---|---|
| `ST_NOVO` | `Novo` | Defeito recém-cadastrado, ninguém trabalhando nele ainda |
| `ST_EM_CORRECAO` | `Em correcao` | Um responsável foi atribuído e está corrigindo |
| `ST_AGUARDANDO` | `Aguardando teste` | Correção feita, falta o analista retestar |
| `ST_APROVADO` | `Aprovado` | O reteste passou, e o defeito pode ser encerrado |
| `ST_REPROVADO` | `Reprovado` | O reteste falhou, e o defeito precisa ser reaberto |
| `ST_ENCERRADO` | `Encerrado` | Defeito resolvido e fechado |
| `ST_REABERTO` | `Reaberto` | Voltou para ser corrigido de novo |

O vetor `STATUS_VALIDOS` reúne os 7 textos, nessa ordem. Ele é usado em dois lugares:
- na **importação**, para verificar se o status lido do arquivo existe;
- no **relatório**, para contar quantos defeitos há em cada status.

---

## 6. Estruturas de dados e variáveis globais

### 6.1 `struct Defeito`

Representa **um** defeito. Uma `struct` agrupa várias informações diferentes sob um único nome.

| Campo | Tipo | Preenchido em | Descrição |
|---|---|---|---|
| `codigo` | `int` | Cadastro / importação | Identificador único (ID). Precisa ser positivo e não pode se repetir |
| `modulo` | `string` | Cadastro / importação / alteração | Um dos 4 módulos válidos, sempre na grafia padrão (ex.: `Login`) |
| `descricao` | `string` | Cadastro / importação / alteração | Resumo curto da falha. Obrigatório e sem `;` |
| `severidade` | `int` | Cadastro / importação / alteração | 1 (Baixa), 2 (Média) ou 3 (Crítica) |
| `status` | `string` | Todas as operações do ciclo de vida | Situação atual (um dos 7 status) |
| `responsavel` | `string` | Atribuir / alterar | Nome de quem corrige. Vazio (`""`) enquanto ninguém foi atribuído |
| `correcao` | `string` | Registrar correção | O que foi feito para corrigir. Vazio até a primeira correção |
| `resultadoReteste` | `string` | Reteste / importação | `"Aprovado"`, `"Reprovado"` ou `""`. É apagado a cada nova correção |
| `reaberturas` | `int` | Cadastro (0) / reabrir (+1) / importação | Quantas vezes o defeito já foi reaberto. Usado no desafio adicional |

**Exemplo de um defeito na memória**, depois de reaberto e atribuído:

```
codigo           = 105
modulo           = "Pagamento"
descricao        = "Cobranca duplicada ao clicar duas vezes"
severidade       = 3
status           = "Em correcao"
responsavel      = "Ana"
correcao         = ""
resultadoReteste = "Reprovado"
reaberturas      = 1
```

### 6.2 `vector<Defeito> cadastro`

- É **a lista de todos os defeitos** do sistema.
- `vector` é um "vetor que cresce sozinho". Cada `cadastro.push_back(d)` adiciona um defeito no final.
- Cada defeito é acessado pela sua **posição** (`cadastro[0]`, `cadastro[1]`...). As funções `buscarDefeito` e `solicitarDefeito` convertem um **código** em **posição**.
- É global (declarado fora das funções) para que todas as funções do sistema possam ler e alterar a mesma lista sem precisar passá-la como parâmetro.

### 6.3 `int defeitos[4][3]` — a matriz obrigatória

```
               coluna 0   coluna 1   coluna 2
               (Baixa)    (Media)    (Critica)
linha 0 Login      .          .          .
linha 1 Cadastro   .          .          .
linha 2 Pagamento  .          .          .
linha 3 Relatorios .          .          .
```

- Cada célula `defeitos[m][s]` guarda **quantos** defeitos existem no módulo `m` com severidade `s + 1`.
- Exemplo: `defeitos[2][2]` é o número de defeitos **críticos** do módulo **Pagamento**.
- A matriz é **derivada** do cadastro: ela é recalculada pela função `atualizarMatriz()` sempre que algo que afeta a contagem muda. Assim ela nunca fica desatualizada (detalhes na seção 10).

---

## 7. Ciclo de vida de um defeito (máquina de estados)

### 7.1 Diagrama

```
                  atribuirDefeito()            registrarCorrecao()
       ┌──────┐  ───────────────►  ┌─────────────┐  ─────────────►  ┌──────────────────┐
       │ Novo │                    │ Em correcao │                  │ Aguardando teste │
       └──────┘                    └─────────────┘                  └──────────────────┘
                                     ▲        ▲ (troca de                │ executarReteste()
                                     │        │  responsável)            ▼
                      atribuirDefeito()       └──────          ┌────────────┴─────────────┐
                                     │                         ▼                          ▼
                                ┌──────────┐            ┌──────────┐               ┌───────────┐
                                │ Reaberto │ ◄───────── │Reprovado │               │ Aprovado  │
                                └──────────┘ reabrir    └──────────┘               └───────────┘
                                     ▲       Defeito()                                    │ encerrarDefeito()
                                     │                                                    ▼
                                     │                 reabrirDefeito()            ┌───────────┐
                                     └─────────────── (defeito recorrente) ─────── │ Encerrado │
                                                                                   └───────────┘
```

### 7.2 Tabela de transições

Esta tabela mostra **exatamente** quais mudanças de status são permitidas. Qualquer tentativa fora dela é **recusada** com uma mensagem que mostra o status atual.

| Operação (função) | Status exigido antes | Status depois | Outros efeitos |
|---|---|---|---|
| Cadastrar (`cadastrarDefeito`) | — (defeito ainda não existe) | `Novo` | `reaberturas = 0` |
| Atribuir (`atribuirDefeito`) | `Novo`, `Reaberto` ou `Em correcao` | `Em correcao` | Grava o `responsavel` |
| Registrar correção (`registrarCorrecao`) | `Em correcao` | `Aguardando teste` | Grava `correcao` e apaga `resultadoReteste` |
| Reteste aprovado (`executarReteste`) | `Aguardando teste` | `Aprovado` | `resultadoReteste = "Aprovado"` |
| Reteste reprovado (`executarReteste`) | `Aguardando teste` | `Reprovado` | `resultadoReteste = "Reprovado"` |
| Encerrar (`encerrarDefeito`) | `Aprovado` | `Encerrado` | — |
| Reabrir (`reabrirDefeito`) | `Reprovado` ou `Encerrado` | `Reaberto` | `reaberturas++` |
| Alterar dados (`alterarDefeito`) | Qualquer um, **exceto** `Encerrado` | (não muda) | Pode mudar módulo, descrição, severidade ou responsável |

### 7.3 Por que essas regras?

- **Só encerrar com reteste aprovado** (requisito 6): garante que nada é fechado sem ter sido confirmado pelo analista.
- **Só reabrir Reprovado ou Encerrado** (requisito 7): Reprovado significa que a correção falhou. Encerrado que volta a dar problema é o defeito **recorrente**. Os outros status ainda estão "em andamento" e não fazem sentido reabrir.
- **Não alterar Encerrado** (requisito 8): o histórico de um defeito fechado fica preservado.
- **Atribuir também em `Em correcao`**: permite trocar o responsável sem quebrar o fluxo.

---

## 8. Funções utilitárias

São funções pequenas, de apoio, usadas por várias funcionalidades. Cada uma resolve um único problema.

### 8.1 `string trim(const string &s)`

| | |
|---|---|
| **Objetivo** | Remover espaços, tabulações e quebras de linha (`\n`, `\r`) do **início** e do **fim** de um texto |
| **Parâmetro** | `s`: texto original (passado por referência constante, sem cópia e sem alteração) |
| **Retorno** | Uma nova `string` sem os espaços das pontas |

**Como funciona:**
1. `ini` começa em 0 e avança enquanto o caractere for espaço.
2. `fim` começa no tamanho do texto e recua enquanto o caractere anterior for espaço.
3. Devolve o pedaço entre `ini` e `fim` com `substr`.

**Exemplos:**

| Entrada | Saída |
|---|---|
| `"  Login "` | `"Login"` |
| `"Novo\r"` (linha de arquivo do Windows) | `"Novo"` |
| `"   "` | `""` |

> O `(unsigned char)` antes de `isspace` evita comportamento indefinido com caracteres acentuados, que em alguns compiladores viram números negativos.

### 8.2 `string minusculo(string s)`

| | |
|---|---|
| **Objetivo** | Converter todas as letras para minúsculas |
| **Parâmetro** | `s`: recebido **por cópia**, então o original não muda |
| **Retorno** | O texto em minúsculas |
| **Uso** | Comparar textos sem diferenciar maiúsculas: `"PAGAMENTO"`, `"Pagamento"` e `"pagamento"` são considerados iguais |

### 8.3 `bool paraInteiro(const string &txt, int &valor)`

| | |
|---|---|
| **Objetivo** | Converter texto em número inteiro **positivo**, com segurança |
| **Parâmetros** | `txt`: o texto; `valor`: variável que **recebe** o número (passagem por referência) |
| **Retorno** | `true` se a conversão deu certo; `false` caso contrário |

**Regras (todas precisam ser verdadeiras):**
1. Depois do `trim`, o texto não pode estar vazio.
2. Todos os caracteres precisam ser dígitos (`0` a `9`). Sinal de menos, letras, ponto ou espaço no meio são recusados.
3. No máximo 9 dígitos, para não ultrapassar o limite do `int` (cerca de 2,1 bilhões).

**Exemplos:**

| `txt` | Retorno | `valor` |
|---|---|---|
| `"105"` | `true` | 105 |
| `" 7 "` | `true` | 7 |
| `"abc"` | `false` | (não alterado) |
| `"-3"` | `false` | (não alterado) |
| `"12a"` | `false` | (não alterado) |
| `"9999999999"` | `false` | (não alterado) |

> **Por que não usar `cin >> numero`?** Se o usuário digita uma letra, o `cin` entra em estado de erro e o programa pode entrar em laço infinito. Ler a linha como texto e validar com `paraInteiro` evita esse problema.

### 8.4 `string lerTexto(const string &msg)`

| | |
|---|---|
| **Objetivo** | Mostrar uma mensagem e ler **uma linha inteira** do teclado |
| **Parâmetro** | `msg`: texto mostrado antes da leitura (o "prompt") |
| **Retorno** | O que o usuário digitou, já com `trim` |

Usa `getline(cin, s)`, que lê até o Enter. Assim aceita textos com espaços ("Cobrança duplicada no checkout") e não deixa sobras no buffer do teclado, que é um problema comum quando se mistura `cin >>` com `getline`.

### 8.5 `int lerInteiro(const string &msg, int min, int max)`

| | |
|---|---|
| **Objetivo** | Ler um número inteiro dentro de uma faixa, insistindo até ser válido |
| **Parâmetros** | `msg`: prompt; `min` e `max`: limites aceitos (inclusive) |
| **Retorno** | Um número garantidamente entre `min` e `max` |

**Fluxo:**
```
repetir para sempre:
    ler linha
    se for inteiro E estiver entre min e max → devolver
    senão → mostrar "Valor invalido. Digite um numero entre min e max." e repetir
```

**Exemplo de interação:**
```
Severidade (1-Baixa, 2-Media, 3-Critica): 5
  Valor invalido. Digite um numero entre 1 e 3.
Severidade (1-Baixa, 2-Media, 3-Critica): x
  Valor invalido. Digite um numero entre 1 e 3.
Severidade (1-Baixa, 2-Media, 3-Critica): 3
```

### 8.6 `string lerTextoObrigatorio(const string &msg)`

| | |
|---|---|
| **Objetivo** | Ler um texto que **não pode ficar vazio** e **não pode conter `;`** |
| **Retorno** | Texto válido |

- Vazio → `Campo obrigatorio.` e pergunta de novo.
- Contém `;` → `O caractere ';' nao e permitido.` e pergunta de novo.

> **Por que proibir `;`?** É o separador de campos do `defeitos.txt`. Uma descrição com `;` quebraria o formato se os dados fossem gravados em arquivo no futuro.

### 8.7 `int indiceModulo(const string &nome)`

| | |
|---|---|
| **Objetivo** | Descobrir a posição (0 a 3) de um módulo pelo nome |
| **Retorno** | A posição, ou `-1` se o nome não for um módulo válido |
| **Detalhe** | Compara em minúsculas e sem espaços nas pontas: `" pagamento "` encontra `Pagamento` (posição 2) |

É usada para validar módulos do arquivo e para saber em **qual linha da matriz** cada defeito entra.

### 8.8 `string normalizarStatus(const string &s)`

| | |
|---|---|
| **Objetivo** | Transformar o status lido do arquivo na grafia padrão do programa |
| **Retorno** | Um dos 7 status padrão, ou `""` se o status for inválido |

**Passos:**
1. `trim` + `minusculo`.
2. Troca letras acentuadas por letras sem acento. Em UTF-8, `ç` ocupa **2 bytes** (`\xC3\xA7`), `ã` ocupa `\xC3\xA3` e `Ç` ocupa `\xC3\x87`. A função procura esses pares de bytes e os substitui por `c` ou `a`.
3. Compara com cada um dos 7 status (também em minúsculas).

**Exemplos:**

| Entrada no arquivo | Resultado |
|---|---|
| `Em correção` | `Em correcao` |
| `EM CORRECAO` | `Em correcao` |
| `aguardando teste` | `Aguardando teste` |
| `Pendente` | `""` (inválido → linha rejeitada) |

### 8.9 `int buscarDefeito(int codigo)`

| | |
|---|---|
| **Objetivo** | Encontrar um defeito pelo código |
| **Retorno** | Posição no vetor `cadastro`, ou `-1` se não existir |
| **Método** | Busca sequencial: percorre o vetor do início ao fim comparando os códigos |

É usada para **evitar duplicidade** (cadastro e importação) e para **localizar** o defeito nas demais operações.

### 8.10 `int solicitarDefeito()`

| | |
|---|---|
| **Objetivo** | Pedir um código ao usuário e devolver a posição do defeito |
| **Retorno** | Posição no `cadastro`, ou `-1` (já com a mensagem de erro exibida) |

**Fluxo:**
1. Se o cadastro está vazio → `Nenhum defeito cadastrado.` → `-1`.
2. Lê o código com `lerInteiro`.
3. Se não encontrar → `Defeito X nao encontrado.` → `-1`.

Ela existe para não repetir esse mesmo trecho em todas as funcionalidades (consultar, atribuir, corrigir, retestar, encerrar, reabrir, alterar).

### 8.11 `int escolherModulo()`

Mostra `Modulos: 1-Login  2-Cadastro  3-Pagamento  4-Relatorios`, lê um número de 1 a 4 e devolve o **índice** (0 a 3). Escolher por número impede que o usuário digite um nome de módulo errado.

### 8.12 `void exibirDefeito(const Defeito &d)`

Mostra a ficha completa de um defeito. Campos vazios aparecem como `-`. A severidade aparece com número e nome. Exemplo real:

```
  ------------------------------------------------
  Codigo........: 105
  Modulo........: Pagamento
  Descricao.....: Cobranca duplicada ao clicar duas vezes
  Severidade....: 3 (Critica)
  Status........: Reprovado
  Responsavel...: -
  Correcao......: -
  Ult. reteste..: Reprovado
  Reaberturas...: 0
  ------------------------------------------------
```

### 8.13 `void atualizarMatriz()`

Recalcula a matriz `defeitos[4][3]` a partir do cadastro. Está explicada em detalhe na [seção 10.2](#102-preenchimento-atualizarmatriz).

---

## 9. Funcionalidades (requisitos 1 a 11)

Todas seguem o mesmo padrão:
1. Mostram um título (`=== NOME DA OPERACAO ===`).
2. Localizam o defeito com `solicitarDefeito()` (quando a operação é sobre um defeito existente).
3. **Verificam o status** e recusam a operação se não for permitida.
4. Leem os dados necessários.
5. Alteram o defeito e mostram uma confirmação.

> **Detalhe técnico importante:** as funções usam `Defeito &d = cadastro[idx];`. O `&` cria uma **referência**, ou seja, `d` passa a ser outro nome para o defeito que está dentro do vetor. Alterar `d.status` altera o defeito do cadastro. Sem o `&`, seria alterada apenas uma **cópia**, e a mudança se perderia.

### 9.1 Requisito 1 — `cadastrarDefeito()`

**O que faz:** registra um novo defeito com status `Novo`.

**Passo a passo:**
1. Lê o código (1 a 999.999.999).
2. Se o código já existe → `Ja existe um defeito com o codigo X. Cadastro cancelado.` e volta ao menu.
3. Lê o módulo (por número), a descrição (obrigatória) e a severidade (1 a 3).
4. Define `status = "Novo"` e `reaberturas = 0`. Os campos de texto ficam vazios.
5. Adiciona ao cadastro (`push_back`) e chama `atualizarMatriz()`.

**Exemplo real:**
```
=== CADASTRAR DEFEITO ===
Codigo (ID numerico): 200
Modulos: 1-Login  2-Cadastro  3-Pagamento  4-Relatorios
Escolha o modulo (1-4): 3
Descricao resumida: Tela trava ao salvar
Severidade (1-Baixa, 2-Media, 3-Critica): 3
  Defeito 200 cadastrado com status "Novo".
```

### 9.2 Requisito 2 — `consultarDefeito()`

**O que faz:** pede um código e mostra a ficha completa com `exibirDefeito` (exemplo na seção 8.12).

### 9.3 Extra — `listarDefeitos()`

**O que faz:** mostra todos os defeitos em uma tabela. Não é um requisito do enunciado, mas facilita a demonstração.

- `setw(n)` reserva `n` colunas para cada campo, o que deixa a tabela alinhada.
- `left` alinha o texto à esquerda. No final, `cout << right;` volta ao padrão para não bagunçar as outras telas.

**Exemplo real (após importar o `defeitos.txt`):**
```
Codigo  Modulo      Sev  Status            Responsavel   Descricao
--------------------------------------------------------------------------------
101     Login       2    Novo              -             Senha aceita com espacos no final
102     Login       3    Em correcao       -             Bloqueio apos 3 tentativas nao funciona
103     Cadastro    3    Novo              -             CPF invalido aceito no formulario
104     Cadastro    1    Aguardando teste  -             Mensagem de erro com texto cortado
105     Pagamento   3    Reprovado         -             Cobranca duplicada ao clicar duas vezes
106     Pagamento   2    Aprovado          -             Arredondamento incorreto no desconto
107     Relatorios  1    Encerrado         -             Exportacao PDF sem cabecalho
108     Relatorios  2    Reaberto          -             Filtro por data ignora o ultimo dia
109     Pagamento   3    Novo              -             Timeout no gateway nao exibe aviso
```

### 9.4 Requisito 3 — `atribuirDefeito()`

**O que faz:** define o responsável e coloca o defeito em `Em correcao`.

| | |
|---|---|
| **Pré-condição** | Status `Novo`, `Reaberto` ou `Em correcao` |
| **Lê** | Nome do responsável (obrigatório) |
| **Efeito** | `responsavel = nome`, `status = "Em correcao"` |
| **Recusa** | `So e possivel atribuir defeitos com status Novo, Reaberto ou Em correcao.` + status atual |

**Exemplo:**
```
Codigo do defeito: 105
Nome do responsavel: Ana
  Defeito 105 atribuido a Ana. Status: Em correcao.
```

### 9.5 Requisito 4 — `registrarCorrecao()`

**O que faz:** registra o que foi corrigido e envia o defeito para reteste.

| | |
|---|---|
| **Pré-condição** | Status `Em correcao` |
| **Lê** | Descrição da correção (obrigatória) |
| **Efeito** | `correcao = texto`, `status = "Aguardando teste"`, `resultadoReteste = ""` |
| **Recusa** | `A correcao so pode ser registrada para defeitos "Em correcao".` |

> O `resultadoReteste` é apagado porque a correção é nova e ainda não foi testada. O resultado antigo não vale mais.

### 9.6 Requisito 5 — `executarReteste()`

**O que faz:** registra se a correção passou no reteste.

| | |
|---|---|
| **Pré-condição** | Status `Aguardando teste` |
| **Lê** | `1` = Aprovado, `2` = Reprovado |
| **Efeito** | `resultadoReteste` e `status` recebem `"Aprovado"` ou `"Reprovado"` |
| **Extra** | Se reprovado, avisa: `Use a opcao "Reabrir defeito" para devolve-lo a correcao.` |

O operador ternário `(r == 1) ? ST_APROVADO : ST_REPROVADO` significa: "se `r` for 1, use Aprovado; senão, use Reprovado".

### 9.7 Requisito 6 — `encerrarDefeito()`

**O que faz:** fecha o defeito, **somente se o reteste estiver aprovado**.

| Situação | Resultado |
|---|---|
| Status `Aprovado` | `Defeito X encerrado.` → status `Encerrado` |
| Status `Encerrado` | `O defeito ja esta encerrado.` |
| Qualquer outro | `Nao e possivel encerrar: o reteste precisa estar APROVADO.` + status atual |

**Exemplo de recusa (real):**
```
Codigo do defeito: 101
  Nao e possivel encerrar: o reteste precisa estar APROVADO.
  Status atual: Novo
```

### 9.8 Requisito 7 — `reabrirDefeito()`

**O que faz:** devolve o defeito para o ciclo de correção.

| Status antes | Motivo exibido | Interpretação |
|---|---|---|
| `Reprovado` | `reprovado no reteste` | A correção não resolveu |
| `Encerrado` | `recorrente` | O defeito tinha sido resolvido, mas voltou a acontecer |

**Efeito:** `status = "Reaberto"` e `reaberturas` aumenta em 1. O motivo é guardado em uma variável **antes** de mudar o status, porque depois não daria mais para saber qual era o status anterior.

**Exemplo real:**
```
Codigo do defeito: 105
  Defeito 105 reaberto (reprovado no reteste). Total de reaberturas: 1.
  Atribua um responsavel para retomar a correcao.
```

### 9.9 Requisito 8 — `alterarDefeito()`

**O que faz:** corrige dados de um defeito **não encerrado**.

**Passo a passo:**
1. Recusa se o status for `Encerrado` (`Defeitos encerrados nao podem ser alterados.`).
2. Mostra a ficha atual.
3. Mostra o submenu:
   ```
   1 - Modulo
   2 - Descricao
   3 - Severidade
   4 - Responsavel
   0 - Cancelar
   ```
4. Aplica a alteração escolhida.
5. Chama `atualizarMatriz()`, porque o módulo ou a severidade podem ter mudado e a matriz precisa refletir isso.

**Regra especial da opção 4:** um defeito `Novo` não pode receber responsável por aqui, porque atribuir também precisa mudar o status para `Em correcao`. O sistema orienta a usar a opção "Atribuir responsavel" do menu principal.

### 9.10 Requisito 9 — `verificarCriticos()`

**O que faz:** emite um alerta para cada defeito com **severidade 3** e status **diferente de `Encerrado`**. Os que ainda não têm responsável recebem o destaque `SEM RESPONSAVEL`.

**Exemplo real:**
```
=== ALERTA: DEFEITOS CRITICOS NAO ENCERRADOS ===
  [!] ALERTA - Defeito 102 | Login | Em correcao | Bloqueio apos 3 tentativas nao funciona | SEM RESPONSAVEL
  [!] ALERTA - Defeito 103 | Cadastro | Novo | CPF invalido aceito no formulario | SEM RESPONSAVEL
  [!] ALERTA - Defeito 109 | Pagamento | Novo | Timeout no gateway nao exibe aviso | SEM RESPONSAVEL
  Total de criticos pendentes: 3
```

> O defeito 102 aparece "Em correcao" mas "SEM RESPONSAVEL" porque foi **importado** já nesse status, e o arquivo não tem a coluna de responsável. É justamente o tipo de situação que o alerta ajuda a encontrar.

Se não houver nenhum: `Nenhum defeito critico pendente.`

### 9.11 Requisito 10 — `gerarRelatorioDefeitos()`

Explicado em detalhe na [seção 12](#12-relatório-e-desafio-adicional-proporção-de-reabertura).

### 9.12 Requisito 11 — `importarDefeitosArquivo()`

Explicado em detalhe na [seção 11](#11-atividade-obrigatória-com-leitura-de-arquivo).

---

## 10. Atividade obrigatória com matriz

### 10.1 Enunciado

> Matriz `int defeitos[4][3]`: armazenar defeitos encontrados em 4 módulos e 3 níveis de severidade. Calcular total por módulo, por severidade e localizar módulo com mais falhas críticas. A implementação deve usar dois laços de repetição aninhados.

### 10.2 Preenchimento: `atualizarMatriz()`

**Passo 1: zerar a matriz com dois laços aninhados.**
```cpp
for (int m = 0; m < NUM_MODULOS; m++)          // percorre as linhas (módulos)
    for (int s = 0; s < NUM_SEVERIDADES; s++)  // percorre as colunas (severidades)
        defeitos[m][s] = 0;
```
O laço externo roda 4 vezes. Para **cada** volta dele, o interno roda 3 vezes. No total, 4 × 3 = 12 células são zeradas.

**Passo 2: contar os defeitos.**
```cpp
for (cada defeito do cadastro) {
    m = indiceModulo(defeito.modulo);   // linha
    s = defeito.severidade - 1;         // coluna
    defeitos[m][s]++;                   // soma 1 na célula
}
```

**Por que recalcular do zero em vez de só somar 1 a cada cadastro?** Porque um defeito pode **mudar** de módulo ou de severidade (opção Alterar). Recalcular tudo garante que a matriz sempre bate com o cadastro, sem risco de contar duas vezes ou esquecer de subtrair.

**Quando é chamada:** no início do `main`, depois de cadastrar, depois de alterar, depois de importar e no começo de `exibirMatriz`.

### 10.3 Cálculos: `exibirMatriz()`

**Totais com dois laços aninhados:**
```cpp
for (int m = 0; m < NUM_MODULOS; m++) {
    for (int s = 0; s < NUM_SEVERIDADES; s++) {
        totalModulo[m]     += defeitos[m][s];  // soma da LINHA m
        totalSeveridade[s] += defeitos[m][s];  // soma da COLUNA s
        totalGeral         += defeitos[m][s];  // soma de tudo
    }
}
```
Em uma única passada pela matriz, cada célula é somada em três acumuladores diferentes.

**Teste de mesa** com os dados do `defeitos.txt` de exemplo:

| m | s | `defeitos[m][s]` | `totalModulo[m]` depois | `totalSeveridade[s]` depois |
|---|---|---|---|---|
| 0 Login | 0 | 0 | 0 | B=0 |
| 0 Login | 1 | 1 | 1 | M=1 |
| 0 Login | 2 | 1 | 2 | C=1 |
| 1 Cadastro | 0 | 1 | 1 | B=1 |
| 1 Cadastro | 1 | 0 | 1 | M=1 |
| 1 Cadastro | 2 | 1 | 2 | C=2 |
| 2 Pagamento | 0 | 0 | 0 | B=1 |
| 2 Pagamento | 1 | 1 | 1 | M=2 |
| 2 Pagamento | 2 | 2 | 3 | C=4 |
| 3 Relatorios | 0 | 1 | 1 | B=2 |
| 3 Relatorios | 1 | 1 | 2 | M=3 |
| 3 Relatorios | 2 | 0 | 2 | C=4 |

**Resultado impresso (real):**
```
--- Matriz defeitos[4][3] (modulo x severidade) ---
Modulo          Baixa    Media  Critica    Total
------------------------------------------------
Login               0        1        1        2
Cadastro            1        0        1        2
Pagamento           0        1        2        3
Relatorios          1        1        0        2
------------------------------------------------
Total               2        3        4        9

Modulo com mais falhas criticas: Pagamento (2)
```

### 10.4 Módulo com mais falhas críticas

O programa procura o **maior valor da coluna 2** (Crítica):

```cpp
maior = -1; idxMaior = -1; empate = false;
para cada módulo m:
    criticos = defeitos[m][2]
    se criticos > maior  → maior = criticos; idxMaior = m; empate = false   (novo recorde)
    senão se criticos == maior → empate = true                             (igualou o recorde)
```

- `maior` começa em `-1` para que o primeiro módulo (mesmo com 0) vire o recorde inicial.
- Quando aparece um novo recorde, `empate` volta a `false`, porque o empate antigo era com um valor menor.

**Três resultados possíveis:**

| Situação | Mensagem |
|---|---|
| Nenhum crítico em nenhum módulo | `Nenhum modulo possui falhas criticas.` |
| Um módulo lidera | `Modulo com mais falhas criticas: Pagamento (2)` |
| Empate | `Modulo com mais falhas criticas: Login (1) - empatado com: Cadastro` |

---

## 11. Atividade obrigatória com leitura de arquivo

### 11.1 Enunciado

> Arquivo `defeitos.txt` (texto UTF-8, uma linha por registro, primeira linha opcional como cabeçalho). Campos separados por ponto e vírgula: `codigo;modulo;descricao;severidade;status`. Validar códigos, severidade e status, evitando duplicidade e mantendo os relatórios atualizados. Usar `ifstream`, percorrer os registros e produzir um resumo de importação. Se o arquivo não existir ou não abrir, apresentar mensagem clara e permitir que o menu continue funcionando.

### 11.2 Formato do arquivo

```
codigo;modulo;descricao;severidade;status
101;Login;Senha aceita com espacos no final;2;Novo
```

| Posição | Campo | Regra de validação |
|---|---|---|
| 1 | `codigo` | Inteiro positivo, não repetido |
| 2 | `modulo` | `Login`, `Cadastro`, `Pagamento` ou `Relatorios` (sem diferenciar maiúsculas) |
| 3 | `descricao` | Não pode ser vazia |
| 4 | `severidade` | Inteiro de 1 a 3 |
| 5 | `status` | Um dos 7 status (aceita com ou sem acento, maiúsculas ou minúsculas) |

### 11.3 Assinatura

```cpp
void importarDefeitosArquivo(const string &nomeArquivo = "defeitos.txt");
```

O parâmetro tem **valor padrão**: chamar `importarDefeitosArquivo()` sem argumentos lê o `defeitos.txt`. Também é possível chamar `importarDefeitosArquivo("outro.txt")` para ler outro arquivo.

### 11.4 Algoritmo passo a passo

```
1. Abrir o arquivo com ifstream.
   Se não abrir → mensagem de erro → return (volta ao menu).

2. Para cada linha do arquivo (getline):
   a. numLinha++
   b. Se for a 1ª linha e começar com o BOM UTF-8 (bytes EF BB BF) → remover.
   c. trim() → remove espaços e o '\r' de arquivos do Windows.
   d. Linha vazia → ignoradas++ → próxima linha.
   e. Quebrar a linha nos ';' com stringstream + getline(ss, campo, ';').
   f. Se ainda não leu nenhum registro e o 1º campo for "codigo" → é o cabeçalho → ignoradas++ → próxima.
   g. lidos++
   h. Validar, na ordem:
        - tem 5 campos?
        - código é inteiro positivo?
        - código já existe no cadastro?
        - módulo é válido?
        - descrição não vazia?
        - severidade entre 1 e 3?
        - status válido?
      Na primeira falha → guarda o motivo em 'erros', rejeitados++ e passa para a próxima linha.
   i. Tudo certo → monta o Defeito, faz push_back no cadastro, importados++.

3. Fechar o arquivo e chamar atualizarMatriz().
4. Mostrar o resumo e a lista de rejeições.
```

### 11.5 Detalhes importantes

**Como a linha é dividida:**
```cpp
stringstream ss(linha);          // trata a linha como se fosse uma "entrada"
while (getline(ss, campo, ';'))  // lê até cada ';'
    campos.push_back(trim(campo));
```
`"101;Login;Erro X;2;Novo"` vira `["101", "Login", "Erro X", "2", "Novo"]`.

**Duplicidade dentro do próprio arquivo:** como cada linha válida já é adicionada ao cadastro antes de ler a próxima, um código repetido **mais abaixo no mesmo arquivo** também é detectado pelo `buscarDefeito`. Também é detectado se o usuário importar o arquivo duas vezes (na segunda vez, os 9 são rejeitados como duplicados).

**BOM UTF-8:** o Bloco de Notas do Windows pode gravar 3 bytes invisíveis (`EF BB BF`) no início do arquivo. Sem tratá-los, o primeiro campo seria `"\xEF\xBB\xBFcodigo"` e o cabeçalho não seria reconhecido.

**Campos derivados na importação:**
- `modulo` é gravado na **grafia padrão** (`login` → `Login`), para que contagens e relatórios funcionem.
- Status `Reaberto` → `reaberturas = 1` (o defeito já foi reaberto pelo menos uma vez).
- Status `Aprovado`/`Reprovado` → `resultadoReteste` recebe o mesmo valor.
- `responsavel` e `correcao` ficam vazios, porque o arquivo não tem esses campos.

**`ostringstream motivo`:** funciona como um `cout` que escreve em uma string. Assim dá para montar mensagens como `"  Linha 13: severidade invalida ("5"), use 1 a 3."` juntando textos e números.

### 11.6 Arquivo inexistente

```
=== IMPORTAR DEFEITOS DO ARQUIVO "defeitos.txt" ===
  ERRO: nao foi possivel abrir o arquivo "defeitos.txt".
  Verifique se ele existe na mesma pasta do programa.
  O sistema continua funcionando normalmente.
```
A função apenas retorna, o menu aparece de novo e todas as outras opções continuam funcionando.

### 11.7 O `defeitos.txt` de exemplo, linha por linha

| Linha | Conteúdo | Resultado | Motivo |
|---|---|---|---|
| 1 | `codigo;modulo;descricao;severidade;status` | Ignorada | Cabeçalho |
| 2 | `101;Login;...;2;Novo` | Importada | — |
| 3 | `102;Login;...;3;Em correcao` | Importada | — |
| 4 | `103;Cadastro;...;3;Novo` | Importada | — |
| 5 | `104;Cadastro;...;1;Aguardando teste` | Importada | — |
| 6 | `105;Pagamento;...;3;Reprovado` | Importada | — |
| 7 | `106;Pagamento;...;2;Aprovado` | Importada | — |
| 8 | `107;Relatorios;...;1;Encerrado` | Importada | — |
| 9 | `108;Relatorios;...;2;Reaberto` | Importada | — |
| 10 | `109;Pagamento;...;3;Novo` | Importada | — |
| 11 | `abc;Login;...;2;Novo` | **Rejeitada** | Código não numérico |
| 12 | `110;Estoque;...;2;Novo` | **Rejeitada** | Módulo inexistente |
| 13 | `111;Login;...;5;Novo` | **Rejeitada** | Severidade fora de 1–3 |
| 14 | `112;Cadastro;...;2;Pendente` | **Rejeitada** | Status desconhecido |
| 15 | `101;Login;...;1;Novo` | **Rejeitada** | Código 101 duplicado |

**Resumo impresso (real):**
```
  --------- Resumo da importacao ---------
  Registros lidos.......: 14
  Importados com sucesso: 9
  Rejeitados............: 5
  Linhas ignoradas......: 1 (cabecalho/vazias)
  Detalhes das rejeicoes:
  Linha 11: codigo invalido ("abc").
  Linha 12: modulo invalido ("Estoque").
  Linha 13: severidade invalida ("5"), use 1 a 3.
  Linha 14: status invalido ("Pendente").
  Linha 15: codigo 101 duplicado (ja cadastrado).
  Total no cadastro agora: 9
```

**Conferência:** lidos (14) = importados (9) + rejeitados (5). As linhas ignoradas (1) não entram em "lidos".

---

## 12. Relatório e desafio adicional (proporção de reabertura)

A função `gerarRelatorioDefeitos()` monta o relatório em **4 partes**.

### 12.1 Parte 1 — Por módulo e por severidade

Chama `exibirMatriz()` (ver seção 10.3): a matriz, os totais de linha e coluna e o módulo com mais falhas críticas.

### 12.2 Parte 2 — Por status

Para cada um dos 7 status, percorre o cadastro e conta quantos defeitos estão nele (laço dentro de laço: 7 status × N defeitos).

```
--- Defeitos por status ---
  Novo                 3
  Em correcao          1
  Aguardando teste     1
  Aprovado             1
  Reprovado            1
  Encerrado            1
  Reaberto             1
```

### 12.3 Parte 3 — Status dentro de cada módulo

Cruza módulo × status (três laços: módulo → status → defeitos). Só mostra os status com pelo menos 1 defeito. Se o módulo não tiver nenhum, mostra `(sem defeitos)`.

```
--- Status por modulo ---
  Login:  Novo=1  Em correcao=1
  Cadastro:  Novo=1  Aguardando teste=1
  Pagamento:  Novo=1  Aprovado=1  Reprovado=1
  Relatorios:  Encerrado=1  Reaberto=1
```

### 12.4 Parte 4 — Desafio adicional: proporção de reabertura

**Fórmula:**

```
proporção (%) = (defeitos com reaberturas > 0) / (total de defeitos) × 100
```

São mostrados dois números diferentes:
- **Defeitos que precisaram de reabertura:** conta cada defeito **uma vez**, mesmo que tenha sido reaberto várias vezes. É a resposta direta ao desafio.
- **Total de reaberturas registradas:** soma todas as reaberturas. Mostra o retrabalho total (um defeito reaberto 3 vezes conta 3).

**Por que `100.0` e não `100`?** Em C++, `int / int` descarta a parte decimal (`1 / 9 = 0`). Multiplicar por `100.0` (um `double`) faz a conta inteira ser feita com casas decimais.

**Formatação:** `fixed << setprecision(1)` mostra exatamente 1 casa decimal. Depois, `cout.unsetf(ios::fixed)` desfaz isso para não afetar outras telas.

```
--- Reabertura (desafio adicional) ---
  Defeitos que precisaram de reabertura: 1 de 9 (11.1%)
  Total de reaberturas registradas: 1
```
Conta: 1 / 9 × 100 = 11,11… → `11.1%`.

---

## 13. Menu principal e função `main()`

### 13.1 Opções

| Opção | Função chamada |
|---|---|
| 1 | `cadastrarDefeito()` |
| 2 | `consultarDefeito()` |
| 3 | `listarDefeitos()` |
| 4 | `atribuirDefeito()` |
| 5 | `registrarCorrecao()` |
| 6 | `executarReteste()` |
| 7 | `encerrarDefeito()` |
| 8 | `reabrirDefeito()` |
| 9 | `alterarDefeito()` |
| 10 | `verificarCriticos()` |
| 11 | `gerarRelatorioDefeitos()` |
| 12 | `importarDefeitosArquivo()` |
| 0 | Sai do programa |

### 13.2 Funcionamento do `main()`

```cpp
atualizarMatriz();                       // matriz começa zerada
do {
    exibirMenu();
    opcao = lerInteiro("Escolha uma opcao: ", 0, 12);   // só aceita 0..12
    switch (opcao) { ... }               // chama a função correspondente
} while (opcao != 0 && cin.good());
```

- **`do ... while`**: o menu aparece **pelo menos uma vez**, e o laço repete até a opção ser 0.
- **`cin.good()`**: se a entrada acabar (por exemplo, Ctrl+D no Linux ou Ctrl+Z no Windows, ou um arquivo redirecionado que terminou), o laço para em vez de girar para sempre.
- Como `lerInteiro` só aceita valores de 0 a 12, o `switch` não precisa de `default`.

---

## 14. Tratamento de erros e validações de entrada

| Situação | Como o sistema reage |
|---|---|
| Letra onde se espera número | `Valor invalido. Digite um numero entre X e Y.` e pergunta de novo |
| Número fora da faixa | Mesma mensagem, pergunta de novo |
| Texto obrigatório vazio | `Campo obrigatorio.` e pergunta de novo |
| Texto com `;` | `O caractere ';' nao e permitido.` e pergunta de novo |
| Código repetido no cadastro | `Ja existe um defeito com o codigo X. Cadastro cancelado.` |
| Código inexistente | `Defeito X nao encontrado.` |
| Cadastro vazio | `Nenhum defeito cadastrado.` |
| Operação fora do ciclo de vida | Mensagem explicando a regra + `Status atual: ...` |
| Alterar defeito encerrado | `Defeitos encerrados nao podem ser alterados.` |
| Arquivo inexistente | Mensagem de erro e o menu continua |
| Linha inválida no arquivo | Linha rejeitada, motivo listado no resumo |
| Fim da entrada (Ctrl+D/Ctrl+Z) | O programa termina sem travar |

**Princípio geral:** o programa **nunca termina por erro de digitação** e **nunca aceita um dado inválido**. Todo erro tem uma mensagem que explica o que aconteceu.

---

## 15. Mapa requisito → função

| # | Requisito do enunciado | Função | Seção |
|---|---|---|---|
| 1 | Cadastrar defeito (ID, módulo, descrição, severidade 1–3, status Novo) | `cadastrarDefeito()` | 9.1 |
| 2 | Consultar defeito | `consultarDefeito()` | 9.2 |
| 3 | Atribuir responsável → Em correção | `atribuirDefeito()` | 9.4 |
| 4 | Registrar correção → Aguardando teste | `registrarCorrecao()` | 9.5 |
| 5 | Registrar reteste (Aprovado/Reprovado) | `executarReteste()` | 9.6 |
| 6 | Encerrar apenas com reteste aprovado | `encerrarDefeito()` | 9.7 |
| 7 | Reabrir defeito reprovado ou recorrente | `reabrirDefeito()` | 9.8 |
| 8 | Alterar dados de defeito não encerrado | `alterarDefeito()` | 9.9 |
| 9 | Alertas de críticos não encerrados | `verificarCriticos()` | 9.10 |
| 10 | Relatório por módulo, severidade e status | `gerarRelatorioDefeitos()` | 12 |
| 11 | Ler `defeitos.txt` | `importarDefeitosArquivo()` | 11 |
| — | Matriz `int defeitos[4][3]` com laços aninhados | `atualizarMatriz()`, `exibirMatriz()` | 10 |
| — | Desafio: proporção de reabertura | `gerarRelatorioDefeitos()` (parte 4) | 12.4 |
| extra | Listar todos os defeitos | `listarDefeitos()` | 9.3 |

---

## 16. Roteiro de teste / demonstração

Sequência sugerida para apresentar o trabalho, mostrando todos os requisitos. Comece com o `defeitos.txt` na pasta do programa.

| Passo | Opção | Entrada | Resultado esperado |
|---|---|---|---|
| 1 | 12 | — | 9 importados, 5 rejeitados com os motivos |
| 2 | 3 | — | Tabela com os 9 defeitos |
| 3 | 11 | — | Matriz, Pagamento com mais críticos (2), proporção 11.1% |
| 4 | 10 | — | 3 alertas (102, 103, 109) |
| 5 | 7 | `101` | Recusado: reteste precisa estar APROVADO |
| 6 | 4 | `105` | Recusado: status Reprovado não pode ser atribuído |
| 7 | 8 | `105` | Reaberto (reprovado no reteste), 1 reabertura |
| 8 | 4 | `105`, `Ana` | Em correção, responsável Ana |
| 9 | 5 | `105`, `Bloqueio de duplo clique` | Aguardando teste |
| 10 | 6 | `105`, `1` | Aprovado |
| 11 | 7 | `105` | Encerrado |
| 12 | 9 | `105` | Recusado: encerrados não podem ser alterados |
| 13 | 8 | `105` | Reaberto (recorrente), 2 reaberturas |
| 14 | 1 | `200`, `3`, `Tela trava ao salvar`, `3` | Defeito 200 cadastrado como Novo |
| 15 | 1 | `200` | Recusado: código já existe |
| 16 | 11 | — | Pagamento agora com 3 críticos; reabertura 2 de 10 (20.0%), 3 reaberturas no total |
| 17 | 12 | — | Todos os 9 rejeitados como duplicados (evita duplicidade) |
| 18 | — | Renomear o arquivo e usar a opção 12 | Mensagem de arquivo inexistente; o menu continua |
| 19 | 0 | — | Encerra o programa |

---

## 17. Decisões de projeto

| Decisão | Motivo |
|---|---|
| `vector` em vez de array fixo para o cadastro | Não há limite artificial de defeitos e não é preciso controlar o tamanho manualmente |
| Matriz recalculada do zero (`atualizarMatriz`) | Garante consistência mesmo quando módulo ou severidade mudam |
| Leitura sempre com `getline` + validação | Evita os problemas clássicos do `cin >>` (laço infinito com letras, buffer sujo) |
| Status como constantes `string` | Os textos aparecem direto na tela e no arquivo, sem conversão, e as constantes evitam erros de digitação |
| Escolha de módulo por número | Impede módulos inválidos no cadastro manual |
| Mensagens sem acento | Evita caracteres estranhos no console do Windows, que muitas vezes não usa UTF-8 |
| Arquivo aceita acentos | Respeita o enunciado (UTF-8) e é tolerante com quem digita "correção" |
| Funções sem parâmetros (como sugerido no enunciado) | Segue as assinaturas sugeridas. Os dados ficam em variáveis globais compartilhadas |
| `importarDefeitosArquivo` com parâmetro opcional | Mantém a chamada sugerida e permite ler outros arquivos |
| `solicitarDefeito()` centralizado | Evita repetir em 7 funções o trecho "ler código → buscar → mensagem de erro" |

---

## 18. Limitações conhecidas e possíveis melhorias

1. **Os dados ficam só na memória:** ao sair do programa, alterações e cadastros manuais são perdidos. Melhoria: uma função `salvarDefeitosArquivo()` usando `ofstream`, no mesmo formato do `defeitos.txt`.
2. **Sem histórico:** só o estado atual é guardado. Melhoria: um `vector<string>` de eventos em cada defeito (data, ação, usuário).
3. **Módulos fixos:** os 4 módulos são fixos porque a matriz tem tamanho fixo `[4][3]`, como pede o enunciado.
4. **O arquivo não tem responsável nem correção:** defeitos importados já "Em correcao" ficam sem responsável (o alerta de críticos mostra isso).
5. **Busca sequencial:** `buscarDefeito` percorre o vetor inteiro. Para milhares de defeitos, um `map<int, Defeito>` seria mais rápido. Para o tamanho do trabalho, a busca sequencial é suficiente e mais simples de entender.
6. **Só reconhece alguns acentos no status:** são tratados `ç`, `Ç` e `ã`, que cobrem os 7 status ("Em correção"). Outros acentos não são necessários.

---

## 19. Glossário

| Termo | Significado |
|---|---|
| **Defeito / bug** | Falha encontrada no software durante os testes |
| **Módulo** | Parte do sistema testado (Login, Cadastro, Pagamento, Relatórios) |
| **Severidade** | Gravidade do defeito (1 Baixa, 2 Média, 3 Crítica) |
| **Status** | Situação atual do defeito no ciclo de vida |
| **Reteste** | Novo teste feito depois da correção para confirmar se ela funcionou |
| **Recorrente** | Defeito encerrado que voltou a acontecer |
| **Reabertura** | Volta de um defeito ao ciclo de correção |
| **Responsável** | Pessoa encarregada de corrigir o defeito |
| **Matriz** | Vetor de duas dimensões (linhas × colunas) |
| **Laços aninhados** | Um laço de repetição dentro de outro |
| **`struct`** | Estrutura que agrupa vários campos de tipos diferentes |
| **`vector`** | Lista dinâmica da biblioteca padrão do C++ |
| **Referência (`&`)** | Outro nome para uma variável já existente. Alterar a referência altera a variável original |
| **`ifstream`** | Objeto do C++ para ler arquivos |
| **BOM** | 3 bytes invisíveis (`EF BB BF`) que alguns editores colocam no início de arquivos UTF-8 |
| **UTF-8** | Forma de codificar texto em que letras acentuadas ocupam 2 bytes |
| **Prompt** | Mensagem exibida antes de pedir uma entrada ao usuário |
