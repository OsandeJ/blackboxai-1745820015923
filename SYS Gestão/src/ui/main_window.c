#include "main_window.h"
#include "product_window.h"
#include "customer_window.h"
#include "order_window.h"
#include "../database.h"
#include "../utils.h"

// Variáveis globais
MainWindow *main_window = NULL;
static gboolean filtro_ativo = FALSE;
static GtkWidget *current_sidebar_button = NULL;

// Callback para navegação da sidebar
static void on_sidebar_button_clicked(GtkWidget *button, gpointer user_data) {
    const char *view_name = (const char *)user_data;
    
    // Atualizar estilo do botão atual
    if (current_sidebar_button) {
        gtk_style_context_remove_class(
            gtk_widget_get_style_context(current_sidebar_button),
            "sidebar-active"
        );
    }
    
    gtk_style_context_add_class(
        gtk_widget_get_style_context(button),
        "sidebar-active"
    );
    
    current_sidebar_button = button;
    
    // Mudar para a view selecionada
    gtk_stack_set_visible_child_name(GTK_STACK(main_window->stack), view_name);
}

// Função auxiliar para formatar valores monetários
static void formatar_valor_monetario(char *buffer, size_t size, double valor) {
    char valor_str[32];
    g_snprintf(valor_str, sizeof(valor_str), "%.2f", valor);
    
    // Substituir ponto por vírgula
    char *ponto = strchr(valor_str, '.');
    if (ponto) {
        *ponto = ',';
    }
    
    g_snprintf(buffer, size, "%s Kz", valor_str);
}

