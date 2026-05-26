# MiniHTTPd

Servidor HTTP/1.1 basico en C para Linux. Esta primera etapa valida la estructura del repositorio, la creacion del socket TCP pasivo y el bucle de eventos con epoll.

## Compilacion

```bash
make
```

## Ejecucion

```bash
./minihttpd 8080
```

## Prueba inicial

```bash
curl -v http://localhost:8080/
```

## Estado actual

Etapa 1:

- socket TCP pasivo
- listen
- epoll_create1
- epoll_wait
- accept
- read
- respuesta HTTP fija

Pendiente:

- parseo completo de GET
- archivos estaticos desde www
- realpath contra directory traversal
- codigos 400, 403, 404, 405, 500
- MIME real por extension
- persistencia con estado por cliente
