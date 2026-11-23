# Bug Tracker em C++

Sistema de acompanhamento de defeitos de software feito em C++, com menu no console (Proposta 4).

Durante os testes de uma aplicação, os analistas registram as falhas encontradas. O sistema acompanha cada falha enquanto ela é corrigida e retestada, até ser encerrada.

> 📘 A documentação técnica completa, com cada função, regra, fluxo e exemplo explicados em detalhe, está em **[DOCUMENTACAO.md](DOCUMENTACAO.md)**.

## Como compilar e executar

```bash
g++ -std=c++11 -Wall -o bugtracker bugtracker.cpp
./bugtracker        # no Windows: bugtracker.exe
```

Deixe o arquivo `defeitos.txt` na mesma pasta do executável para usar a importação (opção 12 do menu).

## Funcionalidades

| Opção | Função | Descrição |
|---|---|---|
| 1 | `cadastrarDefeito()` | Cadastra um defeito (ID, módulo, descrição, severidade). Todo defeito começa com status **Novo** |
| 2 | `consultarDefeito()` | Mostra todos os dados de um defeito |
| 3 | `listarDefeitos()` | Lista todos os defeitos em uma tabela |
| 4 | `atribuirDefeito()` | Define o responsável e muda o status para **Em correcao** |
| 5 | `registrarCorrecao()` | Registra a correção feita e muda o status para **Aguardando teste** |
| 6 | `executarReteste()` | Registra o resultado do reteste: **Aprovado** ou **Reprovado** |
| 7 | `encerrarDefeito()` | Encerra o defeito, mas só se o reteste estiver aprovado |
| 8 | `reabrirDefeito()` | Reabre um defeito reprovado ou que voltou a acontecer (recorrente) |
| 9 | `alterarDefeito()` | Altera os dados de um defeito que ainda não foi encerrado |
| 10 | `verificarCriticos()` | Mostra alertas dos defeitos críticos que ainda não foram encerrados |
| 11 | `gerarRelatorioDefeitos()` | Relatório por módulo, severidade e status, com a proporção de reaberturas |
| 12 | `importarDefeitosArquivo()` | Lê o `defeitos.txt`, valida cada linha e mostra um resumo da importação |

## Ciclo de vida de um defeito

```
Novo -> Em correcao -> Aguardando teste -> Aprovado  -> Encerrado
                                        -> Reprovado -> Reaberto -> Em correcao ...
Encerrado (defeito voltou a acontecer) -> Reaberto
```

- **Severidade:** 1 = Baixa, 2 = Média, 3 = Crítica
- **Módulos:** Login, Cadastro, Pagamento, Relatorios

## Matriz `int defeitos[4][3]`

A matriz guarda quantos defeitos existem em cada **módulo** (linhas) para cada **severidade** (colunas). Com dois laços aninhados, o programa calcula:
- o total de defeitos por módulo;
- o total de defeitos por severidade;
- o módulo com mais falhas críticas (avisando se houver empate).

## Formato do `defeitos.txt`

Arquivo de texto UTF-8, com um defeito por linha. A primeira linha pode ser um cabeçalho (é opcional):

```
codigo;modulo;descricao;severidade;status
101;Login;Senha aceita com espacos no final;2;Novo
```

Uma linha é rejeitada quando o código é inválido ou repetido, o módulo não existe, a severidade não está entre 1 e 3, o status não é reconhecido ou a quantidade de campos está errada.

O arquivo de exemplo tem 9 registros válidos e 5 inválidos de propósito, para mostrar a validação funcionando.

Se o arquivo não existir, o programa mostra uma mensagem de erro e o menu continua funcionando.