void criar_produtos_view(void) {
    main_window->produtos_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    
    // Botões de ação com ícones
    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_box), "button-container");
    gtk_widget_set_margin_top(btn_box, 16);
    gtk_widget_set_margin_bottom(btn_box, 16);
    
    // Novo Produto
    main_window->btn_novo_produto = gtk_button_new();
    GtkWidget *novo_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *novo_icon = gtk_image_new_from_icon_name("list-add-symbolic", GTK_ICON_SIZE_BUTTON);
    GtkWidget *novo_label = gtk_label_new("Novo");
    gtk_widget_set_margin_start(novo_icon, 4);
    gtk_widget_set_margin_end(novo_label, 4);
    gtk_box_pack_start(GTK_BOX(novo_box), novo_icon, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(novo_box), novo_label, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(main_window->btn_novo_produto), novo_box);
    gtk_style_context_add_class(gtk_widget_get_style_context(main_window->btn_novo_produto), "action-button");
    gtk_style_context_add_class(gtk_widget_get_style_context(main_window->btn_novo_produto), "new-product-button");
    gtk_widget_set_tooltip_text(main_window->btn_novo_produto, "Adicionar novo produto");
    
    // Editar
    main_window->btn_editar_produto = gtk_button_new();
    GtkWidget *edit_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *edit_icon = gtk_image_new_from_icon_name("document-edit-symbolic", GTK_ICON_SIZE_BUTTON);
    GtkWidget *edit_label = gtk_label_new("Editar");
    gtk_widget_set_margin_start(edit_icon, 4);
    gtk_widget_set_margin_end(edit_label, 4);
    gtk_box_pack_start(GTK_BOX(edit_box), edit_icon, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(edit_box), edit_label, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(main_window->btn_editar_produto), edit_box);
    gtk_style_context_add_class(gtk_widget_get_style_context(main_window->btn_editar_produto), "action-button");
    gtk_style_context_add_class(gtk_widget_get_style_context(main_window->btn_editar_produto), "edit-button");
    gtk_widget_set_tooltip_text(main_window->btn_editar_produto, "Editar produto selecionado");
    
    // Excluir
    main_window->btn_excluir_produto = gtk_button_new();
    GtkWidget *delete_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *delete_icon = gtk_image_new_from_icon_name("edit-delete-symbolic", GTK_ICON_SIZE_BUTTON);
    GtkWidget *delete_label = gtk_label_new("Excluir");
    gtk_widget_set_margin_start(delete_icon, 4);
    gtk_widget_set_margin_end(delete_label, 4);
    gtk_box_pack_start(GTK_BOX(delete_box), delete_icon, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(delete_box), delete_label, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(main_window->btn_excluir_produto), delete_box);
    gtk_style_context_add_class(gtk_widget_get_style_context(main_window->btn_excluir_produto), "action-button");
    gtk_style_context_add_class(gtk_widget_get_style_context(main_window->btn_excluir_produto), "delete-button");
    gtk_widget_set_tooltip_text(main_window->btn_excluir_produto, "Excluir produto selecionado");
    
    gtk_box_pack_start(GTK_BOX(btn_box), main_window->btn_novo_produto, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box), main_window->btn_editar_produto, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box), main_window->btn_excluir_produto, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_window->produtos_container), btn_box, FALSE, FALSE, 5);
    
    // Expander para filtro de preço com estilo moderno
    GtkWidget *expander = gtk_expander_new("Filtrar por Preço");
    gtk_box_pack_start(GTK_BOX(main_window->produtos_container), expander, FALSE, FALSE, 5);
    gtk_style_context_add_class(gtk_widget_get_style_context(expander), "price-filter");
    
    // Container para o conteúdo do expander
    GtkWidget *filtro_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_container_add(GTK_CONTAINER(expander), filtro_box);
    
    // Container para o valor atual
    GtkWidget *valor_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    GtkWidget *valor_atual_label = gtk_label_new("Valor: ");
    GtkWidget *label_valor = gtk_label_new("0 Kz");
    gtk_style_context_add_class(gtk_widget_get_style_context(label_valor), "price-value");
    gtk_box_pack_start(GTK_BOX(valor_box), valor_atual_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(valor_box), label_valor, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(filtro_box), valor_box, FALSE, FALSE, 0);
    
    // Slider para filtro de preço
    GtkWidget *slider_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    main_window->scale_filtro_preco = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 1000000, 1000);
    gtk_scale_set_draw_value(GTK_SCALE(main_window->scale_filtro_preco), FALSE);
    gtk_box_pack_start(GTK_BOX(slider_box), main_window->scale_filtro_preco, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(filtro_box), slider_box, TRUE, TRUE, 5);
    
    // Conectar sinal para atualizar o valor em Kwanzas
    g_signal_connect(main_window->scale_filtro_preco, "value-changed",
                    G_CALLBACK(on_filtro_valor_changed), label_valor);
    
    // Conectar sinal do expander
    g_signal_connect(expander, "notify::expanded",
                    G_CALLBACK(on_filtro_expander_changed), NULL);
    
    // TreeView para listar produtos com estilo moderno
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_style_context_add_class(gtk_widget_get_style_context(scroll), "products-list");
    main_window->tree_produtos = gtk_tree_view_new();
    gtk_style_context_add_class(gtk_widget_get_style_context(main_window->tree_produtos), "modern-tree-view");
    
    // Colunas da lista de produtos
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    
    // Coluna ID
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("ID", renderer, "text", COL_PRODUTO_ID, NULL);
    gtk_tree_view_column_set_alignment(column, 0.5);
    g_object_set(renderer, "xalign", 0.5, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(main_window->tree_produtos), column);
    
    // Coluna Nome
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Nome", renderer, "text", COL_PRODUTO_NOME, NULL);
    gtk_tree_view_column_set_alignment(column, 0.0);
    g_object_set(renderer, "xalign", 0.0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(main_window->tree_produtos), column);
    
    // Coluna Preço
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Preço", renderer, "text", COL_PRODUTO_PRECO, NULL);
    gtk_tree_view_column_set_alignment(column, 0.5);
    g_object_set(renderer, "xalign", 0.5, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(main_window->tree_produtos), column);
    
    // Coluna IVA
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("IVA (14%)", renderer, "text", COL_PRODUTO_IVA, NULL);
    gtk_tree_view_column_set_alignment(column, 0.5);
    g_object_set(renderer, "xalign", 0.5, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(main_window->tree_produtos), column);
    
    // Coluna Preço com IVA
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Preço com IVA", renderer, "text", COL_PRODUTO_PRECO_TOTAL, NULL);
    gtk_tree_view_column_set_alignment(column, 0.5);
    g_object_set(renderer, "xalign", 0.5, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(main_window->tree_produtos), column);
    
    gtk_container_add(GTK_CONTAINER(scroll), main_window->tree_produtos);
    gtk_box_pack_start(GTK_BOX(main_window->produtos_container), scroll, TRUE, TRUE, 0);
    
    // Conectar sinais
    g_signal_connect(main_window->btn_novo_produto, "clicked", G_CALLBACK(on_novo_produto_clicked), NULL);
    g_signal_connect(main_window->btn_editar_produto, "clicked", G_CALLBACK(on_editar_produto_clicked), NULL);
    g_signal_connect(main_window->btn_excluir_produto, "clicked", G_CALLBACK(on_excluir_produto_clicked), NULL);
    g_signal_connect(main_window->scale_filtro_preco, "value-changed", G_CALLBACK(on_filtro_valor_changed), NULL);
}

