// ============================================================================
//  PROPOSTA 4 - Sistema de acompanhamento de defeitos de software (Bug Tracker)
// ============================================================================
//
//  O QUE ESTE PROGRAMA FAZ
//  -----------------------
//  Durante os testes de uma aplicacao, os analistas encontram falhas (defeitos).
//  Este sistema permite registrar esses defeitos e acompanhar cada um deles ate
//  que seja corrigido, retestado e encerrado. Tudo e feito por um menu de texto.
//
//  COMO COMPILAR E EXECUTAR
//  ------------------------
//    Compilar:  g++ -std=c++11 -Wall -o bugtracker bugtracker.cpp
//    Executar:  ./bugtracker          (no Windows: bugtracker.exe)
//    O arquivo defeitos.txt deve ficar na mesma pasta do executavel.
//
//  CONCEITOS PRINCIPAIS
//  --------------------
//  Severidade (gravidade do defeito):
//      1 = Baixa | 2 = Media | 3 = Critica
//
//  Modulos validos (partes do sistema testado). Sao exatamente 4 porque a
//  matriz obrigatoria do trabalho tem 4 linhas (uma por modulo):
//      Login, Cadastro, Pagamento, Relatorios
//
//  Ciclo de vida (status) de um defeito:
//
//      Novo --(atribuir)--> Em correcao --(registrar correcao)--> Aguardando teste
//                                                                       |
//                                                                  (reteste)
//                                                                  /        \  .
//                                                           Aprovado      Reprovado
//                                                              |              |
//                                                         (encerrar)      (reabrir)
//                                                              |              |
//                                                          Encerrado      Reaberto
//                                                              |              |
//                                         (reabrir, se o defeito voltar)  (atribuir)
//                                                              |              |
//                                                          Reaberto      Em correcao ...
//
//  ORGANIZACAO DO CODIGO (na ordem em que aparece neste arquivo)
//  -------------------------------------------------------------
//    1. Constantes ............ nomes de modulos, severidades e status
//    2. Estrutura de dados .... struct Defeito, vetor do cadastro e a matriz
//    3. Funcoes utilitarias ... leitura do teclado, conversoes, buscas, exibicao
//    4. Funcionalidades ....... uma funcao para cada requisito (1 a 11)
//    5. Menu e main() ......... laco principal que chama as funcionalidades
//
//  OBSERVACAO: as mensagens nao usam acentos para evitar caracteres estranhos
//  no console do Windows.
// ============================================================================

#include <iostream>   // cout, cin (entrada e saida no console)
#include <fstream>    // ifstream (leitura do arquivo defeitos.txt)
#include <sstream>    // stringstream (separar campos de uma linha por ';')
#include <string>     // tipo string
#include <vector>     // vector (lista dinamica de defeitos)
#include <cctype>     // isspace, isdigit, tolower (testes de caracteres)
#include <cstdlib>    // atoi (converter texto em numero)
#include <iomanip>    // setw, setprecision, left, right (formatar tabelas)

using namespace std;

// ============================================================================
// 1. CONSTANTES
// ----------------------------------------------------------------------------
// Valores fixos usados no programa inteiro. Deixa-los em um so lugar evita
// erros de digitacao (ex.: escrever "Em Correcao" em um lugar e "Em correcao"
// em outro) e facilita alterar algum nome no futuro.
// ============================================================================

const int NUM_MODULOS = 4;         // quantidade de modulos (linhas da matriz)
const int NUM_SEVERIDADES = 3;     // quantidade de severidades (colunas da matriz)
const int SEVERIDADE_CRITICA = 3;  // valor que representa um defeito critico

// Nomes dos modulos. A posicao no vetor (0 a 3) e a linha usada na matriz.
const string MODULOS[NUM_MODULOS] = {"Login", "Cadastro", "Pagamento", "Relatorios"};

// Nome de cada severidade. Severidade 1 fica na posicao 0, a 2 na 1 e a 3 na 2.
const string NOMES_SEVERIDADE[NUM_SEVERIDADES] = {"Baixa", "Media", "Critica"};

// Todos os status possiveis de um defeito (ver diagrama no topo do arquivo).
const string ST_NOVO        = "Novo";             // recem-cadastrado
const string ST_EM_CORRECAO = "Em correcao";      // um responsavel esta corrigindo
const string ST_AGUARDANDO  = "Aguardando teste"; // correcao feita, falta retestar
const string ST_APROVADO    = "Aprovado";         // reteste passou, pode encerrar
const string ST_REPROVADO   = "Reprovado";        // reteste falhou, precisa reabrir
const string ST_ENCERRADO   = "Encerrado";        // defeito resolvido e fechado
const string ST_REABERTO    = "Reaberto";         // voltou para ser corrigido de novo

// Lista com todos os status, usada para validar o arquivo e montar relatorios.
const int NUM_STATUS = 7;
const string STATUS_VALIDOS[NUM_STATUS] = {
    ST_NOVO, ST_EM_CORRECAO, ST_AGUARDANDO, ST_APROVADO,
    ST_REPROVADO, ST_ENCERRADO, ST_REABERTO
};

// ============================================================================
// 2. ESTRUTURA DE DADOS
// ============================================================================

