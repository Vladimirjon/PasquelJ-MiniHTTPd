# **MiniHTTPd**

Servidor HTTP/1.1 básico implementado en C para Linux. El proyecto usa sockets TCP, arquitectura dirigida por eventos con `epoll`, entrega de archivos estáticos, validación de rutas, tipos MIME y generación de respuestas HTTP con códigos de estado.

Autor: Johann Pasquel
Asignatura: Computación Distribuida
Proyecto: I Bimestre

## Estructura del repositorio

```text
minihttpd/
├── Makefile
├── README.md
├── include/
│   ├── files.h
│   ├── http.h
│   ├── mime.h
│   └── server.h
├── src/
│   ├── files.c
│   ├── http.c
│   ├── main.c
│   ├── mime.c
│   └── server.c
└── www/
    ├── image.png
    ├── index.html
    └── style.css
```