void criar_clientes_view(void) {
    main_window->clientes_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    
    // Botões de ação
    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_box), "button-container");
    
    // Novo Cliente
    main_window->btn_novo_cliente = gtk_button_new();
    GtkWidget *novo_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *novo_icon = gtk_image_new_from_icon_name("list-add-symbolic", GTK_ICON_SIZE_BUTTON);
    GtkWidget *novo_label = gtk_label_new("Novo Cliente");
    gtk_box_pack_start(GTK_BOX(novo_box), novo_icon, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(novo_box), novo_label, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(main_window->btn_novo_cliente), novo_box);
    gtk_style_context_add_class(gtk_widget_get_style_context(main_window->btn_novo_cliente), "action-button");
    gtk_style_context_add_class(gtk_widget_get_style_context(main_window->btn_novo_cliente), "new-product-button");
    
    // Editar Cliente
    main_window->btn_editar_cliente = gtk_button_new();
    GtkWidget *edit_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *edit_icon = gtk_image_new_from_icon_name("document-edit-symbolic", GTK_ICON_SIZE_BUTTON);
    GtkWidget *edit_label = gtk_label_new("Editar");
    gtk_box_pack_start(GTK_BOX(edit_box), edit_icon, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(edit_box), edit_label, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(main_window->btn_editar_cliente), edit_box);
    gtk_style_context_add_class(gtk_widget_get_style_context(main_window->btn_editar_cliente), "action-button");
    gtk_style_context_add_class(gtk_widget_get_style_context(main_window->btn_editar_cliente), "edit-button");
    
    // Excluir Cliente
    main_window->btn_excluir_cliente = gtk_button_new();
    GtkWidget *delete_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *delete_icon = gtk_image_new_from_icon_name("edit-delete-symbolic", GTK_ICON_SIZE_BUTTON);
    GtkWidget *delete_label = gtk_label_new("Excluir");
    gtk_box_pack_start(GTK_BOX(delete_box), delete_icon, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(delete_box), delete_label, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(main_window->btn_excluir_cliente), delete_box);
    gtk_style_context_add_class(gtk_widget_get_style_context(main_window->btn_excluir_cliente), "action-button");
    gtk_style_context_add_class(gtk_widget_get_style_context(main_window->btn_excluir_cliente), "delete-button");
    
    gtk_box_pack_start(GTK_BOX(btn_box), main_window->btn_novo_cliente, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box), main_window->btn_editar_cliente, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box), main_window->btn_excluir_cliente, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_window->clientes_container), btn_box, FALSE, FALSE, 5);
    
    // TreeView para listar clientes
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    main_window->tree_clientes = gtk_tree_view_new();
    gtk_style_context_add_class(gtk_widget_get_style_context(main_window->tree_clientes), "modern-tree-view");
    
    // Colunas da lista de clientes
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    
    // Coluna ID
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("ID", renderer, "text", COL_CLIENTE_ID, NULL);
    gtk_tree_view_column_set_alignment(column, 0.5);
    g_object_set(renderer, "xalign", 0.5, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(main_window->tree_clientes), column);
    
    // Coluna Nome
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Nome", renderer, "text", COL_CLIENTE_NOME, NULL);
    gtk_tree_view_column_set_alignment(column, 0.0);
    g_object_set(renderer, "xalign", 0.0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(main_window->tree_clientes), column);
    
    // Coluna Telefone
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Telefone", renderer, "text", COL_CLIENTE_TELEFONE, NULL);
    gtk_tree_view_column_set_alignment(column, 0.5);
    g_object_set(renderer, "xalign", 0.5, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(main_window->tree_clientes), column);
    
    // Coluna NIF
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("NIF", renderer, "text", COL_CLIENTE_NIF, NULL);
    gtk_tree_view_column_set_alignment(column, 0.5);
    g_object_set(renderer, "xalign", 0.5, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(main_window->tree_clientes), column);
    
    gtk_container_add(GTK_CONTAINER(scroll), main_window->tree_clientes);
    gtk_box_pack_start(GTK_BOX(main_window->clientes_container), scroll, TRUE, TRUE, 0);
    
    // Conectar sinais
    g_signal_connect(main_window->btn_novo_cliente, "clicked", G_CALLBACK(on_novo_cliente_clicked), NULL);
    g_signal_connect(main_window->btn_editar_cliente, "clicked", G_CALLBACK(on_editar_cliente_clicked), NULL);
    g_signal_connect(main_window->btn_excluir_cliente, "clicked", G_CALLBACK(on_excluir_cliente_clicked), NULL);
}