// Representa UM defeito com todas as informacoes que o sistema acompanha.
struct Defeito {
    int codigo;              // identificador unico (ID) do defeito
    string modulo;           // modulo onde a falha aconteceu (um dos 4 validos)
    string descricao;        // resumo curto do problema
    int severidade;          // 1 (Baixa), 2 (Media) ou 3 (Critica)
    string status;           // situacao atual no ciclo de vida
    string responsavel;      // quem esta corrigindo (vazio se ninguem)
    string correcao;         // descricao da correcao efetuada
    string resultadoReteste; // ultimo resultado: "Aprovado", "Reprovado" ou ""
    int reaberturas;         // quantas vezes o defeito ja foi reaberto
};

// Cadastro geral: lista com todos os defeitos registrados no sistema.
// Usamos vector porque ele cresce sozinho conforme novos defeitos entram.
vector<Defeito> cadastro;

// Matriz obrigatoria do trabalho: defeitos[modulo][severidade].
//   - Linha  (0 a 3) = modulo, na mesma ordem do vetor MODULOS.
//   - Coluna (0 a 2) = severidade - 1 (severidade 1 fica na coluna 0).
// Cada celula guarda QUANTOS defeitos existem daquele modulo com aquela
// severidade. Exemplo: defeitos[2][2] = defeitos criticos do modulo Pagamento.
int defeitos[NUM_MODULOS][NUM_SEVERIDADES];

// ============================================================================
// 3. FUNCOES UTILITARIAS
// ----------------------------------------------------------------------------
// Funcoes pequenas de apoio, usadas por varias funcionalidades. Elas cuidam de
// tarefas repetitivas (ler do teclado, validar numeros, procurar defeitos...).
// ============================================================================

// Remove espacos, tabulacoes e quebras de linha do inicio e do fim do texto.
// Ex.: "  Login \r" vira "Login". Importante para arquivos criados no Windows,
// que terminam as linhas com '\r'.
string trim(const string &s) {
    size_t ini = 0, fim = s.size();
    while (ini < fim && isspace((unsigned char)s[ini])) ini++;      // avanca o inicio
    while (fim > ini && isspace((unsigned char)s[fim - 1])) fim--;  // recua o fim
    return s.substr(ini, fim - ini);
}

// Devolve uma copia do texto toda em letras minusculas.
// Serve para comparar textos sem diferenciar maiusculas ("LOGIN" == "login").
string minusculo(string s) {
    for (size_t i = 0; i < s.size(); i++) s[i] = (char)tolower((unsigned char)s[i]);
    return s;
}

// Tenta converter um texto em numero inteiro positivo.
// Retorna true e coloca o numero em 'valor' se der certo; retorna false se o
// texto estiver vazio, tiver algo que nao seja digito ou for grande demais.
// Ex.: "105" -> true (valor = 105) | "abc" -> false | "-3" -> false
bool paraInteiro(const string &txt, int &valor) {
    string t = trim(txt);
    if (t.empty()) return false;
    for (size_t i = 0; i < t.size(); i++)
        if (!isdigit((unsigned char)t[i])) return false;  // so aceita digitos
    if (t.size() > 9) return false;                       // evita estourar o int
    valor = atoi(t.c_str());
    return true;
}

// Mostra uma mensagem e le uma linha inteira digitada pelo usuario.
// Usamos getline (e nao cin >>) para aceitar textos com espacos e para nao
// deixar "lixo" no buffer do teclado entre uma leitura e outra.
string lerTexto(const string &msg) {
    string s;
    cout << msg;
    getline(cin, s);
    return trim(s);
}

// Le um numero inteiro entre 'min' e 'max'. Se o usuario digitar algo invalido
// (letras, numero fora da faixa...), mostra um aviso e pergunta de novo.
// Assim o programa nunca trava nem aceita valores errados.
int lerInteiro(const string &msg, int min, int max) {
    while (true) {
        string s = lerTexto(msg);
        int v;
        if (paraInteiro(s, v) && v >= min && v <= max) return v;
        cout << "  Valor invalido. Digite um numero entre " << min << " e " << max << ".\n";
    }
}

// Le um texto obrigatorio: nao pode ficar vazio e nao pode conter ';'.
// O ';' e proibido porque ele e o separador de campos no arquivo defeitos.txt.
string lerTextoObrigatorio(const string &msg) {
    while (true) {
        string s = lerTexto(msg);
        if (s.empty()) cout << "  Campo obrigatorio.\n";
        else if (s.find(';') != string::npos) cout << "  O caractere ';' nao e permitido.\n";
        else return s;
    }
}

// Procura um modulo pelo nome e devolve sua posicao (0 a 3) no vetor MODULOS.
// Essa posicao e a linha do modulo na matriz. Devolve -1 se o nome nao existir.
// A comparacao ignora maiusculas/minusculas ("pagamento" encontra "Pagamento").
int indiceModulo(const string &nome) {
    for (int i = 0; i < NUM_MODULOS; i++)
        if (minusculo(MODULOS[i]) == minusculo(trim(nome))) return i;
    return -1;
}

// Converte o status lido do arquivo para a forma padrao usada no programa.
// Aceita variacoes como "em correcao", "EM CORRECAO" e "Em correção" (com
// acento) e devolve sempre "Em correcao". Devolve "" se o status nao existir.
string normalizarStatus(const string &s) {
    string t = minusculo(trim(s));

    // Em UTF-8, letras acentuadas ocupam 2 bytes. Aqui trocamos os bytes de
    // "ç" (\xC3\xA7), "ã" (\xC3\xA3) e "Ç" (\xC3\x87) pela letra sem acento.
    size_t p;
    while ((p = t.find("\xC3\xA7")) != string::npos) t.replace(p, 2, "c");
    while ((p = t.find("\xC3\xA3")) != string::npos) t.replace(p, 2, "a");
    while ((p = t.find("\xC3\x87")) != string::npos) t.replace(p, 2, "c");

    // Compara com cada status valido (tambem em minusculas).
    for (int i = 0; i < NUM_STATUS; i++)
        if (minusculo(STATUS_VALIDOS[i]) == t) return STATUS_VALIDOS[i];
    return "";
}

