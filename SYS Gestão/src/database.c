#include "database.h"
#include "utils.h"
#include <sys/stat.h>
#include <errno.h>
#include <cairo.h>
#include <cairo-pdf.h>



// Variáveis globais
Produto produtos[MAX_PRODUTOS];
Cliente clientes[MAX_CLIENTES];
Pedido pedidos[MAX_PEDIDOS];
int num_produtos = 0;
int num_clientes = 0;
int num_pedidos = 0;

// Mensagens de erro
static const char* mensagens_erro[] = {
    "Operação realizada com sucesso",
    "Arquivo não encontrado",
    "Permissão negada",
    "Memória insuficiente",
    "Dados inválidos",
    "Limite máximo excedido",
    "Registro não encontrado",
    "Falha no backup",
    "Falha na restauração"
};

const char* obter_mensagem_erro(DatabaseError erro) {
    if (erro >= 0 && erro <= DB_ERROR_RESTAURACAO_FALHOU) {
        return mensagens_erro[erro];
    }
    return "Erro desconhecido";
}

// Funções auxiliares
static void criar_diretorio_se_nao_existe(const char *path) {
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        #ifdef _WIN32
            mkdir(path);
        #else
            mkdir(path, 0700);
        #endif
    }
}

void criar_diretorios_necessarios(void) {
    criar_diretorio_se_nao_existe("data");
    criar_diretorio_se_nao_existe("data/backup");
    criar_diretorio_se_nao_existe("data/faturas");
    criar_diretorio_se_nao_existe("data/relatorios");
    criar_diretorio_se_nao_existe("data/relatorios/txt");
    criar_diretorio_se_nao_existe("data/relatorios/pdf");
}

// Implementação das funções de produtos
void calcular_valores_produto(Produto *produto) {
    produto->preco_iva = calcular_preco_com_iva(produto->preco);
}

gboolean salvar_produtos(void) {
    FILE *arquivo = fopen(ARQUIVO_PRODUTOS, "wb");
    if (!arquivo) {
        return FALSE;
    }

    fwrite(&num_produtos, sizeof(int), 1, arquivo);
    fwrite(produtos, sizeof(Produto), num_produtos, arquivo);
    fclose(arquivo);
    return TRUE;
}

gboolean carregar_produtos(void) {
    FILE *arquivo = fopen(ARQUIVO_PRODUTOS, "rb");
    if (!arquivo) {
        num_produtos = 0;
        return FALSE;
    }

    fread(&num_produtos, sizeof(int), 1, arquivo);
    fread(produtos, sizeof(Produto), num_produtos, arquivo);
    fclose(arquivo);
    return TRUE;
}

gboolean adicionar_produto(Produto *produto) {
    if (num_produtos >= MAX_PRODUTOS) {
        return FALSE;
    }

    produto->id = num_produtos + 1;
    calcular_valores_produto(produto);
    produtos[num_produtos++] = *produto;
    return salvar_produtos();
}

gboolean atualizar_produto(Produto *produto) {
    for (int i = 0; i < num_produtos; i++) {
        if (produtos[i].id == produto->id) {
            calcular_valores_produto(produto);
            produtos[i] = *produto;
            return salvar_produtos();
        }
    }
    return FALSE;
}

gboolean remover_produto(int id) {
    for (int i = 0; i < num_produtos; i++) {
        if (produtos[i].id == id) {
            // Mover todos os produtos seguintes uma posição para trás
            for (int j = i; j < num_produtos - 1; j++) {
                produtos[j] = produtos[j + 1];
            }
            num_produtos--;
            return salvar_produtos();
        }
    }
    return FALSE;
}

gboolean atualizar_cliente(Cliente *cliente) {
    for (int i = 0; i < num_clientes; i++) {
        if (clientes[i].id == cliente->id) {
            clientes[i] = *cliente;
            return salvar_clientes();
        }
    }
    return FALSE;
}

gboolean remover_cliente(int id) {
    for (int i = 0; i < num_clientes; i++) {
        if (clientes[i].id == id) {
            // Mover todos os clientes seguintes uma posição para trás
            for (int j = i; j < num_clientes - 1; j++) {
                clientes[j] = clientes[j + 1];
            }
            num_clientes--;
            return salvar_clientes();
        }
    }
    return FALSE;
}