void criar_pedidos_view(void) {
    main_window->pedidos_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    
    // Botões de ação
    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_box), "button-container");
    
    // Novo Pedido
    main_window->btn_novo_pedido = gtk_button_new();
    GtkWidget *novo_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *novo_icon = gtk_image_new_from_icon_name("list-add-symbolic", GTK_ICON_SIZE_BUTTON);
    GtkWidget *novo_label = gtk_label_new("Novo Pedido");
    gtk_box_pack_start(GTK_BOX(novo_box), novo_icon, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(novo_box), novo_label, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(main_window->btn_novo_pedido), novo_box);
    gtk_style_context_add_class(gtk_widget_get_style_context(main_window->btn_novo_pedido), "action-button");
    gtk_style_context_add_class(gtk_widget_get_style_context(main_window->btn_novo_pedido), "new-product-button");
    
    // Visualizar Pedido
    main_window->btn_visualizar_pedido = gtk_button_new();
    GtkWidget *view_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *view_icon = gtk_image_new_from_icon_name("document-open-symbolic", GTK_ICON_SIZE_BUTTON);
    GtkWidget *view_label = gtk_label_new("Visualizar");
    gtk_box_pack_start(GTK_BOX(view_box), view_icon, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(view_box), view_label, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(main_window->btn_visualizar_pedido), view_box);
    gtk_style_context_add_class(gtk_widget_get_style_context(main_window->btn_visualizar_pedido), "action-button");
    gtk_style_context_add_class(gtk_widget_get_style_context(main_window->btn_visualizar_pedido), "edit-button");
    
    // Imprimir Fatura
    main_window->btn_imprimir_fatura = gtk_button_new();
    GtkWidget *print_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *print_icon = gtk_image_new_from_icon_name("document-print-symbolic", GTK_ICON_SIZE_BUTTON);
    GtkWidget *print_label = gtk_label_new("Imprimir Fatura");
    gtk_box_pack_start(GTK_BOX(print_box), print_icon, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(print_box), print_label, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(main_window->btn_imprimir_fatura), print_box);
    gtk_style_context_add_class(gtk_widget_get_style_context(main_window->btn_imprimir_fatura), "action-button");
    gtk_style_context_add_class(gtk_widget_get_style_context(main_window->btn_imprimir_fatura), "edit-button");
    
    gtk_box_pack_start(GTK_BOX(btn_box), main_window->btn_novo_pedido, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box), main_window->btn_visualizar_pedido, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box), main_window->btn_imprimir_fatura, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_window->pedidos_container), btn_box, FALSE, FALSE, 5);
    
    // TreeView para listar pedidos
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    main_window->tree_pedidos = gtk_tree_view_new();
    gtk_style_context_add_class(gtk_widget_get_style_context(main_window->tree_pedidos), "modern-tree-view");
    
    // Colunas da lista de pedidos
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    
    // Coluna ID
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("ID", renderer, "text", COL_PEDIDO_ID, NULL);
    gtk_tree_view_column_set_alignment(column, 0.5);
    g_object_set(renderer, "xalign", 0.5, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(main_window->tree_pedidos), column);
    
    // Coluna Data
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Data", renderer, "text", COL_PEDIDO_DATA, NULL);
    gtk_tree_view_column_set_alignment(column, 0.5);
    g_object_set(renderer, "xalign", 0.5, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(main_window->tree_pedidos), column);
    
    // Coluna Cliente
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Cliente", renderer, "text", COL_PEDIDO_CLIENTE, NULL);
    gtk_tree_view_column_set_alignment(column, 0.0);
    g_object_set(renderer, "xalign", 0.0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(main_window->tree_pedidos), column);
    
    // Coluna Subtotal
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Subtotal", renderer, "text", COL_PEDIDO_SUBTOTAL, NULL);
    gtk_tree_view_column_set_alignment(column, 0.5);
    g_object_set(renderer, "xalign", 0.5, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(main_window->tree_pedidos), column);
    
    // Coluna IVA
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("IVA", renderer, "text", COL_PEDIDO_IVA, NULL);
    gtk_tree_view_column_set_alignment(column, 0.5);
    g_object_set(renderer, "xalign", 0.5, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(main_window->tree_pedidos), column);
    
    // Coluna Total
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Total", renderer, "text", COL_PEDIDO_TOTAL, NULL);
    gtk_tree_view_column_set_alignment(column, 0.5);
    g_object_set(renderer, "xalign", 0.5, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(main_window->tree_pedidos), column);
    
    gtk_container_add(GTK_CONTAINER(scroll), main_window->tree_pedidos);
    gtk_box_pack_start(GTK_BOX(main_window->pedidos_container), scroll, TRUE, TRUE, 0);
    
    // Conectar sinais
    g_signal_connect(main_window->btn_novo_pedido, "clicked", G_CALLBACK(on_novo_pedido_clicked), NULL);
    g_signal_connect(main_window->btn_visualizar_pedido, "clicked", G_CALLBACK(on_visualizar_pedido_clicked), NULL);
    g_signal_connect(main_window->btn_imprimir_fatura, "clicked", G_CALLBACK(on_imprimir_fatura_clicked), NULL);
}

// Função para criar botão da sidebar com ícone
static GtkWidget* criar_botao_sidebar(const char *icon_name, const char *label_text) {
    GtkWidget *button = gtk_button_new();
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    GtkWidget *icon = gtk_image_new_from_icon_name(icon_name, GTK_ICON_SIZE_BUTTON);
    GtkWidget *label = gtk_label_new(label_text);
    
    gtk_box_pack_start(GTK_BOX(box), icon, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(button), box);
    
    gtk_widget_set_size_request(button, 200, 48);
    gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
    gtk_widget_set_halign(box, GTK_ALIGN_START);
    
    return button;
}

