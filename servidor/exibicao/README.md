# Configuração do Sistema de Exibição de Dados

Este diretório contém os arquivos responsáveis pela visualização dos dados recebidos, utilizando uma planilha do Google Sheets com scripts associados via Google Apps Script.

---

## Visão Geral

A planilha de exibição oferece dois modos principais:

- **Potência instantânea por hora** em um determinado dia.
- **Consumo diário em kWh** dos últimos 30 dias.

As visualizações são ativadas por meio da edição de **células-gatilho** na aba principal da planilha. O script interpreta os parâmetros definidos e gera automaticamente os gráficos correspondentes em novas abas.

Os dados utilizados são extraídos de planilhas diárias geradas pela rotina de recebimento do sistema.

---

## Passo a Passo: Instalação na Planilha

1. Crie uma nova planilha no Google Sheets, no mesmo diretório do script de recebimento de dados.
2. No menu `Extensões` → `Apps Script`, abra o editor de scripts.
3. Para cada arquivo em `código/`:
   - Clique em `+` → `Script`.
   - Dê o nome correspondente (`AbaPlanilha`, `Calculadora`, etc.).
   - Cole o conteúdo do respectivo arquivo `.gs`.

**Antes de continuar, execute manualmente a função `PotenciaInstantanea` ou `ConsumoDiario` diretamente pelo editor de script.**  
Isso fará com que o Google solicite as permissões necessárias para o funcionamento do sistema. Aceite todas as permissões solicitadas.

4. Salve o projeto.
5. Caso deseje acionar as visualizações automaticamente ao editar células específicas:
   - No editor de script, vá no menu lateral `Acionadores`.
   - Adicione um gatilho do tipo `onEdit` para executar a função `aoEditar()` presente em `Main.gs`.

## Observação importante sobre múltiplas contas Google

Pode ocorrer falha na criação do Apps Script vinculado à planilha, caso múltiplas contas estejam logadas no navegador. O Google tende a associar o script à primeira conta autenticada.

Recomenda-se utilizar uma **guia anônima** no navegador, logando apenas com a conta dedicada ao sistema, para garantir que o script seja corretamente vinculado à conta pretendida.

---

## Passo a Passo: Estrutura da Planilha

A aba nomeada "Principal" da planilha deve conter:

- Célula `C3`: data alvo (`Date()`).
- Células `C4` e `C5`: hora inicial e hora final (0 a 24).
- Célula `C2`: tensão elétrica (ex: 110 ou 220).
- Células `D7` e `D8`: botões de ativação para os gráficos (editá-los acionará a geração).

Um exemplo pronto de planilha pode ser encontrado no diretório `exemplo_planilha/`.

---

## Integração com o Armazenamento

Para que os gráficos sejam gerados corretamente, os scripts da planilha precisam localizar os arquivos gerados pelo sistema de recebimento. Por padrão, os dados devem estar organizados da seguinte forma no Google Drive:

- Pasta `dadosPart/`: arquivos de planilha, um por dia, no formato YYYY-MM-DD.

Crie essas pastas no mesmo diretório do script de recebimento de dados para garantir que o sistema funcione corretamente.