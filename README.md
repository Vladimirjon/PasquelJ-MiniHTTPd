# **MiniHTTPd**

Servidor HTTP/1.1 básico implementado en C para Linux. El proyecto usa sockets TCP, arquitectura dirigida por eventos con `epoll`, entrega de archivos estáticos, validación de rutas, tipos MIME y generación de respuestas HTTP con códigos de estado.

**Autor:** Johann Pasquel
**Asignatura:** Computación Distribuida
**Proyecto:** I Bimestre

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

## Arquitectura

El servidor corre en un único proceso y utiliza `epoll` en modo nivel-disparado (*level-triggered*). El flujo es:

1. Se crea un socket TCP en escucha, configurado como no bloqueante.
2. Se registra en una instancia de `epoll`.
3. `epoll_wait` bloquea con un timeout de 30 segundos.
4. Al llegar eventos, se distingue entre nuevas conexiones (en el socket de escucha) y solicitudes de clientes registrados.
5. Cada cliente aceptado se pone en modo no bloqueante y se registra en `epoll`.
6. Al vencer el timeout, se cierran los clientes inactivos.

## Funcionalidades HTTP

| Característica     | Detalle                                           |
| ------------------- | ------------------------------------------------- |
| Protocolo           | HTTP/1.1 (compatible con HTTP/1.0)                |
| Método soportado   | GET                                               |
| Concurrencia        | `epoll` nivel-disparado, proceso único         |
| Conexiones          | Persistentes (`keep-alive`) con timeout de 30 s |
| Encabezados leídos | `Host`, `Connection`, `User-Agent`          |
| Archivos servidos   | Estáticos desde el directorio `www/`           |

### Tipos MIME soportados

| Extensión | Tipo MIME                  |
| ---------- | -------------------------- |
| `.html`  | `text/html`              |
| `.css`   | `text/css`               |
| `.js`    | `application/javascript` |
| `.png`   | `image/png`              |
| `.jpg`   | `image/jpeg`             |
| `.txt`   | `text/plain`             |

### Códigos de estado HTTP

| Código | Condición                                                         |
| ------- | ------------------------------------------------------------------ |
| 200     | Archivo encontrado y entregado correctamente                       |
| 400     | Solicitud malformada, cabecera `Host` ausente, o demasiado larga |
| 403     | Ruta fuera del directorio raíz, directorio, o archivo especial    |
| 404     | Archivo no encontrado                                              |
| 405     | Método distinto de GET                                            |
| 500     | Error interno al resolver rutas o leer el archivo                  |

## Seguridad

**Directory traversal:** las rutas se canonicalizan con `realpath()` y se verifica que el resultado esté contenido bajo `www/`. Una solicitud como `/../etc/passwd` recibe 403.

**Buffer overflows:** se usan `snprintf`, `sscanf` con limitadores de ancho y `memcpy` en lugar de `strcpy` o `sprintf` en todo el código.

**Métodos inválidos:** cualquier método distinto de GET recibe 405 Method Not Allowed.

**Solicitudes sobredimensionadas:** si la solicitud supera el buffer sin completar los terminadores `\r\n\r\n`, se responde 400 Bad Request.

**Acceso a no-archivos:** `stat()` combinado con `S_ISREG` garantiza que solo se sirvan archivos regulares; los directorios y archivos especiales reciben 403.