Produto* buscar_produto(int id) {
    for (int i = 0; i < num_produtos; i++) {
        if (produtos[i].id == id) {
            return &produtos[i];
        }
    }
    return NULL;
}

// Implementação das funções de clientes
gboolean salvar_clientes(void) {
    FILE *arquivo = fopen(ARQUIVO_CLIENTES, "wb");
    if (!arquivo) {
        return FALSE;
    }

    fwrite(&num_clientes, sizeof(int), 1, arquivo);
    fwrite(clientes, sizeof(Cliente), num_clientes, arquivo);
    fclose(arquivo);
    return TRUE;
}

gboolean carregar_clientes(void) {
    FILE *arquivo = fopen(ARQUIVO_CLIENTES, "rb");
    if (!arquivo) {
        num_clientes = 0;
        return FALSE;
    }

    fread(&num_clientes, sizeof(int), 1, arquivo);
    fread(clientes, sizeof(Cliente), num_clientes, arquivo);
    fclose(arquivo);
    return TRUE;
}

gboolean adicionar_cliente(Cliente *cliente) {
    if (num_clientes >= MAX_CLIENTES) {
        return FALSE;
    }

    cliente->id = num_clientes + 1;
    clientes[num_clientes++] = *cliente;
    return salvar_clientes();
}

Cliente* buscar_cliente(int id) {
    for (int i = 0; i < num_clientes; i++) {
        if (clientes[i].id == id) {
            return &clientes[i];
        }
    }
    return NULL;
}

Pedido* buscar_pedido(int id) {
    for (int i = 0; i < num_pedidos; i++) {
        if (pedidos[i].id == id) {
            return &pedidos[i];
        }
    }
    return NULL;
}

// Implementação das funções de pedidos
void calcular_valores_pedido(Pedido *pedido) {
    pedido->subtotal = 0;
    pedido->iva_total = 0;
    pedido->total = 0;

    for (int i = 0; i < pedido->numProdutos; i++) {
        Produto *prod = buscar_produto(pedido->idProduto[i]);
        if (prod) {
            double subtotal_item = prod->preco * pedido->quantidade[i];
            double iva_item = calcular_iva(subtotal_item);
            
            pedido->subtotal += subtotal_item;
            pedido->iva_total += iva_item;
        }
    }
    
    pedido->total = pedido->subtotal + pedido->iva_total;
}

gboolean salvar_pedidos(void) {
    FILE *arquivo = fopen(ARQUIVO_PEDIDOS, "wb");
    if (!arquivo) {
        return FALSE;
    }

    fwrite(&num_pedidos, sizeof(int), 1, arquivo);
    fwrite(pedidos, sizeof(Pedido), num_pedidos, arquivo);
    fclose(arquivo);
    return TRUE;
}

gboolean carregar_pedidos(void) {
    FILE *arquivo = fopen(ARQUIVO_PEDIDOS, "rb");
    if (!arquivo) {
        num_pedidos = 0;
        return FALSE;
    }

    fread(&num_pedidos, sizeof(int), 1, arquivo);
    fread(pedidos, sizeof(Pedido), num_pedidos, arquivo);
    fclose(arquivo);
    return TRUE;
}

gboolean adicionar_pedido(Pedido *pedido) {
    if (num_pedidos >= MAX_PEDIDOS) {
        return FALSE;
    }

    pedido->id = num_pedidos + 1;
    obter_data_atual(pedido->data, sizeof(pedido->data));
    obter_hora_atual(pedido->hora, sizeof(pedido->hora));
    
    calcular_valores_pedido(pedido);
    pedidos[num_pedidos++] = *pedido;
    
    return salvar_pedidos();
}

