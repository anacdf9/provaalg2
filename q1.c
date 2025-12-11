#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "q1.h"

//ler html de forma a ver o que encontra nas funções da pagina
// Le o arquivo todo 
char* ler_arquivo(char* nome) {
    FILE* f = fopen(nome, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    long tam = ftell(f);
    rewind(f);

    char* buffer = (char*) malloc(tam + 1);
    fread(buffer, 1, tam, f);
    buffer[tam] = '\0';
    fclose(f);
    return buffer;
}

// Remove as tags 
void limpar_html(char* dest, char* orig) {
    int i = 0, j = 0, tag = 0;
    while (orig[i]) {
        if (orig[i] == '<') tag = 1;
        else if (orig[i] == '>') { tag = 0; if(j>0 && dest[j-1]!=' ') dest[j++]=' '; }
        else if (!tag && orig[i] != '\n' && orig[i] != '\t') dest[j++] = orig[i];
        i++;
    }
    dest[j] = '\0';
}

// Procura e guarda todas as referencias do rodape
Referencia* carregar_referencias(char* html) {
    Referencia *lista = NULL;
    char *p = html;

    // Busca padrao: <li id="cite_note-
    while ((p = strstr(p, "<li id=\"cite_note-"))) {
        Referencia *novo = (Referencia*) malloc(sizeof(Referencia));
        
        // Pega ID
        sscanf(p, "<li id=\"%[^\"]\"", novo->id);

        // Pega o texto (dentro do span reference-text)
        char *inicio = strstr(p, "reference-text\">");
        if (inicio) {
            inicio += 16; 
            char *fim = strstr(inicio, "</span>");
            char temp[2000];
            int tam = fim - inicio;
            strncpy(temp, inicio, tam);
            temp[tam] = '\0';
            limpar_html(novo->texto, temp);
        }

        novo->prox = lista;
        lista = novo;
        p++; // Avanca
    }
    return lista;
}

// Procura e guarda os pesquisadores da tabela
Pesquisador* carregar_pesquisadores(char* html) {
    Pesquisador *lista = NULL;
    char *tabela = strstr(html, "wikitable sortable"); // Acha a tabela
    if (!tabela) return NULL;

    char *linha = strstr(tabela, "<tr>");
    while (linha) {
        // Procura o nome na classe "fn"
        char *nome_tag = strstr(linha, "class=\"fn\"");
        char *prox_linha = strstr(linha + 5, "<tr>");

        // Se achou nome antes de acabar a linha
        if (nome_tag && (!prox_linha || nome_tag < prox_linha)) {
            Pesquisador *novo = (Pesquisador*) malloc(sizeof(Pesquisador));
            novo->refs[0] = '\0';

            // Extrai Nome
            char *inicio = strchr(nome_tag, '>') + 1;
            // Se tiver link <a>, entra nele
            if (strstr(inicio, "<a") && strstr(inicio, "<a") < strstr(inicio, "</span>"))
                inicio = strchr(inicio, '>') + 1;
            
            char *fim = strchr(inicio, '<');
            int tam = fim - inicio;
            strncpy(novo->nome, inicio, tam);
            novo->nome[tam] = '\0';

            // Extrai IDs das referencias nesta linha
            char *cursor = linha;
            while ((cursor = strstr(cursor, "#cite_note-")) && (!prox_linha || cursor < prox_linha)) {
                char ref_id[50];
                sscanf(cursor, "#%[^\"]\"", ref_id); // Pega ate fechar aspas
                strcat(novo->refs, ref_id);
                strcat(novo->refs, " ");
                cursor++;
            }

            novo->prox = lista;
            lista = novo;
        }
        linha = prox_linha;
    }
    return lista;
}

// : Lista os nomes na tela
void listar_nomes(Pesquisador* lista) {
    printf("\n Pesquisador(es) \n");
    while (lista) {
        printf("- %s\n", lista->nome);
        lista = lista->prox;
    }
}

// Busca nome e mostrar referencias
void mostrar_referencias(Pesquisador* p, Referencia* r, char* busca) {
    int achou = 0;
    while (p) {
        if (strstr(p->nome, busca)) { // Busca parcial
            achou = 1;
            printf("\n Pesquisador: %s\n", p->nome);
            printf("Ref:\n");

            // Separa os IDs salvos e busca na lista de referencias
            char copia[200], *token;
            strcpy(copia, p->refs);
            token = strtok(copia, " ");
            
            while (token) {
                Referencia *aux = r;
                while (aux) {
                    if (strcmp(token, aux->id) == 0) {
                        printf(" > %s\n\n", aux->texto);
                    }
                    aux = aux->prox;
                }
                token = strtok(NULL, " ");
            }
        }
        p = p->prox;
    }
    if (!achou) printf("Ninguem encontrado com esse nome.\n");
}

void liberar_memoria(Pesquisador* p, Referencia* r, char* html) {
    if (html) free(html);
    while (p) { void* t = p; p = p->prox; free(t); }
    while (r) { void* t = r; r = r->prox; free(t); }
}

int main() {
    char arq[50];
    printf("Nome do arquivo HTML: ");
    scanf("%s", arq);

    char *html = ler_arquivo(arq);
    if (!html) { printf("Erro ao abrir arquivo.\n"); return 1; }

    printf("Lendo dados...\n");
    Referencia *refs = carregar_referencias(html);
    Pesquisador *pesqs = carregar_pesquisadores(html);

    int op;
    do {
        printf("\n1 - Listar Nomes (a)\n2 - Consultar Referencias (b)\n 0 - Sair: ");
        scanf("%d", &op);
        getchar(); // Limpa buffer

        if (op == 1) {
            listar_nomes(pesqs);
        }
        else if (op == 2) {
            char nome[100];
            printf("Nome do pesquisador: ");
            fgets(nome, 100, stdin);
            nome[strcspn(nome, "\n")] = 0;
            mostrar_referencias(pesqs, refs, nome);
        }

    } while (op != 0);

    liberar_memoria(pesqs, refs, html);
    return 0;
}