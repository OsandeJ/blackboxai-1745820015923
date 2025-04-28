document.addEventListener('DOMContentLoaded', () => {
    // Navigation handling
    const navLinks = document.querySelectorAll('.nav-links li');
    const pages = document.querySelectorAll('.page');

    navLinks.forEach(link => {
        link.addEventListener('click', () => {
            // Update active navigation link
            navLinks.forEach(l => l.classList.remove('active'));
            link.classList.add('active');

            // Show corresponding page
            const pageId = link.getAttribute('data-page');
            pages.forEach(page => {
                if (page.id === pageId) {
                    page.classList.add('active');
                } else {
                    page.classList.remove('active');
                }
            });
        });
    });

    // Table row selection
    const tables = document.querySelectorAll('.data-table');
    tables.forEach(table => {
        const rows = table.querySelectorAll('tbody tr');
        rows.forEach(row => {
            row.addEventListener('click', () => {
                rows.forEach(r => r.classList.remove('selected'));
                row.classList.add('selected');
            });
        });
    });

    // Button actions (placeholder functions)
    const actionButtons = {
        // Produtos
        'novo-produto': () => alert('Adicionar novo produto'),
        'editar-produto': () => alert('Editar produto selecionado'),
        'excluir-produto': () => alert('Excluir produto selecionado'),

        // Clientes
        'novo-cliente': () => alert('Adicionar novo cliente'),
        'editar-cliente': () => alert('Editar cliente selecionado'),
        'excluir-cliente': () => alert('Excluir cliente selecionado'),

        // Pedidos
        'novo-pedido': () => alert('Criar novo pedido'),
        'visualizar-pedido': () => alert('Visualizar pedido selecionado'),
        'imprimir-fatura': () => alert('Imprimir fatura do pedido selecionado')
    };

    // Add click handlers to all buttons
    document.querySelectorAll('.btn').forEach(button => {
        button.addEventListener('click', (e) => {
            const action = e.currentTarget.getAttribute('data-action');
            if (actionButtons[action]) {
                actionButtons[action]();
            }
        });
    });

    // Add data-action attributes to buttons for easier event handling
    const addActionAttributes = () => {
        // Produtos
        document.querySelector('#produtos .btn-primary').setAttribute('data-action', 'novo-produto');
        document.querySelector('#produtos .btn-secondary').setAttribute('data-action', 'editar-produto');
        document.querySelector('#produtos .btn-danger').setAttribute('data-action', 'excluir-produto');

        // Clientes
        document.querySelector('#clientes .btn-primary').setAttribute('data-action', 'novo-cliente');
        document.querySelector('#clientes .btn-secondary').setAttribute('data-action', 'editar-cliente');
        document.querySelector('#clientes .btn-danger').setAttribute('data-action', 'excluir-cliente');

        // Pedidos
        document.querySelector('#pedidos .btn-primary').setAttribute('data-action', 'novo-pedido');
        document.querySelector('#pedidos .btn-secondary').setAttribute('data-action', 'visualizar-pedido');
        document.querySelector('#pedidos .btn-info').setAttribute('data-action', 'imprimir-fatura');
    };

    addActionAttributes();
});

// Adicionar animações suaves
document.addEventListener('DOMContentLoaded', () => {
    // Adiciona classe para fade-in nas páginas
    document.querySelectorAll('.page').forEach(page => {
        page.style.transition = 'opacity 0.3s ease-in-out';
    });

    // Adiciona efeito hover nos botões
    document.querySelectorAll('.btn').forEach(button => {
        button.style.transition = 'all 0.3s ease';
    });

    // Adiciona efeito hover nas linhas da tabela
    document.querySelectorAll('.data-table tbody tr').forEach(row => {
        row.style.transition = 'background-color 0.2s ease';
    });
});