void criar_janela_principal(void) {
    // Alocar e inicializar a estrutura principal
    main_window = g_malloc(sizeof(MainWindow));
    
    // Criar janela principal
    main_window->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window->window), "Sistema de Gestão");
    gtk_window_set_default_size(GTK_WINDOW(main_window->window), 1200, 800);
    
    // Container principal
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    
    // Criar sidebar
    GtkWidget *sidebar = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_style_context_add_class(gtk_widget_get_style_context(sidebar), "sidebar");
    
    // Logo e título na sidebar
    GtkWidget *header_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    GtkWidget *logo = gtk_image_new_from_icon_name("system-software-install", GTK_ICON_SIZE_DIALOG);
    GtkWidget *title = gtk_label_new("Sistema de Gestão");
    gtk_style_context_add_class(gtk_widget_get_style_context(header_box), "sidebar-header");
    gtk_box_pack_start(GTK_BOX(header_box), logo, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(header_box), title, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(sidebar), header_box, FALSE, FALSE, 16);
    
    // Botões de navegação
    GtkWidget *nav_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_top(nav_box, 24);
    
    // Criar botões da sidebar
    GtkWidget *btn_produtos = criar_botao_sidebar("package", "Produtos");
    GtkWidget *btn_clientes = criar_botao_sidebar("person", "Clientes");
    GtkWidget *btn_pedidos = criar_botao_sidebar("shopping-cart", "Pedidos");
    
    // Adicionar classes CSS
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_produtos), "sidebar-button");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_clientes), "sidebar-button");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_pedidos), "sidebar-button");
    
    // Adicionar botões ao nav_box
    gtk_box_pack_start(GTK_BOX(nav_box), btn_produtos, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(nav_box), btn_clientes, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(nav_box), btn_pedidos, FALSE, FALSE, 0);
    
    // Adicionar nav_box à sidebar
    gtk_box_pack_start(GTK_BOX(sidebar), nav_box, FALSE, FALSE, 0);
    
    // Criar stack para as diferentes views
    main_window->stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(main_window->stack), 
                                GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    
    // Criar as diferentes views
    criar_produtos_view();
    criar_clientes_view();
    criar_pedidos_view();
    
    // Configurar as páginas do stack
    gtk_stack_add_named(GTK_STACK(main_window->stack), 
                       main_window->produtos_container, "produtos");
    gtk_stack_add_named(GTK_STACK(main_window->stack), 
                       main_window->clientes_container, "clientes");
    gtk_stack_add_named(GTK_STACK(main_window->stack), 
                       main_window->pedidos_container, "pedidos");
    
    // Conectar sinais dos botões da sidebar
    g_signal_connect(btn_produtos, "clicked", 
                    G_CALLBACK(on_sidebar_button_clicked), "produtos");
    g_signal_connect(btn_clientes, "clicked", 
                    G_CALLBACK(on_sidebar_button_clicked), "clientes");
    g_signal_connect(btn_pedidos, "clicked", 
                    G_CALLBACK(on_sidebar_button_clicked), "pedidos");
    
    // Adicionar sidebar e stack ao container principal
    gtk_box_pack_start(GTK_BOX(main_box), sidebar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), main_window->stack, TRUE, TRUE, 0);
    
    // Adicionar container principal à janela
    gtk_container_add(GTK_CONTAINER(main_window->window), main_box);
    
    // Conectar sinais para fechar o programa corretamente
    g_signal_connect(main_window->window, "delete-event", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(main_window->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    // Mostrar todos os widgets
    gtk_widget_show_all(main_window->window);
    
    // Carregar dados iniciais
    atualizar_lista_produtos();
    atualizar_lista_clientes();
    atualizar_lista_pedidos();
}

void destruir_janela_principal(void) {
    if (main_window) {
        g_free(main_window);
        main_window = NULL;
    }
}

void atualizar_lista_produtos(void) {
    GtkListStore *store = gtk_list_store_new(NUM_COLS_PRODUTO,
                                            G_TYPE_INT,    // ID
                                            G_TYPE_STRING, // Nome
                                            G_TYPE_STRING, // Preço Base (formatado)
                                            G_TYPE_STRING, // IVA (formatado)
                                            G_TYPE_STRING  // Preço Total (formatado)
                                            );
    
    GtkTreeIter iter;
    GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(main_window->scale_filtro_preco));
    double preco_maximo = gtk_adjustment_get_value(adj);
    
    for (int i = 0; i < num_produtos; i++) {
        Produto *p = &produtos[i];
        
        // Aplicar filtro apenas se estiver ativo
        if (!filtro_ativo || p->preco <= preco_maximo) {
            double iva = calcular_iva(p->preco);
            double total = calcular_preco_com_iva(p->preco);
            
            char preco_str[32], iva_str[32], total_str[32];
            formatar_valor_monetario(preco_str, sizeof(preco_str), p->preco);
            formatar_valor_monetario(iva_str, sizeof(iva_str), iva);
            formatar_valor_monetario(total_str, sizeof(total_str), total);
            
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter,
                              COL_PRODUTO_ID, p->id,
                              COL_PRODUTO_NOME, p->nome,
                              COL_PRODUTO_PRECO, preco_str,
                              COL_PRODUTO_IVA, iva_str,
                              COL_PRODUTO_PRECO_TOTAL, total_str,
                              -1);
        }
    }
    
    gtk_tree_view_set_model(GTK_TREE_VIEW(main_window->tree_produtos), 
                           GTK_TREE_MODEL(store));
    g_object_unref(store);
}

