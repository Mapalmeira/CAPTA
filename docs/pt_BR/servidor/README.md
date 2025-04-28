# Recebimento, Armazenamento e Exibição de Dados

Este diretório contém os componentes responsáveis por **receber, armazenar e exibir os dados** enviados pelo ESP32. Toda a lógica de backend é implementada com Google Apps Script, utilizando o Google Drive e o Google Sheets como base de dados e visualização.

---

## Visão Geral

O sistema está dividido em duas partes principais:

### Recebimento

Contém o código responsável por processar requisições do ESP32 e armazenar os dados no Google Drive. O armazenamento é feito em planilhas diárias na pasta `dadosPart/`.

O envio é realizado através de **requisições HTTPS POST**:

- **Envio Contínuo**: pequenas amostras são enviadas periodicamente para visualização em tempo real.

O script inclui verificação por token de segurança, sanitização de entradas e rejeição de requisições inválidas. A instalação envolve a criação de um Web App no Google Apps Script.

### Exibição

Contém os scripts que devem ser inseridos em uma planilha do Google Sheets. A planilha permite a visualização automática dos dados por meio de gráficos ativados por células-gatilho.

Modos de visualização disponíveis:

- Potência instantânea ao longo de um dia (com filtro por horário).
- Consumo total diário dos últimos 30 dias.

---

## Recomendação de Segurança

- Utilize uma conta Google **dedicada exclusivamente para o sistema**.
- Compartilhe as pastas e planilhas com sua conta pessoal apenas como "visualizador" ou "editor".
- Ao criar projetos no Google Apps Script, use uma **guia anônima** para evitar conflitos entre múltiplas contas logadas.

---

## Estrutura Esperada no Google Drive

Para que o sistema funcione corretamente, todos os elementos devem estar reunidos em uma **mesma pasta no Google Drive** (o nome da pasta é irrelevante):

- **Pasta `dadosPart/`**
  Planilhas diárias contendo os dados enviados via POST.

- **Projeto de Apps Script de Recebimento**
  Implantado como Web App, responsável por processar as requisições e armazenar os dados.

- **Planilha de Exibição**
  Contém os scripts de geração de gráficos a partir dos dados armazenados.

---

## Passo a Passo: Implantação do Web App

1. Acesse [Google Apps Script](https://script.google.com/) com uma conta dedicada ao sistema.
2. Crie um novo projeto.
3. Adicione cada arquivo `.gs` da pasta `servidor/recebimento/`:
   - Clique em `+` → `Script`.
   - Cole o conteúdo dos arquivos (`Entrada.gs`, etc.).
4. No menu `Implementar` → `Nova implantação`.
5. Escolha **"Aplicativo da Web"**.
6. Configure:
   - **Executar como**: sua conta dedicada.
   - **Quem tem acesso**: qualquer pessoa, mesmo anônima.
7. Copie o link gerado da implantação e utilize no código do ESP32.
8. Conceda as permissões solicitadas (acesso a Drive e Sheets).

### Observações

- A pasta `dadosPart/` deve ser criada manualmente no mesmo diretório do script.
- O token de segurança deve ser configurado tanto no App Script quanto no ESP32.
- As requisições sem token ou com token incorreto são automaticamente ignoradas.

---

## Passo a Passo: Instalação da Planilha de Exibição

1. Crie uma nova planilha no Google Sheets.
2. No menu `Extensões` → `Apps Script`, abra o editor de scripts.
3. Para cada arquivo em `servidor/exibicao/`:
   - Clique em `+` → `Script`.
   - Cole o conteúdo de `AbaPlanilha.gs`, `Calculadora.gs`, etc.
4. Execute manualmente a função `PotenciaInstantanea` ou `ConsumoDiario` para que o Google solicite as permissões.
5. Configure gatilhos:
   - Vá em `Acionadores`.
   - Adicione um acionador `onEdit` para a função `aoEditar()`.

### Estrutura da Planilha

A aba principal deve conter:

- `C2`: Tensão elétrica (110V, 220V, etc.).
- `C3`: Data alvo (`Date()`).
- `C4`: Hora inicial (0-24).
- `C5`: Hora final (0-24).
- `D7` e `D8`: Gatilhos para geração dos gráficos.

Um exemplo pronto pode ser encontrado na pasta `servidor/exibicao/exemplo_planilha/`.

---

## Integração com o Armazenamento

Os scripts da planilha localizam os dados armazenados no Google Drive automaticamente com base na data.
  
A pasta `dadosPart/` deve conter arquivos no formato padrão: `YYYY-MM-DD`. Se o sistema de envio foi devidamente implementado, isso não deve ser uma preocupação e os arquivos serão salvos no formato correto.

A estrutura correta do Drive é essencial para a geração correta dos gráficos.

---

## Observações Importantes

- Pode ocorrer falha na criação do Apps Script se várias contas Google estiverem logadas no navegador. Para evitar isso, utilize uma **guia anônima** acessando apenas a conta do sistema.
- O projeto utiliza a biblioteca `WiFiClientSecure` com `client.setInsecure()` no ESP32 para facilitar a comunicação com o Apps Script (que usa certificados dinâmicos difíceis de validar).

---

## Próximos Passos

Acesse os READMEs específicos para detalhes aprofundados:

- [`hardware/`](../hardware/README.md): sensores, módulos, lógica de medição.
- [`envio/`](../envio/README.md): lógica do firmware do ESP32 e envio dos dados.
- [`servidor/`](../servidor/README.md): Apps Script, salvamento no Drive e planilha de visualização.

---

## Referências

- [OpenEnergyMonitor – Electricity Monitoring](https://docs.openenergymonitor.org/electricity-monitoring/index.html)