// Procura um defeito no cadastro pelo codigo.
// Devolve a posicao dele no vetor 'cadastro' ou -1 se nao encontrar.
int buscarDefeito(int codigo) {
    for (size_t i = 0; i < cadastro.size(); i++)
        if (cadastro[i].codigo == codigo) return (int)i;
    return -1;
}

// Pede ao usuario o codigo de um defeito e devolve sua posicao no cadastro.
// Ja mostra a mensagem de erro quando o cadastro esta vazio ou o codigo nao
// existe (devolvendo -1). Usada por quase todas as funcionalidades.
int solicitarDefeito() {
    if (cadastro.empty()) {
        cout << "  Nenhum defeito cadastrado.\n";
        return -1;
    }
    int cod = lerInteiro("Codigo do defeito: ", 1, 999999999);
    int idx = buscarDefeito(cod);
    if (idx < 0) cout << "  Defeito " << cod << " nao encontrado.\n";
    return idx;
}

// Mostra os modulos numerados (1 a 4) e devolve o indice (0 a 3) escolhido.
// Escolher por numero evita que o usuario digite um nome de modulo errado.
int escolherModulo() {
    cout << "Modulos: ";
    for (int i = 0; i < NUM_MODULOS; i++)
        cout << (i + 1) << "-" << MODULOS[i] << (i < NUM_MODULOS - 1 ? "  " : "\n");
    return lerInteiro("Escolha o modulo (1-4): ", 1, NUM_MODULOS) - 1;
}

// Exibe todos os dados de um defeito em formato de ficha.
// Campos ainda nao preenchidos aparecem como "-".
void exibirDefeito(const Defeito &d) {
    cout << "  ------------------------------------------------\n";
    cout << "  Codigo........: " << d.codigo << "\n";
    cout << "  Modulo........: " << d.modulo << "\n";
    cout << "  Descricao.....: " << d.descricao << "\n";
    cout << "  Severidade....: " << d.severidade << " (" << NOMES_SEVERIDADE[d.severidade - 1] << ")\n";
    cout << "  Status........: " << d.status << "\n";
    cout << "  Responsavel...: " << (d.responsavel.empty() ? "-" : d.responsavel) << "\n";
    cout << "  Correcao......: " << (d.correcao.empty() ? "-" : d.correcao) << "\n";
    cout << "  Ult. reteste..: " << (d.resultadoReteste.empty() ? "-" : d.resultadoReteste) << "\n";
    cout << "  Reaberturas...: " << d.reaberturas << "\n";
    cout << "  ------------------------------------------------\n";
}

// Recalcula a matriz defeitos[4][3] a partir do cadastro.
// E chamada sempre que um defeito e criado, importado ou muda de modulo ou
// severidade, para que os relatorios estejam sempre atualizados.
void atualizarMatriz() {
    // Passo 1: zera todas as celulas usando dois lacos aninhados
    // (o de fora percorre os modulos/linhas, o de dentro as severidades/colunas).
    for (int m = 0; m < NUM_MODULOS; m++)
        for (int s = 0; s < NUM_SEVERIDADES; s++)
            defeitos[m][s] = 0;

    // Passo 2: percorre o cadastro e soma 1 na celula de cada defeito.
    for (size_t i = 0; i < cadastro.size(); i++) {
        int m = indiceModulo(cadastro[i].modulo);  // linha  = modulo
        int s = cadastro[i].severidade - 1;         // coluna = severidade - 1
        if (m >= 0 && s >= 0 && s < NUM_SEVERIDADES) defeitos[m][s]++;
    }
}

// ============================================================================
// 4. FUNCIONALIDADES (uma funcao para cada requisito do trabalho)
// ============================================================================

// ----------------------------------------------------------------------------
// Requisito 1 - Cadastrar defeito
// Pede codigo, modulo, descricao e severidade. Todo defeito novo comeca com
// status "Novo". Nao permite cadastrar dois defeitos com o mesmo codigo.
// ----------------------------------------------------------------------------
void cadastrarDefeito() {
    cout << "\n=== CADASTRAR DEFEITO ===\n";

    // O codigo precisa ser unico: se ja existir, cancela o cadastro.
    int cod = lerInteiro("Codigo (ID numerico): ", 1, 999999999);
    if (buscarDefeito(cod) >= 0) {
        cout << "  Ja existe um defeito com o codigo " << cod << ". Cadastro cancelado.\n";
        return;
    }

    // Monta o novo defeito com os dados digitados.
    Defeito d;
    d.codigo = cod;
    d.modulo = MODULOS[escolherModulo()];
    d.descricao = lerTextoObrigatorio("Descricao resumida: ");
    d.severidade = lerInteiro("Severidade (1-Baixa, 2-Media, 3-Critica): ", 1, 3);
    d.status = ST_NOVO;   // status inicial obrigatorio
    d.reaberturas = 0;

    cadastro.push_back(d);  // adiciona ao final do cadastro
    atualizarMatriz();      // mantem a matriz em dia
    cout << "  Defeito " << cod << " cadastrado com status \"" << ST_NOVO << "\".\n";
}