void atualizar_lista_clientes(void) {
    GtkListStore *store = gtk_list_store_new(NUM_COLS_CLIENTE,
                                            G_TYPE_INT,    // ID
                                            G_TYPE_STRING, // Nome
                                            G_TYPE_STRING, // Telefone
                                            G_TYPE_STRING  // NIF
                                            );
    
    GtkTreeIter iter;
    for (int i = 0; i < num_clientes; i++) {
        Cliente *c = &clientes[i];
        
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                          COL_CLIENTE_ID, c->id,
                          COL_CLIENTE_NOME, c->nome,
                          COL_CLIENTE_TELEFONE, c->telefone,
                          COL_CLIENTE_NIF, c->nif,
                          -1);
    }
    
    gtk_tree_view_set_model(GTK_TREE_VIEW(main_window->tree_clientes), 
                           GTK_TREE_MODEL(store));
    g_object_unref(store);
}

void atualizar_lista_pedidos(void) {
    GtkListStore *store = gtk_list_store_new(NUM_COLS_PEDIDO,
                                            G_TYPE_INT,    // ID
                                            G_TYPE_STRING, // Data
                                            G_TYPE_STRING, // Cliente
                                            G_TYPE_STRING, // Subtotal
                                            G_TYPE_STRING, // IVA
                                            G_TYPE_STRING  // Total
                                            );
    
    GtkTreeIter iter;
    for (int i = 0; i < num_pedidos; i++) {
        Pedido *p = &pedidos[i];
        Cliente *c = buscar_cliente(p->idCliente);
        
        char subtotal_str[32], iva_str[32], total_str[32];
        formatar_valor_monetario(subtotal_str, sizeof(subtotal_str), p->subtotal);
        formatar_valor_monetario(iva_str, sizeof(iva_str), p->iva_total);
        formatar_valor_monetario(total_str, sizeof(total_str), p->total);
        
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                          COL_PEDIDO_ID, p->id,
                          COL_PEDIDO_DATA, p->data,
                          COL_PEDIDO_CLIENTE, c ? c->nome : "Cliente não encontrado",
                          COL_PEDIDO_SUBTOTAL, subtotal_str,
                          COL_PEDIDO_IVA, iva_str,
                          COL_PEDIDO_TOTAL, total_str,
                          -1);
    }
    
    gtk_tree_view_set_model(GTK_TREE_VIEW(main_window->tree_pedidos), 
                           GTK_TREE_MODEL(store));
    g_object_unref(store);
}

void on_novo_produto_clicked(GtkButton *button, gpointer user_data) {
    (void)button;      // Parâmetro intencionalmente não utilizado
    (void)user_data;   // Parâmetro intencionalmente não utilizado
    mostrar_dialogo_produto(GTK_WINDOW(main_window->window), NULL);
}

void on_editar_produto_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    (void)user_data;
    
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(main_window->tree_produtos));
    GtkTreeModel *model;
    GtkTreeIter iter;
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        int id;
        gtk_tree_model_get(model, &iter, COL_PRODUTO_ID, &id, -1);
        
        Produto *produto = buscar_produto(id);
        if (produto) {
            mostrar_dialogo_produto(GTK_WINDOW(main_window->window), produto);
        }
    } else {
        mostrar_erro(GTK_WINDOW(main_window->window), "Selecione um produto para editar.");
    }
}

void on_excluir_produto_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    (void)user_data;
    
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(main_window->tree_produtos));
    GtkTreeModel *model;
    GtkTreeIter iter;
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        int id;
        gtk_tree_model_get(model, &iter, COL_PRODUTO_ID, &id, -1);
        
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(main_window->window),
                                                 GTK_DIALOG_MODAL,
                                                 GTK_MESSAGE_QUESTION,
                                                 GTK_BUTTONS_YES_NO,
                                                 "Tem certeza que deseja excluir este produto?");
        
        gint response = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        
        if (response == GTK_RESPONSE_YES) {
            gboolean sucesso = remover_produto(id);
            if (sucesso) {
                mostrar_sucesso(GTK_WINDOW(main_window->window), "Produto excluído com sucesso!");
                atualizar_lista_produtos();
            } else {
                mostrar_erro(GTK_WINDOW(main_window->window), 
                           "Erro ao excluir produto. Por favor, tente novamente.");
            }
        }
    } else {
        mostrar_erro(GTK_WINDOW(main_window->window), "Selecione um produto para excluir.");
    }
}

