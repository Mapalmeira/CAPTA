# Configuração do Sistema de Recebimento de Dados

Este diretório contém os scripts responsáveis por receber os dados enviados pelo ESP32 via requisições HTTPS. O sistema utiliza o Google Apps Script para processar, armazenar e organizar os dados recebidos no Google Drive.

---

## Visão Geral

O ESP32 envia os dados para uma **implantação Web App** do Google Apps Script hospedada no Google Drive, utilizando **requisições POST**. O envio é feito de forma contínua:

### Envio Contínuo
- **Objetivo**: Enviar pequenas amostras de dados em tempo real.
- **Frequência**: Periódica, conforme definido no código do ESP32.
- **Armazenamento**: Planilhas do Google Sheets diárias, salvas na pasta `dadosPart/`.

Esse modo é ideal para fornecer uma visualização contínua das medições. Como há risco de perda de pacotes em conexões instáveis, o sistema implementa buffers, lógica de reenvio e reinicialização automática para garantir confiabilidade.

O script no servidor trata os dados recebidos, organiza-os por data e tipo, sanitiza os campos e ignora qualquer requisição não autorizada.

---

## Passo a Passo: Implantação do Web App

1. Acesse [Google Apps Script](https://script.google.com/) com uma conta dedicada ao sistema.
2. Crie um novo projeto.
3. Adicione cada arquivo `.gs` da pasta `codigo/` ao projeto:
   - Clique em `+` → `Script` → cole o conteúdo de `Entrada.gs`.
   - Repita o processo para os outros arquivos.
4. No menu `Implementar` → `Nova implantação`.
5. Escolha **"Aplicativo da Web"**.
6. Configure:
   - **Executar como**: sua conta dedicada.
   - **Quem tem acesso**: qualquer pessoa, mesmo anônima (para permitir acesso pelo ESP32).
7. Copie o link gerado da implantação. Ele será usado no código do ESP32 para envio dos dados.
8. Conceda as permissões solicitadas (leitura e escrita no Google Drive).

### Observações

- Garanta que a pasta `dadosPart/` exista e esteja no mesmo diretório do projeto appscript.

- Crie um token poderoso a sua escolha e o defina no campo "TOKEN_SECRETO" em "Entrada.gs". Esse token também precisa ser definido no sistema de envio. O token é obrigatório em todas as requisições. Caso ausente ou incorreto, o servidor ignora os dados.

---

## Observação importante sobre múltiplas contas Google

Caso esteja logado em mais de uma conta Google no navegador, a criação do Apps Script pode falhar ou se vincular à conta errada, mesmo que a conta dedicada esteja ativa.

Recomenda-se abrir uma guia anônima no navegador, logar apenas com a conta dedicada ao sistema e criar o script por lá. Isso garante que a autoria e os acessos corretos sejam aplicados.

---