// Inicialização e finalização
gboolean inicializar_database(void) {
    // Criar diretórios necessários
    criar_diretorio_se_nao_existe("data");
    criar_diretorio_se_nao_existe("data/backup");
    criar_diretorio_se_nao_existe("data/faturas");
    
    // Se os arquivos não existem, criar arquivos vazios
    FILE *fp;
    
    // Criar arquivo de produtos se não existe
    fp = fopen(ARQUIVO_PRODUTOS, "ab");
    if (fp) {
        fclose(fp);
    } else {
        return FALSE;
    }
    
    // Criar arquivo de clientes se não existe
    fp = fopen(ARQUIVO_CLIENTES, "ab");
    if (fp) {
        fclose(fp);
    } else {
        return FALSE;
    }
    
    // Criar arquivo de pedidos se não existe
    fp = fopen(ARQUIVO_PEDIDOS, "ab");
    if (fp) {
        fclose(fp);
    } else {
        return FALSE;
    }
    
    // Carregar todos os dados
    gboolean success = TRUE;
    success &= carregar_produtos();
    success &= carregar_clientes();
    success &= carregar_pedidos();
    
    return success;
}

void finalizar_database(void) {
    // Salvar todos os dados antes de finalizar
    salvar_produtos();
    salvar_clientes();
    salvar_pedidos();
}

// Função para gerar fatura em PDF
void gerar_fatura_txt(Pedido *pedido) {
    char filename[256];
    time_t now;
    struct tm *timeinfo;
    char timestamp[20];
    
    time(&now);
    timeinfo = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", timeinfo);
    
    snprintf(filename, sizeof(filename), "%s/fatura_%d_%s.txt", 
             DIRETORIO_RELATORIOS_TXT, pedido->id, timestamp);
    
    FILE *file = fopen(filename, "w");
    if (!file) {
        return;
    }
    
    // Cabeçalho
    fprintf(file, "============================================\n");
    fprintf(file, "              FATURA DE VENDA              \n");
    fprintf(file, "============================================\n\n");
    
    // Informações da empresa
    fprintf(file, "SISTEMA DE GESTÃO\n");
    fprintf(file, "Luanda, Angola\n");
    fprintf(file, "Tel: +244 923 456 789\n");
    fprintf(file, "Email: contato@sistemagestao.co.ao\n\n");
    
    // Data e número da fatura
    fprintf(file, "Fatura Nº: %d\n", pedido->id);
    fprintf(file, "Data: %s\n", timestamp);
    fprintf(file, "--------------------------------------------\n\n");
    
    // Informações do cliente
    Cliente *cliente = buscar_cliente(pedido->idCliente);
    if (cliente) {
        fprintf(file, "CLIENTE\n");
        fprintf(file, "Nome: %s\n", cliente->nome);
        fprintf(file, "Telefone: %s\n", cliente->telefone);
        if (strlen(cliente->nif) > 0) {
            fprintf(file, "NIF: %s\n", cliente->nif);
        }
        fprintf(file, "--------------------------------------------\n\n");
    }
    
    // Cabeçalho dos produtos
    fprintf(file, "PRODUTOS\n");
    fprintf(file, "%-30s %8s %12s %12s\n", "Descrição", "Qtd", "Preço", "Total");
    fprintf(file, "--------------------------------------------\n");
    
    // Lista de produtos
    double subtotal = 0;
    double iva_total = 0;
    
    for (int i = 0; i < pedido->numProdutos; i++) {
        Produto *prod = buscar_produto(pedido->idProduto[i]);
        if (prod) {
            int qtd = pedido->quantidade[i];
            double preco = prod->preco;
            double total = preco * qtd;
            double iva = total * 0.14;
            
            subtotal += total;
            iva_total += iva;
            
            fprintf(file, "%-30s %8d %12.2f %12.2f\n",
                    prod->nome, qtd, preco, total);
        }
    }
    
    // Totais
    fprintf(file, "--------------------------------------------\n");
    fprintf(file, "%-42s %12.2f\n", "Subtotal:", subtotal);
    fprintf(file, "%-42s %12.2f\n", "IVA (14%):", iva_total);
    fprintf(file, "%-42s %12.2f\n", "Total:", subtotal + iva_total);
    fprintf(file, "--------------------------------------------\n\n");
    
    // Informações de pagamento
    fprintf(file, "PAGAMENTO\n");
    fprintf(file, "Método: %s\n", pedido->metodoPagamento);
    fprintf(file, "Valor Pago: %.2f\n", pedido->valorPago);
    fprintf(file, "Troco: %.2f\n\n", pedido->troco);
    
    // Rodapé
    fprintf(file, "============================================\n");
    fprintf(file, "          Obrigado pela preferência!        \n");
    fprintf(file, "============================================\n");
    
    fclose(file);
}

