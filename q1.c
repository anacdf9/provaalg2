#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "q1.h"

// --- FUNCOES AUXILIARES ---

// Le o arquivo HTML inteiro para a memoria RAM
char* lerArquivo(const char* nomeArq) {
    FILE *f = fopen(nomeArq, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long tam = ftell(f);
    rewind(f);

    char *buffer = (char*) malloc(tam + 1);
    if (buffer) {
        fread(buffer, 1, tam, f);
        buffer[tam] = '\0';
    }
    fclose(f);
    return buffer;
}

// Remove tags como <b>, <a>, </span> deixando so o texto
void limparTexto(char* destino, char* origem) {
    int i = 0, j = 0;
    int dentroTag = 0;
    while (origem[i] != '\0') {
        if (origem[i] == '<') {
            dentroTag = 1;
        } else if (origem[i] == '>') {
            dentroTag = 0;
            if (j > 0 && destino[j-1] != ' ') destino[j++] = ' '; // Espaco seguro
        } else if (!dentroTag) {
            if (origem[i] != '\n' && origem[i] != '\t') {
                destino[j++] = origem[i];
            }
        }
        i++;
    }
    destino[j] = '\0';
}

// --- FUNCOES DE PARSER (EXTRAIR DADOS) ---

Referencia* buscarReferencias(char* html) {
    Referencia *lista = NULL;
    char *pos = html;

    // Procura por cada nota de rodape no HTML
    while ((pos = strstr(pos, "<li id=\"cite_note-")) != NULL) {
        Referencia *novo = (Referencia*) malloc(sizeof(Referencia));
        
        // Pega o ID (ex: cite_note-1)
        char *inicioId = pos + 8; // Pula <li id="
        char *fimId = strchr(inicioId, '"');
        int tam = fimId - inicioId;
        strncpy(novo->id, inicioId, tam);
        novo->id[tam] = '\0';

        // Pega o texto da referencia
        char *inicioTxt = strstr(pos, "<span class=\"reference-text\">");
        if (inicioTxt) {
            inicioTxt += 29; // Tamanho da tag span
            char *fimTxt = strstr(inicioTxt, "</span>");
            
            // Copia temporaria para limpar
            int tamTxt = fimTxt - inicioTxt;
            char *temp = (char*) malloc(tamTxt + 1);
            strncpy(temp, inicioTxt, tamTxt);
            temp[tamTxt] = '\0';
            
            limparTexto(novo->texto, temp);
            free(temp);
        } else {
            strcpy(novo->texto, "Referencia sem texto.");
        }

        novo->prox = lista;
        lista = novo;
        pos = fimId; // Avanca para a proxima
    }
    return lista;
}

Pesquisador* buscarPesquisadores(char* html) {
    Pesquisador *lista = NULL;
    // Acha onde comeca a tabela
    char *tabela = strstr(html, "<table class=\"wikitable sortable\">");
    if (!tabela) return NULL;

    char *linha = strstr(tabela, "<tr>");
    while (linha != NULL) {
        // Verifica se tem nome nesta linha (usando a classe fn)
        char *tagNome = strstr(linha, "<span class=\"fn\">");
        
        // Se achou um nome antes da proxima linha comecar
        char *proxLinha = strstr(linha + 4, "<tr>");
        if (tagNome && (!proxLinha || tagNome < proxLinha)) {
            Pesquisador *novo = (Pesquisador*) malloc(sizeof(Pesquisador));
            novo->listaRefs[0] = '\0'; 

            // 1. Extrair Nome
            char *inicioNome = strchr(tagNome, '>') + 1;
            // Pula link se tiver
            if (strstr(inicioNome, "<a") && strstr(inicioNome, "<a") < strstr(inicioNome, "</span>")) {
                inicioNome = strchr(inicioNome, '>') + 1; // Entra no <a...
                inicioNome = strchr(inicioNome, '>') + 1; // Sai do >
            }
            char *fimNome = strchr(inicioNome, '<');
            int tam = fimNome - inicioNome;
            strncpy(novo->nome, inicioNome, tam);
            novo->nome[tam] = '\0';

            // 2. Achar Universidade (ultima coluna da linha)
            // Vamos procurar o ultimo <td> antes do fim da linha
            char *cursor = tagNome;
            char *fimLinha = proxLinha ? proxLinha : strchr(cursor, '\0');
            char *ultimaTd = NULL;
            
            // Procura todas as TDs desta linha
            char *tempTd = cursor;
            while ((tempTd = strstr(tempTd, "<td>")) && tempTd < fimLinha) {
                ultimaTd = tempTd;
                tempTd++;
            }
            
            if (ultimaTd) {
                char *conteudo = ultimaTd + 4; // Pula <td>
                char *fimConteudo = strstr(conteudo, "</td>");
                int tamU = fimConteudo - conteudo;
                char *tempUniv = (char*) malloc(tamU + 1);
                strncpy(tempUniv, conteudo, tamU);
                tempUniv[tamU] = '\0';
                limparTexto(novo->universidade, tempUniv);
                free(tempUniv);
            } else {
                strcpy(novo->universidade, "N/A");
            }

            // 3. Pegar IDs das referencias nesta linha
            // Procura links como href="#cite_note-..."
            char *refPtr = linha;
            while ((refPtr = strstr(refPtr, "href=\"#cite_note-")) && refPtr < fimLinha) {
                char *inicioRef = refPtr + 6; // Pula href="
                char *fimRef = strchr(inicioRef, '"');
                int tamR = fimRef - inicioRef;
                char idR[50];
                strncpy(idR, inicioRef, tamR);
                idR[tamR] = '\0'; // Fica algo como #cite_note-1

                strcat(novo->listaRefs, idR); // Guarda na lista
                strcat(novo->listaRefs, " "); // Separador
                refPtr = fimRef;
            }

            novo->prox = lista;
            lista = novo;
        }
        linha = proxLinha;
    }
    return lista;
}

// --- FUNCOES DO MENU ---

void listarNomes(Pesquisador* lista) {
    printf("\n--- PESQUISADORES ENCONTRADOS ---\n");
    while (lista) {
        printf("- %s\n", lista->nome);
        lista = lista->prox;
    }
}

void mostrarReferenciasCompletas(Pesquisador* listaP, Referencia* listaR, char* nomeBusca) {
    int achou = 0;
    while (listaP) {
        // Busca parcial do nome
        if (strstr(listaP->nome, nomeBusca)) {
            achou = 1;
            printf("\n> PESQUISADOR: %s\n", listaP->nome);
            printf("> REFERENCIAS:\n");
            
            // Separa os IDs salvos na string (ex: "#cite_note-1 #cite_note-2")
            char copiaRefs[500];
            strcpy(copiaRefs, listaP->listaRefs);
            
            char *token = strtok(copiaRefs, " ");
            while (token != NULL) {
                // Tira o # do inicio para buscar
                char *idLimpo = token;
                if (token[0] == '#') idLimpo++;

                // Busca na lista de referencias
                Referencia *auxR = listaR;
                while (auxR) {
                    if (strcmp(auxR->id, idLimpo) == 0) {
                        printf(" * %s\n\n", auxR->texto);
                    }
                    auxR = auxR->prox;
                }
                token = strtok(NULL, " ");
            }
        }
        listaP = listaP->prox;
    }
    if (!achou) printf("Pesquisador nao encontrado.\n");
}

void salvarBinario(Pesquisador* lista, char* univ, char* nomeArq) {
    // Abre com "ab" para adicionar ao final (append binary) - Obrigatorio
    FILE *f = fopen(nomeArq, "ab");
    if (!f) {
        printf("Erro ao abrir arquivo %s\n", nomeArq);
        return;
    }

    int cont = 0;
    while (lista) {
        if (strstr(lista->universidade, univ)) {
            DadosBin d;
            // Copia segura
            memset(&d, 0, sizeof(DadosBin));
            strncpy(d.nome, lista->nome, 99);
            strncpy(d.universidade, lista->universidade, 199);
            
            fwrite(&d, sizeof(DadosBin), 1, f);
            cont++;
        }
        lista = lista->prox;
    }
    fclose(f);
    printf("Salvo! %d pesquisadores da universidade '%s' adicionados em '%s'.\n", cont, univ, nomeArq);
}

void liberarTudo(Pesquisador* p, Referencia* r, char* html) {
    if (html) free(html);
    while (p) {
        Pesquisador *t = p;
        p = p->prox;
        free(t);
    }
    while (r) {
        Referencia *t = r;
        r = r->prox;
        free(t);
    }
}

// --- MAIN ---

int main() {
    char caminho[100];
    char *html = NULL;
    Pesquisador *lPesq = NULL;
    Referencia *lRefs = NULL;
    int op;

    printf("Digite o nome do arquivo HTML: ");
    scanf("%s", caminho);

    // Passo A: Ler arquivo e extrair
    html = lerArquivo(caminho);
    if (!html) {
        printf("Erro ao ler arquivo!\n");
        return 1;
    }

    printf("Lendo referencias...\n");
    lRefs = buscarReferencias(html);
    printf("Lendo pesquisadores...\n");
    lPesq = buscarPesquisadores(html);

    do {
        printf("\n1 - Listar Nomes\n");
        printf("2 - Consultar Referencias\n");
        printf("3 - Gerar Arquivo Binario\n");
        printf("0 - Sair\n");
        printf("Opcao: ");
        scanf("%d", &op);
        getchar(); // Limpa buffer

        if (op == 1) {
            listarNomes(lPesq);
        }
        else if (op == 2) {
            char nome[100];
            printf("Digite o nome do pesquisador: ");
            fgets(nome, 100, stdin);
            nome[strcspn(nome, "\n")] = 0; // Tira o enter
            mostrarReferenciasCompletas(lPesq, lRefs, nome);
        }
        else if (op == 3) {
            char uni[100], arq[100];
            printf("Digite parte do nome da Universidade: ");
            fgets(uni, 100, stdin);
            uni[strcspn(uni, "\n")] = 0;
            printf("Nome do arquivo de saida (ex: saida.bin): ");
            scanf("%s", arq);
            salvarBinario(lPesq, uni, arq);
        }

    } while (op != 0);

    liberarTudo(lPesq, lRefs, html);
    return 0;
}