// ----------------------------------------------------------------------------
// Requisito 2 - Consultar defeito
// Pede um codigo e mostra a ficha completa do defeito.
// ----------------------------------------------------------------------------
void consultarDefeito() {
    cout << "\n=== CONSULTAR DEFEITO ===\n";
    int idx = solicitarDefeito();
    if (idx >= 0) exibirDefeito(cadastro[idx]);
}

// ----------------------------------------------------------------------------
// Extra - Listar todos os defeitos
// Mostra o cadastro inteiro em forma de tabela (uma linha por defeito).
// setw(n) reserva n colunas para cada campo, alinhando a tabela.
// ----------------------------------------------------------------------------
void listarDefeitos() {
    cout << "\n=== LISTA DE DEFEITOS ===\n";
    if (cadastro.empty()) {
        cout << "  Nenhum defeito cadastrado.\n";
        return;
    }

    // Cabecalho da tabela ('left' alinha o texto a esquerda).
    cout << left << setw(8) << "Codigo" << setw(12) << "Modulo" << setw(5) << "Sev"
         << setw(18) << "Status" << setw(14) << "Responsavel" << "Descricao\n";
    cout << string(80, '-') << "\n";

    // Uma linha para cada defeito.
    for (size_t i = 0; i < cadastro.size(); i++) {
        const Defeito &d = cadastro[i];
        cout << left << setw(8) << d.codigo << setw(12) << d.modulo << setw(5) << d.severidade
             << setw(18) << d.status << setw(14) << (d.responsavel.empty() ? "-" : d.responsavel)
             << d.descricao << "\n";
    }
    cout << right;  // volta o alinhamento padrao para nao afetar outras telas
}

// ----------------------------------------------------------------------------
// Requisito 3 - Atribuir responsavel
// Define quem vai corrigir o defeito e muda o status para "Em correcao".
// So vale para defeitos Novos, Reabertos ou ja Em correcao (troca de pessoa).
// ----------------------------------------------------------------------------
void atribuirDefeito() {
    cout << "\n=== ATRIBUIR RESPONSAVEL ===\n";
    int idx = solicitarDefeito();
    if (idx < 0) return;

    // Usamos uma referencia (&) para alterar o defeito dentro do cadastro,
    // e nao uma copia dele.
    Defeito &d = cadastro[idx];

    // Regra do ciclo de vida: nao faz sentido atribuir um defeito que ja foi
    // corrigido, retestado ou encerrado.
    if (d.status != ST_NOVO && d.status != ST_REABERTO && d.status != ST_EM_CORRECAO) {
        cout << "  So e possivel atribuir defeitos com status Novo, Reaberto ou Em correcao.\n"
             << "  Status atual: " << d.status << "\n";
        return;
    }

    d.responsavel = lerTextoObrigatorio("Nome do responsavel: ");
    d.status = ST_EM_CORRECAO;
    cout << "  Defeito " << d.codigo << " atribuido a " << d.responsavel
         << ". Status: " << d.status << ".\n";
}

// ----------------------------------------------------------------------------
// Requisito 4 - Registrar correcao
// O responsavel informa o que foi corrigido e o defeito passa para
// "Aguardando teste". So vale para defeitos "Em correcao".
// ----------------------------------------------------------------------------
void registrarCorrecao() {
    cout << "\n=== REGISTRAR CORRECAO ===\n";
    int idx = solicitarDefeito();
    if (idx < 0) return;
    Defeito &d = cadastro[idx];

    if (d.status != ST_EM_CORRECAO) {
        cout << "  A correcao so pode ser registrada para defeitos \"Em correcao\".\n"
             << "  Status atual: " << d.status << "\n";
        return;
    }

    d.correcao = lerTextoObrigatorio("Descreva a correcao efetuada: ");
    d.status = ST_AGUARDANDO;
    d.resultadoReteste = "";  // apaga o resultado de um reteste anterior, se houver
    cout << "  Correcao registrada. Status: " << d.status << ".\n";
}

// ----------------------------------------------------------------------------
// Requisito 5 - Registrar resultado do reteste
// O analista testa de novo a correcao e informa se foi Aprovado ou Reprovado.
// O resultado passa a ser o novo status. So vale para "Aguardando teste".
// ----------------------------------------------------------------------------
void executarReteste() {
    cout << "\n=== REGISTRAR RETESTE ===\n";
    int idx = solicitarDefeito();
    if (idx < 0) return;
    Defeito &d = cadastro[idx];

    if (d.status != ST_AGUARDANDO) {
        cout << "  O reteste so pode ser registrado para defeitos \"Aguardando teste\".\n"
             << "  Status atual: " << d.status << "\n";
        return;
    }

    int r = lerInteiro("Resultado do reteste (1-Aprovado, 2-Reprovado): ", 1, 2);
    d.resultadoReteste = (r == 1) ? ST_APROVADO : ST_REPROVADO;
    d.status = d.resultadoReteste;
    cout << "  Reteste registrado: " << d.status << ".\n";

    // Se reprovou, orienta o usuario sobre o proximo passo.
    if (r == 2) cout << "  Use a opcao \"Reabrir defeito\" para devolve-lo a correcao.\n";
}

