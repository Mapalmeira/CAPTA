# Recebimento, Armazenamento e Exibição de Dados

Este diretório contém os componentes responsáveis por **receber, armazenar e exibir os dados** enviados pelo ESP32. Toda a lógica de backend é implementada com Google Apps Script, utilizando o Google Drive e o Google Sheets como base de dados e visualização.

---

## Visão Geral

O sistema está dividido em duas partes principais:

### [`recebimento/`](./recebimento/)
Contém o código responsável por processar requisições do ESP32 e armazenar os dados no Google Drive. O armazenamento é a partir de planilhas diárias salvas numa pasta `dadosPart/`.

O script inclui verificação simples de um token de segurança e sanitização de entradas. A instalação envolve a criação de um Web App com permissões específicas.

### [`exibicao/`](./exibicao/)
Contém os scripts que devem ser inseridos em uma planilha do Google Sheets. A planilha permite a visualização automática dos dados por meio de gráficos ativados por células-gatilho.

Modos de visualização disponíveis:

- Potência instantânea ao longo de um dia (com filtro por horário).
- Consumo total diário dos últimos 30 dias.

---

## Recomendação de Segurança

- Utilize uma conta Google **dedicada exclusivamente para o sistema**.
- Compartilhe as pastas e planilhas com sua conta pessoal apenas como "visualizador" ou "editor".
- Ao criar projetos no Google Apps Script, use uma **guia anônima** com apenas a conta do sistema para evitar problemas relacionados a múltiplas contas logadas.

## Estrutura Esperada no Google Drive

Para que o sistema funcione corretamente, os seguintes elementos devem estar reunidos em uma **mesma pasta no Google Drive** (o nome da pasta é irrelevante):

- **Pasta `dadosPart/`**  
  Planilhas com dados enviados continuamente (POST). Cada planilha corresponde a um dia.

- **Projeto de Apps Script de recebimento**  
  Implantado como Web App, responsável por processar as requisições POST e armazenar os dados.

- **Planilha de exibição**  
  Contém os scripts que processam os dados armazenados e geram visualizações gráficas.

---

## Para instruções detalhadas:

- [Recebimento de dados](./recebimento/README.md)
- [Exibição e gráficos](./exibicao/README.md)
