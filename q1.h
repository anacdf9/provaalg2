#include "cliente.h"
#include "recurso.h"
#include "equipe.h"
#include "fornecedor.h"
#include "operador.h"
#include "produtora.h"
#include "evento.h"
#include "evento_item.h"
#include "evento_equipe.h"
#include "evento_fornecedor.h"
#include "transacoes.h"
#include "config.h"
#include <stdlib.h>   

/* Inicializa o sistema de persistência (usa TIPO_PERSISTENCIA por padrão) */
void pers_inicializar(TipoPersistencia tipo);
void pers_finalizar(void);

/* CRUD cliente (fachada) */
int pers_salvar_cliente(Cliente c);
int pers_carregar_clientes(Cliente *lista, int max);
int pers_remover_cliente(int id);

/* CRUD recurso */
int pers_salvar_recurso(Recurso r);
int pers_carregar_recursos(Recurso *lista, int max);
int pers_remover_recurso(int id);

/* CRUD equipe */
int pers_salvar_equipe(Equipe e);
int pers_carregar_equipes(Equipe *lista, int max);
int pers_remover_equipe(int id);

/* CRUD fornecedor */
int pers_salvar_fornecedor(Fornecedor f);
int pers_carregar_fornecedores(Fornecedor *lista, int max);
int pers_remover_fornecedor(int id);

/* CRUD operador */
int pers_salvar_operador(Operador o);
int pers_carregar_operadores(Operador *lista, int max);
int pers_remover_operador(int id);

/* Produtora (único) */
int pers_salvar_produtora(Produtora p);
int pers_obter_produtora(Produtora *out);
int pers_remover_produtora(void);

/* Evento (orçamentos e gestão) */
int pers_salvar_evento(Evento e);
int pers_carregar_eventos(Evento *lista, int max);
int pers_remover_evento(int id);

/* Itens de evento (múltiplos recursos por evento) */
int pers_salvar_evento_item(EventoItem it);
int pers_carregar_evento_itens(EventoItem *lista, int max);
int pers_remover_evento_itens_por_evento(int evento_id);

/* Itens de evento (múltiplas equipes por evento) */
int pers_salvar_evento_equipe(EventoEquipe ee);
int pers_carregar_evento_equipes(EventoEquipe *lista, int max);
int pers_remover_evento_equipes_por_evento(int evento_id);

/* Itens de evento (múltiplos fornecedores por evento) */
int pers_salvar_evento_fornecedor(EventoFornecedor ef);
int pers_carregar_evento_fornecedores(EventoFornecedor *lista, int max);
int pers_remover_evento_fornecedores_por_evento(int evento_id);

/* Caixa */
int pers_salvar_caixa(CaixaLancamento l);
int pers_carregar_caixa(CaixaLancamento *lista, int max);

/* Contas a Receber */
int pers_salvar_conta_receber(ContaReceber c);
int pers_carregar_contas_receber(ContaReceber *lista, int max);

/* Compras */
int pers_salvar_compra(Compra c);
int pers_carregar_compras(Compra *lista, int max);
int pers_salvar_compra_item(CompraItem it);
int pers_carregar_compra_itens_por_compra(int compra_id, CompraItem *lista, int max);

/* Contas a Pagar */
int pers_salvar_conta_pagar(ContaPagar c);
int pers_carregar_contas_pagar(ContaPagar *lista, int max);

#endif // PERS_H