// ----------------------------------------------------------------------------
// Requisito 6 - Encerrar defeito
// Fecha o defeito. Regra obrigatoria: so e permitido se o reteste estiver
// APROVADO. Qualquer outro status e recusado com uma mensagem explicativa.
// ----------------------------------------------------------------------------
void encerrarDefeito() {
    cout << "\n=== ENCERRAR DEFEITO ===\n";
    int idx = solicitarDefeito();
    if (idx < 0) return;
    Defeito &d = cadastro[idx];

    if (d.status == ST_ENCERRADO) {
        cout << "  O defeito ja esta encerrado.\n";
        return;
    }
    if (d.status != ST_APROVADO) {
        cout << "  Nao e possivel encerrar: o reteste precisa estar APROVADO.\n"
             << "  Status atual: " << d.status << "\n";
        return;
    }

    d.status = ST_ENCERRADO;
    cout << "  Defeito " << d.codigo << " encerrado.\n";
}

// ----------------------------------------------------------------------------
// Requisito 7 - Reabrir defeito
// Devolve o defeito para correcao em dois casos:
//   - Reprovado: a correcao nao funcionou no reteste;
//   - Encerrado: o defeito voltou a acontecer (recorrente).
// Cada reabertura e contada (usado no calculo da proporcao de reabertura).
// ----------------------------------------------------------------------------
void reabrirDefeito() {
    cout << "\n=== REABRIR DEFEITO ===\n";
    int idx = solicitarDefeito();
    if (idx < 0) return;
    Defeito &d = cadastro[idx];

    if (d.status != ST_REPROVADO && d.status != ST_ENCERRADO) {
        cout << "  So podem ser reabertos defeitos Reprovados ou Encerrados (recorrentes).\n"
             << "  Status atual: " << d.status << "\n";
        return;
    }

    // Guarda o motivo ANTES de mudar o status, so para exibir na mensagem.
    string motivo = (d.status == ST_REPROVADO) ? "reprovado no reteste" : "recorrente";
    d.status = ST_REABERTO;
    d.reaberturas++;  // conta mais uma reabertura

    cout << "  Defeito " << d.codigo << " reaberto (" << motivo << "). "
         << "Total de reaberturas: " << d.reaberturas << ".\n";
    cout << "  Atribua um responsavel para retomar a correcao.\n";
}

// ----------------------------------------------------------------------------
// Requisito 8 - Alterar dados do defeito
// Permite corrigir modulo, descricao, severidade ou responsavel.
// Regra obrigatoria: defeitos ENCERRADOS nao podem ser alterados.
// ----------------------------------------------------------------------------
void alterarDefeito() {
    cout << "\n=== ALTERAR DADOS DO DEFEITO ===\n";
    int idx = solicitarDefeito();
    if (idx < 0) return;
    Defeito &d = cadastro[idx];

    if (d.status == ST_ENCERRADO) {
        cout << "  Defeitos encerrados nao podem ser alterados.\n";
        return;
    }

    // Mostra os dados atuais para o usuario saber o que esta mudando.
    exibirDefeito(d);
    cout << "O que deseja alterar?\n"
         << "  1 - Modulo\n  2 - Descricao\n  3 - Severidade\n  4 - Responsavel\n  0 - Cancelar\n";
    int op = lerInteiro("Opcao: ", 0, 4);

    switch (op) {
        case 1: d.modulo = MODULOS[escolherModulo()]; break;
        case 2: d.descricao = lerTextoObrigatorio("Nova descricao: "); break;
        case 3: d.severidade = lerInteiro("Nova severidade (1-3): ", 1, 3); break;
        case 4:
            // Um defeito Novo ainda nao tem responsavel; a atribuicao correta e
            // pela opcao 4 do menu, que tambem muda o status para Em correcao.
            if (d.status == ST_NOVO) {
                cout << "  Use a opcao \"Atribuir responsavel\" para defeitos novos.\n";
                return;
            }
            d.responsavel = lerTextoObrigatorio("Novo responsavel: ");
            break;
        default: cout << "  Alteracao cancelada.\n"; return;
    }

    atualizarMatriz();  // modulo ou severidade podem ter mudado
    cout << "  Dados atualizados.\n";
}

// ----------------------------------------------------------------------------
// Requisito 9 - Alertas de defeitos criticos
// Lista todos os defeitos de severidade 3 (Critica) que ainda NAO foram
// encerrados, destacando os que ainda estao sem responsavel.
// ----------------------------------------------------------------------------
void verificarCriticos() {
    cout << "\n=== ALERTA: DEFEITOS CRITICOS NAO ENCERRADOS ===\n";
    int qtd = 0;  // contador de criticos pendentes

    for (size_t i = 0; i < cadastro.size(); i++) {
        const Defeito &d = cadastro[i];
        if (d.severidade == SEVERIDADE_CRITICA && d.status != ST_ENCERRADO) {
            cout << "  [!] ALERTA - Defeito " << d.codigo << " | " << d.modulo
                 << " | " << d.status << " | " << d.descricao;
            if (d.responsavel.empty()) cout << " | SEM RESPONSAVEL";
            cout << "\n";
            qtd++;
        }
    }

    if (qtd == 0) cout << "  Nenhum defeito critico pendente.\n";
    else cout << "  Total de criticos pendentes: " << qtd << "\n";
}

