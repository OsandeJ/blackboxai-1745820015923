#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <gtk/gtk.h>

// Enums para as colunas das TreeViews
enum {
    COL_PRODUTO_ID,
    COL_PRODUTO_NOME,
    COL_PRODUTO_PRECO,
    COL_PRODUTO_IVA,
    COL_PRODUTO_PRECO_TOTAL,
    NUM_COLS_PRODUTO
};

enum {
    COL_CLIENTE_ID,
    COL_CLIENTE_NOME,
    COL_CLIENTE_TELEFONE,
    COL_CLIENTE_NIF,
    NUM_COLS_CLIENTE
};

enum {
    COL_PEDIDO_ID,
    COL_PEDIDO_DATA,
    COL_PEDIDO_CLIENTE,
    COL_PEDIDO_SUBTOTAL,
    COL_PEDIDO_IVA,
    COL_PEDIDO_TOTAL,
    NUM_COLS_PEDIDO
};

typedef struct {
    GtkWidget *window;
    GtkWidget *header_bar;
    GtkWidget *stack;
    GtkWidget *stack_switcher;
    
    // Container para cada view
    GtkWidget *produtos_container;
    GtkWidget *clientes_container;
    GtkWidget *pedidos_container;
    
    // Widgets da view de produtos
    GtkWidget *btn_novo_produto;
    GtkWidget *btn_editar_produto;
    GtkWidget *btn_excluir_produto;
    GtkWidget *scale_filtro_preco;
    GtkWidget *tree_produtos;
    
    // Widgets da view de clientes
    GtkWidget *btn_novo_cliente;
    GtkWidget *btn_editar_cliente;
    GtkWidget *btn_excluir_cliente;
    GtkWidget *entry_busca_cliente;
    GtkWidget *tree_clientes;
    
    // Widgets da view de pedidos
    GtkWidget *btn_novo_pedido;
    GtkWidget *btn_visualizar_pedido;
    GtkWidget *btn_imprimir_fatura;
    GtkWidget *tree_pedidos;
} MainWindow;

// Funções principais
void criar_janela_principal(void);
void destruir_janela_principal(void);

// Funções de atualização das listas
void atualizar_lista_produtos(void);
void atualizar_lista_clientes(void);
void atualizar_lista_pedidos(void);

// Callbacks dos botões de produtos
void on_novo_produto_clicked(GtkButton *button, gpointer user_data);
void on_editar_produto_clicked(GtkButton *button, gpointer user_data);
void on_excluir_produto_clicked(GtkButton *button, gpointer user_data);
void on_filtro_valor_changed(GtkRange *range, gpointer user_data);
void on_filtro_expander_changed(GtkExpander *expander, GParamSpec *pspec, gpointer user_data);

// Callbacks dos botões de clientes
void on_novo_cliente_clicked(GtkButton *button, gpointer user_data);
void on_editar_cliente_clicked(GtkButton *button, gpointer user_data);
void on_excluir_cliente_clicked(GtkButton *button, gpointer user_data);

// Callbacks dos botões de pedidos
void on_novo_pedido_clicked(GtkButton *button, gpointer user_data);
void on_visualizar_pedido_clicked(GtkButton *button, gpointer user_data);
void on_imprimir_fatura_clicked(GtkButton *button, gpointer user_data);

#endif // MAIN_WINDOW_H