void on_filtro_valor_changed(GtkRange *range, gpointer user_data) {
    GtkLabel *label = GTK_LABEL(user_data);
    double valor = gtk_range_get_value(range);
    
    // Formatar valor em Kwanzas
    char buffer[32];
    formatar_valor_monetario(buffer, sizeof(buffer), valor);
    gtk_label_set_text(label, buffer);
    
    // Encontrar o preço máximo entre todos os produtos
    double preco_maximo = 0;
    for (int i = 0; i < num_produtos; i++) {
        if (produtos[i].preco > preco_maximo) {
            preco_maximo = produtos[i].preco;
        }
    }
    
    // Atualizar o range máximo do slider se necessário
    GtkAdjustment *adj = gtk_range_get_adjustment(range);
    double upper = gtk_adjustment_get_upper(adj);
    if (preco_maximo > upper) {
        gtk_adjustment_set_upper(adj, preco_maximo);
    }
    
    // Atualizar a lista de produtos com o filtro
    if (filtro_ativo) {
        atualizar_lista_produtos();
    }
}

void on_filtro_expander_changed(GtkExpander *expander, GParamSpec *pspec, gpointer user_data) {
    (void)pspec;
    (void)user_data;
    
    filtro_ativo = gtk_expander_get_expanded(expander);
    atualizar_lista_produtos();
}

void on_novo_cliente_clicked(GtkButton *button, gpointer user_data) {
    (void)button;      // Parâmetro intencionalmente não utilizado
    (void)user_data;   // Parâmetro intencionalmente não utilizado
    mostrar_dialogo_cliente(GTK_WINDOW(main_window->window), NULL);
}

void on_editar_cliente_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    (void)user_data;
    
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(main_window->tree_clientes));
    GtkTreeModel *model;
    GtkTreeIter iter;
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        int id;
        gtk_tree_model_get(model, &iter, COL_CLIENTE_ID, &id, -1);
        
        Cliente *cliente = buscar_cliente(id);
        if (cliente) {
            mostrar_dialogo_cliente(GTK_WINDOW(main_window->window), cliente);
        }
    } else {
        mostrar_erro(GTK_WINDOW(main_window->window), "Selecione um cliente para editar.");
    }
}

void on_excluir_cliente_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    (void)user_data;
    
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(main_window->tree_clientes));
    GtkTreeModel *model;
    GtkTreeIter iter;
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        int id;
        gtk_tree_model_get(model, &iter, COL_CLIENTE_ID, &id, -1);
        
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(main_window->window),
                                                 GTK_DIALOG_MODAL,
                                                 GTK_MESSAGE_QUESTION,
                                                 GTK_BUTTONS_YES_NO,
                                                 "Tem certeza que deseja excluir este cliente?");
        
        gint response = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        
        if (response == GTK_RESPONSE_YES) {
            gboolean sucesso = remover_cliente(id);
            if (sucesso) {
                mostrar_sucesso(GTK_WINDOW(main_window->window), "Cliente excluído com sucesso!");
                atualizar_lista_clientes();
            } else {
                mostrar_erro(GTK_WINDOW(main_window->window), 
                           "Erro ao excluir cliente. Por favor, tente novamente.");
            }
        }
    } else {
        mostrar_erro(GTK_WINDOW(main_window->window), "Selecione um cliente para excluir.");
    }
}

void on_novo_pedido_clicked(GtkButton *button, gpointer user_data) {
    (void)button;      // Parâmetro intencionalmente não utilizado
    (void)user_data;   // Parâmetro intencionalmente não utilizado
    mostrar_dialogo_pedido(GTK_WINDOW(main_window->window), NULL);
}