// ----------------------------------------------------------------------------
// Atividade obrigatoria com matriz
// Usa a matriz defeitos[4][3] para calcular:
//   a) total de defeitos por modulo     (soma de cada LINHA)
//   b) total de defeitos por severidade (soma de cada COLUNA)
//   c) o modulo com mais falhas criticas (maior valor da coluna Critica)
// Os calculos (a) e (b) sao feitos com dois lacos de repeticao aninhados.
// ----------------------------------------------------------------------------
void exibirMatriz() {
    atualizarMatriz();  // garante que a matriz reflete o cadastro atual

    int totalModulo[NUM_MODULOS] = {0};          // um total por linha
    int totalSeveridade[NUM_SEVERIDADES] = {0};  // um total por coluna
    int totalGeral = 0;

    // Lacos aninhados: o externo percorre as linhas (modulos) e o interno
    // percorre as colunas (severidades). Cada celula e somada no total da sua
    // linha, no total da sua coluna e no total geral.
    for (int m = 0; m < NUM_MODULOS; m++) {
        for (int s = 0; s < NUM_SEVERIDADES; s++) {
            totalModulo[m] += defeitos[m][s];
            totalSeveridade[s] += defeitos[m][s];
            totalGeral += defeitos[m][s];
        }
    }

    // --- Impressao da matriz em formato de tabela ---
    cout << "\n--- Matriz defeitos[4][3] (modulo x severidade) ---\n";

    // Cabecalho: nomes das severidades + coluna de total
    cout << left << setw(12) << "Modulo" << right;
    for (int s = 0; s < NUM_SEVERIDADES; s++) cout << setw(9) << NOMES_SEVERIDADE[s];
    cout << setw(9) << "Total" << "\n";
    cout << string(48, '-') << "\n";

    // Uma linha por modulo, com os valores da matriz e o total da linha
    for (int m = 0; m < NUM_MODULOS; m++) {
        cout << left << setw(12) << MODULOS[m] << right;
        for (int s = 0; s < NUM_SEVERIDADES; s++) cout << setw(9) << defeitos[m][s];
        cout << setw(9) << totalModulo[m] << "\n";
    }

    // Ultima linha: total de cada severidade e total geral
    cout << string(48, '-') << "\n";
    cout << left << setw(12) << "Total" << right;
    for (int s = 0; s < NUM_SEVERIDADES; s++) cout << setw(9) << totalSeveridade[s];
    cout << setw(9) << totalGeral << "\n";

    // --- Modulo com mais falhas criticas ---
    // Procura o maior valor na coluna de severidade Critica (coluna 2).
    // Tambem detecta empate, quando dois ou mais modulos tem o mesmo maximo.
    int maior = -1, idxMaior = -1;
    bool empate = false;
    for (int m = 0; m < NUM_MODULOS; m++) {
        int criticos = defeitos[m][SEVERIDADE_CRITICA - 1];
        if (criticos > maior) { maior = criticos; idxMaior = m; empate = false; }  // novo maximo
        else if (criticos == maior) empate = true;                                 // igualou o maximo
    }

    if (maior <= 0) {
        cout << "\nNenhum modulo possui falhas criticas.\n";
    } else {
        cout << "\nModulo com mais falhas criticas: " << MODULOS[idxMaior]
             << " (" << maior << ")";
        if (empate) {
            // Lista os outros modulos que empataram com o primeiro encontrado.
            cout << " - empatado com:";
            for (int m = idxMaior + 1; m < NUM_MODULOS; m++)
                if (defeitos[m][SEVERIDADE_CRITICA - 1] == maior) cout << " " << MODULOS[m];
        }
        cout << "\n";
    }
}

// ----------------------------------------------------------------------------
// Requisito 10 - Relatorio por modulo, severidade e status
// Junta em uma so tela:
//   - a matriz (modulo x severidade) com seus totais;
//   - a quantidade de defeitos em cada status;
//   - os status presentes dentro de cada modulo;
//   - Desafio adicional: proporcao de defeitos que precisaram de reabertura.
// ----------------------------------------------------------------------------
void gerarRelatorioDefeitos() {
    cout << "\n=============== RELATORIO DE DEFEITOS ===============\n";
    if (cadastro.empty()) {
        cout << "  Nenhum defeito cadastrado.\n";
        return;
    }

    // Parte 1: por modulo e por severidade (usa a matriz obrigatoria).
    exibirMatriz();

    // Parte 2: total de defeitos em cada status.
    // Para cada status possivel, conta quantos defeitos estao nele.
    cout << "\n--- Defeitos por status ---\n";
    for (int i = 0; i < NUM_STATUS; i++) {
        int qtd = 0;
        for (size_t j = 0; j < cadastro.size(); j++)
            if (cadastro[j].status == STATUS_VALIDOS[i]) qtd++;
        cout << "  " << left << setw(18) << STATUS_VALIDOS[i] << right << setw(4) << qtd << "\n";
    }

    // Parte 3: cruzamento modulo x status.
    // Para cada modulo, mostra apenas os status que tem pelo menos 1 defeito.
    cout << "\n--- Status por modulo ---\n";
    for (int m = 0; m < NUM_MODULOS; m++) {
        cout << "  " << MODULOS[m] << ":";
        bool algum = false;  // indica se o modulo tem algum defeito
        for (int i = 0; i < NUM_STATUS; i++) {
            int qtd = 0;
            for (size_t j = 0; j < cadastro.size(); j++)
                if (cadastro[j].status == STATUS_VALIDOS[i] && cadastro[j].modulo == MODULOS[m]) qtd++;
            if (qtd > 0) { cout << "  " << STATUS_VALIDOS[i] << "=" << qtd; algum = true; }
        }
        if (!algum) cout << "  (sem defeitos)";
        cout << "\n";
    }

    // Parte 4 (desafio adicional): proporcao de defeitos reabertos.
    //   reabertos        = quantos defeitos foram reabertos pelo menos 1 vez
    //   totalReaberturas = soma de todas as reaberturas (um defeito pode ter varias)
    //   proporcao        = reabertos / total de defeitos * 100
    int reabertos = 0, totalReaberturas = 0;
    for (size_t j = 0; j < cadastro.size(); j++) {
        if (cadastro[j].reaberturas > 0) reabertos++;
        totalReaberturas += cadastro[j].reaberturas;
    }
    // 100.0 (com ponto) forca a divisao com casas decimais, e nao divisao inteira.
    double proporcao = 100.0 * reabertos / cadastro.size();

    cout << "\n--- Reabertura (desafio adicional) ---\n";
    cout << "  Defeitos que precisaram de reabertura: " << reabertos << " de " << cadastro.size()
         << " (" << fixed << setprecision(1) << proporcao << "%)\n";
    cout << "  Total de reaberturas registradas: " << totalReaberturas << "\n";
    cout.unsetf(ios::fixed);  // desfaz o formato decimal fixo para as proximas telas
    cout << "=====================================================\n";
}

