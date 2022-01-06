# RC-Centralized-Messaging
Projeto de Redes

## TO-DO

- [x] Corrigir verificação de Users existentes (verificar pasta a pasta) - usar findFile().
- [x] Arranjar receção das mensagens por parte do servidor.
- [x] Fazer timer para receção de mensagens.
- [x] Fazer logout, showid, e exit.
- [x] Command unregister, fazer unsubscribe.
- [x] Fazer makefile.
- [ ] Mudar UID para char*, por causa das verificações.
- [ ] Implementar concorrência TCP
- [ ] Fazer um comando exit decente
- [ ] Separar o código em ficheiros


# Dúvidas

- Perguntar acerca do timeout, funcionamento do TimerOFF e TimerON, como proceder no timeout (anular operaçao, tentar de novo, etc.)
- Necessário verificar validade de GID? (select)
- Verificação de numero de argumentos é feita localmente e no server?
- Server main: erros exit(1) ou continuar para o proximo comando?
- Tratar SIGPIPE?? Se sim, exit(1) no write ou o q fazer??
- Erro a meio de um comando, desfazer o q ja foi feito?