void on_visualizar_pedido_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    (void)user_data;
    
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(main_window->tree_pedidos));
    GtkTreeModel *model;
    GtkTreeIter iter;
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        int id;
        gtk_tree_model_get(model, &iter, COL_PEDIDO_ID, &id, -1);
        
        // Criar diálogo de visualização
        GtkWidget *dialog = gtk_dialog_new_with_buttons("Detalhes do Pedido",
                                                      GTK_WINDOW(main_window->window),
                                                      GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                      "_Fechar", GTK_RESPONSE_CLOSE,
                                                      NULL);
        
        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        gtk_container_set_border_width(GTK_CONTAINER(content_area), 10);
        
        // Criar uma view de texto com scroll
        GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
        GtkWidget *text_view = gtk_text_view_new();
        gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
        gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
        gtk_container_add(GTK_CONTAINER(scroll), text_view);
        gtk_widget_set_size_request(scroll, 500, 400);
        
        // Buscar o pedido
        Pedido *pedido = buscar_pedido(id);
        if (pedido) {
            GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
            GString *texto = g_string_new("");
            
            // Cabeçalho
            g_string_append_printf(texto, "DETALHES DO PEDIDO #%d\n", pedido->id);
            g_string_append(texto, "=====================================\n\n");
            
            // Informações do cliente
            Cliente *cliente = buscar_cliente(pedido->idCliente);
            if (cliente) {
                g_string_append_printf(texto, "Cliente: %s\n", cliente->nome);
                g_string_append_printf(texto, "Telefone: %s\n", cliente->telefone);
                if (strlen(cliente->nif) > 0) {
                    g_string_append_printf(texto, "NIF: %s\n", cliente->nif);
                }
            }
            g_string_append(texto, "\n");
            
            // Produtos
            g_string_append(texto, "PRODUTOS\n");
            g_string_append(texto, "----------------------------------------\n");
            g_string_append(texto, "Produto          Qtd    Preço    Total\n");
            g_string_append(texto, "----------------------------------------\n");
            
            double subtotal = 0;
            for (int i = 0; i < pedido->numProdutos; i++) {
                Produto *prod = buscar_produto(pedido->idProduto[i]);
                if (prod) {
                    double total_item = prod->preco * pedido->quantidade[i];
                    subtotal += total_item;
                    
                    char preco_str[32], total_str[32];
                    formatar_valor_monetario(preco_str, sizeof(preco_str), prod->preco);
                    formatar_valor_monetario(total_str, sizeof(total_str), total_item);
                    
                    g_string_append_printf(texto, "%-15s %-6d %8s %8s\n",
                                         prod->nome,
                                         pedido->quantidade[i],
                                         preco_str,
                                         total_str);
                }
            }
            
            // Totais
            g_string_append(texto, "----------------------------------------\n");
            char subtotal_str[32], iva_str[32], total_str[32];
            formatar_valor_monetario(subtotal_str, sizeof(subtotal_str), subtotal);
            formatar_valor_monetario(iva_str, sizeof(iva_str), pedido->iva_total);
            formatar_valor_monetario(total_str, sizeof(total_str), pedido->total);
            
            g_string_append_printf(texto, "Subtotal: %8s\n", subtotal_str);
            g_string_append_printf(texto, "IVA (14%%): %8s\n", iva_str);
            g_string_append_printf(texto, "Total: %8s\n\n", total_str);
            
            // Informações de pagamento
            g_string_append_printf(texto, "Método de Pagamento: %s\n", pedido->metodoPagamento);
            
            char valor_pago_str[32], troco_str[32];
            formatar_valor_monetario(valor_pago_str, sizeof(valor_pago_str), pedido->valorPago);
            formatar_valor_monetario(troco_str, sizeof(troco_str), pedido->troco);
            
            g_string_append_printf(texto, "Valor Pago: %s\n", valor_pago_str);
            g_string_append_printf(texto, "Troco: %s\n", troco_str);
            
            gtk_text_buffer_set_text(buffer, texto->str, -1);
            g_string_free(texto, TRUE);
        }
        
        gtk_box_pack_start(GTK_BOX(content_area), scroll, TRUE, TRUE, 0);
        gtk_widget_show_all(dialog);
        
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    } else {
        mostrar_erro(GTK_WINDOW(main_window->window), "Selecione um pedido para visualizar.");
    }
}

void on_imprimir_fatura_clicked(GtkButton *button, gpointer user_data) {
    (void)button;
    (void)user_data;
    
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(main_window->tree_pedidos));
    GtkTreeModel *model;
    GtkTreeIter iter;
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        int id;
        gtk_tree_model_get(model, &iter, COL_PEDIDO_ID, &id, -1);
        
        Pedido *pedido = buscar_pedido(id);
        if (pedido) {
            // Criar diretórios se não existirem
            g_mkdir_with_parents(DIRETORIO_RELATORIOS, 0755);
            g_mkdir_with_parents(DIRETORIO_RELATORIOS_TXT, 0755);
            g_mkdir_with_parents(DIRETORIO_RELATORIOS_PDF, 0755);
            
            // Gerar relatórios
            gerar_fatura_txt(pedido);
            gerar_fatura_pdf(pedido);
            
            mostrar_sucesso(GTK_WINDOW(main_window->window), 
                           "Relatórios gerados com sucesso!\n"
                           "Verifique as pastas:\n"
                           "- data/relatorios/txt\n"
                           "- data/relatorios/pdf");
        }
    } else {
        mostrar_erro(GTK_WINDOW(main_window->window), "Selecione um pedido para imprimir.");
    }
}