// ----------------------------------------------------------------------------
// Requisito 11 - Importar defeitos do arquivo (atividade obrigatoria)
//
// Le o arquivo texto (padrao: defeitos.txt) com uma linha por defeito, no
// formato:   codigo;modulo;descricao;severidade;status
// A primeira linha pode ser um cabecalho (comecando com "codigo"), que e pulado.
//
// Cada linha e validada. Ela e REJEITADA (e o motivo e guardado) quando:
//   - nao tem exatamente 5 campos;
//   - o codigo nao e um numero positivo, ou ja existe no cadastro (duplicado);
//   - o modulo nao e um dos 4 validos;
//   - a descricao esta vazia;
//   - a severidade nao esta entre 1 e 3;
//   - o status nao e um dos 7 validos.
// As linhas validas entram no cadastro. No final, mostra um resumo.
// Se o arquivo nao existir, avisa o usuario e volta ao menu normalmente.
// ----------------------------------------------------------------------------
void importarDefeitosArquivo(const string &nomeArquivo = "defeitos.txt") {
    cout << "\n=== IMPORTAR DEFEITOS DO ARQUIVO \"" << nomeArquivo << "\" ===\n";

    // Abre o arquivo para leitura. Se falhar, mostra o erro e sai da funcao
    // (o programa continua; o menu volta a aparecer).
    ifstream arq(nomeArquivo.c_str());
    if (!arq.is_open()) {
        cout << "  ERRO: nao foi possivel abrir o arquivo \"" << nomeArquivo << "\".\n"
             << "  Verifique se ele existe na mesma pasta do programa.\n"
             << "  O sistema continua funcionando normalmente.\n";
        return;
    }

    // Contadores usados no resumo final da importacao.
    string linha;
    int numLinha = 0;    // numero da linha atual no arquivo (para as mensagens)
    int lidos = 0;       // registros de dados lidos (sem contar cabecalho e vazias)
    int importados = 0;  // registros aceitos
    int rejeitados = 0;  // registros recusados na validacao
    int ignoradas = 0;   // cabecalho e linhas em branco
    vector<string> erros;  // motivo de cada rejeicao

    // Le o arquivo linha por linha ate o fim.
    while (getline(arq, linha)) {
        numLinha++;

        // Alguns editores (ex.: Bloco de Notas) gravam 3 bytes invisiveis no
        // inicio de arquivos UTF-8 (o "BOM"). Se existirem, sao removidos.
        if (numLinha == 1 && linha.size() >= 3 &&
            (unsigned char)linha[0] == 0xEF && (unsigned char)linha[1] == 0xBB &&
            (unsigned char)linha[2] == 0xBF)
            linha = linha.substr(3);
        linha = trim(linha);  // tambem remove o '\r' de arquivos do Windows

        // Linha em branco: apenas ignora.
        if (linha.empty()) { ignoradas++; continue; }

        // Separa a linha em campos usando ';' como separador.
        // Ex.: "101;Login;Erro X;2;Novo" -> ["101","Login","Erro X","2","Novo"]
        vector<string> campos;
        stringstream ss(linha);
        string campo;
        while (getline(ss, campo, ';')) campos.push_back(trim(campo));

        // Cabecalho opcional: se for a primeira linha com conteudo e o primeiro
        // campo for "codigo", nao e um defeito, entao e pulada.
        if (lidos == 0 && !campos.empty() && minusculo(campos[0]) == "codigo") {
            ignoradas++;
            continue;
        }

        lidos++;
        // Comeca a montar a mensagem de erro (usada so se a linha for rejeitada).
        ostringstream motivo;
        motivo << "  Linha " << numLinha << ": ";

        // --- Validacoes (cada uma rejeita a linha e passa para a proxima) ---

        // Quantidade de campos
        if (campos.size() != 5) {
            motivo << "esperados 5 campos, encontrados " << campos.size() << ".";
            erros.push_back(motivo.str()); rejeitados++; continue;
        }

        // Codigo: numero inteiro positivo
        int codigo, severidade;
        if (!paraInteiro(campos[0], codigo) || codigo <= 0) {
            motivo << "codigo invalido (\"" << campos[0] << "\").";
            erros.push_back(motivo.str()); rejeitados++; continue;
        }

        // Codigo: nao pode repetir (nem no cadastro nem em linha anterior do arquivo,
        // pois as linhas validas ja foram adicionadas ao cadastro)
        if (buscarDefeito(codigo) >= 0) {
            motivo << "codigo " << codigo << " duplicado (ja cadastrado).";
            erros.push_back(motivo.str()); rejeitados++; continue;
        }

        // Modulo: precisa ser um dos 4 validos
        int m = indiceModulo(campos[1]);
        if (m < 0) {
            motivo << "modulo invalido (\"" << campos[1] << "\").";
            erros.push_back(motivo.str()); rejeitados++; continue;
        }

        // Descricao: obrigatoria
        if (campos[2].empty()) {
            motivo << "descricao vazia.";
            erros.push_back(motivo.str()); rejeitados++; continue;
        }

        // Severidade: numero de 1 a 3
        if (!paraInteiro(campos[3], severidade) || severidade < 1 || severidade > 3) {
            motivo << "severidade invalida (\"" << campos[3] << "\"), use 1 a 3.";
            erros.push_back(motivo.str()); rejeitados++; continue;
        }

        // Status: precisa ser um dos 7 validos (aceita com ou sem acento)
        string status = normalizarStatus(campos[4]);
        if (status.empty()) {
            motivo << "status invalido (\"" << campos[4] << "\").";
            erros.push_back(motivo.str()); rejeitados++; continue;
        }

        // --- Linha valida: cria o defeito e adiciona ao cadastro ---
        Defeito d;
        d.codigo = codigo;
        d.modulo = MODULOS[m];  // usa o nome padrao (ex.: "login" vira "Login")
        d.descricao = campos[2];
        d.severidade = severidade;
        d.status = status;
        // Se ja chega como Reaberto, conta como 1 reabertura.
        d.reaberturas = (status == ST_REABERTO) ? 1 : 0;
        // Se ja chega Aprovado/Reprovado, esse e o resultado do ultimo reteste.
        if (status == ST_APROVADO || status == ST_REPROVADO) d.resultadoReteste = status;
        cadastro.push_back(d);
        importados++;
    }

    arq.close();
    atualizarMatriz();  // mantem matriz e relatorios atualizados com os novos dados

    // --- Resumo da importacao ---
    cout << "  --------- Resumo da importacao ---------\n";
    cout << "  Registros lidos.......: " << lidos << "\n";
    cout << "  Importados com sucesso: " << importados << "\n";
    cout << "  Rejeitados............: " << rejeitados << "\n";
    cout << "  Linhas ignoradas......: " << ignoradas << " (cabecalho/vazias)\n";
    if (!erros.empty()) {
        cout << "  Detalhes das rejeicoes:\n";
        for (size_t i = 0; i < erros.size(); i++) cout << erros[i] << "\n";
    }
    cout << "  Total no cadastro agora: " << cadastro.size() << "\n";
}

