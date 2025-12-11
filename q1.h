#ifndef Q1_H
#define Q1_H

// Estrutura para guardar as referencias do rodape
typedef struct ref {
    char id[50];      
    char texto[1000]; // O texto da referencia
    struct ref *prox;
} Referencia;

// Estrutura para os pesquisadores
typedef struct pesq {
    char nome[100];
    char refs[200];   // Guarda os IDs ex: "cite_note-1 cite_note-5"
    struct pesq *prox;
} Pesquisador;

// Funcoes
char* ler_arquivo(char* nome);
void limpar_html(char* dest, char* orig);
Referencia* carregar_referencias(char* html);
Pesquisador* carregar_pesquisadores(char* html);
void listar_nomes(Pesquisador* lista);
void mostrar_referencias(Pesquisador* p, Referencia* r, char* busca);
void liberar_memoria(Pesquisador* p, Referencia* r, char* html);

#endif