void gerar_fatura_pdf(Pedido *pedido) {
    char filename[256];
    time_t now;
    struct tm *timeinfo;
    char timestamp[20];
    
    time(&now);
    timeinfo = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", timeinfo);
    
    snprintf(filename, sizeof(filename), "%s/fatura_%d_%s.pdf", 
             DIRETORIO_RELATORIOS_PDF, pedido->id, timestamp);
    
    // Criar superfície PDF
    cairo_surface_t *surface = cairo_pdf_surface_create(filename, 595.0, 842.0); // A4
    cairo_t *cr = cairo_create(surface);
    
    // Configurar fonte
    cairo_select_font_face(cr, "Arial", 
                          CAIRO_FONT_SLANT_NORMAL, 
                          CAIRO_FONT_WEIGHT_NORMAL);
    
    // Cabeçalho da fatura
    cairo_set_font_size(cr, 24);
    cairo_move_to(cr, 200, 50);
    cairo_show_text(cr, "FATURA DE VENDA");
    
    // Informações da empresa
    cairo_set_font_size(cr, 12);
    cairo_move_to(cr, 50, 80);
    cairo_show_text(cr, "SISTEMA DE GESTÃO");
    cairo_move_to(cr, 50, 95);
    cairo_show_text(cr, "Luanda, Angola");
    cairo_move_to(cr, 50, 110);
    cairo_show_text(cr, "Tel: +244 923 456 789");
    cairo_move_to(cr, 50, 125);
    cairo_show_text(cr, "Email: contato@sistemagestao.co.ao");
    
    // Data e número da fatura
    char buffer[100];
    cairo_move_to(cr, 400, 80);
    snprintf(buffer, sizeof(buffer), "Fatura Nº: %d", pedido->id);
    cairo_show_text(cr, buffer);
    cairo_move_to(cr, 400, 95);
    snprintf(buffer, sizeof(buffer), "Data: %s", timestamp);
    cairo_show_text(cr, buffer);
    
    // Linha separadora superior
    cairo_set_line_width(cr, 1);
    cairo_move_to(cr, 50, 140);
    cairo_line_to(cr, 545, 140);
    cairo_stroke(cr);
    
    // Informações do cliente
    Cliente *cliente = buscar_cliente(pedido->idCliente);
    if (cliente) {
        cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_move_to(cr, 50, 160);
        cairo_show_text(cr, "DADOS DO CLIENTE");
        
        cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_move_to(cr, 50, 180);
        snprintf(buffer, sizeof(buffer), "Nome: %s", cliente->nome);
        cairo_show_text(cr, buffer);
        
        cairo_move_to(cr, 50, 195);
        snprintf(buffer, sizeof(buffer), "Telefone: %s", cliente->telefone);
        cairo_show_text(cr, buffer);
        
        if (strlen(cliente->nif) > 0) {
            cairo_move_to(cr, 50, 210);
            snprintf(buffer, sizeof(buffer), "NIF: %s", cliente->nif);
            cairo_show_text(cr, buffer);
        }
    }
    
    // Linha separadora antes dos produtos
    cairo_move_to(cr, 50, 230);
    cairo_line_to(cr, 545, 230);
    cairo_stroke(cr);
    
    // Cabeçalho da tabela de produtos
    cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_move_to(cr, 50, 250);
    cairo_show_text(cr, "PRODUTOS");
    
    cairo_move_to(cr, 50, 270);
    cairo_show_text(cr, "Descrição");
    cairo_move_to(cr, 280, 270);
    cairo_show_text(cr, "Qtd");
    cairo_move_to(cr, 350, 270);
    cairo_show_text(cr, "Preço Unit.");
    cairo_move_to(cr, 450, 270);
    cairo_show_text(cr, "Total");
    
    // Linha separadora após cabeçalho da tabela
    cairo_move_to(cr, 50, 280);
    cairo_line_to(cr, 545, 280);
    cairo_stroke(cr);
    
    // Lista de produtos
    cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    double y = 300;
    double subtotal = 0;
    double iva_total = 0;
    
    for (int i = 0; i < pedido->numProdutos; i++) {
        Produto *prod = buscar_produto(pedido->idProduto[i]);
        if (prod) {
            int qtd = pedido->quantidade[i];
            double preco = prod->preco;
            double total = preco * qtd;
            double iva = total * 0.14;
            
            subtotal += total;
            iva_total += iva;
            
            cairo_move_to(cr, 50, y);
            cairo_show_text(cr, prod->nome);
            
            snprintf(buffer, sizeof(buffer), "%d", qtd);
            cairo_move_to(cr, 280, y);
            cairo_show_text(cr, buffer);
            
            snprintf(buffer, sizeof(buffer), "%.2f Kz", preco);
            cairo_move_to(cr, 350, y);
            cairo_show_text(cr, buffer);
            
            snprintf(buffer, sizeof(buffer), "%.2f Kz", total);
            cairo_move_to(cr, 450, y);
            cairo_show_text(cr, buffer);
            
            y += 20;
        }
    }
    
    // Linha separadora antes dos totais
    y += 10;
    cairo_move_to(cr, 50, y);
    cairo_line_to(cr, 545, y);
    cairo_stroke(cr);
    y += 25;
    
    // Totais
    cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    
    cairo_move_to(cr, 350, y);
    cairo_show_text(cr, "Subtotal:");
    snprintf(buffer, sizeof(buffer), "%.2f Kz", subtotal);
    cairo_move_to(cr, 450, y);
    cairo_show_text(cr, buffer);
    
    y += 20;
    cairo_move_to(cr, 350, y);
    cairo_show_text(cr, "IVA (14%):");
    snprintf(buffer, sizeof(buffer), "%.2f Kz", iva_total);
    cairo_move_to(cr, 450, y);
    cairo_show_text(cr, buffer);
    
    y += 20;
    cairo_move_to(cr, 350, y);
    cairo_show_text(cr, "Total:");
    snprintf(buffer, sizeof(buffer), "%.2f Kz", subtotal + iva_total);
    cairo_move_to(cr, 450, y);
    cairo_show_text(cr, buffer);
    
    // Informações de pagamento
    y += 40;
    cairo_move_to(cr, 50, y);
    cairo_show_text(cr, "INFORMAÇÕES DE PAGAMENTO");
    
    y += 20;
    cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    snprintf(buffer, sizeof(buffer), "Método: %s", pedido->metodoPagamento);
    cairo_move_to(cr, 50, y);
    cairo_show_text(cr, buffer);
    
    y += 20;
    snprintf(buffer, sizeof(buffer), "Valor Pago: %.2f Kz", pedido->valorPago);
    cairo_move_to(cr, 50, y);
    cairo_show_text(cr, buffer);
    
    y += 20;
    snprintf(buffer, sizeof(buffer), "Troco: %.2f Kz", pedido->troco);
    cairo_move_to(cr, 50, y);
    cairo_show_text(cr, buffer);
    
    // Rodapé
    cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 10);
    
    y = 780;
    cairo_move_to(cr, 50, y);
    cairo_line_to(cr, 545, y);
    cairo_stroke(cr);
    
    y += 20;
    cairo_move_to(cr, 200, y);
    cairo_show_text(cr, "Obrigado pela preferência! Volte sempre!");
    
    // Finalizar PDF
    cairo_show_page(cr);
    cairo_surface_destroy(surface);
    cairo_destroy(cr);
}

// Função para backup
gboolean fazer_backup(const char *diretorio) {
    char timestamp[20];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", t);
    
    char arquivo_backup[256];
    snprintf(arquivo_backup, sizeof(arquivo_backup), 
             "%s/backup_%s.dat", diretorio, timestamp);
    
    FILE *backup = fopen(arquivo_backup, "wb");
    if (!backup) {
        return FALSE;
    }
    
    // Salvar todos os dados em um único arquivo de backup
    fwrite(&num_produtos, sizeof(int), 1, backup);
    fwrite(produtos, sizeof(Produto), num_produtos, backup);
    
    fwrite(&num_clientes, sizeof(int), 1, backup);
    fwrite(clientes, sizeof(Cliente), num_clientes, backup);
    
    fwrite(&num_pedidos, sizeof(int), 1, backup);
    fwrite(pedidos, sizeof(Pedido), num_pedidos, backup);
    
    fclose(backup);
    return TRUE;
}