// ============================================================================
// 5. MENU PRINCIPAL E FUNCAO main()
// ============================================================================

// Mostra as opcoes disponiveis no menu.
void exibirMenu() {
    cout << "\n================ BUG TRACKER ================\n"
         << "  1  - Cadastrar defeito\n"
         << "  2  - Consultar defeito\n"
         << "  3  - Listar todos os defeitos\n"
         << "  4  - Atribuir responsavel (Em correcao)\n"
         << "  5  - Registrar correcao (Aguardando teste)\n"
         << "  6  - Registrar resultado do reteste\n"
         << "  7  - Encerrar defeito\n"
         << "  8  - Reabrir defeito\n"
         << "  9  - Alterar dados do defeito\n"
         << "  10 - Alertas de defeitos criticos\n"
         << "  11 - Relatorio (modulo / severidade / status)\n"
         << "  12 - Importar defeitos de defeitos.txt\n"
         << "  0  - Sair\n"
         << "=============================================\n";
}

// Ponto de entrada do programa.
// Repete o ciclo "mostrar menu -> ler opcao -> executar funcao" ate o usuario
// escolher 0 (Sair). O cin.good() tambem encerra o laco se a entrada acabar
// (por exemplo, se o usuario fechar a entrada com Ctrl+D / Ctrl+Z).
int main() {
    atualizarMatriz();  // comeca com a matriz zerada
    int opcao;
    do {
        exibirMenu();
        opcao = lerInteiro("Escolha uma opcao: ", 0, 12);

        // Cada opcao chama a funcao responsavel pelo requisito correspondente.
        switch (opcao) {
            case 1:  cadastrarDefeito(); break;
            case 2:  consultarDefeito(); break;
            case 3:  listarDefeitos(); break;
            case 4:  atribuirDefeito(); break;
            case 5:  registrarCorrecao(); break;
            case 6:  executarReteste(); break;
            case 7:  encerrarDefeito(); break;
            case 8:  reabrirDefeito(); break;
            case 9:  alterarDefeito(); break;
            case 10: verificarCriticos(); break;
            case 11: gerarRelatorioDefeitos(); break;
            case 12: importarDefeitosArquivo(); break;
            case 0:  cout << "Encerrando o sistema. Ate logo!\n"; break;
        }
    } while (opcao != 0 && cin.good());

    return 0;
}
